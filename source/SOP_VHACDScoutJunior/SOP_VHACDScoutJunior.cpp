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

#if _WIN32		
#include <sys/SYS_Math.h>
#else
#include <SYS/SYS_Math.h>
#endif

// hou-hdk-common
#include <Macros/ParameterList.h>
#include <Enums/NodeErrorLevel.h>
#include <Utility/PRM_TemplateAccessors.h>
#include <Utility/GA_AttributeAccessors.h>
#include <Utility/GU_DetailCalculator.h>

// this
#include "Parameters.h"
#include "ProcessedInputType.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator			GET_SOP_Namespace()::SOP_VHACDScoutJunior
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

	UI::mainSectionSwitcher_Parameter,
	UI::addHullCountAttributeToggle_Parameter,
	UI::addHullCountAttributeSeparator_Parameter,
	UI::addHullIDAttributeToggle_Parameter,
	UI::addHullIDAttributeSeparator_Parameter,
	UI::addHullVolumeAttributeToggle_Parameter,
	UI::addHullVolumeAttributeSeparator_Parameter,
	UI::groupPerHullToggle_Parameter,
	UI::groupPerHullSeparator_Parameter,
	UI::specifyHullGroupNameString_Parameter,

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

	return 1;
}

/* -----------------------------------------------------------------
OPERATOR INITIALIZATION                                            |
----------------------------------------------------------------- */

SOP_Operator::~SOP_VHACDScoutJunior() { }

SOP_Operator::SOP_VHACDScoutJunior(OP_Network* network, const char* name, OP_Operator* op)
: SOP_Base_Operator(network, name, op)
{ }

OP_Node* 
SOP_Operator::CreateMe(OP_Network* network, const char* name, OP_Operator* op) 
{ return new SOP_Operator(network, name, op); }

const char*
SOP_Operator::inputLabel(unsigned input) const
{ return std::to_string(input).c_str(); }

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

	this->_hullCountHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_DETAIL, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_COUNT), 1, GA_Defaults(classifier.getNumClass())));
	if (this->_hullCountHandle.isInvalid())
	{
		auto errorMessage = std::string("Failed to create ") + std::string(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_COUNT) + std::string(" attribute."));
		this->addError(SOP_MESSAGE, errorMessage.c_str());
		return ENUMS::MethodProcessResult::FAILURE;
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::AddHullIDATT(UT_AutoInterrupt progress, const GEO_PrimClassifier& classifier)
{
	this->_hullIDHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_ID), 1));
	if (this->_hullIDHandle.isInvalid())
	{
		auto errorMessage = std::string("Failed to create ") + std::string(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_ID) + std::string(" attribute."));
		this->addError(SOP_MESSAGE, errorMessage.c_str());
		return ENUMS::MethodProcessResult::FAILURE;
	}

	for (auto primIt = GA_Iterator(this->gdp->getPrimitiveRange()); !primIt.atEnd(); primIt.advance())
	{
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::INTERRUPT)
		this->_hullIDHandle.set(*primIt, classifier.getClass(primIt.getIndex()));
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::AddHullVolumeATT(UT_AutoInterrupt progress, const GEO_PrimClassifier& classifier)
{
	// pack primitives into groups
	UT_Map<exint, GA_PrimitiveGroup*> mappedData;
	mappedData.clear();
	
	auto gop = GOP_Manager::GroupCreator(this->gdp);

	for (auto primIt = GA_Iterator(this->gdp->getPrimitiveRange()); !primIt.atEnd(); primIt.advance())
	{
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::INTERRUPT)

		const auto currClass = classifier.getClass(primIt.getIndex());

		if (!mappedData.contains(currClass)) mappedData[currClass] = static_cast<GA_PrimitiveGroup*>(gop.createGroup(GA_GroupType::GA_GROUP_PRIMITIVE));
		mappedData[currClass]->addOffset(*primIt);
	}

	// add hull_volume attribute	
	this->_hullVolumeHandle = GA_RWHandleD(this->gdp->addFloatTuple(GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_VOLUME), 1));
	if (this->_hullVolumeHandle.isInvalid())
	{
		auto errorMessage = std::string("Failed to create ") + std::string(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_VOLUME) + std::string(" attribute."));
		this->addError(SOP_MESSAGE, errorMessage.c_str());
		return ENUMS::MethodProcessResult::FAILURE;
	}
	
	// calculate and update hull_volume attribute
	for (const auto map : mappedData)
	{		
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::INTERRUPT)

		fpreal64 hullVolume;		
		const auto processResult = UTILS::GU_DetailCalculator::TriangleMeshSignedVolume(this, progress, this->gdp, map.second, hullVolume, ENUMS::NodeErrorLevel::ERROR);
		if (processResult != ENUMS::MethodProcessResult::SUCCESS) return processResult;

		// update attribute
		if (this->_hullVolumeHandle.isValid())
		{
			for (auto offset : this->gdp->getPrimitiveRange(map.second)) this->_hullVolumeHandle.set(offset, SYSabs(hullVolume));
		}
	}
	
	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::GRPPerHull(UT_AutoInterrupt progress, const UT_String& partialhullgroupname, const GEO_PrimClassifier& classifier, fpreal time)
{
	// create groups
	if (partialhullgroupname.length() < 1)
	{
		addError(SOP_MESSAGE, "Specified hull group name is invalid.");
		return ENUMS::MethodProcessResult::FAILURE;
	}

	UT_Map<exint, GA_PrimitiveGroup*> hullGroups;
	hullGroups.clear();
	for (auto classID = 0; classID < classifier.getNumClass(); classID++)
	{
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::INTERRUPT)
		hullGroups[classID] = this->gdp->newPrimitiveGroup(partialhullgroupname.c_str() + std::to_string(classID));
	}

	// fill groups
	for (auto primIt = GA_Iterator(this->gdp->getPrimitiveRange()); !primIt.atEnd(); primIt.advance())
	{
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::INTERRUPT)
		hullGroups[classifier.getClass(primIt.getIndex())]->addOffset(*primIt);
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

/* -----------------------------------------------------------------
MAIN                                                               |
----------------------------------------------------------------- */

OP_ERROR 
SOP_Operator::cookMySop(OP_Context& context)
{	
	DEFAULTS_CookMySop()		
	
	if (duplicateSource(static_cast<exint>(ENUMS::ProcessedInputType::CONVEX_HULLS), context, this->gdp) <= OP_ERROR::UT_ERROR_WARNING && error() <= OP_ERROR::UT_ERROR_WARNING)
	{
		auto		addHullCountAttributeValue = false;
		auto		addHullIDAttributeValue = false;
		auto		addHullVolumeAttributeValue = false;
		auto		groupPerHullValue = false;	

		PRM_ACCESS::Get::IntPRM(this, addHullCountAttributeValue, UI::addHullCountAttributeToggle_Parameter, currentTime);
		PRM_ACCESS::Get::IntPRM(this, addHullIDAttributeValue, UI::addHullIDAttributeToggle_Parameter, currentTime);
		PRM_ACCESS::Get::IntPRM(this, addHullVolumeAttributeValue, UI::addHullVolumeAttributeToggle_Parameter, currentTime);
		PRM_ACCESS::Get::IntPRM(this, groupPerHullValue, UI::groupPerHullToggle_Parameter, currentTime);		

		if (addHullCountAttributeValue || addHullIDAttributeValue || addHullVolumeAttributeValue || groupPerHullValue)
		{
			// @CLASS of 1999 (for more info check http://www.imdb.com/title/tt0099277/)
			auto primClassifier = GEO_PrimClassifier();
			primClassifier.classifyBySharedPoints(*this->gdp);

			// generate data per toggle
			auto processResult = ENUMS::MethodProcessResult::SUCCESS;

			if (addHullCountAttributeValue)
			{
				processResult = AddHullCountATT(primClassifier);
				if (processResult != ENUMS::MethodProcessResult::SUCCESS) return error();
			}

			if (addHullIDAttributeValue)
			{
				processResult = AddHullIDATT(progress, primClassifier);
				if (processResult != ENUMS::MethodProcessResult::SUCCESS) return error();
			}

			if (addHullVolumeAttributeValue)
			{
				processResult = AddHullVolumeATT(progress, primClassifier);
				if (processResult != ENUMS::MethodProcessResult::SUCCESS) return error();
			}

			if (groupPerHullValue)
			{
				UT_String	partialHullGroupNameValue;
				PRM_ACCESS::Get::StringPRM(this, partialHullGroupNameValue, UI::specifyHullGroupNameString_Parameter, currentTime);

				processResult = GRPPerHull(progress, partialHullGroupNameValue, primClassifier, currentTime);
				if (processResult != ENUMS::MethodProcessResult::SUCCESS) return error();
			}
		}
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
#undef UTILS

#undef UI
#undef COMMON_NAMES

#undef SOP_Base_Operator
#undef SOP_Operator