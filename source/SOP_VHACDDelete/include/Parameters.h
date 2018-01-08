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
#ifndef ____prms_vhacd_delete_h____
#define ____prms_vhacd_delete_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// hou-hdk-common
#include <Macros/SwitcherPRM.h>
#include <Macros/GroupMenuPRM.h>
#include <Macros/FloatPRM.h>
#include <Macros/StringPRM.h>

// this
#include "SOP_VHACDDelete.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator		GET_SOP_Namespace()::SOP_VHACDDelete
#define COMMON_PRM_NAMES	GET_SOP_Namespace()::COMMON_PRM_NAMES
#define CONTAINERS			GET_Base_Namespace()::Containers
#define ENUMS				GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

namespace UI
{
	__DECLARE__Filter_Section_PRM(3)	
	static auto		filterModeChoiceMenuParm_Name = PRM_Name("filtermode", "Mode");
	static auto		filterModeChoiceMenuParm_Range = PRM_Range(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 1);
	static PRM_Name filterModeChoiceMenuParm_Choices[] =
	{
		PRM_Name("0", "By Group"),
		PRM_Name("1", "By Bundle ID"),
		PRM_Name(nullptr)
	};
	static auto		filterModeChoiceMenuParm_ChoiceList = PRM_ChoiceList(PRM_CHOICELIST_SINGLE, filterModeChoiceMenuParm_Choices);
	auto			filterModeChoiceMenu_Parameter = PRM_Template(PRM_ORD, 1, &filterModeChoiceMenuParm_Name, 0, &filterModeChoiceMenuParm_ChoiceList, &filterModeChoiceMenuParm_Range, 0, nullptr, 1, "Specify filter mode.");	
	DECLARE_Default_PrimitiveGroup_Input_0_PRM(input0)
	DECLARE_Custom_Empty_String_PRM("bundleidpattern", "Pattern", "Specify bundle_id attribute pattern.", bundleIDPattern)

	__DECLARE_Main_Section_PRM(0)

	__DECLARE_Additional_Section_PRM(4)
	DECLARE_DescriptionPRM(SOP_Operator)
}
		
DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS
#undef COMMON_PRM_NAMES
#undef SOP_Operator

#endif // !____prms_vhacd_delete_h____