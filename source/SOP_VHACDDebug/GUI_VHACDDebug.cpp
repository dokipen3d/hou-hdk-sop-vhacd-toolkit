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
#include <GT/GT_PrimPolygonMesh.h>
#include <GT/GT_AttributeList.h>
#include <OP/OP_Director.h>

// hou-hdk-common
#include <Utility/OP_NodeTester.h>

// this
#include "GUI_VHACDDebug.h"
#include "SOP_VHACDNode.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define GUI_Hook			GET_GUI_Namespace()::GUI_VHACDDebug

#define COMMON_NAMES		GET_SOP_Namespace()::COMMON_NAMES
#define UTILS				GET_Base_Namespace()::Utility
#define CONTAINERS			GET_Base_Namespace()::Containers
#define ENUMS				GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
HOOK INITIALIZATION                                                |
----------------------------------------------------------------- */

GUI_Hook::~GUI_VHACDDebug() { }

GUI_Hook::GUI_VHACDDebug()
: GUI_PrimitiveHook(COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::GUI_DEBUG_BIGNAME), GUI_RENDER_MASK_ALL)
{ }

/* -----------------------------------------------------------------
HELPERS                                                            |
----------------------------------------------------------------- */

template<typename BufferValueType>
ENUMS::MethodProcessResult
GUI_Hook::ProcessAttribute(const GT_DataArrayHandle& datahandle, GT_Real16Array* vertexcolors)
{
	vertexcolors = new GT_Real16Array(datahandle->entries(), 3, GT_TYPE_COLOR);
	GT_DataArrayHandle vertexColors = vertexcolors;

	const exint BUFFER_SIZE = 1;
	
	UT_Vector3 col;
	for (auto i = 0; i < datahandle->entries(); i++)
	{			
		BufferValueType currVal[BUFFER_SIZE];
		datahandle->import(i, currVal, BUFFER_SIZE);

		std::cout << currVal[0] << std::endl;

		const auto now = new UT_Vector3(col);
		UT_Color::getRandomContrastingColor(now, 0, col);
		std::cout << col.x() << "," << col.y() << "," << col.z() << std::endl;

		delete now;
		//col.normalize();

		//cd->getData(i)[0] = col.x() *0.5 + 0.5;
		//cd->getData(i)[1] = col.y() *0.5 + 0.5;
		//cd->getData(i)[2] = col.z() *0.5 + 0.5;
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

/* -----------------------------------------------------------------
MAIN                                                               |
----------------------------------------------------------------- */

GT_PrimitiveHandle
GUI_Hook::filterPrimitive(const GT_PrimitiveHandle& primhandle, const GEO_Primitive* primitive, const GR_RenderInfo* info, GR_PrimAcceptResult& processed)
{
    GT_PrimitiveHandle primitiveHandle;

	// turn it ON only when on proper operator
	auto processResult = UTILS::OP_NodeTester::IsProperOperator(ENUMS::OP_NodeTypeCommonNameOption::SOP, COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_DEBUG_SMALLNAME), info->getNodePath().c_str());
	if (processResult != ENUMS::MethodProcessResult::SUCCESS)
	{
		processed = GR_PROCESSED;
		return primitiveHandle;
	}

    // As this was registered for a GT prim type, a valid GT prim should be
    // passed to filterPrimitive(), not a GEO primitive.
    UT_ASSERT(primitive == NULL);
    UT_ASSERT(primhandle);
	
    if(!primhandle)
    {
		processed = GR_NOT_PROCESSED;
		return primitiveHandle;
    }
	
	// only if we have any of those attributes
	const auto pointAttribs = primhandle->getPointAttributes();
	if (pointAttribs)
	{	
		GT_Real16Array* vertexColors = nullptr;

		const auto bundlIDHandle = pointAttribs->get(this->_vhacdCommonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID));
		if (bundlIDHandle) ProcessAttribute<exint>(bundlIDHandle, vertexColors);

		const auto hullIDHandle = pointAttribs->get(this->_vhacdCommonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_ID));
		if (hullIDHandle && !bundlIDHandle) ProcessAttribute<exint>(hullIDHandle, vertexColors);

		const auto hullVolumeHandle = pointAttribs->get(this->_vhacdCommonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_VOLUME));
		if (hullVolumeHandle && !hullIDHandle && !bundlIDHandle) ProcessAttribute<fpreal64>(hullVolumeHandle, vertexColors);

		if (vertexColors != nullptr)
		{
			// Copy the input mesh, replacing either its vertex or point attribute list.
			//const auto mesh = UTverify_cast<const GT_PrimPolygonMesh *>(primhandle.get());

			// Add Cd to the point attribute list, replacing it if Cd already
			// exists.
			//const auto ah = primhandle->getPointAttributes()->addAttribute("Cd", cdh, true);
			//primitiveHandle = new GT_PrimPolygonMesh(*mesh, ah, mesh->getVertexAttributes(), mesh->getUniformAttributes(), mesh->getDetailAttributes());
			
		}
	}

    // Fetch the point normals (N) from the polygon mesh.
    GT_DataArrayHandle nh;
    bool point_normals = false;
	
    // Check to see if vertex normals are present.
    if(primhandle->getVertexAttributes())
	nh = primhandle->getVertexAttributes()->get("N");

    // If no vertex normals, check point normals.
    if(!nh)
    {
		if(primhandle->getPointAttributes()) nh = primhandle->getPointAttributes()->get("N");

		// If no point normals, generate smooth point normals from P and the
		// connectivity.
		if(!nh)
		{
			const auto mesh = UTverify_cast<const GT_PrimPolygonMesh *>(primhandle.get());
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
	    const auto mesh=UTverify_cast<const GT_PrimPolygonMesh *>(primhandle.get());

		if(point_normals)
		{
			// Add Cd to the point attribute list, replacing it if Cd already
			// exists.
			const auto ah = primhandle->getPointAttributes()->addAttribute("Cd", cdh, true);

			primitiveHandle = new GT_PrimPolygonMesh(*mesh,
						ah,
						mesh->getVertexAttributes(),
						mesh->getUniformAttributes(),
						mesh->getDetailAttributes());
		}
		else
		{
			// Add Cd to the vertex attribute list, replacing it if Cd already
			// exists.
			const auto ah = primhandle->getVertexAttributes()->addAttribute("Cd", cdh, true);

			primitiveHandle = new GT_PrimPolygonMesh(*mesh,
						mesh->getPointAttributes(),
						ah,
						mesh->getUniformAttributes(),
						mesh->getDetailAttributes());
		}

		processed = GR_PROCESSED;
    }

    // return a new polygon mesh with modified Cd attribute.
    return primitiveHandle;
}

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS
#undef UTILS
#undef COMMON_NAMES

#undef GUI_Hook