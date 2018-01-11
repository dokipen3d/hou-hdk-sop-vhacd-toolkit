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
//#include <Utility/GU_DetailCalculator.h>

// this
#include "Parameters.h"
#include "ProcessedInputType.h"
//#include "FilterModeOption.h"
#include "VisibleInputOption.h"

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
	UI::showHullIDAttributeToggle_Parameter,
	UI::showHullIDAttributeSeparator_Parameter,
	UI::showBundleIDAttributeToggle_Parameter,
	UI::showBundleIDAttributeSeparator_Parameter,
	UI::showHullVolumeAttributeToggle_Parameter,
	UI::showHullVolumeAttributeSeparator_Parameter,
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

	/* ---------------------------- Set States --------------------------------------- */

	const exint is1Connected = this->getInput(static_cast<exint>(ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY)) == nullptr ? 0 : 1;	
	changed |= setVisibleState(UI::filterSectionSwitcher_Parameter.getToken(), is1Connected);

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
	exint cuspVertexNormalValue = static_cast<exint>(UI::cuspVertexNormalsToggle_Parameter.getFactoryDefaults()->getOrdinal());

	exint switchVisibleInputValue;
	PRM_ACCESS::Get::IntPRM(me, switchVisibleInputValue, UI::switchVisibleInputChoiceMenu_Parameter, time);

	switch (switchVisibleInputValue)
	{
		case static_cast<exint>(ENUMS::VisibleInputOption::CONVEX_HULLS) :
		{
			cuspVertexNormalValue = 1;
			auto cuspAngleValue = 0.0;			
			PRM_ACCESS::Set::IntPRM(me, cuspVertexNormalValue, UI::cuspVertexNormalsToggle_Parameter, time);
			PRM_ACCESS::Set::FloatPRM(me, cuspAngleValue, UI::specifyCuspAngleFloat_Parameter, time);
		} break;

		case static_cast<exint>(ENUMS::VisibleInputOption::ORIGINAL_GEOMETRY) :
		{
			cuspVertexNormalValue = 0;	
			PRM_ACCESS::Set::IntPRM(me, cuspVertexNormalValue, UI::cuspVertexNormalsToggle_Parameter, time);
			CallbackCuspVertexNormal(data, index, time, tmp);			
		} break;

		case static_cast<exint>(ENUMS::VisibleInputOption::BOTH) :
		{
			cuspVertexNormalValue = 0;
			PRM_ACCESS::Set::IntPRM(me, cuspVertexNormalValue, UI::cuspVertexNormalsToggle_Parameter, time);
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
SOP_Operator::WhenConvexHullsInput(OP_Context& context, fpreal time)
{
	if (duplicateSource(static_cast<exint>(ENUMS::ProcessedInputType::CONVEX_HULLS), context) <= UT_ErrorSeverity::UT_ERROR_WARNING && error() <= UT_ErrorSeverity::UT_ERROR_WARNING)
	{
		// cusp vertex normals
		auto processResult = CuspConvexInputVertexNormals(this->gdp, time);
		if (processResult != ENUMS::MethodProcessResult::SUCCESS) return processResult;

		ATTRIB_ACCESS::Find::IntATT(this, this->gdp, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_ID), this->_hullIDHandle);
		ATTRIB_ACCESS::Find::FloatATT(this, this->gdp, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_VOLUME), this->_hullVolumeHandle);
		ATTRIB_ACCESS::Find::IntATT(this, this->gdp, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID), this->_bundleIDHandle);
		// create vertex 'hull_id' from primitive hull_id
		// create vertex 'hull_volume' from primitive hull_volume
		// create vertex 'bundle_id' from primitive bundle_id		
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::WhenOriginalGeometryInput(OP_Context& context, fpreal time)
{
	if (duplicateSource(static_cast<exint>(ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY), context) <= UT_ErrorSeverity::UT_ERROR_WARNING && error() <= UT_ErrorSeverity::UT_ERROR_WARNING)
	{
		ATTRIB_ACCESS::Find::IntATT(this, this->gdp, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID), this->_bundleIDHandle);

		if (this->_bundleIDHandle.isValid())
		{
			this->_pointBundleIDHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_POINT, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID), 1));
			if (this->_pointBundleIDHandle.isInvalid())
			{
				addError(SOP_MESSAGE, "Failed to convert 'bunlde_id' to vertex attribute.");
				return ENUMS::MethodProcessResult::FAILURE;
			}
		}
		
		if (this->_pointBundleIDHandle.isValid())
		{
			for (auto offset : this->gdp->getPrimitiveRange())
			{
				const auto currPrim = this->gdp->getPrimitive(offset);
				if (currPrim)
				{
					const auto currBundlID = this->_bundleIDHandle.get(offset);
					for (auto vtxoffset : currPrim->getPointRange()) this->_pointBundleIDHandle.set(vtxoffset, currBundlID);
				}
			}
		}
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
		case static_cast<exint>(ENUMS::VisibleInputOption::CONVEX_HULLS) :
		{
			processResult = WhenConvexHullsInput(context, currentTime);
			if (processResult != ENUMS::MethodProcessResult::SUCCESS) return error();
		} break;

		case static_cast<exint>(ENUMS::VisibleInputOption::ORIGINAL_GEOMETRY) :
		{
			processResult = WhenOriginalGeometryInput(context, currentTime);
			if (processResult != ENUMS::MethodProcessResult::SUCCESS) return error();
		} break;

		case static_cast<exint>(ENUMS::VisibleInputOption::BOTH) :
		{
			processResult = WhenBothInputs(context, currentTime);
			if (processResult != ENUMS::MethodProcessResult::SUCCESS) return error();
		} break;
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