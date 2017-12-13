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
#ifndef ____prms_vhacdmerge_h____
#define ____prms_vhacdmerge_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// hou-hdk-common
#include <Macros/SwitcherPRM.h>
#include <Macros/TogglePRM.h>
#include <Macros/ErrorLevelMenuPRM.h>
#include <Macros/SeparatorPRM.h>

// this
#include "SOP_VHACDMerge.h"
#include "CommonNameComposer.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator GET_SOP_Namespace()::SOP_VHACDMerge

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

namespace UI
{	
	// TODO: expand CommonName with better implementation
	auto names = CommonName();

	__DECLARE__Filter_Section_PRM(3)
	static auto		processModeChoiceMenuParm_Name = PRM_Name("processmode", "Process Mode");
	static auto		processModeChoiceMenuParm_Range = PRM_Range(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 1);
	static PRM_Name processModeChoiceMenuParm_Choices[] =
	{
		PRM_Name("0", "Pairs"),
		PRM_Name("1", "Single"),
		PRM_Name(nullptr)
	};
	static auto		processModeChoiceMenuParm_ChoiceList = PRM_ChoiceList(PRM_CHOICELIST_SINGLE, processModeChoiceMenuParm_Choices);
	auto			processModeChoiceMenu_Parameter = PRM_Template(PRM_ORD, 1, &processModeChoiceMenuParm_Name, 0, &processModeChoiceMenuParm_ChoiceList, &processModeChoiceMenuParm_Range, 0, nullptr, 1, "Define if inputs should be processed as pairs or single input.");
	DECLARE_Custom_Separator_PRM("filtererrorsseparator", filterErrors)
	DECLARE_ErroLevelMenu_PRM("attributemismatcherrormode", "Attribute Mismatch", 1, 0, "Specify error level of attribute mismatch found on inputs.", attributeMismatch)

	__DECLARE_Main_Section_PRM(9)
	static auto		orderModeChoiceMenuParm_Name = PRM_Name("ordermode", "Order Mode");
	static auto		orderModeChoiceMenuParm_Range = PRM_Range(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 1);
	static PRM_Name orderModeChoiceMenuParm_Choices[] =
	{
		PRM_Name("0", "Left First"),
		PRM_Name("1", "Right First"),
		PRM_Name(nullptr)
	};
	static auto		orderModeChoiceMenuParm_ChoiceList = PRM_ChoiceList(PRM_CHOICELIST_SINGLE, orderModeChoiceMenuParm_Choices);
	auto			orderModeChoiceMenu_Parameter = PRM_Template(PRM_ORD, 1, &orderModeChoiceMenuParm_Name, 0, &orderModeChoiceMenuParm_ChoiceList, &orderModeChoiceMenuParm_Range, 0, nullptr, 1, "Define order in which inputs will be processed first.");
		
	DECLARE_Toggle_with_Separator_ON_PRM("recalculatemissinghullcountattribute", "Recalculate Missing Hull Count ATT", "recalculatemissinghullcountattributeseparator", 0, "Recalculate missing hull count detail attribute values before attempting to sum it.", recalculateMissingHullCount)
	DECLARE_Toggle_with_Separator_ON_PRM("recalculatemissinghullidattribute", "Recalculate Missing Hull ID ATT", "recalculatemissinghullidattributeseparator", 0, "Recalculate missing hull ID primitive attribute values before attempting to sum it.", recalculateMissingHullID)
	DECLARE_Toggle_with_Separator_ON_PRM("recalculatemissingbundlecountattribute", "Recalculate Missing Bundle Count ATT", "recalculatemissingbundlecountattributeseparator", 0, "Recalculate missing bundle count detail attribute values before attempting to sum it.", recalculateMissingBundleCount)
	DECLARE_Toggle_with_Separator_ON_PRM("recalculatemissingbundleidattribute", "Recalculate Missing Bundle ID ATT", "recalculatemissingbundleidattributeseparator", 0, "Recalculate missing bundle ID primitive attribute values before attempting to sum it.", recalculateMissingBundleID)	

	__DECLARE_Additional_Section_PRM(4)
	DECLARE_DescriptionPRM(SOP_Operator)
}
		
DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef SOP_Operator

#endif // !____prms_vhacdmerge_h____