/*
	Volumetric-Hierarchical Approximate Convex Decomposition.
	Based on https://github.com/kmammou/v-hacd

	IMPORTANT! ------------------------------------------
	* Macros starting with '____' shouldn't be used anywhere outside of this file.
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
#include <CH/CH_Manager.h>
#include <PRM/PRM_Parm.h>
#include <PRM/PRM_Error.h>
#include <PRM/PRM_Include.h>

#include <UT/UT_Interrupt.h>
#include <GEO/GEO_ConvertParms.h>
#include <OP/OP_AutoLockInputs.h>

// hou-hdk-common
#include <Macros/ParameterList.h>
#include <Utility/GeometryProcessing.h>
#include <Utility/AttributeAccessing.h>
#include <Utility/ParameterAccessing.h>

#include <Enums/NodeErrorLevel.h>
#include <Enums/AttributeClass.h>

// this
#include "Parameters.h"
#include "ReportModeOption.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator					GET_SOP_Namespace()::SOP_VHACDEngine
#define SOP_Base_Operator				SOP_Node
#define SOP_InputName_0					"Geometry"

#define UI								GET_SOP_Namespace()::PRMs_VHACDEngine
#define PRM_ACCESS						GET_Base_Namespace()::Utility::PRM
#define ATTRIB_ACCESS					GET_Base_Namespace()::Utility::Attribute
#define GDP_UTILS						GET_Base_Namespace()::Utility::Geometry
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
{ op->setIconName(UI::names.Get(CommonNameOption::ICON_NAME)); }

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
SOP_Operator::PullIntPRM(GU_Detail* geometry, const PRM_Template& parameter, bool interfaceonly /* = false */, fpreal time /* = 0 */)
{
	exint currentIntValue = 0;

	if (!interfaceonly)
	{
		GA_RWHandleI attributeHandle;

		// check is there attribute with the name that matches parameter name
		// if it exist, use it as parm value, otherwise use UI value		
		const auto success = ATTRIB_ACCESS::Find::IntATT(this, geometry, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, parameter.getToken(), attributeHandle, ENUMS::NodeErrorLevel::NONE);
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
SOP_Operator::PullFloatPRM(GU_Detail* geometry, const PRM_Template& parameter, bool interfaceonly /* = false */, fpreal time /* = 0 */)
{
	fpreal currentFloatValue = 0;

	if (!interfaceonly)
	{
		//GA_RWAttributeRef attributeReference;
		GA_RWHandleR attributeHandle;

		// check is there attribute with the name that matches parameter name
		// if it exist, use it as parm value, otherwise use UI value		
		const auto success = ATTRIB_ACCESS::Find::FloatATT(this, geometry, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, parameter.getToken(), attributeHandle, ENUMS::NodeErrorLevel::NONE);
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

bool 
SOP_Operator::PrepareGeometry(GU_Detail* geometry, UT_AutoInterrupt progress)
{
	auto success = GDP_UTILS::Modify::Triangulate(this, geometry, SMALLPOLYGONS_TOLERANCE, progress);
	if ((success && error() >= UT_ErrorSeverity::UT_ERROR_WARNING) || (!success && error() >= UT_ErrorSeverity::UT_ERROR_NONE)) return false;
	
	// is there anything left after preparation?	
	auto message = UT_String("After removing zero area and open polygons there are no other primitives left.");
	success = GDP_UTILS::Test::IsEnoughPrimitives(this, geometry, 1, message);
	if ((success && error() >= UT_ErrorSeverity::UT_ERROR_WARNING) || (!success && error() >= UT_ErrorSeverity::UT_ERROR_NONE)) return false;

	message = UT_String("Not enough points to create hull.");
	return	GDP_UTILS::Test::IsEnoughPoints(this, gdp, 4, message);	
}

void 
SOP_Operator::SetupVHACD(GU_Detail* geometry, fpreal time)
{		
	PRM_ACCESS::Get::IntPRM(this, this->_allowParametersOverrideValueState, UI::allowParametersOverrideToggle_Parameter, time);

	// main parameters
	this->_parametersVHACD.m_mode						= PullIntPRM(geometry, UI::modeChoiceMenu_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_resolution					= PullIntPRM(geometry, UI::resolutionInteger_Parameter, !this->_allowParametersOverrideValueState, time);	
	this->_parametersVHACD.m_concavity					= PullFloatPRM(geometry, UI::concavityFloat_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_planeDownsampling			= PullIntPRM(geometry, UI::planeDownsamplingInteger_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_convexhullDownsampling		= PullIntPRM(geometry, UI::convexHullDownsamplingInteger_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_alpha						= PullFloatPRM(geometry, UI::alphaFloat_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_beta						= PullFloatPRM(geometry, UI::betaFloat_Parameter, !this->_allowParametersOverrideValueState, time);	
	this->_parametersVHACD.m_maxConvexHulls				= PullIntPRM(geometry, UI::maxConvexHullsCountInteger_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_maxNumVerticesPerCH		= PullIntPRM(geometry, UI::maxTriangleCountInteger_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_minVolumePerCH				= PullFloatPRM(geometry, UI::adaptiveSamplingFloat_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_convexhullApproximation	= PullIntPRM(geometry, UI::approximateConvexHullsToggle_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_projectHullVertices		= PullIntPRM(geometry, UI::projectHullVerticesToggle_Parameter, !this->_allowParametersOverrideValueState, time);
	this->_parametersVHACD.m_pca						= PullIntPRM(geometry, UI::normalizeMeshToggle_Parameter, !this->_allowParametersOverrideValueState, time);	

	PRM_ACCESS::Get::IntPRM(this, this->_parametersVHACD.m_oclAcceleration, UI::useOpenCLToggle_Parameter, time);

	// debug parameters
#define VHACD_ReportModeHelper(showmsg, showoverallprogress, showstageprogress, showoperationprogress) this->_loggerVHACD.showMsg = showmsg; this->_callbackVHACD.showOverallProgress = showoverallprogress; this->_callbackVHACD.showStageProgress = showstageprogress; this->_callbackVHACD.showOperationProgress = showoperationprogress;	
	PRM_ACCESS::Get::IntPRM(this, this->_showReportValueState, UI::showProcessReportToggle_Parameter, time);
	if (this->_showReportValueState)
	{		
		PRM_ACCESS::Get::IntPRM(this, this->_reportModeChoiceValueState, UI::reportModeChoiceMenu_Parameter, time);
		switch (this->_reportModeChoiceValueState)
		{
			case static_cast<exint>(ENUMS::ReportModeOption::PROGESS_ONLY): { VHACD_ReportModeHelper(false, true, true, true) } break;
			case static_cast<exint>(ENUMS::ReportModeOption::DETAILS_ONLY): { VHACD_ReportModeHelper(true, false, false, false) } break;
			case static_cast<exint>(ENUMS::ReportModeOption::FULL): { VHACD_ReportModeHelper(true, true, true, true) } break;
		}
	}
	else { VHACD_ReportModeHelper(false, false, false, false) }
#undef VHACD_ReportModeHelper
}

bool 
SOP_Operator::PrepareDataForVHACD(GU_Detail* geometry, UT_AutoInterrupt progress, fpreal time)
{
	/*
		Store data as indexed face set representation.
		We need three times more to handle cases where a different list of triangles is defined for positions, normals and texture coordinates.
		Not that I use it here, but it seems to be required.
	*/

	// clear containers to make sure nothing is cached
	this->_points.clear();
	this->_triangles.clear();

	// get position attribute
	const auto success = ATTRIB_ACCESS::Find::Vec3ATT(this, geometry, GA_AttributeOwner::GA_ATTRIB_POINT, "P", this->_positionHandle);
	if (success && error() < UT_ErrorSeverity::UT_ERROR_WARNING)
	{
		// store positions, as continuous list from 0 to last, without breaking it per primitive		
		this->_points.reserve(geometry->getNumPoints() * 3);

		auto pointIt = GA_Iterator(geometry->getPointRange());
		for (; !pointIt.atEnd(); pointIt.advance())
		{
			// make sure we can escape the loop
			if (progress.wasInterrupted())
			{
				addWarning(SOP_ErrorCodes::SOP_MESSAGE, "Operation interrupted");
				return false;
			}

			auto currentPosition = this->_positionHandle.get(*pointIt);

			this->_points.push_back(currentPosition.x());
			this->_points.push_back(currentPosition.y());
			this->_points.push_back(currentPosition.z());
		}

		// store indexes, as (0, 1, 2) for each triangle
		this->_triangles.reserve(geometry->getNumPrimitives() * 3);

		auto polyIt = GA_Iterator(geometry->getPrimitiveRange());
		for (; !polyIt.atEnd(); polyIt.advance())
		{
			// make sure we can escape the loop
			if (progress.wasInterrupted())
			{
				addWarning(SOP_ErrorCodes::SOP_MESSAGE, "Operation interrupted");
				return false;
			}

			const auto currPrim = geometry->getPrimitive(*polyIt);
			for (auto i = 0; i < currPrim->getVertexCount(); ++i) this->_triangles.push_back(currPrim->getPointIndex(i));
		}
	}
	else return false;

	return true;
}

bool
SOP_Operator::DrawConvexHull(GU_Detail* geometry, VHACD::IVHACD::ConvexHull hull, UT_AutoInterrupt progress)
{
	// add amount of points that hull consists
	const auto start = geometry->appendPointBlock(hull.m_nPoints);

	// make sure that we have at least 4 points, if we have less, than it's a flat geometry, so ignore it
	if (GDP_UTILS::Test::IsEnoughPoints(this, geometry) && error() >= UT_ErrorSeverity::UT_ERROR_WARNING) return false;

	// set point positions				
	exint hullP = 0;
	std::vector<GA_Offset> pOffsets;
	pOffsets.clear();

	auto pointIt = GA_Iterator(geometry->getPointRangeSlice(gdp->pointIndex(start)));
	while (!pointIt.atEnd())
	{
		// make sure we can escape the loop
		if (progress.wasInterrupted())
		{
			this->_interfaceVHACD->Cancel();
			addWarning(SOP_ErrorCodes::SOP_MESSAGE, "Operation interrupted");
			return false;
		}

		const auto currentPosition = UT_Vector3R(hull.m_points[hullP], hull.m_points[hullP + 1], hull.m_points[hullP + 2]);
		this->_positionHandle.set(*pointIt, currentPosition);

		hullP += 3;
		pOffsets.push_back(*pointIt);
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
			return false;
		}

		// create triangle
		auto polygon = static_cast<GEO_PrimPoly*>(geometry->appendPrimitive(GEO_PRIMPOLY));
		polygon->setSize(0);

		polygon->appendVertex(pOffsets[hull.m_triangles[t + 2]]);
		polygon->appendVertex(pOffsets[hull.m_triangles[t + 1]]);
		polygon->appendVertex(pOffsets[hull.m_triangles[t + 0]]);				

		polygon->close();
	}

	return true;
}

OP_ERROR 
SOP_Operator::GenerateConvexHulls(GU_Detail* geometry, UT_AutoInterrupt progress)
{
	// get interface
	this->_interfaceVHACD = VHACD::CreateVHACD();
	
	auto success = this->_interfaceVHACD->Compute( &this->_points[0], static_cast<unsigned int>(this->_points.size()) / 3, reinterpret_cast<const uint32_t*>(&this->_triangles[0]), static_cast<unsigned int>(this->_triangles.size()) / 3, this->_parametersVHACD);
	if (success)
	{
		const auto hullCount = this->_interfaceVHACD->GetNConvexHulls();

		// generate hulls
		for (auto id = 0; id < hullCount; ++id)
		{
			// make sure we can escape the loop
			if (progress.wasInterrupted())
			{
				this->_interfaceVHACD->Cancel();
				addWarning(SOP_ErrorCodes::SOP_MESSAGE, "Operation interrupted");
				return this->error();
			}

			// draw hull
			VHACD::IVHACD::ConvexHull currentHull;

			this->_interfaceVHACD->GetConvexHull(id, currentHull);
			if (!currentHull.m_nPoints || !currentHull.m_nTriangles) continue;

			success = DrawConvexHull(geometry, currentHull, progress);
			if (!success && error() > UT_ErrorSeverity::UT_ERROR_NONE) return error();
		}		
	}
	else
	{
		addError(SOP_ErrorCodes::SOP_MESSAGE, "Computation failed");
		return error();
	}

	this->_interfaceVHACD->Clean();
	this->_interfaceVHACD->Release();

	return error();
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
		// make sure we got something to work on
		auto message = UT_String("Not enough primitives to create hull.");
		auto success = GDP_UTILS::Test::IsEnoughPrimitives(this, gdp, 1, message);
		if ((success && error() >= OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return error();
			
		// do we want only polygons or do we try to convert anything to polygons?			
		bool convertToPolygonsState;
		PRM_ACCESS::Get::IntPRM(this, convertToPolygonsState, UI::convertToPolygonsToggle_Parameter, currentTime);
		if (convertToPolygonsState)
		{
			GEO_ConvertParms parms;
			gdp->convert(parms);
		}
		else
		{
			for (auto primIt : gdp->getPrimitiveRange())
			{
				// make sure we can escape the loop
				if (progress.wasInterrupted())
				{
					addError(SOP_ErrorCodes::SOP_MESSAGE, "Operation interrupted");
					return error();
				}

				// only polygons allowed
				if (gdp->getPrimitive(primIt)->getTypeId() != GA_PRIMPOLY)
				{
					addError(SOP_ErrorCodes::SOP_MESSAGE, "Non-polygon geometry detected on input.");
					return error();
				}
			}
		}

		// we need at least 4 points to get up from bed
		message = UT_String("Not enough points to create hull.");
		success = GDP_UTILS::Test::IsEnoughPoints(this, gdp, 4, message);
		if ((success && error() >= OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return error();
			
		// we should have only polygons now, but we need to make sure that they are all correct			
		success = PrepareGeometry(gdp, progress);
		if ((success && error() >= OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return error();

		// get parameters and logger/callback
		SetupVHACD(gdp, currentTime);
		this->_parametersVHACD.m_logger = &this->_loggerVHACD;
		this->_parametersVHACD.m_callback = &this->_callbackVHACD;

		// prepare data for V-HACD
		success = PrepareDataForVHACD(gdp, progress, currentTime);
		if ((success && error() >= OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return error();

		// lets make some hulls!
		gdp->clear();
			
		if (GenerateConvexHulls(gdp, progress) >= OP_ERROR::UT_ERROR_WARNING && error() >= OP_ERROR::UT_ERROR_WARNING) return error();
	}

	return error();
}

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef GDP_UTILS
#undef ATTRIB_ACCESS
#undef PRM_ACCESS
#undef UI

#undef SOP_InputName_0
#undef SOP_Base_Operator
#undef SOP_Operator