/*
	Volumetric-Hierarchical Approximate Convex Decomposition.
	Based on https://github.com/kmammou/v-hacd

	IMPORTANT! ------------------------------------------	
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

// hou-hdk-common
#include <Macros/ParameterList.h>
#include <Utility/AttributeAccessing.h>
#include <Utility/ParameterAccessing.h>
#include <Enums/NodeErrorLevel.h>

// this
#include "PRMs_VHACDSetup.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator			GET_SOP_Namespace()::SOP_VHACDSetup
#define SOP_SmallName			"vhacd::setup::1.1"
#define SOP_Input_Name_0		"Geometry"
#define SOP_Base_Operator		SOP_Node
#define MSS_Selector			GET_SOP_Namespace()::MSS_VHACDSetup

// very important
#define SOP_GroupFieldIndex_0	1
#define SOP_GroupPRM			CONST_PrimitiveGroupInput0_Name

#define UI						GET_SOP_Namespace()::PRMs_VHACDSetup
#define PRM_ACCESS				GET_Base_Namespace()::Utility::PRM
#define ATTRIB_ACCESS			GET_Base_Namespace()::Utility::Attribute

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

PARAMETERLIST_Start(SOP_Operator)

	UI::filterSectionSwitcher_Parameter,
	UI::input0PrimitiveGroup_Parameter,
	UI::processModeChoiceMenu_Parameter,
	UI::soloSpecifiedGroupToggle_Parameter,
	UI::soloSpecifiedGroupSeparator_Parameter,
	
	UI::mainSectionSwitcher_Parameter,
	UI::addDecompositionModeAttributeToggle_Parameter,
	UI::addDecompositionModeAttributeSeparator_Parameter,
	UI::decompositionModeValueChoiceMenu_Parameter,
	UI::decompositionModeFallbackInteger_Parameter,

	UI::addResolutionAttributeToggle_Parameter,
	UI::addResolutionAttributeSeparator_Parameter,
	UI::resolutionValueInteger_Parameter,
	UI::resolutionFallbackInteger_Parameter,

	UI::addConcavityAttributeToggle_Parameter,
	UI::addConcavityAttributeSeparator_Parameter,
	UI::concavityValueFloat_Parameter,
	UI::concavityFallbackFloat_Parameter,

	UI::addPlaneDownsamplingAttributeToggle_Parameter,
	UI::addPlaneDownsamplingAttributeSeparator_Parameter,
	UI::planeDownsamplingValueInteger_Parameter,
	UI::planeDownsamplingFallbackInteger_Parameter,

	UI::addConvexHullDownsamplingAttributeToggle_Parameter,
	UI::addConvexHullDownsamplingAttributeSeparator_Parameter,
	UI::convexHullDownsamplingValueInteger_Parameter,
	UI::convexHullDownsamplingFallbackInteger_Parameter,

	UI::addAlphaAttributeToggle_Parameter,
	UI::addAlphaAttributeSeparator_Parameter,
	UI::alphaValueFloat_Parameter,
	UI::alphaFallbackFloat_Parameter,

	UI::addBetaAttributeToggle_Parameter,
	UI::addBetaAttributeSeparator_Parameter,
	UI::betaValueFloat_Parameter,
	UI::betaFallbackFloat_Parameter,

	UI::addMaxConvexHullsAttributeToggle_Parameter,
	UI::addMaxConvexHullsAttributeSeparator_Parameter,
	UI::maxConvexHullsValueInteger_Parameter,
	UI::maxConvexHullsFallbackInteger_Parameter,

	UI::addMaxTriangleCountAttributeToggle_Parameter,
	UI::addMaxTriangleCountAttributeSeparator_Parameter,
	UI::maxTriangleCountValueInteger_Parameter,
	UI::maxTriangleCountFallbackInteger_Parameter,

	UI::addAdaptiveSamplingAttributeToggle_Parameter,
	UI::addAdaptiveSamplingAttributeSeparator_Parameter,
	UI::adaptiveSamplingValueFloat_Parameter,
	UI::adaptiveSamplingFallbackFloat_Parameter,

	UI::addConvexHullApproximationAttributeToggle_Parameter,
	UI::addConvexHullApproximationAttributeSeparator_Parameter,
	UI::convexHullApproximationValueToggle_Parameter,
	UI::convexHullApproximationFallbackInteger_Parameter,

	UI::addProjectVerticesAttributeToggle_Parameter,
	UI::addProjectVerticesAttributeSeparator_Parameter,
	UI::projectVerticesValueToggle_Parameter,
	UI::projectVertivcesFallbackInteger_Parameter,

	UI::addNormalizeMeshAttributeToggle_Parameter,
	UI::addNormalizeMeshAttributeSeparator_Parameter,	
	UI::normalizeMeshValueToggle_Parameter,
	UI::normalizeMeshFallbackInteger_Parameter,

	UI::additionalSectionSwitcher_Parameter,
	PARAMETERLIST_DescriptionPRM(UI),

PARAMETERLIST_End()

bool 
SOP_Operator::updateParmsFlags()
{
	DEFAULTS_UpdateParmsFlags(SOP_Base_Operator)

	// is input connected?
	const exint is0Connected = getInput(0) == nullptr ? 0 : 1;

	/* ---------------------------- Set Global Visibility ---------------------------- */

	visibilityState = is0Connected ? 1 : 0; // TODO: do I still need this?

	/* ---------------------------- Set States --------------------------------------- */

	UT_String primitiveGroupPatern;
	PRM_ACCESS::Get::StringPRM(this, primitiveGroupPatern, UI::input0PrimitiveGroup_Parameter, currentTime);
	changed |= enableParm(UI::processModeChoiceMenu_Parameter.getToken(), primitiveGroupPatern != "");
	changed |= enableParm(UI::soloSpecifiedGroupToggle_Parameter.getToken(), primitiveGroupPatern != "");

	PRM_ACCESS::Get::IntPRM(this, visibilityState, UI::addDecompositionModeAttributeToggle_Parameter, currentTime);
	changed |= setVisibleState(UI::decompositionModeValueChoiceMenu_Parameter.getToken(), visibilityState);
	changed |= setVisibleState(UI::decompositionModeFallbackInteger_Parameter.getToken(), visibilityState);

	PRM_ACCESS::Get::IntPRM(this, visibilityState, UI::addResolutionAttributeToggle_Parameter, currentTime);
	changed |= setVisibleState(UI::resolutionValueInteger_Parameter.getToken(), visibilityState);
	changed |= setVisibleState(UI::resolutionFallbackInteger_Parameter.getToken(), visibilityState);

	PRM_ACCESS::Get::IntPRM(this, visibilityState, UI::addMaxConvexHullsAttributeToggle_Parameter, currentTime);
	changed |= setVisibleState(UI::maxConvexHullsValueInteger_Parameter.getToken(), visibilityState);
	changed |= setVisibleState(UI::maxConvexHullsFallbackInteger_Parameter.getToken(), visibilityState);

	PRM_ACCESS::Get::IntPRM(this, visibilityState, UI::addConcavityAttributeToggle_Parameter, currentTime);
	changed |= setVisibleState(UI::concavityValueFloat_Parameter.getToken(), visibilityState);
	changed |= setVisibleState(UI::concavityFallbackFloat_Parameter.getToken(), visibilityState);

	PRM_ACCESS::Get::IntPRM(this, visibilityState, UI::addPlaneDownsamplingAttributeToggle_Parameter, currentTime);
	changed |= setVisibleState(UI::planeDownsamplingValueInteger_Parameter.getToken(), visibilityState);
	changed |= setVisibleState(UI::planeDownsamplingFallbackInteger_Parameter.getToken(), visibilityState);

	PRM_ACCESS::Get::IntPRM(this, visibilityState, UI::addConvexHullDownsamplingAttributeToggle_Parameter, currentTime);
	changed |= setVisibleState(UI::convexHullDownsamplingValueInteger_Parameter.getToken(), visibilityState);
	changed |= setVisibleState(UI::convexHullDownsamplingFallbackInteger_Parameter.getToken(), visibilityState);

	PRM_ACCESS::Get::IntPRM(this, visibilityState, UI::addAlphaAttributeToggle_Parameter, currentTime);
	changed |= setVisibleState(UI::alphaValueFloat_Parameter.getToken(), visibilityState);
	changed |= setVisibleState(UI::alphaFallbackFloat_Parameter.getToken(), visibilityState);

	PRM_ACCESS::Get::IntPRM(this, visibilityState, UI::addBetaAttributeToggle_Parameter, currentTime);
	changed |= setVisibleState(UI::betaValueFloat_Parameter.getToken(), visibilityState);
	changed |= setVisibleState(UI::betaFallbackFloat_Parameter.getToken(), visibilityState);

	PRM_ACCESS::Get::IntPRM(this, visibilityState, UI::addMaxTriangleCountAttributeToggle_Parameter, currentTime);
	changed |= setVisibleState(UI::maxTriangleCountValueInteger_Parameter.getToken(), visibilityState);
	changed |= setVisibleState(UI::maxTriangleCountFallbackInteger_Parameter.getToken(), visibilityState);

	PRM_ACCESS::Get::IntPRM(this, visibilityState, UI::addAdaptiveSamplingAttributeToggle_Parameter, currentTime);
	changed |= setVisibleState(UI::adaptiveSamplingValueFloat_Parameter.getToken(), visibilityState);
	changed |= setVisibleState(UI::adaptiveSamplingFallbackFloat_Parameter.getToken(), visibilityState);

	PRM_ACCESS::Get::IntPRM(this, visibilityState, UI::addConvexHullApproximationAttributeToggle_Parameter, currentTime);
	changed |= setVisibleState(UI::convexHullApproximationValueToggle_Parameter.getToken(), visibilityState);
	changed |= setVisibleState(UI::convexHullApproximationFallbackInteger_Parameter.getToken(), visibilityState);

	PRM_ACCESS::Get::IntPRM(this, visibilityState, UI::addProjectVerticesAttributeToggle_Parameter, currentTime);
	changed |= setVisibleState(UI::projectVerticesValueToggle_Parameter.getToken(), visibilityState);
	changed |= setVisibleState(UI::projectVertivcesFallbackInteger_Parameter.getToken(), visibilityState);

	PRM_ACCESS::Get::IntPRM(this, visibilityState, UI::addNormalizeMeshAttributeToggle_Parameter, currentTime);
	changed |= setVisibleState(UI::normalizeMeshValueToggle_Parameter.getToken(), visibilityState);
	changed |= setVisibleState(UI::normalizeMeshFallbackInteger_Parameter.getToken(), visibilityState);

	// update description active state
	UPDATE_DescriptionPRM_ActiveState(this, UI)

	return changed;
}

IMPLEMENT_DescriptionPRM_Callback(SOP_Operator, UI)

/* -----------------------------------------------------------------
CALLBACKS                                                          |
----------------------------------------------------------------- */

int 
SOP_Operator::CallbackProcessModeChoiceMenu(void* data, int index, float time, const PRM_Template* tmp)
{
	const auto me = reinterpret_cast<SOP_Operator*>(data);
	if (!me) return 0; 

	UT_String groupPattern;
	PRM_ACCESS::Get::StringPRM(me, groupPattern, UI::mainSectionSwitcher_Parameter, time);	
	if (groupPattern.length() < 1) return 0;

	auto defVal0 = static_cast<exint>(UI::processModeChoiceMenu_Parameter.getFactoryDefaults()->getOrdinal());
	auto defVal1 = static_cast<exint>(UI::soloSpecifiedGroupToggle_Parameter.getFactoryDefaults()->getOrdinal());

	PRM_ACCESS::Set::IntPRM(me, defVal0, UI::processModeChoiceMenu_Parameter, time);
	PRM_ACCESS::Set::IntPRM(me, defVal1, UI::soloSpecifiedGroupToggle_Parameter, time);
	
	return 1; 
}

#define THIS_CALLBACK_Reset_IntPRM(operatorname, callbackcall, parameter1, parameter2) int callbackcall(void* data, int index, float time, const PRM_Template* tmp) { const auto me = reinterpret_cast<operatorname*>(data); if (!me) return 0; auto defVal0 = static_cast<exint>(parameter1.getFactoryDefaults()->getOrdinal()); auto defVal1 = static_cast<exint>(parameter2.getFactoryDefaults()->getOrdinal()); PRM_ACCESS::Set::IntPRM(me, defVal0, parameter1, time); PRM_ACCESS::Set::IntPRM(me, defVal1, parameter2, time); return 1; }
#define THIS_CALLBACK_Reset_FloatPRM(operatorname, callbackcall, parameter1, parameter2) int callbackcall(void* data, int index, float time, const PRM_Template* tmp) { const auto me = reinterpret_cast<operatorname*>(data); if (!me) return 0; auto defVal0 = parameter1.getFactoryDefaults()->getFloat(); auto defVal1 = parameter2.getFactoryDefaults()->getFloat(); PRM_ACCESS::Set::FloatPRM(me, defVal0, parameter1, time); PRM_ACCESS::Set::FloatPRM(me, defVal1, parameter2, time); return 1; }

THIS_CALLBACK_Reset_IntPRM(SOP_Operator, SOP_Operator::CallbackAddDecompositionModeATT, UI::decompositionModeValueChoiceMenu_Parameter, UI::decompositionModeFallbackInteger_Parameter)
THIS_CALLBACK_Reset_IntPRM(SOP_Operator, SOP_Operator::CallbackAddResolutionATT, UI::resolutionValueInteger_Parameter, UI::resolutionFallbackInteger_Parameter)
THIS_CALLBACK_Reset_FloatPRM(SOP_Operator, SOP_Operator::CallbackAddConcavityATT, UI::concavityValueFloat_Parameter, UI::concavityFallbackFloat_Parameter)
THIS_CALLBACK_Reset_IntPRM(SOP_Operator, SOP_Operator::CallbackAddPlaneDownsamplingATT, UI::planeDownsamplingValueInteger_Parameter, UI::planeDownsamplingFallbackInteger_Parameter)
THIS_CALLBACK_Reset_IntPRM(SOP_Operator, SOP_Operator::CallbackAddConvexHullDownsamplingATT, UI::convexHullDownsamplingValueInteger_Parameter, UI::convexHullDownsamplingFallbackInteger_Parameter)
THIS_CALLBACK_Reset_FloatPRM(SOP_Operator, SOP_Operator::CallbackAddAlphaATT, UI::alphaValueFloat_Parameter, UI::alphaFallbackFloat_Parameter)
THIS_CALLBACK_Reset_FloatPRM(SOP_Operator, SOP_Operator::CallbackAddBetaATT, UI::betaValueFloat_Parameter, UI::betaFallbackFloat_Parameter)
THIS_CALLBACK_Reset_IntPRM(SOP_Operator, SOP_Operator::CallbackAddMaxConvexHullsCountATT, UI::maxConvexHullsValueInteger_Parameter, UI::maxConvexHullsFallbackInteger_Parameter)
THIS_CALLBACK_Reset_IntPRM(SOP_Operator, SOP_Operator::CallbackAddMaxTriangleCountATT, UI::maxTriangleCountValueInteger_Parameter, UI::maxTriangleCountFallbackInteger_Parameter)
THIS_CALLBACK_Reset_FloatPRM(SOP_Operator, SOP_Operator::CallbackAddAdaptiveSamplingATT, UI::adaptiveSamplingValueFloat_Parameter, UI::adaptiveSamplingFallbackFloat_Parameter)
THIS_CALLBACK_Reset_IntPRM(SOP_Operator, SOP_Operator::CallbackAddConvexHullApproximationATT, UI::convexHullApproximationValueToggle_Parameter, UI::convexHullApproximationFallbackInteger_Parameter)
THIS_CALLBACK_Reset_IntPRM(SOP_Operator, SOP_Operator::CallbackAddProjectHullVerticeshATT, UI::projectVerticesValueToggle_Parameter, UI::projectVertivcesFallbackInteger_Parameter)
THIS_CALLBACK_Reset_IntPRM(SOP_Operator, SOP_Operator::CallbackAddNormalizeMeshATT, UI::normalizeMeshValueToggle_Parameter, UI::normalizeMeshFallbackInteger_Parameter)

#undef THIS_CALLBACK_Reset_FloatPRM
#undef THIS_CALLBACK_Reset_IntPRM

/* -----------------------------------------------------------------
OPERATOR INITIALIZATION                                            |
----------------------------------------------------------------- */

SOP_Operator::~SOP_VHACDSetup() { }

SOP_Operator::SOP_VHACDSetup(OP_Network* network, const char* name, OP_Operator* op) 
: SOP_Base_Operator(network, name, op), 
_primitiveGroupInput0(nullptr)
{ op->setIconName(UI::names.Get(CommonNameOption::ICON_NAME)); }

OP_Node* 
SOP_Operator::CreateMe(OP_Network* network, const char* name, OP_Operator* op) { return new SOP_Operator(network, name, op); }

const char* 
SOP_Operator::inputLabel(unsigned input) const { return SOP_Input_Name_0; }

OP_ERROR
SOP_Operator::cookInputGroups(OP_Context &context, int alone)
{
	const auto isOrdered = true;
	return cookInputPrimitiveGroups(context, this->_primitiveGroupInput0, alone, true, SOP_GroupFieldIndex_0, -1, true, isOrdered, true, 0);
}

/* -----------------------------------------------------------------
HELPERS                                                            |
----------------------------------------------------------------- */

OP_ERROR
SOP_Operator::ProcessWholeGeometry(UT_AutoInterrupt progress, fpreal time)
{
	const auto primIt = GA_Iterator(this->gdp->getPrimitiveRange());
	return SetupAttributes(primIt, progress, time);
}

OP_ERROR
SOP_Operator::ProcessSpecifiedGeometry(UT_AutoInterrupt progress, fpreal time)
{
	const auto success = !this->_primitiveGroupInput0->isEmpty();
	if (success)
	{			
		auto gop = GOP_Manager();

		bool processModeState;
		bool soloSpecifiedGroupState;

		PRM_ACCESS::Get::IntPRM(this, processModeState, UI::processModeChoiceMenu_Parameter, time);
		PRM_ACCESS::Get::IntPRM(this, soloSpecifiedGroupState, UI::soloSpecifiedGroupToggle_Parameter, time);

		if (!processModeState && !soloSpecifiedGroupState)
		{
			const auto primIt = GA_Iterator(this->gdp->getPrimitiveRange(this->_primitiveGroupInput0));
			return SetupAttributes(primIt, progress, time);
		}
		
		if (!processModeState && soloSpecifiedGroupState)
		{
			auto nonSelectedPrimitives = static_cast<GA_PrimitiveGroup*>(gop.createPrimitiveGroup(*this->gdp));
			nonSelectedPrimitives->addRange(this->gdp->getPrimitiveRange());
			nonSelectedPrimitives->removeRange(this->gdp->getPrimitiveRange(this->_primitiveGroupInput0));
			this->gdp->deletePrimitives(*nonSelectedPrimitives, true);

			const auto primIt = GA_Iterator(this->gdp->getPrimitiveRange());
			return SetupAttributes(primIt, progress, time);
		}
		
		if (processModeState && !soloSpecifiedGroupState)
		{
			auto nonSelectedPrimitives = static_cast<GA_PrimitiveGroup*>(gop.createPrimitiveGroup(*this->gdp));
			nonSelectedPrimitives->addRange(this->gdp->getPrimitiveRange());
			nonSelectedPrimitives->removeRange(this->gdp->getPrimitiveRange(this->_primitiveGroupInput0));

			const auto primIt = GA_Iterator(this->gdp->getPrimitiveRange(nonSelectedPrimitives));
			return SetupAttributes(primIt, progress, time);
		}
		
		if (processModeState && soloSpecifiedGroupState)
		{
			this->gdp->deletePrimitives(*this->_primitiveGroupInput0, true);

			const auto primIt = GA_Iterator(this->gdp->getPrimitiveRange());
			return SetupAttributes(primIt, progress, time);
		}
	}
	else this->addWarning(SOP_MESSAGE, "Specified group is empty. Nothing will be applied.");

	return error();
}

void
SOP_Operator::AddIntATT(GA_RWHandleI& attributeHandle, const PRM_Template& valueparameter, const PRM_Template& fallbackparameter, const char* attributename, fpreal time)
{
	const auto success = ATTRIB_ACCESS::Find::IntATT(this, this->gdp, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, attributename, attributeHandle);
	if (!success)
	{
		exint fallbackValue;
		PRM_ACCESS::Get::IntPRM(this, fallbackValue, fallbackparameter, time);

		attributeHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, attributename, 1, GA_Defaults(fallbackValue)));
	}
}

void
SOP_Operator::AddFloatATT(GA_RWHandleR& attributeHandle, const PRM_Template& valueparameter, const PRM_Template& fallbackparameter, const char* attributename, fpreal time)
{
	const auto success = ATTRIB_ACCESS::Find::FloatATT(this, this->gdp, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, attributename, attributeHandle);
	if (!success)
	{
		fpreal fallbackValue;
		PRM_ACCESS::Get::FloatPRM(this, fallbackValue, fallbackparameter, time);

		attributeHandle = GA_RWHandleR(this->gdp->addFloatTuple(GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, attributename, 1, GA_Defaults(fallbackValue)));
	}
}

void
SOP_Operator::UpdateIntATT(GA_RWHandleI& attributeHandle, GA_Offset offset, const PRM_Template& valueparameter, fpreal time)
{
	exint parameterValue;
	PRM_ACCESS::Get::IntPRM(this, parameterValue, valueparameter, time);
	attributeHandle.set(offset, parameterValue);
}

void
SOP_Operator::UpdateFloatATT(GA_RWHandleR& attributeHandle, GA_Offset offset, const PRM_Template& valueparameter, fpreal time)
{
	fpreal parameterValue;
	PRM_ACCESS::Get::FloatPRM(this, parameterValue, valueparameter, time);
	attributeHandle.set(offset, parameterValue);
}

OP_ERROR
SOP_Operator::SetupAttributes(GA_Iterator primit, UT_AutoInterrupt progress, fpreal time)
{	
	bool addDecompositionModeState;
	bool addResolutionState;	
	bool addConcavityState;
	bool addPlaneDownsamplingState;
	bool addConvexHullDownsamplingState;
	bool addAlphaState;
	bool addBetaState;	
	bool addMaxConvexHullsCountState;
	bool addMaxTriangleCountState;
	bool addAdaptiveSamplingState;
	bool addApproximateHullsState;
	bool addProjectVerticesState;
	bool addNormalizeMeshState;

	GA_RWHandleI decompositionModeHandle;
	GA_RWHandleI resolutionHandle;
	GA_RWHandleI maxConvexHullsCountHandle;
	GA_RWHandleR concavityHandle;
	GA_RWHandleI planeDownsamplingHandle;
	GA_RWHandleI convexHullDownsamplingHandle;
	GA_RWHandleR alphaHandle;
	GA_RWHandleR betaHandle;	
	GA_RWHandleI maxTriangleCountHandle;
	GA_RWHandleR adaptiveSamplingHandle;
	GA_RWHandleI approximateHullsHandle;
	GA_RWHandleI projectVerticesHandle;
	GA_RWHandleI normalizeMeshHandle;

	PRM_ACCESS::Get::IntPRM(this, addDecompositionModeState, UI::addDecompositionModeAttributeToggle_Parameter, time);	
	PRM_ACCESS::Get::IntPRM(this, addResolutionState, UI::addResolutionAttributeToggle_Parameter, time);	
	PRM_ACCESS::Get::IntPRM(this, addConcavityState, UI::addConcavityAttributeToggle_Parameter, time);
	PRM_ACCESS::Get::IntPRM(this, addPlaneDownsamplingState, UI::addPlaneDownsamplingAttributeToggle_Parameter, time);
	PRM_ACCESS::Get::IntPRM(this, addConvexHullDownsamplingState, UI::addConvexHullDownsamplingAttributeToggle_Parameter, time);
	PRM_ACCESS::Get::IntPRM(this, addAlphaState, UI::addAlphaAttributeToggle_Parameter, time);
	PRM_ACCESS::Get::IntPRM(this, addBetaState, UI::addBetaAttributeToggle_Parameter, time);	
	PRM_ACCESS::Get::IntPRM(this, addMaxConvexHullsCountState, UI::addMaxConvexHullsAttributeToggle_Parameter, time);
	PRM_ACCESS::Get::IntPRM(this, addMaxTriangleCountState, UI::addMaxTriangleCountAttributeToggle_Parameter, time);
	PRM_ACCESS::Get::IntPRM(this, addAdaptiveSamplingState, UI::addAdaptiveSamplingAttributeToggle_Parameter, time);
	PRM_ACCESS::Get::IntPRM(this, addApproximateHullsState, UI::addConvexHullApproximationAttributeToggle_Parameter, time);
	PRM_ACCESS::Get::IntPRM(this, addProjectVerticesState, UI::addProjectVerticesAttributeToggle_Parameter, time);
	PRM_ACCESS::Get::IntPRM(this, addNormalizeMeshState, UI::addNormalizeMeshAttributeToggle_Parameter, time);

	if (addDecompositionModeState) AddIntATT(decompositionModeHandle, UI::decompositionModeValueChoiceMenu_Parameter, UI::decompositionModeFallbackInteger_Parameter, UI::names.Get(CommonNameOption::DECOMPOSITION_MODE), time);
	if (addResolutionState) AddIntATT(resolutionHandle, UI::resolutionValueInteger_Parameter, UI::resolutionFallbackInteger_Parameter, UI::names.Get(CommonNameOption::RESOLUTION), time);		
	if (addConcavityState) AddFloatATT(concavityHandle, UI::concavityValueFloat_Parameter, UI::concavityFallbackFloat_Parameter, UI::names.Get(CommonNameOption::CONCAVITY), time);
	if (addPlaneDownsamplingState) AddIntATT(planeDownsamplingHandle, UI::planeDownsamplingValueInteger_Parameter, UI::planeDownsamplingFallbackInteger_Parameter, UI::names.Get(CommonNameOption::PLANE_DOWNSAMPLING), time);		
	if (addConvexHullDownsamplingState) AddIntATT(convexHullDownsamplingHandle, UI::convexHullDownsamplingValueInteger_Parameter, UI::convexHullDownsamplingFallbackInteger_Parameter, UI::names.Get(CommonNameOption::PLANE_DOWNSAMPLING), time);	
	if (addAlphaState) AddFloatATT(alphaHandle, UI::alphaValueFloat_Parameter, UI::alphaFallbackFloat_Parameter, UI::names.Get(CommonNameOption::ALPHA), time);
	if (addBetaState) AddFloatATT(betaHandle, UI::betaValueFloat_Parameter, UI::betaFallbackFloat_Parameter, UI::names.Get(CommonNameOption::BETA), time);	
	if (addMaxConvexHullsCountState) AddIntATT(maxConvexHullsCountHandle, UI::maxConvexHullsValueInteger_Parameter, UI::maxConvexHullsFallbackInteger_Parameter, UI::names.Get(CommonNameOption::MAX_CONVEX_HULLS_COUNT), time);
	if (addMaxTriangleCountState) AddIntATT(maxTriangleCountHandle, UI::maxTriangleCountValueInteger_Parameter, UI::maxTriangleCountFallbackInteger_Parameter, UI::names.Get(CommonNameOption::MAX_TRIANGLE_COUNT), time);
	if (addAdaptiveSamplingState) AddFloatATT(adaptiveSamplingHandle, UI::adaptiveSamplingValueFloat_Parameter, UI::adaptiveSamplingFallbackFloat_Parameter, UI::names.Get(CommonNameOption::ADAPTIVE_SAMPLING), time);		
	if (addApproximateHullsState) AddIntATT(approximateHullsHandle, UI::convexHullApproximationValueToggle_Parameter, UI::convexHullApproximationFallbackInteger_Parameter, UI::names.Get(CommonNameOption::CONVEX_HULL_APPROXIMATION), time);
	if (addProjectVerticesState) AddIntATT(projectVerticesHandle, UI::projectVerticesValueToggle_Parameter, UI::projectVertivcesFallbackInteger_Parameter, UI::names.Get(CommonNameOption::PROJECT_HULL_VERTICES), time);
	if (addNormalizeMeshState) AddIntATT(normalizeMeshHandle, UI::normalizeMeshValueToggle_Parameter, UI::normalizeMeshFallbackInteger_Parameter, UI::names.Get(CommonNameOption::NORMALIZE_MESH), time);
	
	// update attributes
	for (primit; !primit.atEnd(); primit.advance())
	{
		if (progress.wasInterrupted())
		{
			this->addError(SOP_MESSAGE, "Operation interrupted.");
			return error();
		}
		
#ifdef DEBUG_THIS
		std::cout << "Primitive No.: " << *primit << std::endl;
#endif	

		if (addDecompositionModeState && decompositionModeHandle.isValid()) UpdateIntATT(decompositionModeHandle, *primit, UI::decompositionModeValueChoiceMenu_Parameter, time);
		if (addResolutionState && resolutionHandle.isValid()) UpdateIntATT(resolutionHandle, *primit, UI::resolutionValueInteger_Parameter, time);
		if (addMaxConvexHullsCountState && maxConvexHullsCountHandle.isValid()) UpdateIntATT(maxConvexHullsCountHandle, *primit, UI::maxConvexHullsValueInteger_Parameter, time);
		if (addConcavityState && concavityHandle.isValid()) UpdateFloatATT(concavityHandle, *primit, UI::concavityValueFloat_Parameter, time);
		if (addPlaneDownsamplingState && planeDownsamplingHandle.isValid()) UpdateIntATT(planeDownsamplingHandle, *primit, UI::planeDownsamplingValueInteger_Parameter, time);
		if (addConvexHullDownsamplingState && convexHullDownsamplingHandle.isValid()) UpdateIntATT(convexHullDownsamplingHandle, *primit, UI::convexHullDownsamplingValueInteger_Parameter, time);
		if (addAlphaState && alphaHandle.isValid()) UpdateFloatATT(alphaHandle, *primit, UI::alphaValueFloat_Parameter, time);
		if (addBetaState && betaHandle.isValid()) UpdateFloatATT(betaHandle, *primit, UI::betaValueFloat_Parameter, time);		
		if (addMaxTriangleCountState && maxTriangleCountHandle.isValid()) UpdateIntATT(maxTriangleCountHandle, *primit, UI::maxTriangleCountValueInteger_Parameter, time);
		if (addAdaptiveSamplingState && adaptiveSamplingHandle.isValid()) UpdateFloatATT(adaptiveSamplingHandle, *primit, UI::adaptiveSamplingValueFloat_Parameter, time);
		if (addApproximateHullsState && approximateHullsHandle.isValid()) UpdateIntATT(approximateHullsHandle, *primit, UI::convexHullApproximationValueToggle_Parameter, time);
		if (addProjectVerticesState && projectVerticesHandle.isValid()) UpdateIntATT(projectVerticesHandle, *primit, UI::projectVerticesValueToggle_Parameter, time);
		if (addNormalizeMeshState && normalizeMeshHandle.isValid()) UpdateIntATT(normalizeMeshHandle, *primit, UI::normalizeMeshValueToggle_Parameter, time);
	}
	
	return error();
}

/* -----------------------------------------------------------------
MAIN                                                               |
----------------------------------------------------------------- */

OP_ERROR 
SOP_Operator::cookMySop(OP_Context& context)
{	
	DEFAULTS_CookMySop()
	
	if (duplicateSource(0, context) < OP_ERROR::UT_ERROR_WARNING && error() < OP_ERROR::UT_ERROR_WARNING && cookInputGroups(context) < OP_ERROR::UT_ERROR_WARNING)
	{
		if (!this->_primitiveGroupInput0) return ProcessWholeGeometry(progress, currentTime);
		else return ProcessSpecifiedGeometry(progress, currentTime);
	}

	return error();
}

/* -----------------------------------------------------------------
SELECTOR IMPLEMENTATION                                            |
----------------------------------------------------------------- */

MSS_Selector::~MSS_VHACDSetup() { }

MSS_Selector::MSS_VHACDSetup(OP3D_View& viewer, PI_SelectorTemplate& templ) 
: MSS_ReusableSelector(viewer, templ, SOP_SmallName, SOP_GroupPRM, nullptr, true)
{ this->setAllowUseExistingSelection(false); }

BM_InputSelector*
MSS_Selector::CreateMe(BM_View& viewer, PI_SelectorTemplate& templ)
{ return new MSS_Selector(reinterpret_cast<OP3D_View&>(viewer), templ); }

const char*
MSS_Selector::className() const
{ return "MSS_VHACDSetup"; }

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ATTRIB_ACCESS
#undef PRM_ACCESS
#undef UI

#undef SOP_GroupPRM
#undef SOP_GroupFieldIndex_0

#undef MSS_Selector
#undef SOP_Base_Operator
#undef SOP_Input_Name_0
#undef SOP_SmallName
#undef SOP_Operator