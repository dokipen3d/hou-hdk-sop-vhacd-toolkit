/*
	Volumetric-Hierarchical Approximate Convex Decomposition.
	Based on https://github.com/kmammou/v-hacd

	IMPORTANT! ------------------------------------------
	* Macros starting and ending with '____' shouldn't be used anywhere outside of this file.
	-----------------------------------------------------

	Author: 	SWANN
	Email:		sebastianswann@outlook.com

	LICENSE ------------------------------------------

	Copyright (c) 2016-2017 SWANN
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	3. The names of the contributors may not be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once
#ifndef ____ui_vhacdengine_h____
#define ____ui_vhacdengine_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// hou-hdk-common
#include <Macros/FloatPRM.h>
#include <Macros/IntegerPRM.h>
#include <Macros/SeparatorPRM.h>
#include <Macros/SwitcherPRM.h>
#include <Macros/TogglePRM.h>

// this
#include "VHACDEngineOperator.h"
#include "CommonName.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator	GET_SOP_Namespace()::VHACDEngineOperator

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

	namespace VHACDEngineParameters
	{
		auto names = CommonName();

		__DECLARE__Filter_Section_PRM(4)
		DECLARE_Toggle_OFF_Join_PRM("allowparametersoverride", "Allow Parameters Override", 0, "Turn on/off possibility to override parameters with primitive attributes found on incoming geometry.", allowParametersOverride)
		DECLARE_Custom_Separator_PRM("allowparametersoverrideseparator", allowParametersOverride)

		DECLARE_Toggle_OFF_Join_PRM("converttopolygons", "Convert To Polygons", 0, "Convert all incoming geometry to polygons.", convertToPolygons)
		DECLARE_Custom_Separator_PRM("converttopolygonsseparator", convertToPolygons)

		__DECLARE_Main_Section_PRM(14)
		static auto		modeChoiceMenuParm_Name = PRM_Name(names.Get(CommonNameOption::DECOMPOSITION_MODE), "Mode");
		static auto		modeChoiceMenuParm_Range = PRM_Range(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 1);
		static PRM_Name modeChoiceMenuParm_Choices[] =
		{
			PRM_Name("0", names.Get(CommonNameOption::VOXEL)),
			PRM_Name("1", names.Get(CommonNameOption::TETRAHEDRON)),
			PRM_Name(0)
		};
		static auto		modeChoiceMenuParm_ChoiceList = PRM_ChoiceList(PRM_CHOICELIST_SINGLE, modeChoiceMenuParm_Choices);
		auto			modeChoiceMenu_Parameter = PRM_Template(PRM_ORD, 1, &modeChoiceMenuParm_Name, 0, &modeChoiceMenuParm_ChoiceList, &modeChoiceMenuParm_Range, 0, 0, 1, "0: Voxel-based approximate convex decomposition, 1: Tetrahedron-based approximate convex decomposition");

		DECLARE_Custom_Int_MinR_to_MaxR_PRM(names.Get(CommonNameOption::RESOLUTION), "Resolution", 10000, 64000000, 100000, "Maximum number of voxels generated during the voxelization stage.", resolution)
		DECLARE_Custom_Int_MinR_to_MaxR_PRM(names.Get(CommonNameOption::DEPTH), "Depth", 1, 32, 20, "Maximum number of clipping stages. During each split stage, all the model parts (with a concavity higher than the user defined threshold) are clipped according the 'best' clipping plane.", depth)
		DECLARE_Custom_Float_0R_to_1R_PRM(names.Get(CommonNameOption::CONCAVITY), "Concavity", 0.0025f, 0, "Maximum concavity.", concavity)
		DECLARE_Custom_Int_MinR_to_MaxR_PRM(names.Get(CommonNameOption::PLANE_DOWNSAMPLING), "Plane Downsampling", 1, 16, 4, "Controls the granularity of the search for the 'best' clipping plane.", planeDownsampling)
		DECLARE_Custom_Int_MinR_to_MaxR_PRM(names.Get(CommonNameOption::CONVEXHULL_DOWNSAMPLING), "Convex Hull Downsampling", 1, 16, 4, "Controls the precision of the convex-hull generation process during the clipping plane selection stage.", convexHullDownsampling)
		DECLARE_Custom_Float_0R_to_1R_PRM(names.Get(CommonNameOption::ALPHA), "Alpha", 0.5f, 0, "Controls the bias toward clipping along symmetry planes.", alpha)
		DECLARE_Custom_Float_0R_to_1R_PRM(names.Get(CommonNameOption::BETA), "Beta", 0.5f, 0, "Controls the bias toward clipping along revolution axes.", beta)
		DECLARE_Custom_Float_0R_to_1R_PRM(names.Get(CommonNameOption::GAMMA), "Gamma", 0.00125f, 0, "Maximum allowed concavity during the merge stage.", gamma)
		DECLARE_Custom_Int_MinR_to_MaxR_PRM(names.Get(CommonNameOption::MAX_TRIANGLE_COUNT), "Max Triangle Count", 4, 1024, 64, "Controls the maximum number of triangles per convex-hull.", maxTriangleCount)
		DECLARE_Custom_Float_0R_to_MaxR_PRM(names.Get(CommonNameOption::ADAPTIVE_SAMPLING), "Adaptive Sampling", 0.01f, 0.0001f, 0, "Controls the adaptive sampling of the generated convex-hulls.", adaptiveSampling)
		DECLARE_Toggle_with_Separator_OFF_PRM(names.Get(CommonNameOption::NORMALIZE_MESH), "Normalize Mesh", "normalizemeshseparator", 0, "Enable/disable normalizing the mesh before applying the convex decomposition.", normalizeMsh)
		DECLARE_Toggle_with_Separator_ON_PRM(names.Get(CommonNameOption::USE_OPENCL), "Use OpenCL", "useopenclseparator", 0, "Enable/disable OpenCL acceleration.", useOpenCL)

		__DECLARE_Additional_Section_PRM(4)
		DECLARE_DescriptionPRM(SOP_Operator)

		__DECLARE_Debug_Section_PRM(3)
		DECLARE_Toggle_with_Separator_OFF_PRM("showprocessreport", "Show Detailed Report", "showprocessreportseparator", 0, "Prints report in console window, which is more detailed than the information it sends to status bar.", showProcessReport)

		static auto		reportModeChoiceMenuParm_Name = PRM_Name("processreportmode", "Mode");
		static auto		reportModeChoiceMenuParm_Range = PRM_Range(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 2);
		static PRM_Name reportModeChoiceMenuParm_Choices[] =
		{
			PRM_Name("0", "Progress Only"),
			PRM_Name("1", "Details Only"),
			PRM_Name("2", "Full"),						
			PRM_Name(0)
		};
		static auto		reportModeChoiceMenuParm_ChoiceList = PRM_ChoiceList(PRM_CHOICELIST_SINGLE, reportModeChoiceMenuParm_Choices);
		auto			reportModeChoiceMenu_Parameter = PRM_Template(PRM_ORD, 1, &reportModeChoiceMenuParm_Name, 0, &reportModeChoiceMenuParm_ChoiceList, &reportModeChoiceMenuParm_Range, 0, 0, 1, "How detailed report will be printed.");		
	}

DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef SOP_Operator

#endif // !____ui_vhacdengine_h____