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

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator			GET_SOP_Namespace()::SOP_Scout
#define SOP_Base_Operator		SOP_VHACDNode

#define COMMON_NAMES			GET_SOP_Namespace()::COMMON_NAMES
#define UI						GET_SOP_Namespace()::UI
#define GRP_UTILS				GET_Base_Namespace()::Utility::Groups
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
	UI::addBundleCountAttributeToggle_Parameter,
	UI::addBundleCountAttributeSeparator_Parameter,
	UI::addHullIDAttributeToggle_Parameter,
	UI::addHullIDAttributeSeparator_Parameter,
	UI::groupPerHullToggle_Parameter,
	UI::groupPerHullSeparator_Parameter,
	UI::specifyHullGroupNameString_Parameter,
	UI::groupPerBundleToggle_Parameter,
	UI::groupPerBundleSeparator_Parameter,
	UI::specifyBundleGroupNameString_Parameter,

	UI::additionalSectionSwitcher_Parameter,
	PARAMETERLIST_DescriptionPRM(UI),

PARAMETERLIST_End()

bool 
SOP_Operator::updateParmsFlags()
{
	DEFAULTS_UpdateParmsFlags(SOP_Base_Operator)
		
	// hull specific
	bool groupPerHullValue;
	PRM_ACCESS::Get::IntPRM(this, groupPerHullValue, UI::groupPerHullToggle_Parameter, currentTime);
	changed |= setVisibleState(UI::specifyHullGroupNameString_Parameter.getToken(), groupPerHullValue);

	// bundle specific
	const auto is0Connected = getInput(0) == nullptr ? false : true;
	const auto is1Connected = getInput(1) == nullptr ? false : true;

	visibilityState = is0Connected && is1Connected;
	changed |= setVisibleState(UI::addBundleCountAttributeToggle_Parameter.getToken(), visibilityState);
	changed |= setVisibleState(UI::addBundleCountAttributeSeparator_Parameter.getToken(), visibilityState);

	changed |= setVisibleState(UI::groupPerBundleToggle_Parameter.getToken(), visibilityState);
	changed |= setVisibleState(UI::groupPerBundleSeparator_Parameter.getToken(), visibilityState);

	bool groupPerBundleValue;
	PRM_ACCESS::Get::IntPRM(this, groupPerBundleValue, UI::groupPerBundleToggle_Parameter, currentTime);

	visibilityState = visibilityState && groupPerBundleValue;
	changed |= setVisibleState(UI::specifyBundleGroupNameString_Parameter.getToken(), visibilityState);	

	// update description active state
	UPDATE_DescriptionPRM_ActiveState(this, UI)

	return changed;
}


/* -----------------------------------------------------------------
CALLBACKS                                                          |
----------------------------------------------------------------- */

IMPLEMENT_DescriptionPRM_Callback(SOP_Operator, UI)

#define THIS_CALLBACK_Reset_StringPRM(operatorname, callbackcall, parameter) int callbackcall(void* data, int index, float time, const PRM_Template* tmp) {const auto me = reinterpret_cast<operatorname*>(data); if (!me) return 0; auto defVal = UT_String(parameter.getFactoryDefaults()->getString()); PRM_ACCESS::Set::StringPRM(me, defVal, parameter, time); return 1; }

THIS_CALLBACK_Reset_StringPRM(SOP_Operator, SOP_Operator::CallbackGRPPerHull, UI::specifyHullGroupNameString_Parameter)
THIS_CALLBACK_Reset_StringPRM(SOP_Operator, SOP_Operator::CallbackGRPPerBundle, UI::specifyBundleGroupNameString_Parameter)

#undef THIS_CALLBACK_Reset_StringPRM

/* -----------------------------------------------------------------
OPERATOR INITIALIZATION                                            |
----------------------------------------------------------------- */

SOP_Operator::~SOP_Scout() { }

SOP_Operator::SOP_Scout(OP_Network* network, const char* name, OP_Operator* op)
: SOP_Base_Operator(network, name, op),
_addHullCountAttributeValue(false),
_addHullIDAttributeValue(false),
_groupPerHullValue(false),
_rebuildBundleIDAttributeValue(false), 
_addBundleCountAttributeValue(false),
_groupPerBundleValue(false)
{ op->setIconName(COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_SCOUT_ICONNAME_V2)); }

OP_Node* 
SOP_Operator::CreateMe(OP_Network* network, const char* name, OP_Operator* op) 
{ return new SOP_Operator(network, name, op); }

const char*
SOP_Operator::inputLabel(unsigned input) const
{
	switch (input)
	{
		case 0: return COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_OUTPUTNAME_CONVEXHULLS);
		default: return COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_OUTPUTNAME_ORIGINALGEOMETRY);
	}	
}

/* -----------------------------------------------------------------
HELPERS                                                            |
----------------------------------------------------------------- */

ENUMS::MethodProcessResult
SOP_Operator::AddHullCountATT(const GEO_PrimClassifier& classifier)
{	
	// we need at least 3 primitives, otherwise we have no hull
	if (this->gdp->getPrimitiveRange().getEntries() < 3)
	{
		addError(SOP_MESSAGE, "Not enough primitives to set convex hull attribute.");
		return ENUMS::MethodProcessResult::FAILURE;
	}

	auto hullCountHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_DETAIL, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_COUNT), 1, GA_Defaults(classifier.getNumClass())));
	if (hullCountHandle.isInvalid())
	{
		auto errorMessage = std::string("Failed to create ") + std::string(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_COUNT) + std::string(" attribute."));
		this->addError(SOP_MESSAGE, errorMessage.c_str());
		return ENUMS::MethodProcessResult::FAILURE;
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::AddHullIDATT(UT_AutoInterrupt progress, const GEO_PrimClassifier& classifier)
{
	auto hullIdHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_ID), 1));
	if (hullIdHandle.isInvalid())
	{
		auto errorMessage = std::string("Failed to create ") + std::string(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_ID) + std::string(" attribute."));
		this->addError(SOP_MESSAGE, errorMessage.c_str());
		return ENUMS::MethodProcessResult::FAILURE;
	}

	for (auto primIt = GA_Iterator(this->gdp->getPrimitiveRange()); !primIt.atEnd(); primIt.advance())
	{
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::INTERRUPT)
		hullIdHandle.set(*primIt, classifier.getClass(*primIt));
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult 
SOP_Operator::GRPPerHull(UT_AutoInterrupt progress, const GEO_PrimClassifier& classifier, fpreal time)
{
	// create groups
	UT_String hullGroupNameValue;
	PRM_ACCESS::Get::StringPRM(this, hullGroupNameValue, UI::specifyHullGroupNameString_Parameter, time);
	if (hullGroupNameValue.length() < 1)
	{
		addError(SOP_MESSAGE, "Specified hull group name is invalid.");
		return ENUMS::MethodProcessResult::FAILURE;
	}

	UT_Map<exint, GA_PrimitiveGroup*> hullGroups;
	hullGroups.clear();
	for (auto classID = 0; classID < classifier.getNumClass(); classID++)
	{
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::INTERRUPT)				

		hullGroups[classID] = this->gdp->newPrimitiveGroup(hullGroupNameValue.c_str() + std::to_string(classID));
	}

	// fill groups
	for (auto primIt = GA_Iterator(this->gdp->getPrimitiveRange()); !primIt.atEnd(); primIt.advance())
	{
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::INTERRUPT)

		hullGroups[classifier.getClass(*primIt)]->addOffset(*primIt);
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult	
SOP_Operator::ProcessHullSpecific(UT_AutoInterrupt progress, fpreal time)
{
	auto success = ENUMS::MethodProcessResult::SUCCESS;

	// get parameters
	PRM_ACCESS::Get::IntPRM(this, this->_addHullCountAttributeValue, UI::addHullCountAttributeToggle_Parameter, time);	
	PRM_ACCESS::Get::IntPRM(this, this->_addHullIDAttributeValue, UI::addHullIDAttributeToggle_Parameter, time);
	PRM_ACCESS::Get::IntPRM(this, this->_groupPerHullValue, UI::groupPerHullToggle_Parameter, time);

	if (this->_addHullCountAttributeValue || this->_addHullIDAttributeValue || this->_groupPerHullValue)
	{
		// @CLASS of 1999 (for more info check http://www.imdb.com/title/tt0099277/)
		auto primClassifier = GEO_PrimClassifier();
		primClassifier.classifyBySharedPoints(*this->gdp);		
		
		// generate data per toggle
		if (this->_addHullCountAttributeValue)
		{			
			success = AddHullCountATT(primClassifier);
			if (success != ENUMS::MethodProcessResult::SUCCESS) return success;
		}

		if (this->_addHullIDAttributeValue)
		{
			success = AddHullIDATT(progress, primClassifier);
			if (success != ENUMS::MethodProcessResult::SUCCESS) return success;
		}

		if (this->_groupPerHullValue)
		{
			success = GRPPerHull(progress, primClassifier, time);
			if (success != ENUMS::MethodProcessResult::SUCCESS) return success;
		}
	}
	
	return success;
}

ENUMS::MethodProcessResult  
SOP_Operator::AddBundleCountATT(exint bundlescount)
{		
	auto bundleCountHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_DETAIL, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_COUNT), 1, GA_Defaults(bundlescount)));	
	if (bundleCountHandle.isInvalid())
	{
		auto errorMessage = std::string("Failed to create ") + std::string(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_COUNT) + std::string(" attribute."));
		this->addError(SOP_MESSAGE, errorMessage.c_str());
		return ENUMS::MethodProcessResult::FAILURE;
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult  
SOP_Operator::GRPPerBundle(GA_Offset primitiveoffset, UT_Map<exint, GA_PrimitiveGroup*>& mappedbundlegroups, const GA_ROHandleI& bundleidhandle, fpreal time)
{
	const auto bundleID = bundleidhandle.get(primitiveoffset);
	if (!mappedbundlegroups.contains(bundleID))
	{
		// get partial group name
		UT_String bundleGroupNameValue;
		PRM_ACCESS::Get::StringPRM(this, bundleGroupNameValue, UI::specifyBundleGroupNameString_Parameter, time);
		if (bundleGroupNameValue.length() < 1)
		{
			this->addError(SOP_MESSAGE, "Specified bundle group name is invalid.");
			return ENUMS::MethodProcessResult::FAILURE;
		}

		// setup group
		const auto fullBundleGroupName = bundleGroupNameValue.c_str() + std::to_string(bundleID);
		mappedbundlegroups[bundleID] = this->gdp->newPrimitiveGroup(fullBundleGroupName);
	}
	
	if (!mappedbundlegroups.contains(bundleID))
	{
		this->addError(SOP_MESSAGE, "Failed to create bundle group.");
		return ENUMS::MethodProcessResult::FAILURE;
	}

	// store primitive offset
	mappedbundlegroups[bundleID]->addOffset(primitiveoffset);

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::ProcessBundleSpecific(UT_AutoInterrupt progress, OP_Context& context, ENUMS::ProcessedInputType processedinputtype, fpreal time)
{
	auto success = ENUMS::MethodProcessResult::SUCCESS;

	// get parameters
	PRM_ACCESS::Get::IntPRM(this, this->_addBundleCountAttributeValue, UI::addBundleCountAttributeToggle_Parameter, time);
	PRM_ACCESS::Get::IntPRM(this, this->_groupPerBundleValue, UI::groupPerBundleToggle_Parameter, time);

	if (this->_addBundleCountAttributeValue || this->_groupPerBundleValue)
	{
		const auto inputGDP = inputGeo(static_cast<exint>(processedinputtype), context);
		if (inputGDP)
		{
			// get this processed input bundle_id handle
			GA_ROHandleI inputBundleIDHandle;
			ATTRIB_ACCESS::Find::IntATT(this, inputGDP, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID), inputBundleIDHandle);

			if (inputBundleIDHandle.isInvalid())
			{
				auto errorMessage = std::string("Input ") + std::to_string(static_cast<exint>(processedinputtype)) + std::string(" is missing \"") + std::string(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID)) + std::string("\" attribute.");
				this->addError(SOP_MESSAGE, errorMessage.c_str());
				return ENUMS::MethodProcessResult::FAILURE;
			}

			// process requests
			UT_Set<exint>						uniqueBundleIDs;
			UT_Map<exint, GA_PrimitiveGroup*>	mappedBundleGroups;

			uniqueBundleIDs.clear();
			mappedBundleGroups.clear();

			for (auto primIt = GA_Iterator(inputGDP->getPrimitiveRange()); !primIt.atEnd(); primIt.advance())
			{
				PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::INTERRUPT)

				if (this->_addBundleCountAttributeValue) uniqueBundleIDs.insert(inputBundleIDHandle.get(*primIt));
				if (this->_groupPerBundleValue)
				{
					success = GRPPerBundle(*primIt, mappedBundleGroups, inputBundleIDHandle, time);
					if (success != ENUMS::MethodProcessResult::SUCCESS) return success;
				}
			}

			if (this->_addBundleCountAttributeValue) success = AddBundleCountATT(uniqueBundleIDs.size());
		}
	}

	return success;
}

/* -----------------------------------------------------------------
MAIN                                                               |
----------------------------------------------------------------- */

OP_ERROR 
SOP_Operator::cookMySop(OP_Context& context)
{	
	DEFAULTS_CookMySop()
	
	if (duplicateSource(static_cast<exint>(ENUMS::ProcessedInputType::CONVEX_HULLS), context, this->gdp) < OP_ERROR::UT_ERROR_WARNING && error() < OP_ERROR::UT_ERROR_WARNING)
	{
		auto success = ENUMS::MethodProcessResult::FAILURE;

		// check how many inputs is connected
		const auto is0Connected = getInput(0) == nullptr ? false : true;
		const auto is1Connected = getInput(1) == nullptr ? false : true;

		if (is0Connected && is1Connected)
		{
			success = ProcessBundleSpecific(progress, context, ENUMS::ProcessedInputType::CONVEX_HULLS, currentTime);
			if (success != ENUMS::MethodProcessResult::SUCCESS) return error();
		}
	
		success = ProcessHullSpecific(progress, currentTime);
	}

	return error();
}

GU_DetailHandle
SOP_Operator::cookMySopOutput(OP_Context& context, int outputidx, SOP_Node* interests)
{
	DEFAULTS_CookMySopOutput()
		
	// check how many inputs is connected
	const auto is0Connected = getInput(0) == nullptr ? false : true;
	const auto is1Connected = getInput(1) == nullptr ? false : true;
		
	if (is0Connected && !is1Connected) return result;	

	// get second input geometry	
	if (duplicateSource(static_cast<exint>(ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY), context, this->gdp) < OP_ERROR::UT_ERROR_WARNING && error() < OP_ERROR::UT_ERROR_WARNING)
	{
		const auto success = ProcessBundleSpecific(progress, context, ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY, currentTime);
		if (success != ENUMS::MethodProcessResult::SUCCESS)
		{
			this->gdp->clear();
			return result;
		}
	}

	return result;
}

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS
#undef ATTRIB_ACCESS
#undef PRM_ACCESS
#undef UI
#undef GRP_UTILS
#undef COMMON_NAMES

#undef SOP_Base_Operator
#undef SOP_Operator