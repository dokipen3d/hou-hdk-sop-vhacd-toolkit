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

#pragma once
#ifndef ____prms_vhacd_generate_h____
#define ____prms_vhacd_generate_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// hou-hdk-common
#include <Macros/SwitcherPRM.h>
#include <Macros/GroupMenuPRM.h>
#include <Macros/TogglePRM.h>
#include <Macros/IntegerPRM.h>
#include <Macros/FloatPRM.h>

// this
#include "SOP_VHACDGenerate.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator		GET_SOP_Namespace()::SOP_VHACDGenerate
#define COMMON_PRM_NAMES	GET_SOP_Namespace()::COMMON_PRM_NAMES
#define CONTAINERS			GET_Base_Namespace()::Containers
#define ENUMS				GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

namespace UI
{
	__DECLARE__Filter_Section_PRM(4)
	DECLARE_Default_PrimitiveGroup_Input_0_PRM(input0)
	static auto		processModeChoiceMenuParm_Name = PRM_Name("processmode", "Process Mode");
	static auto		processModeChoiceMenuParm_Range = PRM_Range(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 2);
	static PRM_Name processModeChoiceMenuParm_Choices[] =
	{
		PRM_Name("0", "As Whole"),
		PRM_Name("1", "Per Element"),
		PRM_Name("2", "Per Group"),
		PRM_Name(nullptr)
	};
	static auto		processModeChoiceMenuParm_ChoiceList = PRM_ChoiceList(PRM_CHOICELIST_SINGLE, processModeChoiceMenuParm_Choices);
	auto			processModeChoiceMenu_Parameter = PRM_Template(PRM_ORD, 1, &processModeChoiceMenuParm_Name, 0, &processModeChoiceMenuParm_ChoiceList, &processModeChoiceMenuParm_Range, 0, nullptr, 1, "Specify process mode.");

	// TODO: remove it
	DECLARE_Toggle_with_Separator_ON_PRM("forceconverttopolygons", "Force Convert To Polygons", "forceconverttopolygonsseparator", 0, "Force convertion of non-polygon geometry to polygons.", forceConvertToPolygons)
	
	__DECLARE_Main_Section_PRM(18)
	static auto		modeChoiceMenuParm_Name = PRM_Name("decompositionmode", "Mode");
	static auto		modeChoiceMenuParm_Range = PRM_Range(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 1);
	static PRM_Name modeChoiceMenuParm_Choices[] =
	{
		PRM_Name("0", COMMON_PRM_NAMES.Get(ENUMS::VHACDCommonParameterNameOption::VOXEL)),
		PRM_Name("1", COMMON_PRM_NAMES.Get(ENUMS::VHACDCommonParameterNameOption::TETRAHEDRON)),
		PRM_Name(nullptr)
	};
	static auto		modeChoiceMenuParm_ChoiceList = PRM_ChoiceList(PRM_CHOICELIST_SINGLE, modeChoiceMenuParm_Choices);
	auto			modeChoiceMenu_Parameter = PRM_Template(PRM_ORD, 1, &modeChoiceMenuParm_Name, nullptr, &modeChoiceMenuParm_ChoiceList, &modeChoiceMenuParm_Range, 0, nullptr, 1, "0: Voxel-based approximate convex decomposition, 1: Tetrahedron-based approximate convex decomposition");	
	DECLARE_Custom_Int_MinR_to_MaxR_PRM(COMMON_PRM_NAMES.Get(ENUMS::VHACDCommonParameterNameOption::RESOLUTION), "Resolution", 10000, 64000000, 100000, "Maximum number of voxels generated during the voxelization stage.", resolution)
	DECLARE_Custom_Float_0R_to_1R_PRM(COMMON_PRM_NAMES.Get(ENUMS::VHACDCommonParameterNameOption::CONCAVITY), "Concavity", 0.0025f, 0, "Maximum concavity.", concavity)
	DECLARE_Custom_Int_MinR_to_MaxR_PRM(COMMON_PRM_NAMES.Get(ENUMS::VHACDCommonParameterNameOption::PLANE_DOWNSAMPLING), "Plane Downsampling", 1, 16, 4, "Controls the granularity of the search for the 'best' clipping plane.", planeDownsampling)
	DECLARE_Custom_Int_MinR_to_MaxR_PRM(COMMON_PRM_NAMES.Get(ENUMS::VHACDCommonParameterNameOption::CONVEXHULL_DOWNSAMPLING), "Convex Hull Downsampling", 1, 16, 4, "Controls the precision of the convex-hull generation process during the clipping plane selection stage.", convexHullDownsampling)
	DECLARE_Custom_Float_0R_to_1R_PRM(COMMON_PRM_NAMES.Get(ENUMS::VHACDCommonParameterNameOption::ALPHA), "Alpha", 1.0f, 0, "Controls the bias toward clipping along symmetry planes.", alpha)
	DECLARE_Custom_Float_0R_to_1R_PRM(COMMON_PRM_NAMES.Get(ENUMS::VHACDCommonParameterNameOption::BETA), "Beta", 0.0f, 0, "Controls the bias toward clipping along revolution axes.", beta)
	DECLARE_Custom_Int_MinR_to_MaxR_PRM(COMMON_PRM_NAMES.Get(ENUMS::VHACDCommonParameterNameOption::MAX_CONVEXHULLS_COUNT), "Max Hull Count", 1, 1024, 64, "Controls the maximum amount of convex hulls that will be generated.", maxConvexHullsCount)
	DECLARE_Custom_Int_MinR_to_MaxR_PRM(COMMON_PRM_NAMES.Get(ENUMS::VHACDCommonParameterNameOption::MAX_TRIANGLE_COUNT), "Max Triangle Count", 4, 1024, 64, "Controls the maximum number of triangles per convex-hull.", maxTriangleCount)
	DECLARE_Custom_Float_0R_to_MaxR_PRM(COMMON_PRM_NAMES.Get(ENUMS::VHACDCommonParameterNameOption::ADAPTIVE_SAMPLING), "Adaptive Sampling", 0.01f, 0.0001f, 0, "Controls the adaptive sampling of the generated convex-hulls.", adaptiveSampling)
	DECLARE_Toggle_with_Separator_OFF_PRM(COMMON_PRM_NAMES.Get(ENUMS::VHACDCommonParameterNameOption::CONVEXHULL_APPROXIMATION), "Approximate Hulls", "convexhullapproximationseparator", 0, "This will project the output convex hull vertices onto the original source mesh to increase the floating point accuracy of the results.", approximateConvexHulls)
	DECLARE_Toggle_with_Separator_OFF_PRM(COMMON_PRM_NAMES.Get(ENUMS::VHACDCommonParameterNameOption::PROJECT_HULL_VERTICES), "Project Vertices", "projecthullverticesseparator", 0, "WTF?", projectHullVertices)
	DECLARE_Toggle_with_Separator_OFF_PRM(COMMON_PRM_NAMES.Get(ENUMS::VHACDCommonParameterNameOption::NORMALIZE_MESH), "Normalize Mesh", "normalizemeshseparator", 0, "Enable/disable normalizing the mesh before applying the convex decomposition.", normalizeMesh)
	DECLARE_Toggle_with_Separator_ON_PRM(COMMON_PRM_NAMES.Get(ENUMS::VHACDCommonParameterNameOption::USE_OCL), "Use OpenCL", "useopenclseparator", 0, "Enable/disable OpenCL acceleration.", useOpenCL)

	__DECLARE_Additional_Section_PRM(4)
	DECLARE_DescriptionPRM(SOP_Operator)

	__DECLARE_Debug_Section_PRM(3)
	DECLARE_Toggle_with_Separator_OFF_PRM("showprocessreport", "Show Detailed Report", "showprocessreportseparator", &SOP_Operator::CallbackShowProcessReport, "Prints report in console window, which is more detailed than the information it sends to status bar.", showProcessReport)
	static auto		reportModeChoiceMenuParm_Name = PRM_Name("processreportmode", "Mode");
	static auto		reportModeChoiceMenuParm_Range = PRM_Range(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 2);
	static auto		reportModeChoiceMenuParm_Default = PRM_Default(0);
	static PRM_Name reportModeChoiceMenuParm_Choices[] =
	{
		PRM_Name("0", "Progress Only"),
		PRM_Name("1", "Details Only"),
		PRM_Name("2", "Full"),
		PRM_Name(nullptr)
	};
	static auto		reportModeChoiceMenuParm_ChoiceList = PRM_ChoiceList(PRM_CHOICELIST_SINGLE, reportModeChoiceMenuParm_Choices);
	auto			reportModeChoiceMenu_Parameter = PRM_Template(PRM_ORD, 1, &reportModeChoiceMenuParm_Name, &reportModeChoiceMenuParm_Default, &reportModeChoiceMenuParm_ChoiceList, &reportModeChoiceMenuParm_Range, 0, nullptr, 1, "How detailed report will be printed.");
}
		
DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS
#undef COMMON_PRM_NAMES
#undef SOP_Operator

#endif // !____prms_vhacd_generate_h____