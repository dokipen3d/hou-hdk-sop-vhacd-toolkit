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
#include <Utility/ParameterAccessing.h>
#include <Utility/AttributeAccessing.h>

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
#define PRM_ACCESS				GET_Base_Namespace()::Utility::PRM
#define ATTRIB_ACCESS			GET_Base_Namespace()::Utility::Attribute
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

	UI::mainSectionSwitcher_Parameter,	
	UI::orderModeChoiceMenu_Parameter,

	UI::additionalSectionSwitcher_Parameter,
	PARAMETERLIST_DescriptionPRM(UI),

PARAMETERLIST_End()

bool 
SOP_Operator::updateParmsFlags()
{
	DEFAULTS_UpdateParmsFlags(SOP_Base_Operator)

	// is input connected?
	const exint is0Connected = getInput(0) == nullptr ? 0 : 1;
	const exint is1Connected = getInput(1) == nullptr ? 0 : 1;
	const exint is2Connected = getInput(2) == nullptr ? 0 : 1;
	const exint is3Connected = getInput(3) == nullptr ? 0 : 1;
	const exint is4Connected = getInput(4) == nullptr ? 0 : 1;
	const exint is5Connected = getInput(5) == nullptr ? 0 : 1;
	const exint is6Connected = getInput(6) == nullptr ? 0 : 1;
	const exint is7Connected = getInput(7) == nullptr ? 0 : 1;
	const exint is8Connected = getInput(8) == nullptr ? 0 : 1;
	const exint is9Connected = getInput(9) == nullptr ? 0 : 1;

	/* ---------------------------- Set Global Visibility ---------------------------- */

	visibilityState = is0Connected ? 1 : 0; // TODO: do I still need this?

	/* ---------------------------- Set States --------------------------------------- */

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
{ op->setIconName(COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_MERGE_ICONNAME_V2)); }

OP_Node* 
SOP_Operator::CreateMe(OP_Network* network, const char* name, OP_Operator* op) 
{ return new SOP_Operator(network, name, op); }

const char* 
SOP_Operator::inputLabel(unsigned input) const
{ return std::to_string(input).c_str(); }

/* -----------------------------------------------------------------
HELPERS                                                            |
----------------------------------------------------------------- */

bool
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
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_BOOL(this, progress, false)

		const auto currDetail = inputGeo(inputNumber);
		if (currDetail) details.append(currDetail);
		else
		{
			this->addError(SOP_MESSAGE, "Found NULL input.");
			return false;
		}
	}

	return true;
}

bool
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
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_BOOL(this, progress, false)

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
		if (this->_createHullCount) this->_hullCountHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_DETAIL, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_COUNT), 1));
		if (this->_createHullID) this->_hullIDHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_ID), 1));
	}

	if (this->_createBundleCount) this->_bundleCountHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_DETAIL, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_COUNT), 1));
	if (this->_createBundleID) this->_bundleIDHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID), 1));

	return true;
}

bool 
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
				return success;
			}
			
		}
		else if (iteration == detailscount - 1)
		{
			success = this->gdp->copy(*detail, GEO_CopyMethod::GEO_COPY_END);
			if (!success)
			{
				this->addError(SOP_MESSAGE, "Geometry merge failure on GEO_COPY_END");
				return success;
			}			
		}
		else
		{
			success = this->gdp->copy(*detail, GEO_CopyMethod::GEO_COPY_ADD);
			if (!success)
			{
				this->addError(SOP_MESSAGE, "Geometry merge failure on GEO_COPY_ADD");
				return success;
			}
		}
	}
	else
	{
		success = this->gdp->copy(*detail, GEO_CopyMethod::GEO_COPY_ONCE);
		if (!success)
		{
			this->addError(SOP_MESSAGE, "Geometry merge failure on GEO_COPY_ONCE");
			return success;
		}
	}

	return success;
}

void
SOP_Operator::MergeEachInput(UT_AutoInterrupt progress, UT_Array<const GU_Detail*>& details, ENUMS::ProcessedInputType processedinput, fpreal time)
{	
#define THIS_GET_DEATIL_VALUE(mainvalue, handle, maindetail, currentdetail, attributename, errormessage) { mainvalue = maindetail->getPrimitiveRange().getEntries() > 0 ? handle.get(GA_Offset(0)) : 0; auto currHandle = GA_ROHandleI(currentdetail->findIntTuple(GA_AttributeOwner::GA_ATTRIB_DETAIL, GA_AttributeScope::GA_SCOPE_PUBLIC, attributename)); if (currHandle.isValid()) mainvalue += currHandle.get(GA_Offset(0)); else this->addWarning(SOP_MESSAGE, errormessage); }
#define THIS_SET_DETAIL_VALUE(maindetail, attributename, handle, mainvalue) success = ATTRIB_ACCESS::Find::IntATT(this, maindetail, GA_AttributeOwner::GA_ATTRIB_DETAIL, attributename, handle); if (success) handle.set(GA_Offset(0), mainvalue);

	exint	hullCountMainValue = -1;
	exint	hullIDMainValue = -1;
	exint	bundleCountMainValue = -1;
	exint	bundleDMainValue = -1;
	exint	currIter = 0;

	for (auto currDetail : details)
	{
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_RETURN(this, progress)

		// only if we need to create attribute
		if (this->_hullCountHandle.isValid() && processedinput == ENUMS::ProcessedInputType::CONVEX_HULLS) THIS_GET_DEATIL_VALUE(hullCountMainValue, this->_hullCountHandle, this->gdp, currDetail, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_COUNT), "Hull Count attribute was not present on one or more inputs. Merged value for this attribute will be inproper.")
		if (this->_hullIDHandle.isValid() && processedinput == ENUMS::ProcessedInputType::CONVEX_HULLS)
		{
			// sort attributes
			// shift attributes
		}
		
		if (this->_bundleCountHandle.isValid()) THIS_GET_DEATIL_VALUE(bundleCountMainValue, this->_bundleCountHandle, this->gdp, currDetail, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_COUNT), "Bundle Count attribute was not present on one or more inputs. Merged value for this attribute will be inproper.")
		if (this->_bundleIDHandle.isValid())
		{
			// sort attributes
			// shift attributes			
		}
		
		// merge current detail into main detail	
		auto success = MergeCurrentDetail(currDetail, currIter, details.size());
		if (!success) return;

		// re-assign (otherwise it crashes) handles to main detail and update attribute values
		if (processedinput == ENUMS::ProcessedInputType::CONVEX_HULLS)
		{				
			THIS_SET_DETAIL_VALUE(this->gdp, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_COUNT), this->_hullCountHandle, hullCountMainValue)

			//success = ATTRIB_ACCESS::Find::IntATT(this, this->gdp, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_ID), this->_hullIDHandle);
			//if (success)	;
		}
			
		THIS_SET_DETAIL_VALUE(this->gdp, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_COUNT), this->_bundleCountHandle, bundleCountMainValue)
		
		// bump iterartion
		currIter++;
	}

#undef THIS_SET_DETAIL_VALUE
#undef THIS_GET_DEATIL_VALUE	
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
		
	auto success = GetAllDetailsOfType(progress, allInputDetails, ENUMS::ProcessedInputType::CONVEX_HULLS, currentTime);
	if ((success && error() > OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return error();
	
	// check for attributes mismatch	
	exint attributeMismatchErrorLevelValue;
	PRM_ACCESS::Get::IntPRM(this, attributeMismatchErrorLevelValue, UI::attributeMismatchErrorModeChoiceMenu_Parameter, currentTime);

	success = ATTRIB_ACCESS::Check::MismatchOfAllOwners(this, progress, allInputDetails, GA_AttributeScope::GA_SCOPE_PUBLIC, static_cast<ENUMS::NodeErrorLevel>(attributeMismatchErrorLevelValue));
	if ((success && error() > OP_ERROR::UT_ERROR_WARNING) || (!success && error() > OP_ERROR::UT_ERROR_WARNING)) return error();
		
	// does any input have V-HACD specific attributes?	
	success = AddFoundVHACDAttributes(progress, allInputDetails, ENUMS::ProcessedInputType::CONVEX_HULLS, currentTime);
	if ((success && error() > OP_ERROR::UT_ERROR_WARNING) || (!success && error() > OP_ERROR::UT_ERROR_WARNING)) return error();
	
	// update and merge all input details
	MergeEachInput( progress, allInputDetails, ENUMS::ProcessedInputType::CONVEX_HULLS, currentTime);		

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

			auto success = GetAllDetailsOfType(progress, allInputDetails, ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY, currentTime);
			if ((success && error() > OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return result;

			// check for attribute mismatch	
			exint attributeMismatchErrorLevelValue;
			PRM_ACCESS::Get::IntPRM(this, attributeMismatchErrorLevelValue, UI::attributeMismatchErrorModeChoiceMenu_Parameter, currentTime);

			success = ATTRIB_ACCESS::Check::MismatchOfAllOwners(this, progress, allInputDetails, GA_AttributeScope::GA_SCOPE_PUBLIC, static_cast<ENUMS::NodeErrorLevel>(attributeMismatchErrorLevelValue));				
			if ((success && error() > OP_ERROR::UT_ERROR_WARNING) || (!success && error() > OP_ERROR::UT_ERROR_WARNING)) return result;
			
			// does any input have V-HACD specific attributes?	
			success = AddFoundVHACDAttributes(progress, allInputDetails, ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY, currentTime);
			if ((success && error() > OP_ERROR::UT_ERROR_WARNING) || (!success && error() > OP_ERROR::UT_ERROR_WARNING)) return result;
						
			// update and merge all input details
			MergeEachInput(progress, allInputDetails, ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY, currentTime);			
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
#undef UI

#undef COMMON_NAMES
#undef SOP_Base_Operator
#undef SOP_Operator