/*
	Volumetric-Hierarchical Approximate Convex Decomposition.
	Based on https://github.com/kmammou/v-hacd

	IMPORTANT! ------------------------------------------
	* Macros starting with '____' shouldn't be used anywhere outside of this file.
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
#include <CH/CH_Manager.h>
#include <PRM/PRM_Parm.h>
#include <PRM/PRM_Error.h>
#include <PRM/PRM_Include.h>

#include <UT/UT_Interrupt.h>
#include <GEO/GEO_ConvertParms.h>
#include <OP/OP_AutoLockInputs.h>

// hou-hdk-common
#include <Macros/ParameterList.h>
#include <Macros/ProgressEscape.h>
#include <Utility/GU_DetailModifier.h>
#include <Utility/GU_DetailTester.h>
#include <Utility/GA_AttributeAccessors.h>
#include <Utility/PRM_TemplateAccessors.h>

#include <Enums/NodeErrorLevel.h>
#include <Enums/AttributeClass.h>

// this
#include "Parameters.h"
#include "ReportModeOption.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator					GET_SOP_Namespace()::SOP_VHACDEngine
#define SOP_Base_Operator				SOP_VHACDNode
#define SOP_InputName_0					"Geometry"

#define COMMON_NAMES					GET_SOP_Namespace()::COMMON_NAMES
#define UI								GET_SOP_Namespace()::PRMs_VHACDEngine

#define UTILS							GET_Base_Namespace()::Utility
#define PRM_ACCESS						UTILS::PRM_TemplateAccessors
#define ATTRIB_ACCESS					UTILS::GA_AttributeAccessors

#define ENUMS							GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

PARAMETERLIST_Start(SOP_Operator)

	UI::filterSectionSwitcher_Parameter,
	UI::allowParametersOverrideToggle_Parameter,
	UI::allowParametersOverrideSeparator_Parameter,
	UI::convertToPolygonsToggle_Parameter,
	UI::convertToPolygonsSeparator_Parameter,

	UI::mainSectionSwitcher_Parameter,
	UI::modeChoiceMenu_Parameter,
	UI::resolutionInteger_Parameter,	
	UI::concavityFloat_Parameter,
	UI::planeDownsamplingInteger_Parameter,
	UI::convexHullDownsamplingInteger_Parameter,
	UI::alphaFloat_Parameter,
	UI::betaFloat_Parameter,	
	UI::maxConvexHullsCountInteger_Parameter,
	UI::maxTriangleCountInteger_Parameter,
	UI::adaptiveSamplingFloat_Parameter,
	UI::approximateConvexHullsToggle_Parameter,
	UI::approximateConvexHullsSeparator_Parameter,
	UI::projectHullVerticesToggle_Parameter,
	UI::projectHullVerticesSeparator_Parameter,
	UI::normalizeMeshToggle_Parameter,
	UI::normalizeMeshSeparator_Parameter,

	UI::useOpenCLToggle_Parameter,
	UI::useOpenCLSeparator_Parameter,	
	UI::additionalSectionSwitcher_Parameter,
	PARAMETERLIST_DescriptionPRM(UI),

	UI::debugSectionSwitcher_Parameter,
	UI::showProcessReportToggle_Parameter,
	UI::showProcessReportSeparator_Parameter,
	UI::reportModeChoiceMenu_Parameter,

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

	// set output report state
	PRM_ACCESS::Get::IntPRM(this, _showReportValueState, UI::showProcessReportToggle_Parameter, currentTime);
	visibilityState = this->_showReportValueState ? 1 : 0;
	changed |= setVisibleState(UI::reportModeChoiceMenu_Parameter.getToken(), visibilityState);

	// update description active state
	UPDATE_DescriptionPRM_ActiveState(this, UI)

	return changed;
}

IMPLEMENT_DescriptionPRM_Callback(SOP_Operator, UI)

/* -----------------------------------------------------------------
OPERATOR INITIALIZATION                                            |
----------------------------------------------------------------- */

SOP_Operator::SOP_VHACDEngine(OP_Network* network, const char* name, OP_Operator* op) 
: SOP_Base_Operator(network, name, op), 
_interfaceVHACD(nullptr), 
_allowParametersOverrideValueState(false), 
_showReportValueState(false), 
_reportModeChoiceValueState(0)
{ op->setIconName(COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_ENGINE_ICONNAME)); }

SOP_Operator::~SOP_VHACDEngine() { }

OP_Node* 
SOP_Operator::CreateMe(OP_Network* network, const char* name, OP_Operator* op) 
{ return new SOP_Operator(network, name, op); }

const char* 
SOP_Operator::inputLabel(unsigned input) const 
{ return SOP_InputName_0; }

/* -----------------------------------------------------------------
HELPERS                                                            |
----------------------------------------------------------------- */

exint 
SOP_Operator::PullIntPRM(GU_Detail* detail, const PRM_Template& parameter, bool interfaceonly /* = false */, fpreal time /* = 0 */)
{
	exint currentIntValue = 0;

	if (!interfaceonly)
	{
		GA_RWHandleI attributeHandle;

		// check is there attribute with the name that matches parameter name
		// if it exist, use it as parm value, otherwise use UI value		
		const auto success = ATTRIB_ACCESS::Find::IntATT(this, detail, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, parameter.getToken(), attributeHandle, ENUMS::NodeErrorLevel::NONE);
		if (success)
		{
			currentIntValue = attributeHandle.get(GA_Offset(0));
			const auto currentRangeValue = parameter.getRangePtr();

			if (currentIntValue < currentRangeValue->getParmMin() || currentIntValue > currentRangeValue->getParmMax()) PRM_ACCESS::Get::IntPRM(this, currentIntValue, parameter, time);
		}
		else  PRM_ACCESS::Get::IntPRM(this, currentIntValue, parameter, time);
	}
	else PRM_ACCESS::Get::IntPRM(this, currentIntValue, parameter, time);
	
	return currentIntValue;
}

fpreal 
SOP_Operator::PullFloatPRM(GU_Detail* detail, const PRM_Template& parameter, bool interfaceonly /* = false */, fpreal time /* = 0 */)
{
	fpreal currentFloatValue = 0;

	if (!interfaceonly)
	{
		//GA_RWAttributeRef attributeReference;
		GA_RWHandleR attributeHandle;

		// check is there attribute with the name that matches parameter name
		// if it exist, use it as parm value, otherwise use UI value		
		const auto success = ATTRIB_ACCESS::Find::FloatATT(this, detail, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, parameter.getToken(), attributeHandle, ENUMS::NodeErrorLevel::NONE);
		if (success)
		{			
			currentFloatValue = attributeHandle.get(GA_Offset(0));
			const auto currentRangeValue = parameter.getRangePtr();

			if (currentFloatValue < currentRangeValue->getParmMin() || currentFloatValue > currentRangeValue->getParmMax()) PRM_ACCESS::Get::FloatPRM(this, currentFloatValue, parameter, time);
		}
		else PRM_ACCESS::Get::FloatPRM(this, currentFloatValue, parameter, time);
	}
	else PRM_ACCESS::Get::FloatPRM(this, currentFloatValue, parameter, time);

	return currentFloatValue;
}

ENUMS::MethodProcessResult
SOP_Operator::PrepareGeometry(GU_Detail* detail, UT_AutoInterrupt progress, fpreal time)
{
	/*
	Required element counts:
	- one primitive, so that we could have a chance to get some polygons from it, even if it's non-polygon geometry
	- 4 points, which gives us combination of 4 triangles that create shape
	*/

	const exint requiredPrimCount = 1;
	const exint requiredPointCount = 4;

	// make sure we got something to work on
	auto errorMessage = UT_String("Not enough primitives to create hull.");
	auto success = UTILS::GU_DetailTester::IsEnoughPrimitives(this, detail, requiredPrimCount, errorMessage);
	if ((success && error() >= OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return ENUMS::MethodProcessResult::FAILURE;

	// do we want only polygons or do we try to convert anything to polygons?			
	bool convertToPolygonsState;
	PRM_ACCESS::Get::IntPRM(this, convertToPolygonsState, UI::convertToPolygonsToggle_Parameter, time);
	if (convertToPolygonsState)
	{
		GEO_ConvertParms parms;
		detail->convert(parms);
	}
	else
	{
		for (auto primIt : detail->getPrimitiveRange())
		{
			PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::INTERRUPT)

			// only polygons allowed
			if (detail->getPrimitive(primIt)->getTypeId() != GA_PRIMPOLY)
			{
				addError(SOP_ErrorCodes::SOP_MESSAGE, "Non-polygon geometry detected on input.");
				return ENUMS::MethodProcessResult::FAILURE;
			}
		}
	}

	// we need at least 4 points to get up from bed
	errorMessage = UT_String("Not enough points to create hull.");
	success = UTILS::GU_DetailTester::IsEnoughPoints(this, detail, requiredPointCount, errorMessage);
	if ((success && error() >= OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return ENUMS::MethodProcessResult::FAILURE;

	const auto processResult = UTILS::GU_DetailModifier::Triangulate(this, progress, detail);
	if ((processResult == ENUMS::MethodProcessResult::SUCCESS && error() >= UT_ErrorSeverity::UT_ERROR_WARNING) || (processResult != ENUMS::MethodProcessResult::SUCCESS && error() >= UT_ErrorSeverity::UT_ERROR_NONE)) return ENUMS::MethodProcessResult::FAILURE;
	
	// is there anything left after preparation?	
	errorMessage = UT_String("After removing zero area and open polygons there are no other primitives left.");
	success = UTILS::GU_DetailTester::IsEnoughPrimitives(this, detail, requiredPrimCount, errorMessage);
	if ((success && error() >= UT_ErrorSeverity::UT_ERROR_WARNING) || (!success && error() >= UT_ErrorSeverity::UT_ERROR_NONE)) return ENUMS::MethodProcessResult::FAILURE;

	errorMessage = UT_String("Not enough points to create hull.");
	return UTILS::GU_DetailTester::IsEnoughPoints(this, detail, requiredPointCount, errorMessage) ? ENUMS::MethodProcessResult::SUCCESS : ENUMS::MethodProcessResult::FAILURE;
}

void 
SOP_Operator::SetupParametersVHACD(GU_Detail* detail, fpreal time)
{		
	PRM_ACCESS::Get::IntPRM(this, this->_allowParametersOverrideValueState, UI::allowParametersOverrideToggle_Parameter, time);

	// logger/callback
	this->_parametersVHACD.m_logger = &this->_loggerVHACD;
	this->_parametersVHACD.m_callback = &this->_callbackVHACD;

	// main parameters
	this->_parametersVHACD.m_mode						= PullIntPRM(detail, UI::modeChoiceMenu_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_resolution					= PullIntPRM(detail, UI::resolutionInteger_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_concavity					= PullFloatPRM(detail, UI::concavityFloat_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_planeDownsampling			= PullIntPRM(detail, UI::planeDownsamplingInteger_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_convexhullDownsampling		= PullIntPRM(detail, UI::convexHullDownsamplingInteger_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_alpha						= PullFloatPRM(detail, UI::alphaFloat_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_beta						= PullFloatPRM(detail, UI::betaFloat_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_maxConvexHulls				= PullIntPRM(detail, UI::maxConvexHullsCountInteger_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_maxNumVerticesPerCH		= PullIntPRM(detail, UI::maxTriangleCountInteger_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_minVolumePerCH				= PullFloatPRM(detail, UI::adaptiveSamplingFloat_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_convexhullApproximation	= PullIntPRM(detail, UI::approximateConvexHullsToggle_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_projectHullVertices		= PullIntPRM(detail, UI::projectHullVerticesToggle_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_pca						= PullIntPRM(detail, UI::normalizeMeshToggle_Parameter, !this->_allowParametersOverrideValueState, time);

	PRM_ACCESS::Get::IntPRM(this, this->_parametersVHACD.m_oclAcceleration, UI::useOpenCLToggle_Parameter, time);

	// debug parameters
#define VHACD_ReportModeHelper(showmsg, showoverallprogress, showstageprogress, showoperationprogress) this->_loggerVHACD.showMsg = showmsg; this->_callbackVHACD.showOverallProgress = showoverallprogress; this->_callbackVHACD.showStageProgress = showstageprogress; this->_callbackVHACD.showOperationProgress = showoperationprogress;	
	PRM_ACCESS::Get::IntPRM(this, this->_showReportValueState, UI::showProcessReportToggle_Parameter, time);
	if (this->_showReportValueState)
	{		
		PRM_ACCESS::Get::IntPRM(this, this->_reportModeChoiceValueState, UI::reportModeChoiceMenu_Parameter, time);
		switch (this->_reportModeChoiceValueState)
		{
			case static_cast<exint>(ENUMS::ReportModeOption::PROGESS_ONLY):		{ VHACD_ReportModeHelper(false, true, true, true) } break;
			case static_cast<exint>(ENUMS::ReportModeOption::DETAILS_ONLY):		{ VHACD_ReportModeHelper(true, false, false, false) } break;
			case static_cast<exint>(ENUMS::ReportModeOption::FULL):				{ VHACD_ReportModeHelper(true, true, true, true) } break;
		}
	}
	else { VHACD_ReportModeHelper(false, false, false, false) }
#undef VHACD_ReportModeHelper
}

ENUMS::MethodProcessResult
SOP_Operator::GatherDataForVHACD(GU_Detail* detail, UT_AutoInterrupt progress, fpreal time)
{
	/*
		Store data as indexed face set representation.
		We need three times more to handle cases where a different list of triangles is defined for positions, normals and texture coordinates.
		Not that I use it here, but it seems to be required.
	*/

	// clear containers to make sure nothing is cached
	this->_pointPositions.clear();
	this->_triangleIndexes.clear();

	// get position attribute
	const auto success = ATTRIB_ACCESS::Find::Vec3ATT(this, detail, GA_AttributeOwner::GA_ATTRIB_POINT, "P", this->_positionHandle);
	if (success && error() < UT_ErrorSeverity::UT_ERROR_WARNING)
	{
		// store positions, as continuous list from 0 to last, without breaking it per primitive		
		this->_pointPositions.reserve(detail->getNumPoints() * 3);
		
		for (auto pointIt = GA_Iterator(detail->getPointRange()); !pointIt.atEnd(); pointIt.advance())
		{
			PROGRESS_WAS_INTERRUPTED_WITH_WARNING_AND_BOOL(this, progress, ENUMS::MethodProcessResult::INTERRUPT)

			auto currentPosition = this->_positionHandle.get(*pointIt);

			this->_pointPositions.push_back(currentPosition.x());
			this->_pointPositions.push_back(currentPosition.y());
			this->_pointPositions.push_back(currentPosition.z());
		}

		// store indexes, as (0, 1, 2) for each triangle
		this->_triangleIndexes.reserve(detail->getNumPrimitives() * 3);
		
		for (auto polyIt = GA_Iterator(detail->getPrimitiveRange()); !polyIt.atEnd(); polyIt.advance())
		{			
			PROGRESS_WAS_INTERRUPTED_WITH_WARNING_AND_BOOL(this, progress, ENUMS::MethodProcessResult::INTERRUPT)

			const auto currPrim = detail->getPrimitive(*polyIt);
			for (auto i = 0; i < currPrim->getVertexCount(); ++i) this->_triangleIndexes.push_back(currPrim->getPointIndex(i));
		}
	}
	else
	{
		this->addError(SOP_MESSAGE, "Failed to find position attribute.");
		return ENUMS::MethodProcessResult::FAILURE;
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::DrawConvexHull(GU_Detail* detail, const VHACD::IVHACD::ConvexHull hull, UT_AutoInterrupt progress)
{
	// add amount of points that hull consists
	const auto start = detail->appendPointBlock(hull.m_nPoints);

	// make sure that we have at least 4 points, if we have less, than it's a flat geometry, so ignore it
	if (UTILS::GU_DetailTester::IsEnoughPoints(this, detail) && error() >= UT_ErrorSeverity::UT_ERROR_WARNING) return ENUMS::MethodProcessResult::FAILURE;

	// set point positions				
	exint hullP = 0;
	GA_OffsetArray	pointOffsets;
	pointOffsets.clear();

	auto pointIt = GA_Iterator(detail->getPointRangeSlice(detail->pointIndex(start)));
	while (!pointIt.atEnd())
	{
		// make sure we can escape the loop
		if (progress.wasInterrupted())
		{
			this->_interfaceVHACD->Cancel();
			addWarning(SOP_ErrorCodes::SOP_MESSAGE, "Operation interrupted");
			return ENUMS::MethodProcessResult::FAILURE;
		}

		const auto currentPosition = UT_Vector3R(hull.m_points[hullP], hull.m_points[hullP + 1], hull.m_points[hullP + 2]);
		this->_positionHandle.set(*pointIt, currentPosition);

		hullP += 3;
		pointOffsets.append(*pointIt);

		pointIt.advance();
	}

	// draw hull
	for (unsigned int t = 0; t < hull.m_nTriangles * 3; t += 3)
	{
		// make sure we can escape the loop
		if (progress.wasInterrupted())
		{
			this->_interfaceVHACD->Cancel();
			addWarning(SOP_ErrorCodes::SOP_MESSAGE, "Operation interrupted");
			return ENUMS::MethodProcessResult::FAILURE;
		}

		// create triangle
		auto polygon = static_cast<GEO_PrimPoly*>(detail->appendPrimitive(GEO_PRIMPOLY));
		polygon->setSize(0);

		polygon->appendVertex(pointOffsets[hull.m_triangles[t + 2]]);
		polygon->appendVertex(pointOffsets[hull.m_triangles[t + 1]]);
		polygon->appendVertex(pointOffsets[hull.m_triangles[t + 0]]);

		polygon->close();

		// set 'hull_volume' and 'hull_center' attributes
		const auto currPolyOffset = polygon->getMapOffset();
		this->_hullVolumeHandle.set(currPolyOffset, hull.m_volume);
		this->_hullCenterHandle.set(currPolyOffset, UT_Vector3(hull.m_center[0], hull.m_center[1], hull.m_center[2]));
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::GenerateConvexHulls(GU_Detail* detail, UT_AutoInterrupt progress)
{
	// get interface	
	this->_interfaceVHACD = VHACD::CreateVHACD();
	//this->_interfaceVHACD = VHACD::CreateVHACD_ASYNC();

	const auto success = this->_interfaceVHACD->Compute( &this->_pointPositions[0], static_cast<unsigned int>(this->_pointPositions.size()) / 3, reinterpret_cast<const uint32_t*>(&this->_triangleIndexes[0]), static_cast<unsigned int>(this->_triangleIndexes.size()) / 3, this->_parametersVHACD);

	/*
		TODO:
		This is shitty hack, just to use async version of this->_interfaceVHACD
		Figure out hot to do asynch await in node.

		Can it be done at all ?! x_X
	*/
	//while (!this->_interfaceVHACD->IsReady()) PROGRESS_WAS_INTERRUPTED_WITH_OBJECT(this, progress, error())

	if (success)
	{
		//if (this->_interfaceVHACD->IsReady())
		{
			// add hull attributes
			this->_hullVolumeHandle = GA_RWHandleD(detail->addFloatTuple(GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_VOLUME), 1));
			if (this->_hullVolumeHandle.isInvalid())
			{
				auto errorMessage = std::string("Failed to create \"") + std::string(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_VOLUME)) + std::string("\" attribute.");
				this->addError(SOP_MESSAGE, errorMessage.c_str());
				return ENUMS::MethodProcessResult::FAILURE;
			}

			this->_hullCenterHandle = GA_RWHandleV3(detail->addFloatTuple(GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_CENTER), 3));
			if (this->_hullCenterHandle.isInvalid())
			{
				auto errorMessage = std::string("Failed to create \"") + std::string(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_CENTER)) + std::string("\" attribute.");
				this->addError(SOP_MESSAGE, errorMessage.c_str());
				return ENUMS::MethodProcessResult::FAILURE;
			}

			const auto hullCount = this->_interfaceVHACD->GetNConvexHulls();

			// generate hulls
			for (auto id = 0; id < hullCount; ++id)
			{
				// make sure we can escape the loop
				if (progress.wasInterrupted())
				{
					this->_interfaceVHACD->Cancel();
					addWarning(SOP_ErrorCodes::SOP_MESSAGE, "Operation interrupted");
					return ENUMS::MethodProcessResult::INTERRUPT;
				}

				// draw hull
				VHACD::IVHACD::ConvexHull currentHull;

				this->_interfaceVHACD->GetConvexHull(id, currentHull);
				if (!currentHull.m_nPoints || !currentHull.m_nTriangles) continue;

				const auto processResult = DrawConvexHull(detail, currentHull, progress);
				if (processResult != ENUMS::MethodProcessResult::SUCCESS && error() > UT_ErrorSeverity::UT_ERROR_NONE) return processResult;
			}
		}
	}
	else
	{
		addError(SOP_ErrorCodes::SOP_MESSAGE, "Computation failed");
		return ENUMS::MethodProcessResult::FAILURE;
	}

	this->_interfaceVHACD->Clean();
	this->_interfaceVHACD->Release();

	return ENUMS::MethodProcessResult::SUCCESS;
}

/* -----------------------------------------------------------------
MAIN                                                               |
----------------------------------------------------------------- */

OP_ERROR 
SOP_Operator::cookMySop(OP_Context& context)
{
	DEFAULTS_CookMySop()
		
	if (duplicateSource(0, context) < OP_ERROR::UT_ERROR_WARNING && error() < OP_ERROR::UT_ERROR_WARNING)
	{		
		// we should have only polygons now, but we need to make sure that they are all correct			
		auto processResult = PrepareGeometry(this->gdp, progress, currentTime);
		if ((processResult == ENUMS::MethodProcessResult::SUCCESS && error() >= OP_ERROR::UT_ERROR_WARNING) || (processResult != ENUMS::MethodProcessResult::SUCCESS && error() >= OP_ERROR::UT_ERROR_NONE)) return error();

		// get parameters
		SetupParametersVHACD(this->gdp, currentTime);

		// prepare data for V-HACD
		processResult = GatherDataForVHACD(this->gdp, progress, currentTime);
		if ((processResult == ENUMS::MethodProcessResult::SUCCESS && error() >= OP_ERROR::UT_ERROR_WARNING) || (processResult != ENUMS::MethodProcessResult::SUCCESS && error() >= OP_ERROR::UT_ERROR_NONE)) return error();

		// lets make some hulls!
		this->gdp->clear();
			
		processResult = GenerateConvexHulls(this->gdp, progress);
		if ((processResult == ENUMS::MethodProcessResult::SUCCESS && error() >= OP_ERROR::UT_ERROR_WARNING) || (processResult != ENUMS::MethodProcessResult::SUCCESS && error() >= OP_ERROR::UT_ERROR_NONE)) return error();
	}

	return error();
}

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS

#undef ATTRIB_ACCESS
#undef PRM_ACCESS
#undef UTILS

#undef UI
#undef COMMON_NAMES

#undef SOP_InputName_0
#undef SOP_Base_Operator
#undef SOP_Operator