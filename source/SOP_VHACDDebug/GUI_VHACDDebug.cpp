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
#include <UT/UT_Map.h>
#include <UT/UT_Set.h>
#include <GEO/GEO_Primitive.h>

#if _WIN32		
#include <sys/SYS_Types.h>
#else
#include <SYS/SYS_Types.h>
#endif

// hou-hdk-common
#include <Utility/OP_NodeTester.h>
#include <Containers/OP_NodeTypeCommonName.h>

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
GUI_Hook::ProcessAttribute(const GT_DataArrayHandle& datahandle, GT_DataArrayHandle& newvertexcolors) const
{
	auto vertexColors = new GT_Real16Array(datahandle->entries(), 3, GT_TYPE_COLOR);	

	const exint BUFFER_SIZE = 3;
	for (auto i = 0; i < datahandle->entries(); i++)
	{			
		// get current attribute value
		UT_Vector3F currAttributeValue;
		datahandle->import(i, currAttributeValue.data(), 3);
		
		// assign color			
		vertexColors->getData(i)[0] = currAttributeValue.x();
		vertexColors->getData(i)[1] = currAttributeValue.y();
		vertexColors->getData(i)[2] = currAttributeValue.z();
	}

	//std::cout << mappedColors.size() << std::endl;
	//for (auto col : mappedColors) std::cout << col.second.x() << "," << col.second.y() << "," << col.second.z() << std::endl;

	newvertexcolors = vertexColors;

	return ENUMS::MethodProcessResult::SUCCESS;
}

/* -----------------------------------------------------------------
MAIN                                                               |
----------------------------------------------------------------- */

GT_PrimitiveHandle
GUI_Hook::filterPrimitive(const GT_PrimitiveHandle& primhandle, const GEO_Primitive* primitive, const GR_RenderInfo* info, GR_PrimAcceptResult& processed)
{
    GT_PrimitiveHandle newPrimitiveHandle;
	
	// turn it ON only when proper operator is selected
	auto processResult = UTILS::OP_NodeTester::IsProperOperator(ENUMS::OP_NodeTypeCommonNameOption::SOP, COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_DEBUG_SMALLNAME), info->getNodePath().c_str());
	if (processResult != ENUMS::MethodProcessResult::SUCCESS)
	{
		processed = GR_PROCESSED;
		return newPrimitiveHandle;
	}

    // as this was registered for a GT prim type, a valid GT prim should be passed to filterPrimitive(), not a GEO primitive.
    UT_ASSERT(primitive == NULL);
    UT_ASSERT(primhandle);	
	
    if(!primhandle)
    {
		processed = GR_NOT_PROCESSED;
		return newPrimitiveHandle;
    }
	
	// only if we have any of those attributes
	const auto pointAttribs = primhandle->getPointAttributes();
	if (pointAttribs)
	{
		GT_DataArrayHandle newVertexColors = nullptr;
		
		const auto bundlIDHandle = pointAttribs->get(this->_vhacdCommonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID));
		if (bundlIDHandle) processResult = ProcessAttribute<exint>(bundlIDHandle, newVertexColors);

		const auto hullIDHandle = pointAttribs->get(this->_vhacdCommonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_ID));
		if (hullIDHandle && !bundlIDHandle) processResult = ProcessAttribute<exint>(hullIDHandle, newVertexColors);

		const auto hullVolumeHandle = pointAttribs->get(this->_vhacdCommonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_VOLUME));
		if (hullVolumeHandle && !hullIDHandle && !bundlIDHandle) processResult = ProcessAttribute<fpreal64>(hullVolumeHandle, newVertexColors);
		
		if (processResult == ENUMS::MethodProcessResult::SUCCESS && newVertexColors != nullptr)
		{
			// copy the input mesh, replacing either its vertex or point attribute list
			const auto mesh = UTverify_cast<const GT_PrimPolygonMesh*>(primhandle.get());
			if (mesh)
			{
				// add Cd to the point attribute list, replacing it if Cd already exists
				const auto ah = primhandle->getPointAttributes()->addAttribute("Cd", newVertexColors, true);
				newPrimitiveHandle = new GT_PrimPolygonMesh(*mesh, ah, mesh->getVertexAttributes(), mesh->getUniformAttributes(), mesh->getDetailAttributes());				
			}
			else
			{
				processed = GR_NOT_PROCESSED;
				return newPrimitiveHandle;
			}
		}
		else
		{
			processed = GR_NOT_PROCESSED;
			return newPrimitiveHandle;
		}
	}
	else
	{
		processed = GR_NOT_PROCESSED;
		return newPrimitiveHandle;
	}

	// return a new polygon mesh with modified Cd attribute.
	processed = GR_PROCESSED;
    
    return newPrimitiveHandle;
}

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS
#undef UTILS
#undef COMMON_NAMES

#undef GUI_Hook