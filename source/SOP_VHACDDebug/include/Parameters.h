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
#ifndef ____prms_vhacd_debug_h____
#define ____prms_vhacd_debug_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// hou-hdk-common
#include <Macros/SwitcherPRM.h>
//#include <Macros/GroupMenuPRM.h>
#include <Macros/TogglePRM.h>
#include <Macros/FloatPRM.h>

// this
#include "SOP_VHACDDebug.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator		GET_SOP_Namespace()::SOP_VHACDDebug
#define CONTAINERS			GET_Base_Namespace()::Containers
#define ENUMS				GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

namespace UI
{	
	__DECLARE__Filter_Section_PRM(1)
	static auto		switchVisibleInputChoiceMenuParm_Name = PRM_Name("switchvisibleinput", "Visible Input");
	static auto		switchVisibleInputChoiceMenuParm_Range = PRM_Range(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 2);
	static PRM_Name switchVisibleInputChoiceMenuParm_Choices[] =
	{
		PRM_Name("0", "Convex Hulls"),
		PRM_Name("1", "Original Geometry"),
		PRM_Name("2", "Both"),
		PRM_Name(nullptr)
	};
	static auto		switchVisibleInputChoiceMenuParm_ChoiceList = PRM_ChoiceList(PRM_CHOICELIST_SINGLE, switchVisibleInputChoiceMenuParm_Choices);
	auto			switchVisibleInputChoiceMenu_Parameter = PRM_Template(PRM_ORD, 1, &switchVisibleInputChoiceMenuParm_Name, nullptr, &switchVisibleInputChoiceMenuParm_ChoiceList, &switchVisibleInputChoiceMenuParm_Range, &SOP_Operator::CallbackSwitchVisibleInput, nullptr, 1, "Specify geometry of which input should be visible.");

	__DECLARE_Main_Section_PRM(10)
	DECLARE_Toggle_with_Separator_OFF_PRM("showhullid", "Show Hull ID ATT", "showhullidseparator", 0, "Visualize 'hull_id' attribute by assigning random color to each convex hull ID.", showHullIDAttribute)
	DECLARE_Toggle_with_Separator_OFF_PRM("showbundleid", "Show Bundle ID ATT", "showbundleidseparator", 0, "Visualize 'bundle_id' attribute by assigning random color to each bundle ID.", showBundleIDAttribute)
	DECLARE_Toggle_with_Separator_OFF_PRM("showwhullvolume", "Show Hull Volume ATT", "showwhullvolumeseparator", 0, "Visualize 'hull_volume' attribute.", showHullVolumeAttribute)
	DECLARE_Toggle_with_Separator_OFF_PRM("explodehullid", "Explode By Hull ID ATT", "explodehullidseparator", 0, "Explode geometry from convex hulls input by using 'hull_id' attribute.", explodeByHullIDAttribute)
	DECLARE_Toggle_with_Separator_OFF_PRM("explodebundleid", "Explode By Bundle ID ATT", "explodebundleidseparator", 0, "Explode geometry by using 'bundle_id' attribute.", explodeByBundleIDAttribute)

	__DECLARE_Additional_Section_PRM(7)
	DECLARE_Toggle_with_Separator_OFF_PRM("cuspvertexnormals", "Cusp Normals", "cuspvertexnormalsseparator", &SOP_Operator::CallbackCuspVertexNormal, "Cusp vertex normals. Affects only convex hulls input geometry.", cuspVertexNormals)
	DECLARE_Custom_Float_0R_to_MaxR_PRM("specifycuspangle", "Angle", 180.0f, 40.0f, 0, "Specify cusp angle.", specifyCuspAngle)
	DECLARE_DescriptionPRM(SOP_Operator)	
}
		
DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS
#undef SOP_Operator

#endif // !____prms_vhacd_debug_h____