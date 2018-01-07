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
#include <GA/GA_AttributeDict.h>

// hou-hdk-common
#include <Macros/ParameterList.h>
#include <Macros/ProgressEscape.h>
#include <Utility/PRM_TemplateAccessors.h>
#include <Utility/GA_AttributeAccessors.h>
#include <Utility/GA_AttributeMismatchTester.h>

// this
#include "Parameters.h"
#include "ProcessModeOption.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator			GET_SOP_Namespace()::SOP_VHACDMerge
#define SOP_Base_Operator		SOP_VHACDNode

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
	UI::processModeChoiceMenu_Parameter,
	UI::filterErrorsSeparator_Parameter,
	UI::attributeMismatchErrorModeChoiceMenu_Parameter,
	UI::hullCountMismatchErrorModeChoiceMenu_Parameter,
	UI::hullIDMismatchErrorModeChoiceMenu_Parameter,
	UI::bundleCountMismatchErrorModeChoiceMenu_Parameter,
	UI::bundleIDMismatchErrorModeChoiceMenu_Parameter,

	UI::additionalSectionSwitcher_Parameter,
	PARAMETERLIST_DescriptionPRM(UI),

PARAMETERLIST_End()

bool 
SOP_Operator::updateParmsFlags()
{
	DEFAULTS_UpdateParmsFlags(SOP_Base_Operator)

	/*
	// is input connected?
	const exint is0Connected = getInput(0) == nullptr ? 0 : 1;
	const exint is1Connected = getInput(1) == nullptr ? 0 : 1;
	const exint is2Connected = getInput(2) == nullptr ? 0 : 1;
	const exint is3Connected = getInput(3) == nullptr ? 0 : 1;
	const exint is4Connected = getInput(4) == nullptr ? 0 : 1;
	const exint is5Connected = getInput(5) == nullptr ? 0 : 1;
	const exint is6Connected = getInput(6) == nullptr ? 0 : 1;
	const exint is7Connected = getInput(7) == nullptr ? 0 : 1;
	*/

	/* ---------------------------- Set Global Visibility ---------------------------- */

	//visibilityState = is0Connected ? 1 : 0; // TODO: do I still need this?

	/* ---------------------------- Set States --------------------------------------- */

	// check for attributes mismatch	
	exint attributeMismatchErrorLevelValue;
	PRM_ACCESS::Get::IntPRM(this, attributeMismatchErrorLevelValue, UI::attributeMismatchErrorModeChoiceMenu_Parameter, currentTime);
	
	switch (attributeMismatchErrorLevelValue)
	{
		case static_cast<exint>(ENUMS::MismatchErrorModeOption::NONE) : { visibilityState = 0; } break;
		case static_cast<exint>(ENUMS::MismatchErrorModeOption::NONE_AND_OVERRIDE) : { visibilityState = 1; } break;
		case static_cast<exint>(ENUMS::MismatchErrorModeOption::WARNING) : { visibilityState = 0; } break;
		case static_cast<exint>(ENUMS::MismatchErrorModeOption::WARNING_AND_OVERRIDE) : { visibilityState = 1; } break;
		case static_cast<exint>(ENUMS::MismatchErrorModeOption::ERROR) : { visibilityState = 0; } break;
	}
	
	changed |= setVisibleState(UI::hullCountMismatchErrorModeChoiceMenu_Parameter.getToken(), visibilityState);
	changed |= setVisibleState(UI::hullIDMismatchErrorModeChoiceMenu_Parameter.getToken(), visibilityState);
	changed |= setVisibleState(UI::bundleCountMismatchErrorModeChoiceMenu_Parameter.getToken(), visibilityState);
	changed |= setVisibleState(UI::bundleIDMismatchErrorModeChoiceMenu_Parameter.getToken(), visibilityState);

	// update description active state
	UPDATE_DescriptionPRM_ActiveState(this, UI)

	return changed;
}

IMPLEMENT_DescriptionPRM_Callback(SOP_Operator, UI)

/* -----------------------------------------------------------------
OPERATOR INITIALIZATION                                            |
----------------------------------------------------------------- */

SOP_Operator::~SOP_VHACDMerge() { }

SOP_Operator::SOP_VHACDMerge(OP_Network* network, const char* name, OP_Operator* op) :
SOP_Base_Operator(network, name, op), 
_createHullCount(false), 
_createHullID(false), 
_createBundleCount(false),
_createBundleID(false)
{ op->setIconName(COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_MERGE_ICONNAME)); }

OP_Node* 
SOP_Operator::CreateMe(OP_Network* network, const char* name, OP_Operator* op) 
{ return new SOP_Operator(network, name, op); }

const char* 
SOP_Operator::inputLabel(unsigned input) const
{ return std::to_string(input).c_str(); }

/* -----------------------------------------------------------------
CALLBACKS                                                          |
----------------------------------------------------------------- */

int	
SOP_Operator::CallbackAttributeMismatchErrorModeChoiceMenu(void* data, int index, float time, const PRM_Template* tmp)
{
	const auto me = reinterpret_cast<SOP_Operator*>(data);
	if (!me) return 0;

	exint attributeMismatchErrorLevelValue;
	PRM_ACCESS::Get::IntPRM(me, attributeMismatchErrorLevelValue, UI::attributeMismatchErrorModeChoiceMenu_Parameter, time);

	if (attributeMismatchErrorLevelValue == static_cast<exint>(ENUMS::MismatchErrorModeOption::NONE) || attributeMismatchErrorLevelValue == static_cast<exint>(ENUMS::MismatchErrorModeOption::WARNING) || attributeMismatchErrorLevelValue == static_cast<exint>(ENUMS::MismatchErrorModeOption::ERROR))
	{
		// TODO: figure out why restoreFactoryDefaults() doesn't work
		auto defVal0 = static_cast<exint>(UI::hullCountMismatchErrorModeChoiceMenu_Parameter.getFactoryDefaults()->getOrdinal());
		auto defVal1 = static_cast<exint>(UI::hullCountMismatchErrorModeChoiceMenu_Parameter.getFactoryDefaults()->getOrdinal());
		auto defVal2 = static_cast<exint>(UI::hullCountMismatchErrorModeChoiceMenu_Parameter.getFactoryDefaults()->getOrdinal());
		auto defVal3 = static_cast<exint>(UI::hullCountMismatchErrorModeChoiceMenu_Parameter.getFactoryDefaults()->getOrdinal());

		PRM_ACCESS::Set::IntPRM(me, defVal0, UI::hullCountMismatchErrorModeChoiceMenu_Parameter, time);
		PRM_ACCESS::Set::IntPRM(me, defVal0, UI::hullIDMismatchErrorModeChoiceMenu_Parameter, time);
		PRM_ACCESS::Set::IntPRM(me, defVal0, UI::bundleCountMismatchErrorModeChoiceMenu_Parameter, time);
		PRM_ACCESS::Set::IntPRM(me, defVal0, UI::bundleIDMismatchErrorModeChoiceMenu_Parameter, time);
	}	

	return 1;
}

/* -----------------------------------------------------------------
HELPERS                                                            |
----------------------------------------------------------------- */

ENUMS::MethodProcessResult
SOP_Operator::GetAllDetailsOfType(UT_AutoInterrupt progress, UT_Array<const GU_Detail*>& details, ENUMS::ProcessedInputType processedinput, fpreal time)
{	
	// get which input is processed
	const exint processedInputStart = processedinput == ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY;

	bool processModeChoiceMenuValue;
	PRM_ACCESS::Get::IntPRM(this, processModeChoiceMenuValue, UI::processModeChoiceMenu_Parameter, time);

	// collect convex hulls detail
	const auto inputsCount = nInputs();	
	for (auto inputNumber = processedInputStart; inputNumber < inputsCount; inputNumber += (processModeChoiceMenuValue? 1 : 2 ))
	{		
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_BOOL(this, progress, ENUMS::MethodProcessResult::INTERRUPT)

		const auto currDetail = inputGeo(inputNumber);
		if (currDetail) details.append(currDetail);
		else
		{
			this->addError(SOP_MESSAGE, "Found NULL input.");
			return ENUMS::MethodProcessResult::FAILURE;
		}
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

void
SOP_Operator::WhenSpecificAttributeMismatch(const PRM_Template& parameter, UT_String attributename, fpreal time)
{
	exint mismatchErrorModeValue;
	PRM_ACCESS::Get::IntPRM(this, mismatchErrorModeValue, parameter, time);

	auto errorMessage = std::string(attributename) + std::string(" attribute is missing in one or more inputs.");
	auto warningMessage = std::string(errorMessage) + std::string(" Merged value will be calculated inproperly.");

	switch (mismatchErrorModeValue)
	{
		case static_cast<exint>(ENUMS::NodeErrorLevel::NONE) :		break; 
		case static_cast<exint>(ENUMS::NodeErrorLevel::WARNING) :	{ this->addWarning(SOP_MESSAGE, warningMessage.c_str()); } break; 
		case static_cast<exint>(ENUMS::NodeErrorLevel::ERROR) :		{ this->addError(SOP_MESSAGE, errorMessage.c_str()); } break;
	}
}

void
SOP_Operator::WhenOverrideAttributeMismatch(UT_AutoInterrupt progress, UT_Array<const GU_Detail*>& details, ENUMS::MismatchErrorModeOption mismatchoption, ENUMS::ProcessedInputType processedinput, fpreal time)
{	
	UT_Array<UT_StringSet>	filteredNames;
	exint					processModeChoiceMenuOption;
	
	filteredNames.clear();
	PRM_ACCESS::Get::IntPRM(this, processModeChoiceMenuOption, UI::processModeChoiceMenu_Parameter, time);	

	auto mismatchInfos = UTILS::GA_AttributeMismatchTester::MismatchOfAllOwners(this, progress, details, GA_AttributeScope::GA_SCOPE_PUBLIC, ENUMS::NodeErrorLevel::NONE);
	for (auto mismatch : mismatchInfos)
	{	
		// remove V-HACD specific attributes from list
		auto currNames = mismatch.GetNames();		
		if (mismatchoption == ENUMS::MismatchErrorModeOption::WARNING_AND_OVERRIDE)
		{			
			if (mismatch.GetOwner() == GA_AttributeOwner::GA_ATTRIB_DETAIL)
			{
				if (mismatch.Contains(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_COUNT))) currNames.erase(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_COUNT));
				if (mismatch.Contains(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_COUNT))) currNames.erase(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_COUNT));
			}

			if (mismatch.GetOwner() == GA_AttributeOwner::GA_ATTRIB_PRIMITIVE)
			{
				if (mismatch.Contains(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_ID))) currNames.erase(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_ID));
				if (mismatch.Contains(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID))) currNames.erase(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID));
			}
		}		
		filteredNames.append(currNames);
				
		// set V-HACD specific attributes mismatch
		if (mismatchoption == ENUMS::MismatchErrorModeOption::NONE_AND_OVERRIDE || mismatchoption == ENUMS::MismatchErrorModeOption::WARNING_AND_OVERRIDE)
		{			
			if (mismatch.GetOwner() == GA_AttributeOwner::GA_ATTRIB_DETAIL)
			{
				if ((mismatch.Contains(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_COUNT)) && processedinput == ENUMS::ProcessedInputType::CONVEX_HULLS) || (mismatch.Contains(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_COUNT)) && processModeChoiceMenuOption == static_cast<exint>(ENUMS::ProcessedModeOption::SINGLE))) WhenSpecificAttributeMismatch(UI::hullCountMismatchErrorModeChoiceMenu_Parameter, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_COUNT), time);
				if (mismatch.Contains(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_COUNT))) WhenSpecificAttributeMismatch(UI::bundleCountMismatchErrorModeChoiceMenu_Parameter, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_COUNT), time);
			}

			if (mismatch.GetOwner() == GA_AttributeOwner::GA_ATTRIB_PRIMITIVE)
			{
				if ((mismatch.Contains(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_ID)) && processedinput == ENUMS::ProcessedInputType::CONVEX_HULLS) || (mismatch.Contains(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_ID)) && processModeChoiceMenuOption == static_cast<exint>(ENUMS::ProcessedModeOption::SINGLE))) WhenSpecificAttributeMismatch(UI::hullIDMismatchErrorModeChoiceMenu_Parameter, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_ID), time);
				if (mismatch.Contains(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID))) WhenSpecificAttributeMismatch(UI::bundleIDMismatchErrorModeChoiceMenu_Parameter, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID), time);
			}
		}
	}

	// set global mismatch warning
	if (mismatchoption == ENUMS::MismatchErrorModeOption::WARNING_AND_OVERRIDE)
	{
		const auto mismatchMessage = UTILS::GA_AttributeMismatchTester::ComposeMismatchMessage(this, progress, filteredNames);
		if (mismatchMessage != nullptr) this->addWarning(SOP_MESSAGE, mismatchMessage.c_str());
	}
}

void
SOP_Operator::HandleAttributesMismatch(UT_AutoInterrupt progress, UT_Array<const GU_Detail*>& details, ENUMS::ProcessedInputType processedinput, fpreal time)
{
	exint attributeMismatchErrorLevelValue;
	PRM_ACCESS::Get::IntPRM(this, attributeMismatchErrorLevelValue, UI::attributeMismatchErrorModeChoiceMenu_Parameter, time);
	
	switch (attributeMismatchErrorLevelValue)
	{		
		case static_cast<exint>(ENUMS::MismatchErrorModeOption::NONE_AND_OVERRIDE) :		{ WhenOverrideAttributeMismatch(progress, details, ENUMS::MismatchErrorModeOption::NONE_AND_OVERRIDE, processedinput, time); } break;
		case static_cast<exint>(ENUMS::MismatchErrorModeOption::WARNING_AND_OVERRIDE) :		{ WhenOverrideAttributeMismatch(progress, details, ENUMS::MismatchErrorModeOption::WARNING_AND_OVERRIDE, processedinput, time); } break;
		case static_cast<exint>(ENUMS::MismatchErrorModeOption::NONE) :						break;
		case static_cast<exint>(ENUMS::MismatchErrorModeOption::WARNING) :					{ UTILS::GA_AttributeMismatchTester::MismatchOfAllOwners(this, progress, details, GA_AttributeScope::GA_SCOPE_PUBLIC, ENUMS::NodeErrorLevel::WARNING); } break;
		case static_cast<exint>(ENUMS::MismatchErrorModeOption::ERROR) :					{ UTILS::GA_AttributeMismatchTester::MismatchOfAllOwners(this, progress, details, GA_AttributeScope::GA_SCOPE_PUBLIC, ENUMS::NodeErrorLevel::ERROR); } break;
	}
}

ENUMS::MethodProcessResult
SOP_Operator::AddFoundVHACDAttributes(UT_AutoInterrupt progress, UT_Array<const GU_Detail*>& details, ENUMS::ProcessedInputType processedinput, fpreal time)
{
	// reset on each cook
	if (processedinput == ENUMS::ProcessedInputType::CONVEX_HULLS)
	{
		this->_createHullCount = false;
		this->_createHullID = false;

		this->_hullCountHandle.clear();
		this->_hullIDHandle.clear();
	}

	this->_createBundleCount = false;
	this->_createBundleID = false;
	
	this->_bundleCountHandle.clear();
	this->_bundleIDHandle.clear();

	// check which attributes should be created
	for (auto currDetail : details)
	{
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_BOOL(this, progress, ENUMS::MethodProcessResult::INTERRUPT)

		// check each attribute
		if (processedinput == ENUMS::ProcessedInputType::CONVEX_HULLS)
		{
			const auto hullCountAttribute = currDetail->findIntTuple(GA_AttributeOwner::GA_ATTRIB_DETAIL, GA_AttributeScope::GA_SCOPE_PUBLIC, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_COUNT));
			const auto hullIDAttribute = currDetail->findIntTuple(GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, GA_AttributeScope::GA_SCOPE_PUBLIC, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_ID));

			if (!this->_createHullCount) this->_createHullCount = hullCountAttribute != nullptr;
			if (!this->_createHullID) this->_createHullID = hullIDAttribute != nullptr;
		}

		const auto bundleCountAttribute = currDetail->findIntTuple(GA_AttributeOwner::GA_ATTRIB_DETAIL, GA_AttributeScope::GA_SCOPE_PUBLIC, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_COUNT));
		const auto bundleIDAttribute = currDetail->findIntTuple(GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, GA_AttributeScope::GA_SCOPE_PUBLIC, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID));

		if (!this->_createBundleCount) this->_createBundleCount = bundleCountAttribute != nullptr;
		if (!this->_createBundleID) this->_createBundleID = bundleIDAttribute != nullptr;
	}

	// create attributes	
	this->gdp->clearAndDestroy();

	if (processedinput == ENUMS::ProcessedInputType::CONVEX_HULLS)
	{
		if (this->_createHullCount) this->_hullCountHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_DETAIL, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_COUNT), 1, GA_Defaults(exint(0))));
		if (this->_createHullID) this->_hullIDHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_ID), 1));
	}

	if (this->_createBundleCount) this->_bundleCountHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_DETAIL, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_COUNT), 1, GA_Defaults(exint(0))));
	if (this->_createBundleID) this->_bundleIDHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID), 1));

	return ENUMS::MethodProcessResult::SUCCESS;
}

void
SOP_Operator::ShiftCurrentDetailPrimitiveAttributes(GU_Detail* currentdetail, UT_AutoInterrupt progress, exint iteration, exint& hullshiftvalue, exint& bundleshiftvalue, ENUMS::ProcessedInputType processedinput)
{
	auto			success = false;
	GA_RWHandleI	currHullIDHandle;
	GA_RWHandleI	currBundleIDHandle;

	if (this->_hullIDHandle.isValid() && processedinput == ENUMS::ProcessedInputType::CONVEX_HULLS)
	{
		// find attribute in current detail
		success = ATTRIB_ACCESS::Find::IntATT(this, currentdetail, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_ID), currHullIDHandle);
		if (success)
		{
			// shift values to create 0 - N range
			UT_Map<exint, exint> remapedValues;				
			remapedValues.clear();
				
			exint currIter = 0;
			for (auto primIt = GA_Iterator(currentdetail->getPrimitiveRange()); !primIt.atEnd(); primIt.advance())
			{
				PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_RETURN(this, progress)
					
				// get current value from detail
				const auto currValue = currHullIDHandle.get(*primIt);

				// check if we remaped it before
				success = remapedValues.contains(currValue);
				if (!success)
				{
					remapedValues[currValue] = currIter + hullshiftvalue;
					currHullIDHandle.set(*primIt, currIter + hullshiftvalue);
					currIter++;
				}
				else currHullIDHandle.set(*primIt, remapedValues[currValue]);
			}

			// update shift value for next iteration
			if (iteration == 0) hullshiftvalue = currIter;
			else hullshiftvalue += currIter;				
		}	
	}

	if (this->_bundleIDHandle.isValid())
	{
		// find attribute in current detail
		success = ATTRIB_ACCESS::Find::IntATT(this, currentdetail, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID), currBundleIDHandle);
		if (success)
		{
			// shift values to create 0 - N range
			UT_Map<exint, exint> remapedValues;
			remapedValues.clear();

			exint currIter = 0;
			for (auto primIt = GA_Iterator(currentdetail->getPrimitiveRange()); !primIt.atEnd(); primIt.advance())
			{
				PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_RETURN(this, progress)

				// get current value from detail
				const auto currValue = currBundleIDHandle.get(*primIt);

				// check if we remaped it before
				success = remapedValues.contains(currValue);
				if (!success)
				{
					remapedValues[currValue] = currIter + bundleshiftvalue;
					currBundleIDHandle.set(*primIt, currIter + bundleshiftvalue);
					currIter++;
				}
				else currBundleIDHandle.set(*primIt, remapedValues[currValue]);
			}

			// update shift value for next iteration
			if (iteration == 0) bundleshiftvalue = currIter;
			else bundleshiftvalue += currIter;
		}
	}
}

ENUMS::MethodProcessResult
SOP_Operator::MergeCurrentDetail(const GU_Detail* detail, exint iteration, exint detailscount)
{
	auto success = false;

	// merge current detail into main detail
	if (detailscount > 1)
	{
		if (iteration == 0)
		{
			success = this->gdp->copy(*detail, GEO_CopyMethod::GEO_COPY_START);
			if (!success)
			{
				this->addError(SOP_MESSAGE, "Geometry merge failure on GEO_COPY_START");
				return ENUMS::MethodProcessResult::FAILURE;
			}			
		}
		else if (iteration == detailscount - 1)
		{
			success = this->gdp->copy(*detail, GEO_CopyMethod::GEO_COPY_END);
			if (!success)
			{
				this->addError(SOP_MESSAGE, "Geometry merge failure on GEO_COPY_END");
				return ENUMS::MethodProcessResult::FAILURE;
			}			
		}
		else
		{
			success = this->gdp->copy(*detail, GEO_CopyMethod::GEO_COPY_ADD);
			if (!success)
			{
				this->addError(SOP_MESSAGE, "Geometry merge failure on GEO_COPY_ADD");
				return ENUMS::MethodProcessResult::FAILURE;
			}
		}
	}
	else
	{
		success = this->gdp->copy(*detail, GEO_CopyMethod::GEO_COPY_ONCE);
		if (!success)
		{
			this->addError(SOP_MESSAGE, "Geometry merge failure on GEO_COPY_ONCE");
			return ENUMS::MethodProcessResult::FAILURE;
		}
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

void
SOP_Operator::MergeAllInputDetails(UT_AutoInterrupt progress, UT_Array<const GU_Detail*>& details, ENUMS::ProcessedInputType processedinput, fpreal time)
{
	auto			success = false;
	exint			currIter = 0;
	exint			hullCountMainValue = 0;
	exint			bundleCountMainValue = 0;
	exint			hullShiftValue = 0;
	exint			bundleShiftValue = 0;
	GA_RWHandleI	currHullCountHandle;
	GA_RWHandleI	currBundleCountHandle;

	for (auto currDetail : details)
	{
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_RETURN(this, progress)

		// make it writable to allow attribute updates
		const auto currDetailRW = new GU_Detail(currDetail);
		
		// aggregate detail attributes
		if (this->_hullCountHandle.isValid() && processedinput == ENUMS::ProcessedInputType::CONVEX_HULLS)
		{
			success = ATTRIB_ACCESS::Find::IntATT(this, currDetailRW, GA_AttributeOwner::GA_ATTRIB_DETAIL, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_COUNT), currHullCountHandle);
			if (success) hullCountMainValue += currHullCountHandle.get(GA_Offset(0));
		}

		if (this->_bundleCountHandle.isValid())
		{
			success = ATTRIB_ACCESS::Find::IntATT(this, currDetailRW, GA_AttributeOwner::GA_ATTRIB_DETAIL, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_COUNT), currBundleCountHandle);
			if (success) bundleCountMainValue += currBundleCountHandle.get(GA_Offset(0));
		}			

		// shift primitive values to create continuous range from (last value of main detail + 1) to N
		ShiftCurrentDetailPrimitiveAttributes(currDetailRW, progress, currIter, hullShiftValue, bundleShiftValue, processedinput);
		
		// merge current detail into main detail	
		const auto processResult = MergeCurrentDetail(currDetailRW, currIter, details.size());
		if (processResult != ENUMS::MethodProcessResult::SUCCESS) return;
		
		// delete current detail as it's not needed anymore
		delete currDetailRW;

		// bump iterartion
		currIter++;
	}

	// set detail attributes
	if (this->_hullCountHandle.isValid() && processedinput == ENUMS::ProcessedInputType::CONVEX_HULLS)
	{
		success = ATTRIB_ACCESS::Find::IntATT(this, this->gdp, GA_AttributeOwner::GA_ATTRIB_DETAIL, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_COUNT), this->_hullCountHandle);
		if (success) this->_hullCountHandle.set(GA_Offset(0), hullCountMainValue);		
	}

	if (this->_bundleCountHandle.isValid())
	{
		success = ATTRIB_ACCESS::Find::IntATT(this, this->gdp, GA_AttributeOwner::GA_ATTRIB_DETAIL, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_COUNT), this->_bundleCountHandle);
		if (success) this->_bundleCountHandle.set(GA_Offset(0), bundleCountMainValue);
	}
}

/* -----------------------------------------------------------------
MAIN                                                               |
----------------------------------------------------------------- */

OP_ERROR 
SOP_Operator::cookMySop(OP_Context& context)
{
	DEFAULTS_CookMySop()
	
	// check which inputs have geometry connected	
	UT_Array<const GU_Detail*> allInputDetails;
	allInputDetails.clear();
		
	auto processResult = GetAllDetailsOfType(progress, allInputDetails, ENUMS::ProcessedInputType::CONVEX_HULLS, currentTime);
	if ((processResult == ENUMS::MethodProcessResult::SUCCESS && error() > OP_ERROR::UT_ERROR_WARNING) || (processResult != ENUMS::MethodProcessResult::SUCCESS && error() >= OP_ERROR::UT_ERROR_NONE)) return error();
	
	// check for attributes mismatch	
	HandleAttributesMismatch(progress, allInputDetails, ENUMS::ProcessedInputType::CONVEX_HULLS, currentTime);	
	if (error() > OP_ERROR::UT_ERROR_WARNING) return error();
		
	// does any input have V-HACD specific attributes?	
	processResult = AddFoundVHACDAttributes(progress, allInputDetails, ENUMS::ProcessedInputType::CONVEX_HULLS, currentTime);
	if ((processResult == ENUMS::MethodProcessResult::SUCCESS && error() > OP_ERROR::UT_ERROR_WARNING) || (processResult != ENUMS::MethodProcessResult::SUCCESS && error() > OP_ERROR::UT_ERROR_WARNING)) return error();
	
	// update and merge all input details
	MergeAllInputDetails( progress, allInputDetails, ENUMS::ProcessedInputType::CONVEX_HULLS, currentTime);
	
	return error();
}

GU_DetailHandle
SOP_Operator::cookMySopOutput(OP_Context& context, int outputidx, SOP_Node* interests)
{
	DEFAULTS_CookMySopOutput()	
	
	bool processModeChoiceMenuValue;
	PRM_ACCESS::Get::IntPRM(this, processModeChoiceMenuValue, UI::processModeChoiceMenu_Parameter, currentTime);
	
	switch(processModeChoiceMenuValue)
	{			
		default: return result;
		case static_cast<exint>(ENUMS::ProcessedModeOption::PAIRS):
		{				
			// check which inputs have geometry connected and mismatched attributes			
			UT_Array<const GU_Detail*> allInputDetails;
			allInputDetails.clear();

			auto processResult = GetAllDetailsOfType(progress, allInputDetails, ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY, currentTime);
			if ((processResult == ENUMS::MethodProcessResult::SUCCESS && error() > OP_ERROR::UT_ERROR_WARNING) || (processResult != ENUMS::MethodProcessResult::SUCCESS && error() >= OP_ERROR::UT_ERROR_NONE)) return result;

			// check for attribute mismatch	
			exint attributeMismatchErrorLevelValue;
			PRM_ACCESS::Get::IntPRM(this, attributeMismatchErrorLevelValue, UI::attributeMismatchErrorModeChoiceMenu_Parameter, currentTime);

			HandleAttributesMismatch(progress, allInputDetails, ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY, currentTime);
			if (error() > OP_ERROR::UT_ERROR_WARNING) return result;
			
			// does any input have V-HACD specific attributes?	
			processResult = AddFoundVHACDAttributes(progress, allInputDetails, ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY, currentTime);
			if ((processResult == ENUMS::MethodProcessResult::SUCCESS && error() > OP_ERROR::UT_ERROR_WARNING) || (processResult != ENUMS::MethodProcessResult::SUCCESS && error() > OP_ERROR::UT_ERROR_WARNING)) return result;

			// update and merge all input details
			MergeAllInputDetails(progress, allInputDetails, ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY, currentTime);
		}
		break;		
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
#undef UTILS

#undef UI
#undef COMMON_NAMES

#undef SOP_Base_Operator
#undef SOP_Operator