/*
	Volumetric-Hierarchical Approximate Convex Decomposition.
	Based on https://github.com/kmammou/v-hacd

	IMPORTANT! ------------------------------------------
	* Macros starting and ending with '____' shouldn't be used anywhere outside of this file.
	-----------------------------------------------------

	Author: 	SWANN
	Email:		sebastianswann@outlook.com

	LICENSE ------------------------------------------

	Copyright (c) 2016-2018 SWANN
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	3. The names of the contributors may not be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// SESI
#include <GR/GR_DisplayOption.h>
#include <GT/GT_PrimPolygonMesh.h>
#include <GT/GT_AttributeList.h>
#include <GT/GT_DANumeric.h>

// hou-hdk-common

// this
#include "GUI_VHACDDebug.h"
#include "SOP_VHACDNode.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define GUI_Hook			GET_GUI_Namespace()::GUI_VHACDDebug
#define COMMON_NAMES		GET_SOP_Namespace()::COMMON_NAMES
#define ENUMS				GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
HOOK INITIALIZATION                                                |
----------------------------------------------------------------- */

GUI_Hook::~GUI_VHACDDebug() { }

GUI_Hook::GUI_VHACDDebug()
: GUI_PrimitiveHook(COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::GUI_DEBUG_BIGNAME), GUI_RENDER_MASK_ALL)
{ }

/* -----------------------------------------------------------------
MAIN                                                               |
----------------------------------------------------------------- */

GT_PrimitiveHandle
GUI_Hook::filterPrimitive(const GT_PrimitiveHandle& gt_prm, const GEO_Primitive* geo_prm, const GR_RenderInfo* info, GR_PrimAcceptResult& processed)
{
    GT_PrimitiveHandle ph;

	/*
    if(!info->getDisplayOption()->getUserOptionState(COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::GUI_DEBUG_SMALLNAME)))
    {
	processed = GR_PROCESSED;
	return ph;
    }
	*/

    // As this was registered for a GT prim type, a valid GT prim should be
    // passed to filterPrimitive(), not a GEO primitive.
    UT_ASSERT(geo_prm == NULL);
    UT_ASSERT(gt_prm);

    if(!gt_prm)
    {
		processed = GR_NOT_PROCESSED;
		return ph;
    }

    // Fetch the point normals (N) from the polygon mesh.
    GT_DataArrayHandle nh;
    bool point_normals = false;

    // Check to see if vertex normals are present.
    if(gt_prm->getVertexAttributes())
	nh = gt_prm->getVertexAttributes()->get("N");

    // If no vertex normals, check point normals.
    if(!nh)
    {
	if(gt_prm->getPointAttributes())
	    nh = gt_prm->getPointAttributes()->get("N");

	// If no point normals, generate smooth point normals from P and the
	// connectivity.
	if(!nh)
	{
		const auto mesh = UTverify_cast<const GT_PrimPolygonMesh *>(gt_prm.get());
	    if(mesh)
		nh = mesh->createPointNormals();
	}

	point_normals = true;
    }

    // Normals found: generate Cd from them.
    if(nh)
    {
	// Create a new data array for Cd (vec3 in FP16 format) and map the
	// normalized normal values from [-1,1] to [0,1].
	auto *cd =
	  new GT_Real16Array(nh->entries(), 3, GT_TYPE_COLOR);
	GT_DataArrayHandle cdh = cd;
	
	for(int i=0; i<nh->entries(); i++)
	{
	    UT_Vector3F col;

	    nh->import(i, col.data(), 3);
	    col.normalize();

	    cd->getData(i)[0]= col.x() *0.5 + 0.5;
	    cd->getData(i)[1]= col.y() *0.5 + 0.5;
	    cd->getData(i)[2]= col.z() *0.5 + 0.5;
	}

	// Copy the input mesh, replacing either its vertex or point attribute
	// list.
	    const auto mesh=UTverify_cast<const GT_PrimPolygonMesh *>(gt_prm.get());

	if(point_normals)
	{
	    // Add Cd to the point attribute list, replacing it if Cd already
	    // exists.
		const auto ah = gt_prm->getPointAttributes()->addAttribute("Cd", cdh, true);

	    ph = new GT_PrimPolygonMesh(*mesh,
					ah,
					mesh->getVertexAttributes(),
					mesh->getUniformAttributes(),
					mesh->getDetailAttributes());
	}
	else
	{
	    // Add Cd to the vertex attribute list, replacing it if Cd already
	    // exists.
		const auto ah = gt_prm->getVertexAttributes()->addAttribute("Cd", cdh, true);

	    ph = new GT_PrimPolygonMesh(*mesh,
					mesh->getPointAttributes(),
					ah,
					mesh->getUniformAttributes(),
					mesh->getDetailAttributes());
	}
	processed = GR_PROCESSED;
    }

    // return a new polygon mesh with modified Cd attribute.
    return ph;
}

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef COMMON_NAMES
#undef GUI_Hook