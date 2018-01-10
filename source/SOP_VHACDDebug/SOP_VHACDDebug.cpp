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
#include <Utility/GU_DetailCalculator.h>

// this
#include "Parameters.h"
#include "FilterModeOption.h"
#include <Utility/GA_AttributeMismatchTester.h>
#include <Utility/GA_AttributeMismatchTester.h>

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

	// is input connected?
	const exint is0Connected = this->getInput(0) == nullptr ? 0 : 1;

	/* ---------------------------- Set Global Visibility ---------------------------- */

	visibilityState = is0Connected ? 1 : 0;

	/* ---------------------------- Set States --------------------------------------- */

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

/* -----------------------------------------------------------------
OPERATOR INITIALIZATION                                            |
----------------------------------------------------------------- */

SOP_Operator::~SOP_VHACDDebug() { }

SOP_Operator::SOP_VHACDDebug(OP_Network* network, const char* name, OP_Operator* op) :
SOP_Base_Operator(network, name, op),
_convexGDP(nullptr),
_originalGDP(nullptr)
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

// YOUR CODE GOES HERE...

/* -----------------------------------------------------------------
MAIN                                                               |
----------------------------------------------------------------- */

OP_ERROR 
SOP_Operator::cookMySop(OP_Context& context)
{
	DEFAULTS_CookMySop()
	
	// duplicate first input
	this->_convexGDP = new GU_Detail(inputGeo(static_cast<exint>(ENUMS::ProcessedInputType::CONVEX_HULLS), context));

	// duplicate second input
	const exint is1Connected = this->getInput(static_cast<exint>(ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY)) == nullptr ? 0 : 1;
	if (is1Connected) this->_originalGDP = new GU_Detail(inputGeo(static_cast<exint>(ENUMS::ProcessedInputType::ORIGINAL_GEOMETRY), context));

	// we can work with only first input geometry
	if (this->_convexGDP&& error() <= UT_ErrorSeverity::UT_ERROR_WARNING)
	{
		// do we want to cusp normals of convex hulls?
		bool cuspVertexNormalsValue;
		PRM_ACCESS::Get::IntPRM(this, cuspVertexNormalsValue, UI::cuspVertexNormalsToggle_Parameter, currentTime);
		if (cuspVertexNormalsValue)
		{
			fpreal specifyCuspAngleValue;
			PRM_ACCESS::Get::FloatPRM(this, specifyCuspAngleValue, UI::specifyCuspAngleFloat_Parameter, currentTime);

			const auto vertexNormalHandle = GA_RWHandleV3(this->_convexGDP->addFloatTuple(GA_AttributeOwner::GA_ATTRIB_VERTEX, "N", 3));
			GEOcomputeNormals(*this->_convexGDP, vertexNormalHandle, nullptr, specifyCuspAngleValue);
		}
	
		if (this->_originalGDP)
		{
			std::cout << "Copying both" << std::endl;
			this->gdp->copy(*this->_convexGDP, GEO_CopyMethod::GEO_COPY_START);
			this->gdp->copy(*this->_originalGDP, GEO_CopyMethod::GEO_COPY_END);
		}
		else this->gdp->copy(*this->_convexGDP, GEO_CopyMethod::GEO_COPY_ONCE);
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