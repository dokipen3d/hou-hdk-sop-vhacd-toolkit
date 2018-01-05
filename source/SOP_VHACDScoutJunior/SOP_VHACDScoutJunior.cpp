/*
	Volumetric-Hierarchical Approximate Convex Decomposition.
	Based on https://github.com/kmammou/v-hacd

	IMPORTANT! ------------------------------------------	
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
#include <GEO/GEO_PrimClassifier.h>

// hou-hdk-common
#include <Macros/ParameterList.h>
#include <Utility/ParameterAccessing.h>
#include <Utility/AttributeAccessing.h>

// this
#include "Parameters.h"
#include "ProcessedInputType.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator			GET_SOP_Namespace()::SOP_VHACDScoutJunior
#define SOP_Base_Operator		SOP_VHACDScout

#define COMMON_NAMES			GET_SOP_Namespace()::COMMON_NAMES
#define UI						GET_SOP_Namespace()::UI
#define PRM_ACCESS				GET_Base_Namespace()::Utility::PRM
#define ATTRIB_ACCESS			GET_Base_Namespace()::Utility::Attribute
#define CONTAINERS				GET_Base_Namespace()::Containers
#define ENUMS					GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

PARAMETERLIST_Start(SOP_Operator)

	UI::mainSectionSwitcher_Parameter,
	UI::addHullCountAttributeToggle_Parameter,
	UI::addHullCountAttributeSeparator_Parameter,
	UI::addHullIDAttributeToggle_Parameter,
	UI::addHullIDAttributeSeparator_Parameter,
	UI::groupPerHullToggle_Parameter,
	UI::groupPerHullSeparator_Parameter,
	UI::specifyHullGroupNameString_Parameter,
	UI::pointPerHullCenterToggle_Parameter,
	UI::pointPerHullCenterSeparator_Parameter,

	UI::additionalSectionSwitcher_Parameter,
	PARAMETERLIST_DescriptionPRM(UI),

PARAMETERLIST_End()

bool 
SOP_Operator::updateParmsFlags()
{
	DEFAULTS_UpdateParmsFlags(SOP_Base_Operator)

	// update visibility
	bool groupPerHullValue;
	PRM_ACCESS::Get::IntPRM(this, groupPerHullValue, UI::groupPerHullToggle_Parameter, currentTime);	
	changed |= setVisibleState(UI::specifyHullGroupNameString_Parameter.getToken(), groupPerHullValue);

	// update description active state
	UPDATE_DescriptionPRM_ActiveState(this, UI)

	return changed;
}

/* -----------------------------------------------------------------
CALLBACKS                                                          |
----------------------------------------------------------------- */

IMPLEMENT_DescriptionPRM_Callback(SOP_Operator, UI)

int
SOP_Operator::CallbackGRPPerHull(void* data, int index, float time, const PRM_Template* tmp)
{
	const auto me = reinterpret_cast<SOP_Operator*>(data);
	if (!me) return 0;

	auto defVal1 = UT_String(UI::specifyHullGroupNameString_Parameter.getFactoryDefaults()->getString());
	PRM_ACCESS::Set::StringPRM(me, defVal1, UI::specifyHullGroupNameString_Parameter, time);

	auto defVal2 = static_cast<exint>(UI::pointPerHullCenterToggle_Parameter.getFactoryDefaults()->getOrdinal());
	PRM_ACCESS::Set::IntPRM(me, defVal2, UI::pointPerHullCenterToggle_Parameter, time);

	return 1;
}

int
SOP_Operator::CallbackPointPerHullCenter(void* data, int index, float time, const PRM_Template* tmp)
{
	const auto me = reinterpret_cast<SOP_Operator*>(data);
	if (!me) return 0;

	// TODO: figure out why restoreFactoryDefaults() doesn't work
	auto defVal = static_cast<exint>(UI::groupPerHullToggle_Parameter.getFactoryDefaults()->getOrdinal());
	PRM_ACCESS::Set::IntPRM(me, defVal, UI::groupPerHullToggle_Parameter, time);

	return 1;
}

/* -----------------------------------------------------------------
OPERATOR INITIALIZATION                                            |
----------------------------------------------------------------- */

SOP_Operator::~SOP_VHACDScoutJunior() { }

SOP_Operator::SOP_VHACDScoutJunior(OP_Network* network, const char* name, OP_Operator* op)
: SOP_Base_Operator(network, name, op)
{ op->setIconName(COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_SCOUT_JUNIOR_ICONNAME)); }

OP_Node* 
SOP_Operator::CreateMe(OP_Network* network, const char* name, OP_Operator* op) 
{ return new SOP_Operator(network, name, op); }

const char*
SOP_Operator::inputLabel(unsigned input) const
{ return std::to_string(input).c_str(); }

/* -----------------------------------------------------------------
MAIN                                                               |
----------------------------------------------------------------- */

OP_ERROR 
SOP_Operator::cookMySop(OP_Context& context)
{	
	DEFAULTS_CookMySop()		
	
	if (duplicateSource(static_cast<exint>(ENUMS::ProcessedInputType::CONVEX_HULLS), context, this->gdp) <= OP_ERROR::UT_ERROR_WARNING && error() <= OP_ERROR::UT_ERROR_WARNING)
	{
		// get parameters
		PRM_ACCESS::Get::IntPRM(this, this->_addHullCountAttributeValue, UI::addHullCountAttributeToggle_Parameter, currentTime);
		PRM_ACCESS::Get::IntPRM(this, this->_addHullIDAttributeValue, UI::addHullIDAttributeToggle_Parameter, currentTime);
		PRM_ACCESS::Get::IntPRM(this, this->_groupPerHullValue, UI::groupPerHullToggle_Parameter, currentTime);		
		PRM_ACCESS::Get::StringPRM(this, this->_partialHullGroupNameValue, UI::specifyHullGroupNameString_Parameter, currentTime);
		PRM_ACCESS::Get::IntPRM(this, this->_pointPerHullCenterValue, UI::pointPerHullCenterToggle_Parameter, currentTime);

		ProcessHullSpecific(progress, currentTime);
	}
	
	return error();
}

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS
#undef ATTRIB_ACCESS
#undef PRM_ACCESS
#undef UI
#undef COMMON_NAMES

#undef SOP_Base_Operator
#undef SOP_Operator