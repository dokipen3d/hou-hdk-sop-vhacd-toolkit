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

#include "../../hou-hdk-common/Operators/SOP/NW_SOP_Operator.h"
#include "../VHADC/public/VHACD.h"

/* -----------------------------------------------------------------
USING                                                              |
----------------------------------------------------------------- */

using namespace VHACD;

/* -----------------------------------------------------------------
V-HACD ENGINE SPECIFIC DEFINES                                     |
----------------------------------------------------------------- */

#define SOP_VHACDENGINE_NAME												VHACDEngine
#define SOP_VHACDENGINE_DATE												"2016"
// internal/external/icon name
#define SOP_VHACDENGINE_OP_SMALLNAME										"nodeway::vhacdengine::1.0"
#define SOP_VHACDENGINE_OP_BIGNAME											"V-HACD Engine"
#define SOP_VHACDENGINE_OP_ICONNAME											"SOP_VHACDEngine.jpg"
// title
#define SOP_VHACDENGINE_OP_TITLE											"V-HACD ENGINE v1.0"
// submenu
#define SOP_VHACDENGINE_SUBMENUPATH											"Convex Decomposition"
// inputs
#define SOP_VHACDENGINE_OP_INPUTNAME0										"Geometry"
// parameters

#define SOP_VHACDENGINE_OP_SMALLPOLYGONS_TOLERANCE							0.000001f

#define SOP_VHACDENGINE_OP_ALLOWPRMOVERRIDE_SMALLNAME						"allowparametersoverride"
#define SOP_VHACDENGINE_OP_ALLOWPRMOVERRIDE_BIGNAME							"Allow Parameters Override"
#define SOP_VHACDENGINE_OP_ALLOWPRMOVERRIDE_SEPARATOR_SMALLNAME				"allowparametersoverrideseparator"
#define SOP_VHACDENGINE_OP_ALLOWOVERRIDE_HELPTEXT							"Turn on/off possibility to override parameters with primitive attributes found on incoming geometry."

#define SOP_VHACDENGINE_OP_POLYGONIZE_SMALLNAME								"polygonize"
#define SOP_VHACDENGINE_OP_POLYGONIZE_BIGNAME								"Polygonize"
#define SOP_VHACDENGINE_OP_POLYGONIZE_SEPARATOR_SMALLNAME					"polygonizeseparator"
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
#define SOP_VHACDENGINE_OP_CONVEXHULLDOWNSAMPLING_BIGNAME					"ConvexHull Downsampling"
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

#define SOP_VHACDENGINE_OP_REPORTTOCONSOLE_SMALLNAME						"detailedreport"
#define SOP_VHACDENGINE_OP_REPORTTOCONSOLE_BIGNAME							"Show Detailed Report"
#define SOP_VHACDENGINE_OP_REPORTTOCONSOLESEPARATOR_SMALLNAME				"detailedreportseparator"
#define SOP_VHACDENGINE_OP_REPORTTOCONSOLE_HELPTEXT							"Prints report in console window, which is more detailed than the information it sends to status bar."
#define SOP_VHACDENGINE_OP_REPORTMODE_SMALLNAME								"reportmode"
#define SOP_VHACDENGINE_OP_REPORTMODE_BIGNAME								"Mode"
#define SOP_VHACDENGINE_OP_REPORTMODE_HELPTEXT								"How detailed report will be printed."