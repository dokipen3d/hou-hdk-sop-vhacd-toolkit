/*
	Volumetric-Hierarchical Approximate Convex Decomposition.
	Based on https://github.com/kmammou/v-hacd

	IMPORTANT! ------------------------------------------
	DO NOT MODIFY THIS FILE.
	Doing so may break every extension that uses it as a base or utility.
	Modify it ONLY when you know what you are doing. That means NEVER!
	-----------------------------------------------------

	TODO! -----------------------------------------------
	-----------------------------------------------------

	Author: 	Nodeway (2016)

	Email:		nodeway@hotmail.com
	Vimeo:		https://vimeo.com/nodeway
	Twitter:	https://twitter.com/nodeway
	Github:		https://github.com/nodeway

	LICENSE ------------------------------------------

	Copyright (c) 2016 Nodeway
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	3. The names of the contributors may not be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

#include "SOP_VHACDEngine_Operator.h"

/* -----------------------------------------------------------------
USING                                                              |
----------------------------------------------------------------- */

GET_SOP_NAMESPACE()

/* -----------------------------------------------------------------
DEFINES                                                              |
----------------------------------------------------------------- */

#define ____unique_sop_title____		SOP_VHACDENGINE_NAME

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

SOP_PARMS_NAMESPACE()
{
	OPERATOR_TITLE_SECTION(SOP_VHACDENGINE_OP_TITLE)
	namespace TITLE
	{
		DEFAULT_INPUT_0_NOT_CONNECTED_ERROR_MESSAGE()
	}
	
	namespace UI
	{
		__FILTER_SECTION_PRM(4)
		CUSTOM_TOOGLE_OFF_JOIN_PRM				(SOP_VHACDENGINE_OP_ALLOWPRMOVERRIDE_SMALLNAME, SOP_VHACDENGINE_OP_ALLOWPRMOVERRIDE_BIGNAME, 0, SOP_VHACDENGINE_OP_ALLOWOVERRIDE_HELPTEXT, allowParametersOverride)
		CUSTOM_SEPARATOR_PARM					(SOP_VHACDENGINE_OP_ALLOWPRMOVERRIDE_SEPARATOR_SMALLNAME, allowParametersOverride)

		CUSTOM_TOOGLE_OFF_JOIN_PRM				(SOP_VHACDENGINE_OP_POLYGONIZE_SMALLNAME, SOP_VHACDENGINE_OP_POLYGONIZE_BIGNAME, 0, SOP_VHACDENGINE_OP_POLYGONIZE_HELPTEXT, polygonize)
		CUSTOM_SEPARATOR_PARM					(SOP_VHACDENGINE_OP_POLYGONIZE_SEPARATOR_SMALLNAME, polygonize)

		__MAIN_SECTION_PRM(14)
		static auto modeChoiceMenuParm_Name = PRM_Name(SOP_VHACDENGINE_OP_DECOMPOSITIONMODE_SMALLNAME, SOP_VHACDENGINE_OP_DECOMPOSITIONMODE_BIGNAME);
		static auto modeChoiceMenuParm_Range = PRM_Range(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 1);
		static PRM_Name modeChoiceMenuParm_Choices[] =
		{
			PRM_Name("0", "Voxel"),
			PRM_Name("1", "Tetrahedron"),
			PRM_Name(0)
		};
		static auto modeChoiceMenuParm_ChoiceList = PRM_ChoiceList(PRM_CHOICELIST_SINGLE, modeChoiceMenuParm_Choices);
		auto modeChoiceMenu_Parameter = PRM_Template(PRM_ORD, 1, &modeChoiceMenuParm_Name, 0, &modeChoiceMenuParm_ChoiceList, &modeChoiceMenuParm_Range, 0, 0, 1, SOP_VHACDENGINE_OP_DECOMPOSITIONMODE_HELPTEXT);
					
		CUSTOM_INT_MINR_TO_MAXR_PRM				(SOP_VHACDENGINE_OP_RESOLUTION_SMALLNAME, SOP_VHACDENGINE_OP_RESOLUTION_BIGNAME, SOP_VHACDENGINE_OP_RESOLUTION_RANGEMIN, SOP_VHACDENGINE_OP_RESOLUTION_RANGEMAX, SOP_VHACDENGINE_OP_RESOLUTION_DEFAULT, SOP_VHACDENGINE_OP_RESOLUTION_HELPTEXT, resolution)
		CUSTOM_INT_MINR_TO_MAXR_PRM				(SOP_VHACDENGINE_OP_DEPTH_SMALLNAME, SOP_VHACDENGINE_OP_DEPTH_BIGNAME, SOP_VHACDENGINE_OP_DEPTH_RANGEMIN, SOP_VHACDENGINE_OP_DEPTH_RANGEMAX, SOP_VHACDENGINE_OP_DEPTH_DEFAULT, SOP_VHACDENGINE_OP_DEPTH_HELPTEXT, depth)
		CUSTOM_FLOAT_0R_TO_1R_PRM				(SOP_VHACDENGINE_OP_CONCAVITY_SMALLNAME, SOP_VHACDENGINE_OP_CONCAVITY_BIGNAME, SOP_VHACDENGINE_OP_CONCAVITY_DEFAULT, SOP_VHACDENGINE_OP_CONCAVITY_HELPTEXT, concavity)
		CUSTOM_INT_MINR_TO_MAXR_PRM				(SOP_VHACDENGINE_OP_PLANEDOWNSAMPLING_SMALLNAME, SOP_VHACDENGINE_OP_PLANEDOWNSAMPLING_BIGNAME, SOP_VHACDENGINE_OP_PLANEDOWNSAMPLING_RANGEMIN, SOP_VHACDENGINE_OP_PLANEDOWNSAMPLING_RANGEMAX, SOP_VHACDENGINE_OP_PLANEDOWNSAMPLING_DEFAULT, SOP_VHACDENGINE_OP_PLANEDOWNSAMPLING_HELPTEXT, planeDownsampling)
		CUSTOM_INT_MINR_TO_MAXR_PRM				(SOP_VHACDENGINE_OP_CONVEXHULLDOWNSAMPLING_SMALLNAME, SOP_VHACDENGINE_OP_CONVEXHULLDOWNSAMPLING_BIGNAME, SOP_VHACDENGINE_OP_CONVEXHULLDOWNSAMPLING_RANGEMIN, SOP_VHACDENGINE_OP_CONVEXHULLDOWNSAMPLING_RANGEMAX, SOP_VHACDENGINE_OP_CONVEXHULLDOWNSAMPLING_DEFAULT, SOP_VHACDENGINE_OP_CONVEXHULLDOWNSAMPLING_HELPTEXT, convexHullDownsampling)
		CUSTOM_FLOAT_0R_TO_1R_PRM				(SOP_VHACDENGINE_OP_ALPHA_SMALLNAME, SOP_VHACDENGINE_OP_ALPHA_BIGNAME, SOP_VHACDENGINE_OP_ALPHA_DEFAULT, SOP_VHACDENGINE_OP_ALPHA_HELPTEXT, alpha)
		CUSTOM_FLOAT_0R_TO_1R_PRM				(SOP_VHACDENGINE_OP_BETA_SMALLNAME, SOP_VHACDENGINE_OP_BETA_BIGNAME, SOP_VHACDENGINE_OP_BETA_DEFAULT, SOP_VHACDENGINE_OP_BETA_HELPTEXT, beta)
		CUSTOM_FLOAT_0R_TO_1R_PRM				(SOP_VHACDENGINE_OP_GAMMA_SMALLNAME, SOP_VHACDENGINE_OP_GAMMA_BIGNAME, SOP_VHACDENGINE_OP_GAMMA_DEFAULT, SOP_VHACDENGINE_OP_GAMMA_HELPTEXT, gamma)
		CUSTOM_INT_MINR_TO_MAXR_PRM				(SOP_VHACDENGINE_OP_MAXTRIANGLECOUNT_SMALLNAME, SOP_VHACDENGINE_OP_MAXTRIANGLECOUNT_BIGNAME, SOP_VHACDENGINE_OP_MAXTRIANGLECOUNT_RANGEMIN, SOP_VHACDENGINE_OP_MAXTRIANGLECOUNT_RANGEMAX, SOP_VHACDENGINE_OP_MAXTRIANGLECOUNT_DEFAULT, SOP_VHACDENGINE_OP_MAXTRIANGLECOUNT_HELPTEXT, maxTriangleCount)
		CUSTOM_FLOAT_0R_TO_MAXR_PRM				(SOP_VHACDENGINE_OP_ADAPTIVESAMPLING_SMALLNAME, SOP_VHACDENGINE_OP_ADAPTIVESAMPLING_BIGNAME, SOP_VHACDENGINE_OP_ADAPTIVESAMPLING_RANGEMAX, SOP_VHACDENGINE_OP_ADAPTIVESAMPLING_DEFAULT, SOP_VHACDENGINE_OP_ADAPTIVESAMPLING_HELPTEXT, adaptiveSampling)
		CUSTOM_TOOGLE_OFF_JOIN_PRM				(SOP_VHACDENGINE_OP_PCA_SMALLNAME, SOP_VHACDENGINE_OP_PCA_BIGNAME, 0, SOP_VHACDENGINE_OP_PCA_HELPTEXT, normalizeMesh)
		CUSTOM_SEPARATOR_PARM					(SOP_VHACDENGINE_OP_PCA_SEPARATOR_SMALLNAME, normalizeMeshSeparator)
		CUSTOM_TOOGLE_ON_JOIN_PRM				(SOP_VHACDENGINE_OP_OPENCL_SMALLNAME, SOP_VHACDENGINE_OP_OPENCL_BIGNAME, 0, SOP_VHACDENGINE_OP_OPENCL_HELPTEXT, useOpenCL)
		CUSTOM_SEPARATOR_PARM					(SOP_VHACDENGINE_OP_OPENCL_SEPARATOR_SMALLNAME, useOpenCLSeparator)

		__ADDITIONAL_SECTION_PRM(4)
		DESCRIPTION_PRM()

		__DEBUG_SECTION_PRM(3)
		CUSTOM_TOGGLE_WITH_SEPARATOR_OFF_PRM	(SOP_VHACDENGINE_OP_REPORTTOCONSOLE_SMALLNAME, SOP_VHACDENGINE_OP_REPORTTOCONSOLE_BIGNAME, SOP_VHACDENGINE_OP_REPORTTOCONSOLESEPARATOR_SMALLNAME, 0, SOP_VHACDENGINE_OP_REPORTTOCONSOLE_HELPTEXT, consoleReport)

		static auto reportModeChoiceMenuParm_Name = PRM_Name(SOP_VHACDENGINE_OP_REPORTMODE_SMALLNAME, SOP_VHACDENGINE_OP_REPORTMODE_BIGNAME);
		static auto reportModeChoiceMenuParm_Range = PRM_Range(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 2);
		static PRM_Name reportModeChoiceMenuParm_Choices[] =
		{
			PRM_Name("0", "Full"),
			PRM_Name("1", "Details Only"),
			PRM_Name("2", "Progress Only"),
			PRM_Name(0)
		};
		static auto reportModeChoiceMenuParm_ChoiceList = PRM_ChoiceList(PRM_CHOICELIST_SINGLE, reportModeChoiceMenuParm_Choices);
		auto reportModeChoiceMenu_Parameter = PRM_Template(PRM_ORD, 1, &reportModeChoiceMenuParm_Name, 0, &reportModeChoiceMenuParm_ChoiceList, &reportModeChoiceMenuParm_Range, 0, 0, 1, SOP_VHACDENGINE_OP_REPORTMODE_HELPTEXT);
	}
	
	__CUSTOM_AUTHOR_SECTION_PRM(2)
	namespace AUTHOR
	{
		CUSTOM_CENTERED_LABEL_PRM				("basedon", "Based On: https://github.com/kmammou/v-hacd", basedOn)
		DEFAULT_CENTERED_EMPTY_LABEL_PRM		("blankspace", blankSpace)
	}
}

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ____unique_sop_title____