/*
	Volumetric-Hierarchical Approximate Convex Decomposition.
	Based on https://github.com/kmammou/v-hacd

	IMPORTANT! ------------------------------------------
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
#ifndef ____prms_vhacdscout_h____
#define ____prms_vhacdscout_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// hou-hdk-common
#include <Macros/SwitcherPRM.h>
#include <Macros/TogglePRM.h>
#include <Macros/StringPRM.h>

// this
#include "SOP_VHACDScout.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator GET_SOP_Namespace()::SOP_Scout

/* -----------------------------------------------------------------
USING                                                              |
----------------------------------------------------------------- */

// YOUR CODE GOES HERE...

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

	namespace UI
	{		
		__DECLARE__Filter_Section_PRM(1)
		static auto		processModeChoiceMenuParm_Name = PRM_Name("processmode", "Process Mode");
		static auto		processModeChoiceMenuParm_Range = PRM_Range(PRM_RANGE_RESTRICTED, 0, PRM_RANGE_RESTRICTED, 1);
		static PRM_Name processModeChoiceMenuParm_Choices[] =
		{
			PRM_Name("0", "Pair"),
			PRM_Name("1", "Singles"),
			PRM_Name(nullptr)
		};
		static auto		processModeChoiceMenuParm_ChoiceList = PRM_ChoiceList(PRM_CHOICELIST_SINGLE, processModeChoiceMenuParm_Choices);
		auto			processModeChoiceMenu_Parameter = PRM_Template(PRM_ORD, 1, &processModeChoiceMenuParm_Name, nullptr, &processModeChoiceMenuParm_ChoiceList, &processModeChoiceMenuParm_Range, 0, nullptr, 1, "Define if inputs should be processed as pair or each separate.");

		__DECLARE_Main_Section_PRM(12)
		DECLARE_Toggle_with_Separator_OFF_PRM("addhullcountattribute", "Add Hull Count ATT", "addhullcountattributeseparator", 0, "Create detail hull count attribute with infomation about how many convex hulls were detected.", addHullCountAttribute)
		DECLARE_Toggle_with_Separator_OFF_PRM("addhullidattribute", "Add Hull ID ATT", "addhullidattributeseparator", 0, "Create primitive hull id attribute with membership infomation that helps identify to which convex hull polygons belongs to.", addHullIDAttribute)
		DECLARE_Toggle_with_Separator_OFF_PRM("groupperhull", "GRP Per Hull", "groupperhullseparator", &SOP_Operator::CallbackGRPPerHull, "Create primitive bundle id attribute with membership infomation that helps identify to which bundle polygons belongs to.", groupPerHull)
		DECLARE_Custom_String_PRM("specifyhullgroupname", "Name", "hull_", "Pick partial name for hull groups.", specifyHullGroupName)
		DECLARE_Toggle_with_Separator_OFF_PRM("addbundlecountattribute", "Add Bundle Count ATT", "addbundlecountattributeseparator", 0, "Create detail bundle count attribute with infomation about how many convex hulls bundles were detected.", addBundleCountAttribute)			
		DECLARE_Toggle_with_Separator_OFF_PRM("groupperbundle", "GRP Per Bundle", "groupperbundleseparator", &SOP_Operator::CallbackGRPPerBundle, "Create primitive bundle id attribute with membership infomation that helps identify to which bundle polygons belongs to.", groupPerBundle)
		DECLARE_Custom_String_PRM("specifybundlegroupname", "Name", "bundle_", "Pick partial name for bundle groups.", specifyBundleGroupName)

		__DECLARE_Additional_Section_PRM(4)
		DECLARE_DescriptionPRM(SOP_Operator)
	}
		
DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef SOP_Operator

#endif // !____prms_vhacdscout_h____