/*
	Base node class for VHACD Scout nodes.

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
#ifndef ____sop_vhacd_scout_h____
#define ____sop_vhacd_scout_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// SESI
#include <UT/UT_Interrupt.h>
#include <GEO/GEO_PrimClassifier.h>

// hou-hdk-common
#include <Macros/Namespace.h>
#include <Macros/ProgressEscape.h>

// this
#include "SOP_VHACDNode.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define CONTAINERS				GET_Base_Namespace()::Containers
#define ENUMS					GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
DECLARATION                                                        |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

class SOP_VHACDScout : public SOP_VHACDNode
{	
protected:
	SOP_VHACDScout(OP_Network* network, const char* name, OP_Operator* op) : SOP_VHACDNode(network, name, op) { }

	ENUMS::MethodProcessResult  AddHullCountATT(const GEO_PrimClassifier& classifier)
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
	ENUMS::MethodProcessResult  AddHullIDATT(UT_AutoInterrupt progress, const GEO_PrimClassifier& classifier)
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
	ENUMS::MethodProcessResult  GRPPerHull(UT_AutoInterrupt progress, const UT_String& partialhullgroupname, const GEO_PrimClassifier& classifier, fpreal time)
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

			hullGroups[classifier.getClass(*primIt)]->addOffset(*primIt);
		}

		return ENUMS::MethodProcessResult::SUCCESS;
	}
	ENUMS::MethodProcessResult  ProcessHullSpecific(UT_AutoInterrupt progress, fpreal time)
	{
		auto success = ENUMS::MethodProcessResult::SUCCESS;

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
				success = GRPPerHull(progress, this->_partialHullGroupNameValue, primClassifier, time);
				if (success != ENUMS::MethodProcessResult::SUCCESS) return success;
			}
		}

		return success;
	}

	bool						_addHullCountAttributeValue = false;
	bool						_addHullIDAttributeValue = false;
	bool						_groupPerHullValue = false;

	UT_String					_partialHullGroupNameValue;
};

DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS

#endif // !____sop_vhacd_scout_h____