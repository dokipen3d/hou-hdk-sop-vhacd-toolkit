/*
	Volumetric-Hierarchical Approximate Convex Decomposition.
	Based on https://github.com/kmammou/v-hacd

	IMPORTANT! ------------------------------------------
	* Macros starting and ending with '____' shouldn't be used anywhere outside of this file.
	* External macros used:		
		GET_SOP_Namespace()				- comes from "Macros_Namespace.h"
		DECLARE_SOP_Namespace_Start()	- comes from "Macros_Namespace.h"
		DECLARE_SOP_Namespace_End		- comes from "Macros_Namespace.h"
		__DECLARE__Filter_Section_PRM()
		__DECLARE_Main_Section_PRM()
		__DECLARE_Additional_Section_PRM()
		DECLARE_DescriptionPRM()		- comes from "Macros_DescriptionPRM.h"
		__DECLARE_Debug_Section_PRM()
	-----------------------------------------------------

	Author: 	SNOWFLAKE
	Email:		snowflake@outlook.com

	LICENSE ------------------------------------------

	Copyright (c) 2016-2017 SNOWFLAKE
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
#include <SOP/Macros_FloatPRM.h>
#include <SOP/Macros_IntegerPRM.h>
#include <SOP/Macros_SeparatorPRM.h>
#include <SOP/Macros_SwitcherPRM.h>
#include <SOP/Macros_TogglePRM.h>

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator												GET_SOP_Namespace()::SOP_VHACDEngine_Operator

#define SOP_VHACDENGINE_OP_SMALLPOLYGONS_TOLERANCE							0.000001f

#define SOP_VHACDENGINE_OP_ALLOWPRMOVERRIDE_SMALLNAME						"allowparametersoverride"
#define SOP_VHACDENGINE_OP_ALLOWPRMOVERRIDE_BIGNAME							"Allow Parameters Override"
#define SOP_VHACDENGINE_OP_ALLOWPRMOVERRIDE_SEPARATOR_SMALLNAME				"allowparametersoverrideseparator"
#define SOP_VHACDENGINE_OP_ALLOWOVERRIDE_HELPTEXT							"Turn on/off possibility to override parameters with primitive attributes found on incoming geometry."

#define SOP_VHACDENGINE_OP_POLYGONIZE_SMALLNAME								"converttopolygons"
#define SOP_VHACDENGINE_OP_POLYGONIZE_BIGNAME								"Convert To Polygons"
#define SOP_VHACDENGINE_OP_POLYGONIZE_SEPARATOR_SMALLNAME					"converttopolygonsseparator"
#define SOP_VHACDENGINE_OP_POLYGONIZE_HELPTEXT								"Convert all incoming geometry to polygons."

#define SOP_VHACDENGINE_OP_DECOMPOSITIONMODE_SMALLNAME						"decompositionmode"
#define SOP_VHACDENGINE_OP_DECOMPOSITIONMODE_BIGNAME						"Mode"
#define SOP_VHACDENGINE_OP_DECOMPOSITIONMODE_HELPTEXT						"0: Voxel-based approximate convex decomposition, 1: Tetrahedron-based approximate convex decomposition"
#define SOP_VHACDENGINE_OP_RESOLUTION_SMALLNAME								"resolution"
#define SOP_VHACDENGINE_OP_RESOLUTION_BIGNAME								"Resolution"
#define SOP_VHACDENGINE_OP_RESOLUTION_RANGEMIN								10000
#define SOP_VHACDENGINE_OP_RESOLUTION_RANGEMAX								64000000
#define SOP_VHACDENGINE_OP_RESOLUTION_DEFAULT								100000
#define SOP_VHACDENGINE_OP_RESOLUTION_HELPTEXT								"Maximum number of voxels generated during the voxelization stage."
#define SOP_VHACDENGINE_OP_DEPTH_SMALLNAME									"depth"
#define SOP_VHACDENGINE_OP_DEPTH_BIGNAME									"Depth"
#define SOP_VHACDENGINE_OP_DEPTH_RANGEMIN									1
#define SOP_VHACDENGINE_OP_DEPTH_RANGEMAX									32
#define SOP_VHACDENGINE_OP_DEPTH_DEFAULT									20
#define SOP_VHACDENGINE_OP_DEPTH_HELPTEXT									"Maximum number of clipping stages. During each split stage, all the model parts (with a concavity higher than the user defined threshold) are clipped according the 'best' clipping plane."
#define SOP_VHACDENGINE_OP_CONCAVITY_SMALLNAME								"concavity"
#define SOP_VHACDENGINE_OP_CONCAVITY_BIGNAME								"Concavity"
#define SOP_VHACDENGINE_OP_CONCAVITY_DEFAULT								0.0025f
#define SOP_VHACDENGINE_OP_CONCAVITY_HELPTEXT								"Maximum concavity."
#define SOP_VHACDENGINE_OP_PLANEDOWNSAMPLING_SMALLNAME						"planedownsampling"
#define SOP_VHACDENGINE_OP_PLANEDOWNSAMPLING_BIGNAME						"Plane Downsampling"
#define SOP_VHACDENGINE_OP_PLANEDOWNSAMPLING_RANGEMIN						1
#define SOP_VHACDENGINE_OP_PLANEDOWNSAMPLING_RANGEMAX						16
#define SOP_VHACDENGINE_OP_PLANEDOWNSAMPLING_DEFAULT						4
#define SOP_VHACDENGINE_OP_PLANEDOWNSAMPLING_HELPTEXT						"Controls the granularity of the search for the 'best' clipping plane."
#define SOP_VHACDENGINE_OP_CONVEXHULLDOWNSAMPLING_SMALLNAME					"convexhulldownsampling"
#define SOP_VHACDENGINE_OP_CONVEXHULLDOWNSAMPLING_BIGNAME					"Convex Hull Downsampling"
#define SOP_VHACDENGINE_OP_CONVEXHULLDOWNSAMPLING_RANGEMIN					1
#define SOP_VHACDENGINE_OP_CONVEXHULLDOWNSAMPLING_RANGEMAX					16
#define SOP_VHACDENGINE_OP_CONVEXHULLDOWNSAMPLING_DEFAULT					4
#define SOP_VHACDENGINE_OP_CONVEXHULLDOWNSAMPLING_HELPTEXT					"Controls the precision of the convex-hull generation process during the clipping plane selection stage."
#define SOP_VHACDENGINE_OP_ALPHA_SMALLNAME									"alpha"
#define SOP_VHACDENGINE_OP_ALPHA_BIGNAME									"Alpha"
#define SOP_VHACDENGINE_OP_ALPHA_DEFAULT									0.5f
#define SOP_VHACDENGINE_OP_ALPHA_HELPTEXT									"Controls the bias toward clipping along symmetry planes."
#define SOP_VHACDENGINE_OP_BETA_SMALLNAME									"beta"
#define SOP_VHACDENGINE_OP_BETA_BIGNAME										"Beta"
#define SOP_VHACDENGINE_OP_BETA_DEFAULT										0.5f
#define SOP_VHACDENGINE_OP_BETA_HELPTEXT									"Controls the bias toward clipping along revolution axes."
#define SOP_VHACDENGINE_OP_GAMMA_SMALLNAME									"gamma"
#define SOP_VHACDENGINE_OP_GAMMA_BIGNAME									"Gamma"
#define SOP_VHACDENGINE_OP_GAMMA_DEFAULT									0.00125f
#define SOP_VHACDENGINE_OP_GAMMA_HELPTEXT									"Maximum allowed concavity during the merge stage."
#define SOP_VHACDENGINE_OP_MAXTRIANGLECOUNT_SMALLNAME						"maxtrianglecount"
#define SOP_VHACDENGINE_OP_MAXTRIANGLECOUNT_BIGNAME							"Max Triangle Count"
#define SOP_VHACDENGINE_OP_MAXTRIANGLECOUNT_RANGEMIN						4
#define SOP_VHACDENGINE_OP_MAXTRIANGLECOUNT_RANGEMAX						1024
#define SOP_VHACDENGINE_OP_MAXTRIANGLECOUNT_DEFAULT							64
#define SOP_VHACDENGINE_OP_MAXTRIANGLECOUNT_HELPTEXT						"Controls the maximum number of triangles per convex-hull."
#define SOP_VHACDENGINE_OP_ADAPTIVESAMPLING_SMALLNAME						"adaptivesampling"
#define SOP_VHACDENGINE_OP_ADAPTIVESAMPLING_BIGNAME							"Adaptive Sampling"	
#define SOP_VHACDENGINE_OP_ADAPTIVESAMPLING_RANGEMAX						0.01f
#define SOP_VHACDENGINE_OP_ADAPTIVESAMPLING_DEFAULT							0.0001f
#define SOP_VHACDENGINE_OP_ADAPTIVESAMPLING_HELPTEXT						"Controls the adaptive sampling of the generated convex-hulls."
#define SOP_VHACDENGINE_OP_PCA_SMALLNAME									"normalizemesh"
#define SOP_VHACDENGINE_OP_PCA_BIGNAME										"Normalize Mesh"
#define SOP_VHACDENGINE_OP_PCA_SEPARATOR_SMALLNAME							"normalizemeshseparator"
#define SOP_VHACDENGINE_OP_PCA_HELPTEXT										"Enable/disable normalizing the mesh before applying the convex decomposition."
#define SOP_VHACDENGINE_OP_OPENCL_SMALLNAME									"useopencl"
#define SOP_VHACDENGINE_OP_OPENCL_BIGNAME									"Use OpenCL"
#define SOP_VHACDENGINE_OP_OPENCL_SEPARATOR_SMALLNAME						"useopenclseparator"
#define SOP_VHACDENGINE_OP_OPENCL_HELPTEXT									"Enable/disable OpenCL acceleration."

#define SOP_VHACDENGINE_OP_REPORTTOCONSOLE_SMALLNAME						"showprocessreport"
#define SOP_VHACDENGINE_OP_REPORTTOCONSOLE_BIGNAME							"Show Detailed Report"
#define SOP_VHACDENGINE_OP_REPORTTOCONSOLESEPARATOR_SMALLNAME				"showprocessreportseparator"
#define SOP_VHACDENGINE_OP_REPORTTOCONSOLE_HELPTEXT							"Prints report in console window, which is more detailed than the information it sends to status bar."
#define SOP_VHACDENGINE_OP_REPORTMODE_SMALLNAME								"processreportmode"
#define SOP_VHACDENGINE_OP_REPORTMODE_BIGNAME								"Mode"
#define SOP_VHACDENGINE_OP_REPORTMODE_HELPTEXT								"How detailed report will be printed."

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

	namespace UI
	{
		__DECLARE__Filter_Section_PRM(4)
		DECLARE_Custom_Toggle_OFF_Join_PRM(SOP_VHACDENGINE_OP_ALLOWPRMOVERRIDE_SMALLNAME, SOP_VHACDENGINE_OP_ALLOWPRMOVERRIDE_BIGNAME, 0, SOP_VHACDENGINE_OP_ALLOWOVERRIDE_HELPTEXT, allowParametersOverride)
		DECLARE_Custom_Separator_PRM(SOP_VHACDENGINE_OP_ALLOWPRMOVERRIDE_SEPARATOR_SMALLNAME, allowParametersOverride)

		DECLARE_Custom_Toggle_OFF_Join_PRM(SOP_VHACDENGINE_OP_POLYGONIZE_SMALLNAME, SOP_VHACDENGINE_OP_POLYGONIZE_BIGNAME, 0, SOP_VHACDENGINE_OP_POLYGONIZE_HELPTEXT, convertToPolygons)
		DECLARE_Custom_Separator_PRM(SOP_VHACDENGINE_OP_POLYGONIZE_SEPARATOR_SMALLNAME, convertToPolygons)

		__DECLARE_Main_Section_PRM(14)
		static auto		modeChoiceMenuParm_Name = PRM_Name(SOP_VHACDENGINE_OP_DECOMPOSITIONMODE_SMALLNAME, SOP_VHACDENGINE_OP_DECOMPOSITIONMODE_BIGNAME);
		static auto		modeChoiceMenuParm_Range = PRM_Range(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 1);
		static PRM_Name modeChoiceMenuParm_Choices[] =
		{
			PRM_Name("0", "Voxel"),
			PRM_Name("1", "Tetrahedron"),
			PRM_Name(0)
		};
		static auto		modeChoiceMenuParm_ChoiceList = PRM_ChoiceList(PRM_CHOICELIST_SINGLE, modeChoiceMenuParm_Choices);
		auto			modeChoiceMenu_Parameter = PRM_Template(PRM_ORD, 1, &modeChoiceMenuParm_Name, 0, &modeChoiceMenuParm_ChoiceList, &modeChoiceMenuParm_Range, 0, 0, 1, SOP_VHACDENGINE_OP_DECOMPOSITIONMODE_HELPTEXT);

		DECLARE_Custom_Int_MinR_to_MaxR_PRM(SOP_VHACDENGINE_OP_RESOLUTION_SMALLNAME, SOP_VHACDENGINE_OP_RESOLUTION_BIGNAME, SOP_VHACDENGINE_OP_RESOLUTION_RANGEMIN, SOP_VHACDENGINE_OP_RESOLUTION_RANGEMAX, SOP_VHACDENGINE_OP_RESOLUTION_DEFAULT, SOP_VHACDENGINE_OP_RESOLUTION_HELPTEXT, resolution)
		DECLARE_Custom_Int_MinR_to_MaxR_PRM(SOP_VHACDENGINE_OP_DEPTH_SMALLNAME, SOP_VHACDENGINE_OP_DEPTH_BIGNAME, SOP_VHACDENGINE_OP_DEPTH_RANGEMIN, SOP_VHACDENGINE_OP_DEPTH_RANGEMAX, SOP_VHACDENGINE_OP_DEPTH_DEFAULT, SOP_VHACDENGINE_OP_DEPTH_HELPTEXT, depth)
		DECLARE_Custom_FLOAT_0R_to_1R_PRM(SOP_VHACDENGINE_OP_CONCAVITY_SMALLNAME, SOP_VHACDENGINE_OP_CONCAVITY_BIGNAME, SOP_VHACDENGINE_OP_CONCAVITY_DEFAULT, SOP_VHACDENGINE_OP_CONCAVITY_HELPTEXT, concavity)
		DECLARE_Custom_Int_MinR_to_MaxR_PRM(SOP_VHACDENGINE_OP_PLANEDOWNSAMPLING_SMALLNAME, SOP_VHACDENGINE_OP_PLANEDOWNSAMPLING_BIGNAME, SOP_VHACDENGINE_OP_PLANEDOWNSAMPLING_RANGEMIN, SOP_VHACDENGINE_OP_PLANEDOWNSAMPLING_RANGEMAX, SOP_VHACDENGINE_OP_PLANEDOWNSAMPLING_DEFAULT, SOP_VHACDENGINE_OP_PLANEDOWNSAMPLING_HELPTEXT, planeDownsampling)
		DECLARE_Custom_Int_MinR_to_MaxR_PRM(SOP_VHACDENGINE_OP_CONVEXHULLDOWNSAMPLING_SMALLNAME, SOP_VHACDENGINE_OP_CONVEXHULLDOWNSAMPLING_BIGNAME, SOP_VHACDENGINE_OP_CONVEXHULLDOWNSAMPLING_RANGEMIN, SOP_VHACDENGINE_OP_CONVEXHULLDOWNSAMPLING_RANGEMAX, SOP_VHACDENGINE_OP_CONVEXHULLDOWNSAMPLING_DEFAULT, SOP_VHACDENGINE_OP_CONVEXHULLDOWNSAMPLING_HELPTEXT, convexHullDownsampling)
		DECLARE_Custom_FLOAT_0R_to_1R_PRM(SOP_VHACDENGINE_OP_ALPHA_SMALLNAME, SOP_VHACDENGINE_OP_ALPHA_BIGNAME, SOP_VHACDENGINE_OP_ALPHA_DEFAULT, SOP_VHACDENGINE_OP_ALPHA_HELPTEXT, alpha)
		DECLARE_Custom_FLOAT_0R_to_1R_PRM(SOP_VHACDENGINE_OP_BETA_SMALLNAME, SOP_VHACDENGINE_OP_BETA_BIGNAME, SOP_VHACDENGINE_OP_BETA_DEFAULT, SOP_VHACDENGINE_OP_BETA_HELPTEXT, beta)
		DECLARE_Custom_FLOAT_0R_to_1R_PRM(SOP_VHACDENGINE_OP_GAMMA_SMALLNAME, SOP_VHACDENGINE_OP_GAMMA_BIGNAME, SOP_VHACDENGINE_OP_GAMMA_DEFAULT, SOP_VHACDENGINE_OP_GAMMA_HELPTEXT, gamma)
		DECLARE_Custom_Int_MinR_to_MaxR_PRM(SOP_VHACDENGINE_OP_MAXTRIANGLECOUNT_SMALLNAME, SOP_VHACDENGINE_OP_MAXTRIANGLECOUNT_BIGNAME, SOP_VHACDENGINE_OP_MAXTRIANGLECOUNT_RANGEMIN, SOP_VHACDENGINE_OP_MAXTRIANGLECOUNT_RANGEMAX, SOP_VHACDENGINE_OP_MAXTRIANGLECOUNT_DEFAULT, SOP_VHACDENGINE_OP_MAXTRIANGLECOUNT_HELPTEXT, maxTriangleCount)
		DECLARE_Custom_FLOAT_0R_to_MaxR_PRM(SOP_VHACDENGINE_OP_ADAPTIVESAMPLING_SMALLNAME, SOP_VHACDENGINE_OP_ADAPTIVESAMPLING_BIGNAME, SOP_VHACDENGINE_OP_ADAPTIVESAMPLING_RANGEMAX, SOP_VHACDENGINE_OP_ADAPTIVESAMPLING_DEFAULT, SOP_VHACDENGINE_OP_ADAPTIVESAMPLING_HELPTEXT, adaptiveSampling)
		DECLARE_Custom_Toggle_OFF_Join_PRM(SOP_VHACDENGINE_OP_PCA_SMALLNAME, SOP_VHACDENGINE_OP_PCA_BIGNAME, 0, SOP_VHACDENGINE_OP_PCA_HELPTEXT, normalizeMesh)
		DECLARE_Custom_Separator_PRM(SOP_VHACDENGINE_OP_PCA_SEPARATOR_SMALLNAME, normalizeMeshSeparator)
		DECLARE_Custom_Toggle_ON_Join_PRM(SOP_VHACDENGINE_OP_OPENCL_SMALLNAME, SOP_VHACDENGINE_OP_OPENCL_BIGNAME, 0, SOP_VHACDENGINE_OP_OPENCL_HELPTEXT, useOpenCL)
		DECLARE_Custom_Separator_PRM(SOP_VHACDENGINE_OP_OPENCL_SEPARATOR_SMALLNAME, useOpenCLSeparator)

		__DECLARE_Additional_Section_PRM(4)
		DECLARE_DescriptionPRM(SOP_Operator)

		__DECLARE_Debug_Section_PRM(3)
		DECLARE_Custom_Toggle_with_Separator_PRM(SOP_VHACDENGINE_OP_REPORTTOCONSOLE_SMALLNAME, SOP_VHACDENGINE_OP_REPORTTOCONSOLE_BIGNAME, SOP_VHACDENGINE_OP_REPORTTOCONSOLESEPARATOR_SMALLNAME, 0, SOP_VHACDENGINE_OP_REPORTTOCONSOLE_HELPTEXT, showProcessReport)

		static auto		reportModeChoiceMenuParm_Name = PRM_Name(SOP_VHACDENGINE_OP_REPORTMODE_SMALLNAME, SOP_VHACDENGINE_OP_REPORTMODE_BIGNAME);
		static auto		reportModeChoiceMenuParm_Range = PRM_Range(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 2);
		static PRM_Name reportModeChoiceMenuParm_Choices[] =
		{
			PRM_Name("0", "Progress Only"),
			PRM_Name("1", "Details Only"),
			PRM_Name("2", "Full"),						
			PRM_Name(0)
		};
		static auto		reportModeChoiceMenuParm_ChoiceList = PRM_ChoiceList(PRM_CHOICELIST_SINGLE, reportModeChoiceMenuParm_Choices);
		auto			reportModeChoiceMenu_Parameter = PRM_Template(PRM_ORD, 1, &reportModeChoiceMenuParm_Name, 0, &reportModeChoiceMenuParm_ChoiceList, &reportModeChoiceMenuParm_Range, 0, 0, 1, SOP_VHACDENGINE_OP_REPORTMODE_HELPTEXT);
	}

DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef SOP_Operator