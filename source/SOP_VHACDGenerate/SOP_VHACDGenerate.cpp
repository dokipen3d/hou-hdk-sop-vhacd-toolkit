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
#include <GEO/GEO_ConvertParms.h>
#include <GEO/GEO_PrimClassifier.h>

// hou-hdk-common
#include <Macros/ParameterList.h>
#include <Macros/ProgressEscape.h>
#include <Utility/PRM_TemplateAccessors.h>
#include <Utility/GA_AttributeAccessors.h>
#include <Utility/GU_DetailModifier.h>
#include <Utility/GU_DetailTester.h>

// this
#include "Parameters.h"
#include "ProcessModeOption.h"
#include "ReportModeOption.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator			GET_SOP_Namespace()::SOP_VHACDGenerate
#define SOP_Base_Operator		SOP_VHACDNode
#define MSS_Selector			GET_SOP_Namespace()::MSS_VHACDGenerate

// very important
#define SOP_GroupFieldIndex_0	1

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
	UI::input0PrimitiveGroup_Parameter,
	UI::processModeChoiceMenu_Parameter,
	UI::forceConvertToPolygonsToggle_Parameter,
	UI::forceConvertToPolygonsSeparator_Parameter,

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
	bool showReportValueState;
	PRM_ACCESS::Get::IntPRM(this, showReportValueState, UI::showProcessReportToggle_Parameter, currentTime);
	changed |= setVisibleState(UI::reportModeChoiceMenu_Parameter.getToken(), showReportValueState);

	// TODO: remove those when convert parameters from attributes will be added
	changed |= setVisibleState(UI::forceConvertToPolygonsToggle_Parameter.getToken(), false);
	changed |= setVisibleState(UI::forceConvertToPolygonsSeparator_Parameter.getToken(), false);

	// update description active state
	UPDATE_DescriptionPRM_ActiveState(this, UI)

	return changed;
}

/* -----------------------------------------------------------------
CALLBACKS                                                          |
----------------------------------------------------------------- */

IMPLEMENT_DescriptionPRM_Callback(SOP_Operator, UI)

int
SOP_Operator::CallbackShowProcessReport(void* data, int index, float time, const PRM_Template* tmp)
{
	const auto me = reinterpret_cast<SOP_Operator*>(data);
	if (!me) return 0;

	// TODO: figure out why restoreFactoryDefaults() doesn't work
	auto defVal = static_cast<exint>(UI::reportModeChoiceMenu_Parameter.getFactoryDefaults()->getOrdinal());
	PRM_ACCESS::Set::IntPRM(me, defVal, UI::reportModeChoiceMenu_Parameter, time);

	return 1;
}

/* -----------------------------------------------------------------
OPERATOR INITIALIZATION                                            |
----------------------------------------------------------------- */

SOP_Operator::~SOP_VHACDGenerate() { }

SOP_Operator::SOP_VHACDGenerate(OP_Network* network, const char* name, OP_Operator* op) :
SOP_Base_Operator(network, name, op), 
_inputGDP(nullptr),
_primitiveGroupInput0(nullptr),
_interfaceVHACD(nullptr)
{
	op->setIconName(COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_GENERATE_ICONNAME));

	exint toggleFlag = 1;
	auto message = UT_String("Version 2.0 is still work in progress. \nThis is full C++ rewrite of Generate node. \nOpen issue if you found some error https://github.com/sebastianswann/hou-hdk-sop-vhacd-toolkit/issues.");
	for (auto p : parametersList)
	{
		if (p.getNamePtr()->getToken() == UI::descriptionToggle_Parameter.getToken()) PRM_ACCESS::Set::IntPRM(this, toggleFlag, p);
		else if (p.getNamePtr()->getToken() == UI::descriptionTextField_Parameter.getToken()) PRM_ACCESS::Set::StringPRM(this, message, p);
	}
}

OP_Node* 
SOP_Operator::CreateMe(OP_Network* network, const char* name, OP_Operator* op) 
{ return new SOP_Operator(network, name, op); }

const char* 
SOP_Operator::inputLabel(unsigned input) const
{ return std::to_string(input).c_str(); }

OP_ERROR
SOP_Operator::cookInputGroups(OP_Context& context, int alone)
{	
	const auto isOrdered = true;	
	this->_gop = GroupCreator(this->_inputGDP);
	
	return cookInputPrimitiveGroups(context, this->_primitiveGroupInput0, alone, true, SOP_GroupFieldIndex_0, -1, true, isOrdered, this->_gop);
}

/* -----------------------------------------------------------------
HELPERS                                                            |
----------------------------------------------------------------- */

exint
SOP_Operator::PullIntPRM(GU_Detail* geometry, const PRM_Template& parameter, fpreal time)
{
	exint currentIntValue = 0;

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

	return currentIntValue;
}

fpreal
SOP_Operator::PullFloatPRM(GU_Detail* geometry, const PRM_Template& parameter, fpreal time)
{
	fpreal currentFloatValue = 0;

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
	PRM_ACCESS::Get::IntPRM(this, convertToPolygonsState, UI::forceConvertToPolygonsToggle_Parameter, time);
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
	// logger & callback
	this->_parametersVHACD.m_logger = &this->_loggerVHACD;
	this->_parametersVHACD.m_callback = &this->_callbackVHACD;

	// main parameters	
	this->_parametersVHACD.m_mode =						PullIntPRM(detail, UI::modeChoiceMenu_Parameter, time);
	this->_parametersVHACD.m_resolution =				PullIntPRM(detail, UI::resolutionInteger_Parameter, time);
	this->_parametersVHACD.m_concavity =				PullFloatPRM(detail, UI::concavityFloat_Parameter, time);
	this->_parametersVHACD.m_planeDownsampling =		PullIntPRM(detail, UI::planeDownsamplingInteger_Parameter, time);
	this->_parametersVHACD.m_convexhullDownsampling =	PullIntPRM(detail, UI::convexHullDownsamplingInteger_Parameter, time);
	this->_parametersVHACD.m_alpha =					PullFloatPRM(detail, UI::alphaFloat_Parameter, time);
	this->_parametersVHACD.m_beta =						PullFloatPRM(detail, UI::betaFloat_Parameter, time);
	this->_parametersVHACD.m_maxConvexHulls =			PullIntPRM(detail, UI::maxConvexHullsCountInteger_Parameter, time);
	this->_parametersVHACD.m_maxNumVerticesPerCH =		PullIntPRM(detail, UI::maxTriangleCountInteger_Parameter, time);
	this->_parametersVHACD.m_minVolumePerCH =			PullFloatPRM(detail, UI::adaptiveSamplingFloat_Parameter, time);
	this->_parametersVHACD.m_convexhullApproximation =	PullIntPRM(detail, UI::approximateConvexHullsToggle_Parameter, time);
	this->_parametersVHACD.m_projectHullVertices =		PullIntPRM(detail, UI::projectHullVerticesToggle_Parameter, time);
	this->_parametersVHACD.m_pca =						PullIntPRM(detail, UI::normalizeMeshToggle_Parameter, time);

	PRM_ACCESS::Get::IntPRM(this, this->_parametersVHACD.m_oclAcceleration, UI::useOpenCLToggle_Parameter, time);
		
	// debug parameters
#define VHACD_ReportModeHelper(showmsg, showoverallprogress, showstageprogress, showoperationprogress) this->_loggerVHACD.showMsg = showmsg; this->_callbackVHACD.showOverallProgress = showoverallprogress; this->_callbackVHACD.showStageProgress = showstageprogress; this->_callbackVHACD.showOperationProgress = showoperationprogress;

	exint showReportValueState;
	PRM_ACCESS::Get::IntPRM(this, showReportValueState, UI::showProcessReportToggle_Parameter, time);
	if (showReportValueState)
	{
		exint reportModeChoiceValueState;
		PRM_ACCESS::Get::IntPRM(this, reportModeChoiceValueState, UI::reportModeChoiceMenu_Parameter, time);
		switch (reportModeChoiceValueState)
		{
			case static_cast<exint>(ENUMS::ReportModeOption::PROGESS_ONLY) :	{ VHACD_ReportModeHelper(false, true, true, true) } break;
			case static_cast<exint>(ENUMS::ReportModeOption::DETAILS_ONLY) :	{ VHACD_ReportModeHelper(true, false, false, false) } break;
			case static_cast<exint>(ENUMS::ReportModeOption::FULL) :			{ VHACD_ReportModeHelper(true, true, true, true) } break;
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
	if (success)
	{
		// store positions, as continuous list from 0 to last, without breaking it per primitive		
		this->_pointPositions.reserve(detail->getNumPoints() * 3);

		for (auto pointIt = GA_Iterator(detail->getPointRange()); !pointIt.atEnd(); pointIt.advance())
		{
			PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::FAILURE)

			auto currentPosition = this->_positionHandle.get(*pointIt);

			this->_pointPositions.push_back(currentPosition.x());
			this->_pointPositions.push_back(currentPosition.y());
			this->_pointPositions.push_back(currentPosition.z());
		}

		// store indexes, as (0, 1, 2) for each triangle
		this->_triangleIndexes.reserve(detail->getNumPrimitives() * 3);

		for (auto polyIt = GA_Iterator(detail->getPrimitiveRange()); !polyIt.atEnd(); polyIt.advance())
		{
			PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::FAILURE)

			const auto currPrim = detail->getPrimitive(*polyIt);
			for (auto i = 0; i < currPrim->getVertexCount(); ++i) this->_triangleIndexes.push_back(currPrim->getPointIndex(i));
		}
	}
	else
	{
		addError(SOP_MESSAGE, "Failed to find position attribute.");
		return ENUMS::MethodProcessResult::FAILURE;
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::DrawConvexHull(GU_Detail* detail, const VHACD::IVHACD::ConvexHull& hull, UT_AutoInterrupt progress)
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
	
	// calculate 'bundle_mass_center' center	
	double centerOfMass[3];
	const auto addMassCenter = this->_interfaceVHACD->ComputeCenterOfMass(centerOfMass);

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

		if (this->_hullMassCenterHandle.isValid()) this->_hullMassCenterHandle.set(currPolyOffset, UT_Vector3(hull.m_center[0], hull.m_center[1], hull.m_center[2]));
		if (this->_bundleMassCenterHandle.isValid() && addMassCenter) this->_bundleMassCenterHandle.set(currPolyOffset, UT_Vector3(centerOfMass[0], centerOfMass[1], centerOfMass[2]));
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::GenerateConvexHulls(GU_Detail* detail, UT_AutoInterrupt progress)
{
	// get interface	
	this->_interfaceVHACD = VHACD::CreateVHACD_ASYNC();
	//this->_interfaceVHACD = VHACD::CreateVHACD();
	const auto success = this->_interfaceVHACD->Compute(&this->_pointPositions[0], static_cast<unsigned int>(this->_pointPositions.size()) / 3, reinterpret_cast<const uint32_t*>(&this->_triangleIndexes[0]), static_cast<unsigned int>(this->_triangleIndexes.size()) / 3, this->_parametersVHACD);
	
	/*		
		TODO:	
		This is shitty hack, just to use async version of this->_interfaceVHACD
		Figure out hot to do asynch await in node. 

		Can it be done at all ?! x_X
	 */
	while (!this->_interfaceVHACD->IsReady()) PROGRESS_WAS_INTERRUPTED_WITH_OBJECT(this, progress, ENUMS::MethodProcessResult::FAILURE)

	if (success)
	{
		if (this->_interfaceVHACD->IsReady())
		{
			// add hull/bundle attributes
			this->_hullMassCenterHandle = GA_RWHandleV3(detail->addFloatTuple(GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_MASS_CENTER), 3));
			if (this->_hullMassCenterHandle.isInvalid())
			{
				auto errorMessage = std::string("Failed to create \"") + std::string(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::HULL_MASS_CENTER)) + std::string("\" attribute.");
				addError(SOP_MESSAGE, errorMessage.c_str());
				return ENUMS::MethodProcessResult::FAILURE;
			}

			this->_bundleMassCenterHandle = GA_RWHandleV3(detail->addFloatTuple(GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_MASS_CENTER), 3));
			if (this->_bundleMassCenterHandle.isInvalid())
			{
				auto errorMessage = std::string("Failed to create \"") + std::string(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_MASS_CENTER)) + std::string("\" attribute.");
				addError(SOP_MESSAGE, errorMessage.c_str());
				return ENUMS::MethodProcessResult::FAILURE;
			}
			
			// generate hulls
			const auto hullCount = this->_interfaceVHACD->GetNConvexHulls();
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

ENUMS::MethodProcessResult
SOP_Operator::MergeCurrentDetail(const GU_Detail* detail, exint detailscount /* = 1 */, exint iteration /* = 0 */)
{
	auto success = false;

#define THIS_MERGE_FAILURE(node, errormessage) if (!success) { node->addError(SOP_MESSAGE, errormessage); return ENUMS::MethodProcessResult::FAILURE; }

	// merge current detail into main detail
	if (detailscount > 1)
	{
		if (iteration == 0)
		{
			success = this->gdp->copy(*detail, GEO_CopyMethod::GEO_COPY_START);
			THIS_MERGE_FAILURE(this ,"Geometry merge failure on GEO_COPY_START")
		}
		else if (iteration == detailscount - 1)
		{
			success = this->gdp->copy(*detail, GEO_CopyMethod::GEO_COPY_END);
			THIS_MERGE_FAILURE(this, "Geometry merge failure on GEO_COPY_END")
		}
		else
		{
			success = this->gdp->copy(*detail, GEO_CopyMethod::GEO_COPY_ADD);
			THIS_MERGE_FAILURE(this, "Geometry merge failure on GEO_COPY_ADD")
		}
	}
	else
	{
		success = this->gdp->copy(*detail, GEO_CopyMethod::GEO_COPY_ONCE);
		THIS_MERGE_FAILURE(this, "Geometry merge failure on GEO_COPY_ONCE")
	}

#undef THIS_MERGE_FAILURE

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::ProcessCurrentDetail(GU_Detail* detail, UT_AutoInterrupt progress, ENUMS::ProcessedOutputType processedoutputtype, exint iteration, fpreal time)
{
	// handle convex hulls
	if (processedoutputtype == ENUMS::ProcessedOutputType::CONVEX_HULLS)
	{
		// make sure we have proper geometry before we try to gather data for V-HACD
		auto processResult = PrepareGeometry(detail, progress, time);
		if (processResult != ENUMS::MethodProcessResult::SUCCESS || error() > OP_ERROR::UT_ERROR_WARNING) return processResult;

		// setup V-HACD and collect all required data
		SetupParametersVHACD(detail, time);

		processResult = GatherDataForVHACD(detail, progress, time);
		if (processResult != ENUMS::MethodProcessResult::SUCCESS || error() > OP_ERROR::UT_ERROR_WARNING) return processResult;
		
		// lets make some hulls!
		detail->clear();
		
		processResult = GenerateConvexHulls(detail, progress);
		if (processResult != ENUMS::MethodProcessResult::SUCCESS || error() > OP_ERROR::UT_ERROR_WARNING) return processResult;
	}
	
	// add bundle_id attribute
	this->_bundleIDHandle = GA_RWHandleI(detail->addIntTuple(GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID), 1, GA_Defaults(iteration)));
	if (this->_bundleIDHandle.isInvalid())
	{
		auto errorMessage = std::string("Failed to add \"") + std::string(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID)) + std::string("\" attribute.");
		addError(SOP_MESSAGE, errorMessage.c_str());
		return ENUMS::MethodProcessResult::FAILURE;
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::WhenAsWhole(UT_AutoInterrupt progress, ENUMS::ProcessedOutputType processedoutputtype, fpreal time)
{
	auto processResult = ENUMS::MethodProcessResult::SUCCESS;

	// separate range of geometry we want to work on
	if (this->_primitiveGroupInput0 && this->_primitiveGroupInput0->entries() > 0)
	{
		processResult = UTILS::GU_DetailModifier::SeparatePrimitives(this, this->_inputGDP, this->_primitiveGroupInput0);
		if (processResult != ENUMS::MethodProcessResult::SUCCESS || error() > OP_ERROR::UT_ERROR_WARNING) return processResult;
	}

	// this time we do steps backwards, by first merging...
	this->gdp->clear();

	processResult = MergeCurrentDetail(this->_inputGDP);
	if (processResult != ENUMS::MethodProcessResult::SUCCESS) return processResult;
	
	// ... and the processing mesh
	processResult = ProcessCurrentDetail(this->gdp, progress, processedoutputtype, 0, time);

	return processResult;
}

ENUMS::MethodProcessResult
SOP_Operator::WhenPerElement(UT_AutoInterrupt progress, ENUMS::ProcessedOutputType processedoutputtype, fpreal time)
{
	auto processResult = ENUMS::MethodProcessResult::SUCCESS;

	// separate range of geometry we want to work on
	if (this->_primitiveGroupInput0 && this->_primitiveGroupInput0->entries() > 0)
	{
		processResult = UTILS::GU_DetailModifier::SeparatePrimitives(this, this->_inputGDP, this->_primitiveGroupInput0);
		if (processResult != ENUMS::MethodProcessResult::SUCCESS || error() > OP_ERROR::UT_ERROR_WARNING) return processResult;
	}

	// @CLASS of 1999
	auto primClassifier = GEO_PrimClassifier();	
	primClassifier.classifyBySharedPoints(*this->_inputGDP);	
	
	// group primitives by class and...
	this->_gop = GroupCreator(this->_inputGDP);

	UT_Map<exint, GA_PrimitiveGroup*> mappedGroups;
	mappedGroups.clear();

	for (auto primIt = GA_Iterator(this->_inputGDP->getPrimitiveRange()); !primIt.atEnd(); primIt.advance())	
	{
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::FAILURE)
			
		const auto currClass = primClassifier.getClass(primIt.getIndex());
		if (!mappedGroups.contains(currClass)) mappedGroups[currClass] = static_cast<GA_PrimitiveGroup*>(this->_gop.createGroup(GA_GroupType::GA_GROUP_PRIMITIVE));
		
		mappedGroups[currClass]->addOffset(*primIt);
	}
	
	if (mappedGroups.size() < 1)
	{
		addError(SOP_MESSAGE, "There is no geometry to process.");
		return ENUMS::MethodProcessResult::FAILURE;
	}

	// ... then start the real job	
	UT_Array<GU_Detail*> processedDetails;
	exint currIter = 0;

	processedDetails.clear();
	
	for (const auto entry : mappedGroups)
	{
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::FAILURE)

		// get group
		const auto currGroup = entry.second;

		// create separate detail from it
		const auto currDetail = new GU_Detail(this->_inputGDP, currGroup);

		processResult = ProcessCurrentDetail(currDetail, progress, processedoutputtype, currIter, time);
		if (processResult != ENUMS::MethodProcessResult::SUCCESS)
		{
			delete currDetail;
			return processResult;
		}

		processedDetails.append(currDetail);
		currIter++;
	}

	if (processedDetails.size() == 0)
	{
		addError(SOP_MESSAGE, "Failed to process details.");
		return ENUMS::MethodProcessResult::FAILURE;
	}

	// merge all details
	this->gdp->clear();
	currIter = 0;

	for (auto detail : processedDetails)
	{
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::FAILURE)

		processResult = MergeCurrentDetail(detail, processedDetails.size(), currIter);
		if (processResult != ENUMS::MethodProcessResult::SUCCESS) return processResult;

		currIter++;
	}

	return processResult;
}

ENUMS::MethodProcessResult
SOP_Operator::WhenPerGroup(UT_AutoInterrupt progress, ENUMS::ProcessedOutputType processedoutputtype, fpreal time)
{
	auto					processResult = ENUMS::MethodProcessResult::SUCCESS;
	
	UT_String				input0PrimitiveGroupNameValue;
	UT_StringList			tokens;
	UT_Array<GU_Detail*>	processedDetails;
	exint					currIter = 0;

	tokens.clear();
	processedDetails.clear();

	PRM_ACCESS::Get::StringPRM(this, input0PrimitiveGroupNameValue, UI::input0PrimitiveGroup_Parameter, time);
	input0PrimitiveGroupNameValue.tokenizeInPlace(tokens, " ");

	if (tokens.size() < 1)
	{
		// just merge and don't do anything
		processResult = MergeCurrentDetail(this->_inputGDP);
		if (processResult != ENUMS::MethodProcessResult::SUCCESS) return processResult;

		addWarning(SOP_ERR_BADGROUP);
		return processResult;
	}
	
	for (auto token : tokens)
	{
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::FAILURE)

		const auto currGroup = this->_inputGDP->findPrimitiveGroup(token);
		if (currGroup && !currGroup->isEmpty())
		{
			// create separate detail from it
			const auto currDetail = new GU_Detail(this->_inputGDP, currGroup);
			
			processResult = ProcessCurrentDetail(currDetail, progress, processedoutputtype, currIter, time);
			if (processResult != ENUMS::MethodProcessResult::SUCCESS)
			{
				delete currDetail;
				processedDetails.clear();
				return processResult;
			}
			
			processedDetails.append(currDetail);
			currIter++;
		}
	}
		
	if (processedDetails.size() == 0)
	{
		addError(SOP_MESSAGE, "Failed to process details.");
		return ENUMS::MethodProcessResult::FAILURE;
	}	
	
	// merge all details
	this->gdp->clear();	
	currIter = 0;
	
	for (auto detail : processedDetails)
	{
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::FAILURE)

		processResult = MergeCurrentDetail(detail, processedDetails.size(), currIter);
		if (processResult != ENUMS::MethodProcessResult::SUCCESS) return processResult;

		currIter++;
	}
	
	return processResult;
}

/* -----------------------------------------------------------------
MAIN                                                               |
----------------------------------------------------------------- */

OP_ERROR 
SOP_Operator::cookMySop(OP_Context& context)
{
	DEFAULTS_CookMySop()
	
	this->_inputGDP = new GU_Detail(inputGeo(0, context));
	if (this->_inputGDP && error() < OP_ERROR::UT_ERROR_WARNING && cookInputGroups(context) < OP_ERROR::UT_ERROR_WARNING)
	{
		auto processResult = ENUMS::MethodProcessResult::SUCCESS;
		
		// process geometry
		exint processModeChoiceMenuValue;
		PRM_ACCESS::Get::IntPRM(this, processModeChoiceMenuValue, UI::processModeChoiceMenu_Parameter, currentTime);			

		switch(processModeChoiceMenuValue)
		{
			case static_cast<exint>(ENUMS::ProcessModeOption::AS_WHOLE) :		{ processResult = WhenAsWhole(progress, ENUMS::ProcessedOutputType::CONVEX_HULLS, currentTime); } break;
			case static_cast<exint>(ENUMS::ProcessModeOption::PER_ELEMENT) :	{ processResult = WhenPerElement(progress, ENUMS::ProcessedOutputType::CONVEX_HULLS, currentTime); } break;
			case static_cast<exint>(ENUMS::ProcessModeOption::PER_GROUP) :		{ processResult = WhenPerGroup(progress, ENUMS::ProcessedOutputType::CONVEX_HULLS, currentTime); } break;
		}
	}
	
	return error();
}

GU_DetailHandle
SOP_Operator::cookMySopOutput(OP_Context& context, int outputidx, SOP_Node* interests)
{
	DEFAULTS_CookMySopOutput()
	
	this->_inputGDP = new GU_Detail(inputGeo(0, context));
	if (this->_inputGDP && error() < OP_ERROR::UT_ERROR_WARNING && cookInputGroups(context) < OP_ERROR::UT_ERROR_WARNING)
	{		
		auto processResult = ENUMS::MethodProcessResult::SUCCESS;

		// process geometry		
		exint processModeChoiceMenuValue;
		PRM_ACCESS::Get::IntPRM(this, processModeChoiceMenuValue, UI::processModeChoiceMenu_Parameter, currentTime);
		
		switch (processModeChoiceMenuValue)
		{
			case static_cast<exint>(ENUMS::ProcessModeOption::AS_WHOLE) :		{ processResult = WhenAsWhole(progress, ENUMS::ProcessedOutputType::ORIGINAL_GEOMETRY, currentTime); } break;
			case static_cast<exint>(ENUMS::ProcessModeOption::PER_ELEMENT) :	{ processResult = WhenPerElement(progress, ENUMS::ProcessedOutputType::ORIGINAL_GEOMETRY, currentTime); } break;
			case static_cast<exint>(ENUMS::ProcessModeOption::PER_GROUP) :		{ processResult = WhenPerGroup(progress, ENUMS::ProcessedOutputType::ORIGINAL_GEOMETRY, currentTime); } break;
		}
	}

	return result;
}

/* -----------------------------------------------------------------
SELECTOR IMPLEMENTATION                                            |
----------------------------------------------------------------- */

MSS_Selector::~MSS_VHACDGenerate() { }

MSS_Selector::MSS_VHACDGenerate(OP3D_View& viewer, PI_SelectorTemplate& templ) : MSS_ReusableSelector(viewer, templ, COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_GENERATE_SMALLNAME), COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_GENERATE_GROUP_PRMNAME), nullptr, true)
{ this->setAllowUseExistingSelection(false); }

BM_InputSelector*
MSS_Selector::CreateMe(BM_View& viewer, PI_SelectorTemplate& templ)
{ return new MSS_Selector(reinterpret_cast<OP3D_View&>(viewer), templ); }

const char*
MSS_Selector::className() const
{ return "MSS_VHACDGenerate"; }

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

#undef MSS_Selector
#undef SOP_Base_Operator
#undef SOP_Operator