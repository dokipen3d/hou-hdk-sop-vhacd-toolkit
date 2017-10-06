/*
	VHACDSetup parameters.

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
#ifndef ____prms_vhacdsetup_h____
#define ____prms_vhacdsetup_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// hou-hdk-common
#include <Macros/GroupMenuPRM.h>
#include <Macros/FloatPRM.h>
#include <Macros/IntegerPRM.h>
#include <Macros/SeparatorPRM.h>
#include <Macros/SwitcherPRM.h>
#include <Macros/TogglePRM.h>

// this
#include "SOP_VHACDSetup.h"
#include "CommonNameComposer.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator				GET_SOP_Namespace()::SOP_VHACDSetup

#define COMMON_VALUE_HELPTEXT		"Specify value of attribute."
#define COMMON_FALLBACK_HELPTEXT	"Specify fallback value of attribute."
#define COMMON_VALUE_BIGNAME		"Value"
#define COMMON_FALLBACK_BIGNAME		"Fallback"

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

	namespace PRMs_VHACDSetup
	{
		auto names = CommonName();		
		__DECLARE__Filter_Section_PRM(4)

		DECLARE_PrimitiveGroup_Input_0_PRM(&SOP_Operator::CallbackProcessModeChoiceMenu, input0)
		static auto		processModeChoiceMenuParm_Name = PRM_Name("processmode", "Process Mode");
		static auto		processModeChoiceMenuParm_Default = PRM_Default(0);
		static auto		processModeChoiceMenuParm_Range = PRM_Range(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 1);
		static PRM_Name processModeChoiceMenuParm_Choices[] =
		{
			PRM_Name("0", "Selected"),
			PRM_Name("1", "Non-Selected"),
			PRM_Name(nullptr)
		};
		static auto		processModeChoiceMenuParm_ChoiceList = PRM_ChoiceList(PRM_CHOICELIST_SINGLE, processModeChoiceMenuParm_Choices);
		auto			processModeChoiceMenu_Parameter = PRM_Template(PRM_ORD, 1, &processModeChoiceMenuParm_Name, &processModeChoiceMenuParm_Default, &processModeChoiceMenuParm_ChoiceList, &processModeChoiceMenuParm_Range, 0, nullptr, 1, "Specify to which part of geometry apply effect.");
		DECLARE_Toggle_with_Separator_OFF_PRM("solospecifiedgroup", "Solo Group", "solospecifiedgroupseparator", 0, "Remove unselected geometry.", soloSpecifiedGroup)

		__DECLARE_Main_Section_PRM(52)
		auto composer0 = CommonNameComposer(names, CommonNameOption::DECOMPOSITION_MODE);		
		DECLARE_Toggle_with_Separator_OFF_PRM(composer0.AddName(), "Add Decomposition Mode ATT", composer0.AddSeparatorName(), &SOP_Operator::CallbackAddDecompositionModeATT, "0: Voxel-based approximate convex decomposition.\n1: Tetrahedron-based approximate convex decomposition.", addDecompositionModeAttribute)
		static auto		decompositionModeValueChoiceMenuParm_Name = PRM_Name(composer0.ValueName(), COMMON_VALUE_BIGNAME);
		static auto		decompositionModeValueChoiceMenuParm_Default = PRM_Default(0);
		static auto		decompositionModeValueChoiceMenuParm_Range = PRM_Range(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 1);
		static PRM_Name decompositionModeValueChoiceMenuParm_Choices[] =
		{
			PRM_Name("0", names.Get(CommonNameOption::VOXEL)),
			PRM_Name("1", names.Get(CommonNameOption::TETRAHEDRON)),
			PRM_Name(nullptr)
		};
		static auto		decompositionModeValueChoiceMenuParm_ChoiceList = PRM_ChoiceList(PRM_CHOICELIST_SINGLE, decompositionModeValueChoiceMenuParm_Choices);
		auto			decompositionModeValueChoiceMenu_Parameter = PRM_Template(PRM_ORD, 1, &decompositionModeValueChoiceMenuParm_Name, &decompositionModeValueChoiceMenuParm_Default, &decompositionModeValueChoiceMenuParm_ChoiceList, &decompositionModeValueChoiceMenuParm_Range, 0, nullptr, 1, COMMON_VALUE_HELPTEXT);
		DECLARE_Custom_INT_Minus1R_to_1R_PRM(composer0.FallbackName(), COMMON_FALLBACK_BIGNAME, -1, COMMON_FALLBACK_HELPTEXT, decompositionModeFallback)
		
		auto composer1 = CommonNameComposer(names, CommonNameOption::RESOLUTION);
		DECLARE_Toggle_with_Separator_OFF_PRM(composer1.AddName(), "Add Resolution ATT", composer1.AddSeparatorName(), &SOP_Operator::CallbackAddResolutionATT, "Maximum number of voxels generated during the voxelization stage.", addResolutionAttribute)
		DECLARE_Custom_Int_MinR_to_MaxR_PRM(composer1.ValueName(), COMMON_VALUE_BIGNAME, 10000, 64000000, 100000, COMMON_VALUE_HELPTEXT, resolutionValue)
		DECLARE_Custom_Int_MinR_to_MaxR_PRM(composer1.FallbackName(), COMMON_FALLBACK_BIGNAME, -1, resolutionValueInteger_Parameter.getRangePtr()->getParmMax(), -1, COMMON_FALLBACK_HELPTEXT, resolutionFallback)

		auto composer3 = CommonNameComposer(names, CommonNameOption::CONCAVITY);
		DECLARE_Toggle_with_Separator_OFF_PRM(composer3.AddName(), "Add Concavity ATT", composer3.AddSeparatorName(), &SOP_Operator::CallbackAddConcavityATT, "Maximum concavity.", addConcavityAttribute)
		DECLARE_Custom_Float_0R_to_1R_PRM(composer3.ValueName(), COMMON_VALUE_BIGNAME, 0.0025f, 0, COMMON_VALUE_HELPTEXT, concavityValue)
		DECLARE_Custom_Float_MinR_to_MaxR_PRM(composer3.FallbackName(), COMMON_FALLBACK_BIGNAME, -1.0f, concavityValueFloat_Parameter.getRangePtr()->getParmMax(), -1.0f, 0, COMMON_FALLBACK_HELPTEXT, concavityFallback)

		auto composer4 = CommonNameComposer(names, CommonNameOption::PLANE_DOWNSAMPLING);
		DECLARE_Toggle_with_Separator_OFF_PRM(composer4.AddName(), "Add Plane Downsampling ATT", composer4.AddSeparatorName(), &SOP_Operator::CallbackAddPlaneDownsamplingATT, "Controls the granularity of the search for the 'best' clipping plane.", addPlaneDownsamplingAttribute)
		DECLARE_Custom_Int_MinR_to_MaxR_PRM(composer4.ValueName(), COMMON_VALUE_BIGNAME, 1, 16, 4, COMMON_VALUE_HELPTEXT, planeDownsamplingValue)
		DECLARE_Custom_Int_MinR_to_MaxR_PRM(composer4.FallbackName(), COMMON_FALLBACK_BIGNAME, -1, planeDownsamplingValueInteger_Parameter.getRangePtr()->getParmMax(), -1, COMMON_FALLBACK_HELPTEXT, planeDownsamplingFallback)

		auto composer5 = CommonNameComposer(names, CommonNameOption::CONVEXHULL_DOWNSAMPLING);
		DECLARE_Toggle_with_Separator_OFF_PRM(composer5.AddName(), "Add ConvexHull Downsampling ATT", composer5.AddSeparatorName(), &SOP_Operator::CallbackAddConvexHullDownsamplingATT, "Controls the precision of the convex-hull generation process during the clipping plane selection stage.", addConvexHullDownsamplingAttribute)
		DECLARE_Custom_Int_MinR_to_MaxR_PRM(composer5.ValueName(), COMMON_VALUE_BIGNAME, 1, 16, 4, COMMON_VALUE_HELPTEXT, convexHullDownsamplingValue)
		DECLARE_Custom_Int_MinR_to_MaxR_PRM(composer5.FallbackName(), COMMON_FALLBACK_BIGNAME, -1, convexHullDownsamplingValueInteger_Parameter.getRangePtr()->getParmMax(), -1, COMMON_FALLBACK_HELPTEXT, convexHullDownsamplingFallback)

		auto composer6 = CommonNameComposer(names, CommonNameOption::ALPHA);
		DECLARE_Toggle_with_Separator_OFF_PRM(composer6.AddName(), "Add Alpha ATT", composer6.AddSeparatorName(), &SOP_Operator::CallbackAddAlphaATT, "Controls the bias toward clipping along symmetry planes.", addAlphaAttribute)
		DECLARE_Custom_Float_0R_to_1R_PRM(composer6.ValueName(), COMMON_VALUE_BIGNAME, 0.5f, 0, COMMON_VALUE_HELPTEXT, alphaValue)
		DECLARE_Custom_Float_MinR_to_MaxR_PRM(composer6.FallbackName(), COMMON_FALLBACK_BIGNAME, -1.0f, alphaValueFloat_Parameter.getRangePtr()->getParmMax(), -1.0f, 0, COMMON_FALLBACK_HELPTEXT, alphaFallback)

		auto composer7 = CommonNameComposer(names, CommonNameOption::BETA);
		DECLARE_Toggle_with_Separator_OFF_PRM(composer7.AddName(), "Add Beta ATT", composer7.AddSeparatorName(), &SOP_Operator::CallbackAddBetaATT, "Controls the bias toward clipping along revolution axes.", addBetaAttribute)
		DECLARE_Custom_Float_0R_to_1R_PRM(composer7.ValueName(), COMMON_VALUE_BIGNAME, 0.5f, 0, COMMON_VALUE_HELPTEXT, betaValue)
		DECLARE_Custom_Float_MinR_to_MaxR_PRM(composer7.FallbackName(), COMMON_FALLBACK_BIGNAME, -1.0f, betaValueFloat_Parameter.getRangePtr()->getParmMax(), -1.0f, 0, COMMON_FALLBACK_HELPTEXT, betaFallback)

		auto composer2 = CommonNameComposer(names, CommonNameOption::MAX_CONVEX_HULLS_COUNT);
		DECLARE_Toggle_with_Separator_OFF_PRM(composer2.AddName(), "Add Max Convex Hulls ATT", composer2.AddSeparatorName(), &SOP_Operator::CallbackAddMaxConvexHullsCountATT, "Controls the maximum amount of convex hulls that will be generated.", addMaxConvexHullsAttribute)
		DECLARE_Custom_Int_MinR_to_MaxR_PRM(composer2.ValueName(), COMMON_VALUE_BIGNAME, 1, 1024, 64, COMMON_VALUE_HELPTEXT, maxConvexHullsValue)
		DECLARE_Custom_Int_MinR_to_MaxR_PRM(composer2.FallbackName(), COMMON_FALLBACK_BIGNAME, -1, maxConvexHullsValueInteger_Parameter.getRangePtr()->getParmMax(), -1, COMMON_FALLBACK_HELPTEXT, maxConvexHullsFallback)

		auto composer9 = CommonNameComposer(names, CommonNameOption::MAX_TRIANGLE_COUNT);
		DECLARE_Toggle_with_Separator_OFF_PRM(composer9.AddName(), "Add Max Triangle Count ATT", composer9.AddSeparatorName(), &SOP_Operator::CallbackAddMaxTriangleCountATT, "Controls the maximum number of triangles per convex-hull.", addMaxTriangleCountAttribute)
		DECLARE_Custom_Int_MinR_to_MaxR_PRM(composer9.ValueName(), COMMON_VALUE_BIGNAME, 4, 1024, 64, COMMON_VALUE_HELPTEXT, maxTriangleCountValue)
		DECLARE_Custom_Int_MinR_to_MaxR_PRM(composer9.FallbackName(), COMMON_FALLBACK_BIGNAME, -1, maxTriangleCountValueInteger_Parameter.getRangePtr()->getParmMax(), -1, COMMON_FALLBACK_HELPTEXT, maxTriangleCountFallback)

		auto composer10 = CommonNameComposer(names, CommonNameOption::ADAPTIVE_SAMPLING);
		DECLARE_Toggle_with_Separator_OFF_PRM(composer10.AddName(), "Add Adaptive Sampling ATT", composer10.AddSeparatorName(), &SOP_Operator::CallbackAddAdaptiveSamplingATT, "Controls the adaptive sampling of the generated convex-hulls.", addAdaptiveSamplingAttribute)
		DECLARE_Custom_Float_0R_to_MaxR_PRM(composer10.ValueName(), COMMON_VALUE_BIGNAME, 0.01f, 0.0001f, 0, COMMON_VALUE_HELPTEXT, adaptiveSamplingValue)
		DECLARE_Custom_Float_MinR_to_MaxR_PRM(composer10.FallbackName(), COMMON_FALLBACK_BIGNAME, -1.0f, adaptiveSamplingValueFloat_Parameter.getRangePtr()->getParmMax(), -1.0f, 0, COMMON_FALLBACK_HELPTEXT, adaptiveSamplingFallback)

		auto composer11 = CommonNameComposer(names, CommonNameOption::CONVEX_HULL_APPROXIMATION);
		DECLARE_Toggle_with_Separator_OFF_PRM(composer11.AddName(), "Add Approximate Hulls ATT", composer11.AddSeparatorName(), &SOP_Operator::CallbackAddConvexHullApproximationATT, "Enable/disable approximation of convex hulls.", addConvexHullApproximationAttribute)
		DECLARE_Toggle_OFF_PRM(composer11.ValueName(), COMMON_VALUE_BIGNAME, 0, COMMON_VALUE_HELPTEXT, convexHullApproximationValue)
		DECLARE_Custom_INT_Minus1R_to_1R_PRM(composer11.FallbackName(), COMMON_FALLBACK_BIGNAME, -1, COMMON_FALLBACK_HELPTEXT, convexHullApproximationFallback)

		auto composer12 = CommonNameComposer(names, CommonNameOption::PROJECT_HULL_VERTICES);
		DECLARE_Toggle_with_Separator_OFF_PRM(composer12.AddName(), "Add Project Vertices ATT", composer12.AddSeparatorName(), &SOP_Operator::CallbackAddProjectHullVerticeshATT, "This will project the output convex hull vertices onto the original source mesh to increase the floating point accuracy of the results.", addProjectVerticesAttribute)
		DECLARE_Toggle_OFF_PRM(composer12.ValueName(), COMMON_VALUE_BIGNAME, 0, COMMON_VALUE_HELPTEXT, projectVerticesValue)
		DECLARE_Custom_INT_Minus1R_to_1R_PRM(composer12.FallbackName(), COMMON_FALLBACK_BIGNAME, -1, COMMON_FALLBACK_HELPTEXT, projectVertivcesFallback)

		auto composer13 = CommonNameComposer(names, CommonNameOption::NORMALIZE_MESH);
		DECLARE_Toggle_with_Separator_OFF_PRM(composer13.AddName(), "Add Normalize Mesh ATT", composer13.AddSeparatorName(), &SOP_Operator::CallbackAddNormalizeMeshATT, "Enable/disable normalizing the mesh before applying the convex decomposition.", addNormalizeMeshAttribute)
		DECLARE_Toggle_OFF_PRM(composer13.ValueName(), COMMON_VALUE_BIGNAME, 0, COMMON_VALUE_HELPTEXT, normalizeMeshValue)
		DECLARE_Custom_INT_Minus1R_to_1R_PRM(composer13.FallbackName(), COMMON_FALLBACK_BIGNAME, -1, COMMON_FALLBACK_HELPTEXT, normalizeMeshFallback)

		__DECLARE_Additional_Section_PRM(4)
		DECLARE_DescriptionPRM(SOP_Operator)
	}

DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef COMMON_FALLBACK_BIGNAME
#undef COMMON_VALUE_BIGNAME
#undef COMMON_FALLBACK_HELPTEXT
#undef COMMON_VALUE_HELPTEXT

#undef SOP_Operator

#endif // !____prms_vhacdsetup_h____