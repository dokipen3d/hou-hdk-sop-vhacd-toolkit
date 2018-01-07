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
#include <Utility/GA_AttributeAccessors.h>

// this
#include "SOP_VHACDNode.h"
#include "HullCenterData.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define UTILS					GET_Base_Namespace()::Utility
#define ATTRIB_ACCESS			UTILS::GA_AttributeAccessors

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

	ENUMS::MethodProcessResult  
	AddHullCountATT(const GEO_PrimClassifier& classifier)
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
	AddHullIDATT(UT_AutoInterrupt progress, const GEO_PrimClassifier& classifier)
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
			this->_hullIDHandle.set(*primIt, classifier.getClass(*primIt));
		}

		return ENUMS::MethodProcessResult::SUCCESS;
	}

	ENUMS::MethodProcessResult  
	GRPPerHull(UT_AutoInterrupt progress, const UT_String& partialhullgroupname, const GEO_PrimClassifier& classifier, fpreal time)
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

	ENUMS::MethodProcessResult	
	PointPerHullCenter(UT_AutoInterrupt progress, const GEO_PrimClassifier& classifier)
	{
		// find 'hull_center' attribute
		GA_RWHandleV3 hullCenterHandle;
		
		auto success = ATTRIB_ACCESS::Find::Vec3ATT(this, this->gdp, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_MASS_CENTER), hullCenterHandle, ENUMS::NodeErrorLevel::WARNING);
		if (!success) return ENUMS::MethodProcessResult::FAILURE;
		
		// find attributes that could be preserved		
		success = ATTRIB_ACCESS::Find::IntATT(this, this->gdp, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_ID), this->_hullIDHandle);		
		success = ATTRIB_ACCESS::Find::IntATT(this, this->gdp, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID), this->_bundleIDHandle);
		success = ATTRIB_ACCESS::Find::IntATT(this, this->gdp, GA_AttributeOwner::GA_ATTRIB_DETAIL, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_COUNT), this->_hullCountHandle);
		
		// get each unique 'hull_center'
		UT_Map<exint, CONTAINERS::HullCenterData> mappedCenters;
		mappedCenters.clear();

		for (auto offset : this->gdp->getPrimitiveRange())
		{
			PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::INTERRUPT)

			auto currClass = classifier.getClass(offset);
			if (!mappedCenters.contains(currClass))
			{
				// collect data
				if (this->_hullIDHandle.isValid() && this->_bundleIDHandle.isInvalid()) mappedCenters[currClass] = CONTAINERS::HullCenterData(hullCenterHandle.get(offset), this->_hullIDHandle.get(offset));
				else if (this->_hullIDHandle.isValid() && this->_bundleIDHandle.isValid()) mappedCenters[currClass] = CONTAINERS::HullCenterData(hullCenterHandle.get(offset), this->_hullIDHandle.get(offset), this->_bundleIDHandle.get(offset));
				else if (this->_hullIDHandle.isInvalid() && this->_bundleIDHandle.isValid()) mappedCenters[currClass] = CONTAINERS::HullCenterData(hullCenterHandle.get(offset), -1, this->_bundleIDHandle.get(offset));
				else mappedCenters[currClass] = CONTAINERS::HullCenterData(hullCenterHandle.get(offset));
			}
		}

		// add centers
		this->gdp->clear();			
		
		GA_RWHandleI hullIDPointHandle;
		GA_RWHandleI bundlIDPointHandle;

		if (this->_hullIDHandle.isValid()) hullIDPointHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_POINT, GA_AttributeScope::GA_SCOPE_PUBLIC, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_ID), 1));
		if (this->_bundleIDHandle.isValid()) bundlIDPointHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_POINT, GA_AttributeScope::GA_SCOPE_PUBLIC, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID), 1));

		for (auto map : mappedCenters)
		{
			PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::INTERRUPT)

			auto currOffset = this->gdp->appendPoint();
			this->gdp->setPos3(currOffset, map.second.GetCenter());
			
			// add preserved attributes
			auto currHullID = map.second.GetHullID();
			auto currBundleID = map.second.GetBundleID();

			if (currHullID > -1 && hullIDPointHandle.isValid()) hullIDPointHandle.set(currOffset, currHullID);
			if (currBundleID > -1 && bundlIDPointHandle.isValid()) bundlIDPointHandle.set(currOffset, currBundleID);
		}

		// reasign 'hull_count'
		if (success)
		{
			this->_hullCountHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_DETAIL, GA_AttributeScope::GA_SCOPE_PUBLIC, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_COUNT), 1));
			if (this->_hullCountHandle.isValid()) this->_hullCountHandle.set(GA_Offset(0), mappedCenters.size());
		}

		return ENUMS::MethodProcessResult::SUCCESS;
	}

	ENUMS::MethodProcessResult  
	ProcessHullSpecific(UT_AutoInterrupt progress, fpreal time)
	{
		auto processResult = ENUMS::MethodProcessResult::SUCCESS;

		if (this->_addHullCountAttributeValue || this->_addHullIDAttributeValue || this->_groupPerHullValue || this->_pointPerHullMassCenterValue)
		{
			// @CLASS of 1999 (for more info check http://www.imdb.com/title/tt0099277/)
			auto primClassifier = GEO_PrimClassifier();
			primClassifier.classifyBySharedPoints(*this->gdp);

			// generate data per toggle
			if (this->_addHullCountAttributeValue)
			{
				processResult = AddHullCountATT(primClassifier);
				if (processResult != ENUMS::MethodProcessResult::SUCCESS) return processResult;
			}

			if (this->_addHullIDAttributeValue)
			{
				processResult = AddHullIDATT(progress, primClassifier);
				if (processResult != ENUMS::MethodProcessResult::SUCCESS) return processResult;
			}

			if (this->_groupPerHullValue)
			{
				processResult = GRPPerHull(progress, this->_partialHullGroupNameValue, primClassifier, time);
				if (processResult != ENUMS::MethodProcessResult::SUCCESS) return processResult;
			}

			if (this->_pointPerHullMassCenterValue)
			{
				processResult = PointPerHullCenter(progress, primClassifier);
				if (processResult != ENUMS::MethodProcessResult::SUCCESS) return processResult;
			}
		}

		return processResult;
	}

	bool						_addHullCountAttributeValue = false;
	bool						_addHullIDAttributeValue = false;
	bool						_groupPerHullValue = false;	
	bool						_pointPerHullMassCenterValue = false;

	UT_String					_partialHullGroupNameValue;
};

DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS

#undef ATTRIB_ACCESS
#undef UTILS

#endif // !____sop_vhacd_scout_h____