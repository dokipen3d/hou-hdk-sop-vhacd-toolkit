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
#include <GEO/GEO_Normal.h>

// hou-hdk-common
#include <Macros/ParameterList.h>
#include <Macros/ProgressEscape.h>
#include <Utility/PRM_TemplateAccessors.h>
#include <Utility/GA_AttributeAccessors.h>

// this
#include "Parameters.h"
#include "ProcessedInputType.h"
#include "VisibleInputOption.h"
#include "VisualizeAttributeOption.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator			GET_SOP_Namespace()::SOP_VHACDDebug
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
	UI::switchVisibleInputChoiceMenu_Parameter,

	UI::mainSectionSwitcher_Parameter,
	UI::visualizeAttributeChoiceMenu_Parameter,
	UI::explodeByHullIDAttributeToggle_Parameter,
	UI::explodeByHullIDAttributeSeparator_Parameter,
	UI::explodeByBundleIDAttributeToggle_Parameter,
	UI::explodeByBundleIDAttributeSeparator_Parameter,

	UI::additionalSectionSwitcher_Parameter,
	UI::cuspVertexNormalsToggle_Parameter,
	UI::cuspVertexNormalsSeparator_Parameter,
	UI::specifyCuspAngleFloat_Parameter,
	PARAMETERLIST_DescriptionPRM(UI),

PARAMETERLIST_End()

bool 
SOP_Operator::updateParmsFlags()
{
	DEFAULTS_UpdateParmsFlags(SOP_Base_Operator)
	
	const auto is1connected = this->getInput(static_cast<exint>(ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY)) == nullptr ? 0 : 1;

	// filter section
	activeState = is1connected;
	changed |= enableParm(UI::switchVisibleInputChoiceMenu_Parameter.getToken(), activeState);

	// main section
	exint switchVisibleInputMenyValue;
	PRM_ACCESS::Get::IntPRM(this, switchVisibleInputMenyValue, UI::switchVisibleInputChoiceMenu_Parameter, currentTime);

	activeState = is1connected && (switchVisibleInputMenyValue != static_cast<exint>(ENUMS::VisibleInputOption::CONVEX_HULLS))? 0 : 1;	
	changed |= setVisibleState(UI::explodeByHullIDAttributeToggle_Parameter.getToken(), activeState);
	changed |= setVisibleState(UI::explodeByHullIDAttributeSeparator_Parameter.getToken(), activeState);

	activeState = is1connected && (switchVisibleInputMenyValue != static_cast<exint>(ENUMS::VisibleInputOption::BOTH)) ? 1 : (!is1connected ? 1 : 0);
	changed |= enableParm(UI::visualizeAttributeChoiceMenu_Parameter.getToken(), activeState);
	
	exint explodeByHullIDValue;
	exint explodeByBundleIDValue;
	PRM_ACCESS::Get::IntPRM(this, explodeByHullIDValue, UI::explodeByHullIDAttributeToggle_Parameter, currentTime);
	PRM_ACCESS::Get::IntPRM(this, explodeByBundleIDValue, UI::explodeByBundleIDAttributeToggle_Parameter, currentTime);

	activeState = explodeByBundleIDValue ? 0 : 1;
	changed |= enableParm(UI::explodeByHullIDAttributeToggle_Parameter.getToken(), activeState);
	activeState = explodeByHullIDValue ? 0 : 1;
	changed |= enableParm(UI::explodeByBundleIDAttributeToggle_Parameter.getToken(), activeState);

	// additional section
	bool cuspVertexNormalsValue;
	PRM_ACCESS::Get::IntPRM(this, cuspVertexNormalsValue, UI::cuspVertexNormalsToggle_Parameter, currentTime);	
	changed |= setVisibleState(UI::specifyCuspAngleFloat_Parameter.getToken(), cuspVertexNormalsValue);

	// update description active state
	UPDATE_DescriptionPRM_ActiveState(this, UI)

	return changed;
}

/* -----------------------------------------------------------------
CALLBACKS                                                          |
----------------------------------------------------------------- */

IMPLEMENT_DescriptionPRM_Callback(SOP_Operator, UI)

int
SOP_Operator::CallbackSwitchVisibleInput(void* data, int index, float time, const PRM_Template* tmp)
{
	const auto me = reinterpret_cast<SOP_Operator*>(data);
	if (!me) return 0;

	// // set vertex normals option	
	exint newValue = static_cast<exint>(UI::cuspVertexNormalsToggle_Parameter.getFactoryDefaults()->getOrdinal());

	exint switchVisibleInputValue;
	PRM_ACCESS::Get::IntPRM(me, switchVisibleInputValue, UI::switchVisibleInputChoiceMenu_Parameter, time);

	switch (switchVisibleInputValue)
	{
		case static_cast<exint>(ENUMS::VisibleInputOption::CONVEX_HULLS) :
		{
			newValue = 1;
			auto cuspAngleValue = 0.0;			
			PRM_ACCESS::Set::IntPRM(me, newValue, UI::cuspVertexNormalsToggle_Parameter, time);
			PRM_ACCESS::Set::FloatPRM(me, cuspAngleValue, UI::specifyCuspAngleFloat_Parameter, time);
		} break;

		case static_cast<exint>(ENUMS::VisibleInputOption::ORIGINAL_GEOMETRY) :
		{
			newValue = 0;
			PRM_ACCESS::Set::IntPRM(me, newValue, UI::cuspVertexNormalsToggle_Parameter, time);			
			PRM_ACCESS::Set::IntPRM(me, newValue, UI::visualizeAttributeChoiceMenu_Parameter, time);
			PRM_ACCESS::Set::IntPRM(me, newValue, UI::explodeByHullIDAttributeToggle_Parameter, time);
			CallbackCuspVertexNormal(data, index, time, tmp);			
		} break;

		case static_cast<exint>(ENUMS::VisibleInputOption::BOTH) :
		{
			newValue = 0;
			PRM_ACCESS::Set::IntPRM(me, newValue, UI::cuspVertexNormalsToggle_Parameter, time);
			PRM_ACCESS::Set::IntPRM(me, newValue, UI::visualizeAttributeChoiceMenu_Parameter, time);
			PRM_ACCESS::Set::IntPRM(me, newValue, UI::explodeByHullIDAttributeToggle_Parameter, time);		
			CallbackCuspVertexNormal(data, index, time, tmp);
		} break;
	}

	return 1;
}

int
SOP_Operator::CallbackCuspVertexNormal(void* data, int index, float time, const PRM_Template* tmp)
{
	const auto me = reinterpret_cast<SOP_Operator*>(data);
	if (!me) return 0;

	// TODO: figure out why restoreFactoryDefaults() doesn't work
	auto cuspAngleValue = static_cast<fpreal>(UI::specifyCuspAngleFloat_Parameter.getFactoryDefaults()->getFloat());
	PRM_ACCESS::Set::FloatPRM(me, cuspAngleValue, UI::specifyCuspAngleFloat_Parameter, time);

	return 1;
}

void
SOP_Operator::CallbakcVisualizeAttributeMenu(void* data, PRM_Name* choicenames, int listsize, const PRM_SpareData* spare, const PRM_Parm* parm)
{	
	const auto me = reinterpret_cast<SOP_Operator*>(data);	
	if (me)
	{
		exint switchVisibleInputMenuValue;
		PRM_ACCESS::Get::IntPRM(me, switchVisibleInputMenuValue, UI::switchVisibleInputChoiceMenu_Parameter);

		switch (switchVisibleInputMenuValue)
		{
			case static_cast<exint>(ENUMS::VisibleInputOption::CONVEX_HULLS) :
			{
				choicenames[0].setTokenAndLabel("0", "None");
				choicenames[1].setTokenAndLabel("1", "Hull ID");
				choicenames[2].setTokenAndLabel("2", "Hull Volume");
				choicenames[3].setTokenAndLabel("3", "Bundle ID");
				choicenames[4].setTokenAndLabel(nullptr, nullptr);
			} break;

			case static_cast<exint>(ENUMS::VisibleInputOption::ORIGINAL_GEOMETRY) :
			{
				choicenames[0].setTokenAndLabel("0", "None");				
				choicenames[1].setTokenAndLabel("1", "Bundle ID");
				choicenames[2].setTokenAndLabel(nullptr, nullptr);
			} break;

			case static_cast<exint>(ENUMS::VisibleInputOption::BOTH) :
			{
				choicenames[0].setTokenAndLabel("0", "None");				
				choicenames[1].setTokenAndLabel(nullptr, nullptr);
			} break;
		}
	}
	else
	{
		choicenames[0].setToken(nullptr);
		choicenames[0].setLabel(nullptr);
	}
}

/* -----------------------------------------------------------------
OPERATOR INITIALIZATION                                            |
----------------------------------------------------------------- */

SOP_Operator::~SOP_VHACDDebug() { }

SOP_Operator::SOP_VHACDDebug(OP_Network* network, const char* name, OP_Operator* op) : SOP_Base_Operator(network, name, op) { }

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

void
SOP_Operator::inputConnectChanged(int which)
{
	if (which == static_cast<exint>(ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY))
	{
		// reset choice menu
		exint switchVisibleInputValue;
		PRM_ACCESS::Get::IntPRM(this, switchVisibleInputValue, UI::switchVisibleInputChoiceMenu_Parameter);

		if (switchVisibleInputValue == static_cast<exint>(ENUMS::VisibleInputOption::ORIGINAL_GEOMETRY) || switchVisibleInputValue == static_cast<exint>(ENUMS::VisibleInputOption::BOTH))
		{
			//exint defVal = static_cast<exint>(UI::switchVisibleInputChoiceMenu_Parameter.getFactoryDefaults()->getOrdinal());
			exint defVal = 0;			
			PRM_ACCESS::Set::IntPRM(this, defVal, UI::switchVisibleInputChoiceMenu_Parameter);			
			PRM_ACCESS::Set::IntPRM(this, defVal, UI::visualizeAttributeChoiceMenu_Parameter);
			
			exint cuspVertexNormalValue = 1;
			auto cuspAngleValue = 0.0;
			PRM_ACCESS::Set::IntPRM(this, cuspVertexNormalValue, UI::cuspVertexNormalsToggle_Parameter);
			PRM_ACCESS::Set::FloatPRM(this, cuspAngleValue, UI::specifyCuspAngleFloat_Parameter);
		}
	}
}

/* -----------------------------------------------------------------
HELPERS                                                            |
----------------------------------------------------------------- */

ENUMS::MethodProcessResult
SOP_Operator::CuspConvexInputVertexNormals(GU_Detail* detail, fpreal time)
{
	exint switchVisibleInputValue;
	PRM_ACCESS::Get::IntPRM(this, switchVisibleInputValue, UI::switchVisibleInputChoiceMenu_Parameter, time);

	if (switchVisibleInputValue > 0) return ENUMS::MethodProcessResult::SUCCESS;

	// do we want to cusp normals of convex hulls?
	bool cuspVertexNormalsValue;
	PRM_ACCESS::Get::IntPRM(this, cuspVertexNormalsValue, UI::cuspVertexNormalsToggle_Parameter, time);
	if (cuspVertexNormalsValue)
	{
		fpreal specifyCuspAngleValue;
		PRM_ACCESS::Get::FloatPRM(this, specifyCuspAngleValue, UI::specifyCuspAngleFloat_Parameter, time);

		const auto vertexNormalHandle = GA_RWHandleV3(detail->addFloatTuple(GA_AttributeOwner::GA_ATTRIB_VERTEX, "N", 3));
		if (vertexNormalHandle.isInvalid())
		{
			this->addError(SOP_MESSAGE, "Failed to add vertex normal attribute.");
			return ENUMS::MethodProcessResult::FAILURE;
		}
			
		GEOcomputeNormals(*detail, vertexNormalHandle, nullptr, specifyCuspAngleValue);
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::PrepareIntATTForGUI(UT_AutoInterrupt& progress, ENUMS::VHACDCommonAttributeNameOption attributename, GA_RWHandleI& attributehandle)
{
	const auto success = ATTRIB_ACCESS::Find::IntATT(this, this->gdp, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(attributename), attributehandle);
	if (!success)
	{
		addWarning(SOP_ATTRIBUTE_INVALID);
		return ENUMS::MethodProcessResult::FAILURE;
	}
	
	auto pointAttributeHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_POINT, attributehandle->getScope(), this->_commonAttributeNames.Get(attributename), 1));
	if (pointAttributeHandle.isValid())
	{
		for (auto primIt : this->gdp->getPrimitiveRange())
		{
			PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::INTERRUPT)

			const auto currPrim = this->gdp->getPrimitive(primIt);
			if (currPrim)
			{
				const auto currVal = attributehandle.get(primIt);				
				for (auto pointIt : currPrim->getPointRange()) pointAttributeHandle.set(pointIt, currVal);
			}
		}
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::PrepareFloatATTForGUI(UT_AutoInterrupt& progress, ENUMS::VHACDCommonAttributeNameOption attributename, GA_RWHandleD& attributehandle)
{
	const auto success = ATTRIB_ACCESS::Find::FloatATT(this, this->gdp, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(attributename), attributehandle);
	if (!success)
	{
		addWarning(SOP_ATTRIBUTE_INVALID);
		return ENUMS::MethodProcessResult::FAILURE;
	}

	auto pointAttributeHandle = GA_RWHandleD(this->gdp->addFloatTuple(GA_AttributeOwner::GA_ATTRIB_POINT, attributehandle->getScope(), this->_commonAttributeNames.Get(attributename), 1));
	if (pointAttributeHandle.isValid())
	{
		for (auto primIt : this->gdp->getPrimitiveRange())
		{
			PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::INTERRUPT)

			const auto currPrim = this->gdp->getPrimitive(primIt);
			if (currPrim)
			{
				const auto currVal = attributehandle.get(primIt);
				for (auto pointIt : currPrim->getPointRange()) pointAttributeHandle.set(pointIt, currVal);
			}
		}
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::WhenConvexHullsInput(OP_Context& context, UT_AutoInterrupt& progress, fpreal time)
{
	if (duplicateSource(static_cast<exint>(ENUMS::ProcessedInputType::CONVEX_HULLS), context) <= UT_ErrorSeverity::UT_ERROR_WARNING && error() <= UT_ErrorSeverity::UT_ERROR_WARNING)
	{		
		// cusp vertex normals
		auto processResult = CuspConvexInputVertexNormals(this->gdp, time);
		if (processResult != ENUMS::MethodProcessResult::SUCCESS) return processResult;

		// visualize attributes
		exint visualizeAttributeChoiceMenuValue;
		PRM_ACCESS::Get::IntPRM(this, visualizeAttributeChoiceMenuValue, UI::visualizeAttributeChoiceMenu_Parameter, time);

		switch (visualizeAttributeChoiceMenuValue)
		{			
			default: /* do nothing */ break;			
			case static_cast<exint>(ENUMS::ConvexVisualizeAttributeOption::HULL_ID) :		{ processResult = PrepareIntATTForGUI(progress, ENUMS::VHACDCommonAttributeNameOption::HULL_ID, this->_hullIDHandle); } break;
			case static_cast<exint>(ENUMS::ConvexVisualizeAttributeOption::HULL_VOLUME) :	{ processResult = PrepareFloatATTForGUI(progress, ENUMS::VHACDCommonAttributeNameOption::HULL_VOLUME, this->_hullVolumeHandle); } break;
			case static_cast<exint>(ENUMS::ConvexVisualizeAttributeOption::BUNDLE_ID) :		{ processResult = PrepareIntATTForGUI(progress, ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID, this->_bundleIDHandle); } break;
		}

		// explode by 'hull_id'
		// explode by 'bundle_id'
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::WhenOriginalGeometryInput(OP_Context& context, UT_AutoInterrupt& progress, fpreal time)
{
	if (duplicateSource(static_cast<exint>(ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY), context) <= UT_ErrorSeverity::UT_ERROR_WARNING && error() <= UT_ErrorSeverity::UT_ERROR_WARNING)
	{
		auto processResult = ENUMS::MethodProcessResult::SUCCESS;

		// visualize attribute
		exint visualizeAttributeChoiceMenuValue;
		PRM_ACCESS::Get::IntPRM(this, visualizeAttributeChoiceMenuValue, UI::visualizeAttributeChoiceMenu_Parameter, time);

		switch (visualizeAttributeChoiceMenuValue)
		{
			default: /* do nothing */ break;
			case static_cast<exint>(ENUMS::OriginalVisualizeAttributeOption::BUNDLE_ID) :	{ processResult = PrepareIntATTForGUI(progress, ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID, this->_bundleIDHandle); } break;
		}				

		// explode by 'bundle_id'
	}		

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::WhenBothInputs(OP_Context& context, fpreal time)
{
	const auto is1connected = this->getInput(static_cast<exint>(ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY)) == nullptr ? false : true;
	if (!is1connected)
	{
		addError(SOP_MESSAGE, "Both inputs have to be connected in this visibility mode.");
		return ENUMS::MethodProcessResult::FAILURE;
	}
	
	const auto convexGDP = new GU_Detail(inputGeo(static_cast<exint>(ENUMS::ProcessedInputType::CONVEX_HULLS), context));
	const auto originalGDP = new GU_Detail(inputGeo(static_cast<exint>(ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY), context));	
	if (convexGDP && originalGDP && error() <= UT_ErrorSeverity::UT_ERROR_WARNING)
	{
		auto success = this->gdp->copy(*convexGDP, GEO_CopyMethod::GEO_COPY_START);
		if (!success)
		{
			addError(SOP_MESSAGE, "Failed to merge convex hulls into current detail.");
			return ENUMS::MethodProcessResult::FAILURE;
		}

		success = this->gdp->copy(*originalGDP, GEO_CopyMethod::GEO_COPY_END);
		if (!success)
		{
			addError(SOP_MESSAGE, "Failed to merge original geometry into current detail.");
			return ENUMS::MethodProcessResult::FAILURE;
		}

		delete convexGDP;
		delete originalGDP;

		//auto processResult = CuspConvexInputVertexNormals(convexGDP, time);
		//if (processResult != ENUMS::MethodProcessResult::SUCCESS) return processResult;

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
		
	exint switchVisibleInputValue;
	PRM_ACCESS::Get::IntPRM(this, switchVisibleInputValue, UI::switchVisibleInputChoiceMenu_Parameter, currentTime);

	auto processResult = ENUMS::MethodProcessResult::SUCCESS;

	switch (switchVisibleInputValue)
	{
		default: /* do nothing */ break;
		case static_cast<exint>(ENUMS::VisibleInputOption::CONVEX_HULLS) :			{ processResult = WhenConvexHullsInput(context, progress, currentTime); if (processResult != ENUMS::MethodProcessResult::SUCCESS) return error(); } break;
		case static_cast<exint>(ENUMS::VisibleInputOption::ORIGINAL_GEOMETRY) :		{ processResult = WhenOriginalGeometryInput(context, progress, currentTime); if (processResult != ENUMS::MethodProcessResult::SUCCESS) return error(); } break;
		case static_cast<exint>(ENUMS::VisibleInputOption::BOTH) :					{ processResult = WhenBothInputs(context, currentTime); if (processResult != ENUMS::MethodProcessResult::SUCCESS) return error(); } break;
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