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

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// SESI
#include <UT/UT_Interrupt.h>
#include <OP/OP_AutoLockInputs.h>
#include <CH/CH_Manager.h>
#include <PRM/PRM_Parm.h>
#include <PRM/PRM_Error.h>
#include <PRM/PRM_Include.h>

// hou-hdk-common
#include <Macros/ParameterList.h>
#include <Macros/ProgressEscape.h>
#include <Utility/PRM_TemplateAccessors.h>
#include <Utility/GA_AttributeAccessors.h>
#include <Utility/GU_DetailCalculator.h>

// this
#include "Parameters.h"
#include "FilterModeOption.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator			GET_SOP_Namespace()::SOP_VHACDTransform
#define SOP_Base_Operator		SOP_VHACDNode
#define MSS_Selector			GET_SOP_Namespace()::MSS_VHACDTransform

// very important
#define SOP_GroupFieldIndex_0	2

#define COMMON_NAMES			GET_SOP_Namespace()::COMMON_NAMES
#define UI						GET_SOP_Namespace()::UI

#define UTILS					GET_Base_Namespace()::Utility
#define PRM_ACCESS				UTILS::PRM_TemplateAccessors
#define ATTRIB_ACCESS			UTILS::GA_AttributeAccessors

#define CONTAINERS				GET_Base_Namespace()::Containers
#define ENUMS					GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

PARAMETERLIST_Start(SOP_Operator)

	UI::filterSectionSwitcher_Parameter,		
	UI::filterModeChoiceMenu_Parameter,
	UI::input0PrimitiveGroup_Parameter,
	UI::bundleIDPatternString_Parameter,

	UI::mainSectionSwitcher_Parameter,

	UI::additionalSectionSwitcher_Parameter,
	PARAMETERLIST_DescriptionPRM(UI),

PARAMETERLIST_End()

bool 
SOP_Operator::updateParmsFlags()
{
	DEFAULTS_UpdateParmsFlags(SOP_Base_Operator)

	// is input connected?
	const exint is0Connected = this->getInput(0) == nullptr ? 0 : 1;

	/* ---------------------------- Set Global Visibility ---------------------------- */

	visibilityState = is0Connected ? 1 : 0;

	/* ---------------------------- Set States --------------------------------------- */

	// update description active state
	UPDATE_DescriptionPRM_ActiveState(this, UI)

	return changed;
}

/* -----------------------------------------------------------------
CALLBACKS                                                          |
----------------------------------------------------------------- */

IMPLEMENT_DescriptionPRM_Callback(SOP_Operator, UI)

/* -----------------------------------------------------------------
OPERATOR INITIALIZATION                                            |
----------------------------------------------------------------- */

SOP_Operator::~SOP_VHACDTransform() { }

SOP_Operator::SOP_VHACDTransform(OP_Network* network, const char* name, OP_Operator* op) :
SOP_Base_Operator(network, name, op),
_processedInputType(ENUMS::ProcessedInputType::NO_TYPE),
_convexGDP(nullptr),
_originalGDP(nullptr),
_primitiveGroupInput0(nullptr),
_primitiveGroupInput1(nullptr)
{ op->setIconName(COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_TRANSFORM_ICONNAME)); }

OP_Node* 
SOP_Operator::CreateMe(OP_Network* network, const char* name, OP_Operator* op) 
{ return new SOP_Operator(network, name, op); }

const char* 
SOP_Operator::inputLabel(unsigned input) const
{ return std::to_string(input).c_str(); }

OP_ERROR
SOP_Operator::cookInputGroups(OP_Context& context, int alone)
{
	// if we are called by the handle, then "alone" equals 1, and we have to lock the inputs ourselves, and unlock them before exiting this method
	if (alone)
	{
		if (lockInputs(context) >= UT_ErrorSeverity::UT_ERROR_ABORT) return error();
	}

	// for this node, we get geometry ourself, even if we are not called by handles
	this->_convexGDP = const_cast<GU_Detail*>(inputGeo(static_cast<exint>(ENUMS::ProcessedInputType::CONVEX_HULLS), context));
	this->_originalGDP = const_cast<GU_Detail*>(inputGeo(static_cast<exint>(ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY), context));
	
	// get group string
	UT_String inputPrimitiveGroupNameValue;
	PRM_ACCESS::Get::StringPRM(this, inputPrimitiveGroupNameValue, UI::input0PrimitiveGroup_Parameter, context.getTime());

	// parse groups
	if (inputPrimitiveGroupNameValue.isstring())
	{
		this->_primitiveGroupInput0 = parsePrimitiveGroups(inputPrimitiveGroupNameValue.c_str(), GroupCreator(this->_convexGDP, false));
		this->_primitiveGroupInput1 = parsePrimitiveGroups(inputPrimitiveGroupNameValue.c_str(), GroupCreator(this->_originalGDP, false));

		if (!this->_primitiveGroupInput0 || !this->_primitiveGroupInput1) this->addError(SOP_ERR_BADGROUP, inputPrimitiveGroupNameValue);
		else if (!alone)
		{
			select(*const_cast<GA_PrimitiveGroup*>(this->_primitiveGroupInput0), true);
			select(*const_cast<GA_PrimitiveGroup*>(this->_primitiveGroupInput1), true);
		}
	}
	else if (!alone) clearSelection();

	// notify handles
	notifyGroupParmListeners(SOP_GroupFieldIndex_0, -1, static_cast<const GU_Detail*>(this->_convexGDP), this->_primitiveGroupInput0);
	notifyGroupParmListeners(SOP_GroupFieldIndex_0, -1, static_cast<const GU_Detail*>(this->_originalGDP), this->_primitiveGroupInput1);

	// if we are called by the handles, then we have to unlock our inputs
	if (alone)
	{
		destroyAdhocGroups();
		unlockInputs();
	}

	return error();
}

/* -----------------------------------------------------------------
HELPERS                                                            |
----------------------------------------------------------------- */

// YOUR CODE GOES HERE...

/* -----------------------------------------------------------------
MAIN                                                               |
----------------------------------------------------------------- */

OP_ERROR 
SOP_Operator::cookMySop(OP_Context& context)
{
	DEFAULTS_CookMySop()

	this->_processedInputType = ENUMS::ProcessedInputType::CONVEX_HULLS;

	if (duplicateSource(static_cast<exint>(ENUMS::ProcessedInputType::CONVEX_HULLS), context) < OP_ERROR::UT_ERROR_WARNING && error() < OP_ERROR::UT_ERROR_WARNING && cookInputGroups(context) < OP_ERROR::UT_ERROR_WARNING)
	{		
		if (this->_primitiveGroupInput0)
		{
			std::cout << "Convex: " << this->_primitiveGroupInput0->entries() << std::endl;
		}

		exint filterModeOptionValue;
		PRM_ACCESS::Get::IntPRM(this, filterModeOptionValue, UI::filterModeChoiceMenu_Parameter, currentTime);

		switch (filterModeOptionValue)
		{
			case static_cast<exint>(ENUMS::FilterModeOption::BY_BUNDLE_ID) : break;
			default: break;
		}
	}

	return error();
}

GU_DetailHandle
SOP_Operator::cookMySopOutput(OP_Context& context, int outputidx, SOP_Node* interests)
{
	DEFAULTS_CookMySopOutput()
			
	this->_processedInputType = ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY;

	if (duplicateSource(static_cast<exint>(ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY), context) < OP_ERROR::UT_ERROR_WARNING && error() < OP_ERROR::UT_ERROR_WARNING && cookInputGroups(context) < OP_ERROR::UT_ERROR_WARNING)
	{
		if (this->_primitiveGroupInput1)
		{
			std::cout << "Original: " << this->_primitiveGroupInput1->entries() << std::endl;
		}

		exint filterModeOptionValue;
		PRM_ACCESS::Get::IntPRM(this, filterModeOptionValue, UI::filterModeChoiceMenu_Parameter, currentTime);

		switch (filterModeOptionValue)
		{
			case static_cast<exint>(ENUMS::FilterModeOption::BY_BUNDLE_ID) : break;
			default: break;
		}
	}

	return result;
}

/* -----------------------------------------------------------------
SELECTOR IMPLEMENTATION                                            |
----------------------------------------------------------------- */

MSS_Selector::~MSS_VHACDTransform() { }

MSS_Selector::MSS_VHACDTransform(OP3D_View& viewer, PI_SelectorTemplate& templ) 
: MSS_ReusableSelector(viewer, templ, COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_TRANSFORM_SMALLNAME), COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_TRANSFORM_GROUP_PRMNAME), nullptr, true)
{ this->setAllowUseExistingSelection(false); }

BM_InputSelector*
MSS_Selector::CreateMe(BM_View& viewer, PI_SelectorTemplate& templ)
{ return new MSS_Selector(reinterpret_cast<OP3D_View&>(viewer), templ); }

const char*
MSS_Selector::className() const
{ return "MSS_VHACDTransform"; }

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS

#undef ATTRIB_ACCESS
#undef PRM_ACCESS
#undef UTILS

#undef UI
#undef COMMON_NAMES

#undef MSS_Selector
#undef SOP_Base_Operator
#undef SOP_Operator