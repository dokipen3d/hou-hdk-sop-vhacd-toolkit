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
#include <Utility/ParameterAccessing.h>
#include <Utility/AttributeAccessing.h>
#include <Utility/GeometryProcessing.h>

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
#define PRM_ACCESS				GET_Base_Namespace()::Utility::PRM
#define ATTRIB_ACCESS			GET_Base_Namespace()::Utility::Attribute
#define GDP_UTILS				GET_Base_Namespace()::Utility::Geometry
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

	// set output report state
	bool showReportValueState;
	PRM_ACCESS::Get::IntPRM(this, showReportValueState, UI::showProcessReportToggle_Parameter, currentTime);
	changed |= setVisibleState(UI::reportModeChoiceMenu_Parameter.getToken(), showReportValueState);

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

SOP_Operator::~SOP_VHACDGenerate() { }

SOP_Operator::SOP_VHACDGenerate(OP_Network* network, const char* name, OP_Operator* op) :
SOP_Base_Operator(network, name, op),
_primitiveGroupInput0(nullptr), 
_interfaceVHACD(nullptr)
{ op->setIconName(COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_GENERATE_ICONNAME)); }

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
	return cookInputPrimitiveGroups(context, this->_primitiveGroupInput0, alone, true, SOP_GroupFieldIndex_0, -1, true, isOrdered, true, 0);
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
SOP_Operator::SeparatePrimitiveRange()
{
	auto processReult = ENUMS::MethodProcessResult::SUCCESS;

	if (this->_primitiveGroupInput0 && this->_primitiveGroupInput0->entries() > 0)
	{
		const auto killPrimiGroup = new GA_PrimitiveGroup(*this->gdp);
		if (killPrimiGroup)
		{
			killPrimiGroup->addRange(this->gdp->getPrimitiveRange());
			killPrimiGroup->removeRange(this->gdp->getPrimitiveRange(this->_primitiveGroupInput0));

			this->gdp->deletePrimitives(*killPrimiGroup, true);
		}
		else
		{
			delete killPrimiGroup;
			this->addError(SOP_MESSAGE, "Failed to create separation group.");
			processReult = ENUMS::MethodProcessResult::FAILURE;
		}

		delete killPrimiGroup;
	}

	return processReult;
}

ENUMS::MethodProcessResult
SOP_Operator::PrepareGeometry(GU_Detail* detail, UT_AutoInterrupt progress, fpreal time)
{	
	/*
		Required element counts:
		- one primitive, so that we could have a chance to get some polygons from it, even if it's non-polygon geometry
		- 4 points, which gives us combination of 4 triangles that create shape
		- 12 vertices, 3 for each triangle
	 */

	const exint requiredPrimCount = 1;
	const exint requiredPointCount = 4;
	const exint requiredVertexCount = 12;	

	// make sure we got something to work on
	auto errorMessage = UT_String("Not enough primitives to create hull.");
	if (!GDP_UTILS::Test::IsEnoughPrimitives(this, detail, requiredPrimCount, errorMessage, ENUMS::NodeErrorLevel::ERROR) || error() >= UT_ErrorSeverity::UT_ERROR_WARNING) return ENUMS::MethodProcessResult::FAILURE;

	// check for non-polygon geometry
	bool forceConvertToPolygonsValue;
	PRM_ACCESS::Get::IntPRM(this, forceConvertToPolygonsValue, UI::forceConvertToPolygonsToggle_Parameter, time);

	if (forceConvertToPolygonsValue)
	{
		// this is default convertion, so we can lose some data on volumes and other primitives which required bigger resolution/settings to maintain their shape better		
		GEO_ConvertParms parms;		
		detail->convert(parms);
	}
	else
	{
		for (auto primIt : detail->getPrimitiveRange())
		{
			PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::FAILURE)
			
			if (detail->getPrimitive(primIt)->getTypeId() != GA_PRIMPOLY)
			{
				errorMessage = UT_String("Non-polygon geometry detected on input.");
				this->addError(SOP_ErrorCodes::SOP_MESSAGE, errorMessage.c_str());

				return ENUMS::MethodProcessResult::FAILURE;
			}
		}
	}
	
	// we need at least 4 points...	
	errorMessage = UT_String("Not enough points to create hull.");
	if (!GDP_UTILS::Test::IsEnoughPoints(this, detail, requiredPointCount, errorMessage, ENUMS::NodeErrorLevel::ERROR) || error() >= UT_ErrorSeverity::UT_ERROR_WARNING) return ENUMS::MethodProcessResult::FAILURE;
	
	// ... and at least 12 vertices to get up from bed	
	errorMessage = UT_String("Not enough vertices to create hull.");
	if (!GDP_UTILS::Test::IsEnoughVertices(this, detail, requiredVertexCount, errorMessage, ENUMS::NodeErrorLevel::ERROR) || error() >= UT_ErrorSeverity::UT_ERROR_WARNING) return ENUMS::MethodProcessResult::FAILURE;

	// try to triangulate
	const auto processResult = GDP_UTILS::Modify::Triangulate(this, progress, detail, SMALLPOLYGONS_TOLERANCE);
	if (processResult != ENUMS::MethodProcessResult::SUCCESS || error() > UT_ErrorSeverity::UT_ERROR_WARNING) return ENUMS::MethodProcessResult::FAILURE;

	// is there anything left after preparation?	
	errorMessage = UT_String("After removing zero area and open polygons there are no other primitives left.");
	if (!GDP_UTILS::Test::IsEnoughPrimitives(this, detail, requiredPrimCount, errorMessage) || error() >= UT_ErrorSeverity::UT_ERROR_WARNING) return ENUMS::MethodProcessResult::FAILURE;
	
	// again test if triangulated data contains enough points...
	errorMessage = UT_String("After removing zero area and open polygons there are not enough points to create hull.");
	if (!GDP_UTILS::Test::IsEnoughPoints(this, detail, requiredPointCount, errorMessage, ENUMS::NodeErrorLevel::ERROR) || error() >= UT_ErrorSeverity::UT_ERROR_WARNING) return ENUMS::MethodProcessResult::FAILURE;

	// ... and vertices
	errorMessage = UT_String("After removing zero area and open polygons there are not enough vertices to create hull.");
	if (!GDP_UTILS::Test::IsEnoughVertices(this, detail, requiredVertexCount, errorMessage, ENUMS::NodeErrorLevel::ERROR) || error() >= UT_ErrorSeverity::UT_ERROR_WARNING) return ENUMS::MethodProcessResult::FAILURE;

	return ENUMS::MethodProcessResult::SUCCESS;
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
		this->addError(SOP_MESSAGE, "Failed to find position attribute.");
		return ENUMS::MethodProcessResult::FAILURE;
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::GenerateConvexHulls(GU_Detail* geometry, UT_AutoInterrupt progress)
{
	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::WhenAsOne(UT_AutoInterrupt progress, ENUMS::ProcessedOutputType processedoutputtype, fpreal time)
{
	auto processResult = ENUMS::MethodProcessResult::SUCCESS;
	
	if (processedoutputtype == ENUMS::ProcessedOutputType::CONVEX_HULLS)
	{
		// make sure we have proper geometry before we try to gather data for V-HACD
		processResult = PrepareGeometry(this->gdp, progress, time);
		if (processResult != ENUMS::MethodProcessResult::SUCCESS || error() > OP_ERROR::UT_ERROR_WARNING) return processResult;

		// setup V-HACD and collect all required data
		SetupParametersVHACD(this->gdp, time);

		processResult = GatherDataForVHACD(this->gdp, progress, time);
		if (processResult != ENUMS::MethodProcessResult::SUCCESS || error() > OP_ERROR::UT_ERROR_WARNING) return processResult;

		// lets make some hulls!
		this->gdp->clear();

		processResult = GenerateConvexHulls(this->gdp, progress);
		if (processResult != ENUMS::MethodProcessResult::SUCCESS || error() > OP_ERROR::UT_ERROR_WARNING) return processResult;
	}

	// add bundle_id attribute
	this->_bundleIDHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID), 1, GA_Defaults(0)));
	if (this->_bundleIDHandle.isInvalid())
	{
		auto errorMessage = std::string("Failed to add \"") + std::string(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID)) + std::string("\" attribute.");
		this->addError(SOP_MESSAGE, errorMessage.c_str());
		processResult = ENUMS::MethodProcessResult::FAILURE;
	}

	return processResult;
}

ENUMS::MethodProcessResult
SOP_Operator::WhenPerElement(UT_AutoInterrupt progress, ENUMS::ProcessedOutputType processedoutputtype, fpreal time)
{
	// partition primitives by class
	auto primClassifier = GEO_PrimClassifier();
	primClassifier.classifyBySharedPoints(*this->gdp);

	UT_Map<exint, GA_OffsetArray> mappedOffsets;
	mappedOffsets.clear();
	
	for (auto offset : this->gdp->getPrimitiveRange())
	{
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::FAILURE)
			
		const auto currClass = primClassifier.getClass(offset);
		if (!mappedOffsets.contains(currClass)) mappedOffsets[currClass] = GA_OffsetArray();
		
		mappedOffsets[currClass].append(offset);
	}
	
	// add bundle_id attribute
	this->_bundleIDHandle = GA_RWHandleI(this->gdp->addIntTuple(GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID), 1));
	if (this->_bundleIDHandle.isInvalid())
	{
		auto errorMessage = std::string("Failed to add \"") + std::string(this->_commonAttributeNames.Get(ENUMS::VHACDCommonAttributeNameOption::BUNDLE_ID)) + std::string("\" attribute.");
		this->addError(SOP_MESSAGE, errorMessage.c_str());
		return ENUMS::MethodProcessResult::FAILURE;
	}

	// per each element	
	for (auto entry : mappedOffsets)
	{	
		auto& offsets = entry.second;

		if (processedoutputtype == ENUMS::ProcessedOutputType::CONVEX_HULLS)
		{
			
		}

		//entry.first
	}

	if (processedoutputtype == ENUMS::ProcessedOutputType::CONVEX_HULLS)
	{
		
	}

	return ENUMS::MethodProcessResult::SUCCESS;
}

ENUMS::MethodProcessResult
SOP_Operator::WhenPerGroup(UT_AutoInterrupt progress, ENUMS::ProcessedOutputType processedoutputtype, fpreal time)
{
	UT_String input0PrimitiveGroupNameValue;
	PRM_ACCESS::Get::StringPRM(this, input0PrimitiveGroupNameValue, UI::input0PrimitiveGroup_Parameter, time);
		
	UT_StringList tokens;
	input0PrimitiveGroupNameValue.tokenizeInPlace(tokens, " ");
	//std::cout << tokens.size() << std::endl;
	for (auto token : tokens) std::cout << token << std::endl;
	
	for (auto groupIt = this->gdp->primitiveGroups().beginTraverse(); !groupIt.atEnd(); ++groupIt)
	{
		PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(this, progress, ENUMS::MethodProcessResult::FAILURE)		

		// get group and...		
		const auto currGroup = groupIt.group();
		if (!currGroup->isInternal())
		{
			if (tokens.find(currGroup->getName()) > -1) continue;

			// ... create separate detail from it
			const auto currDetail = new GU_Detail(this->gdp, currGroup);
			std::cout << currGroup->getName() << std::endl;
			/*
			if (processedoutputtype == ENUMS::ProcessedOutputType::CONVEX_HULLS)
			{
				// make sure we have proper geometry before we try to gather data for V-HACD
				auto processResult = PrepareGeometry(currDetail, progress, time);
				if (processResult != ENUMS::MethodProcessResult::SUCCESS || error() > OP_ERROR::UT_ERROR_WARNING)
				{
					delete currDetail;
					return processResult;
				}

				// setup V-HACD and collect all required data
				SetupParametersVHACD(currDetail, time);

				processResult = GatherDataForVHACD(currDetail, progress, time);
				if (processResult != ENUMS::MethodProcessResult::SUCCESS || error() > OP_ERROR::UT_ERROR_WARNING)
				{
					delete currDetail;
					return processResult;
				}
			}
			*/
		}
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

	if (duplicateSource(0, context) < OP_ERROR::UT_ERROR_WARNING && error() < OP_ERROR::UT_ERROR_WARNING && cookInputGroups(context) < OP_ERROR::UT_ERROR_WARNING)	
	{
		// separate range of geometry we want to work on
		auto processResult = SeparatePrimitiveRange();
		if (processResult != ENUMS::MethodProcessResult::SUCCESS || error() > OP_ERROR::UT_ERROR_WARNING) return error();

		// process geometry
		exint processModeChoiceMenuValue;
		PRM_ACCESS::Get::IntPRM(this, processModeChoiceMenuValue, UI::processModeChoiceMenu_Parameter, currentTime);
		
		switch(processModeChoiceMenuValue)
		{
			case static_cast<exint>(ENUMS::ProcessedModeOption::AS_WHOLE) :			{ processResult = WhenAsOne(progress, ENUMS::ProcessedOutputType::CONVEX_HULLS, currentTime); } break;
			case static_cast<exint>(ENUMS::ProcessedModeOption::PER_ELEMENT) :		{ processResult = WhenPerElement(progress, ENUMS::ProcessedOutputType::CONVEX_HULLS, currentTime); } break;
			case static_cast<exint>(ENUMS::ProcessedModeOption::PER_GROUP) :		{ processResult = WhenPerGroup(progress, ENUMS::ProcessedOutputType::CONVEX_HULLS, currentTime); } break;
		}
	}

	return error();
}

GU_DetailHandle
SOP_Operator::cookMySopOutput(OP_Context& context, int outputidx, SOP_Node* interests)
{
	DEFAULTS_CookMySopOutput()

	if (duplicateSource(0, context) < OP_ERROR::UT_ERROR_WARNING && error() < OP_ERROR::UT_ERROR_WARNING && cookInputGroups(context) < OP_ERROR::UT_ERROR_WARNING)
	{
		// separate range of geometry we want to work on
		auto processResult = SeparatePrimitiveRange();
		if (processResult != ENUMS::MethodProcessResult::SUCCESS || error() > OP_ERROR::UT_ERROR_WARNING) return result;

		// process geometry		
		exint processModeChoiceMenuValue;
		PRM_ACCESS::Get::IntPRM(this, processModeChoiceMenuValue, UI::processModeChoiceMenu_Parameter, currentTime);
		
		switch (processModeChoiceMenuValue)
		{
			case static_cast<exint>(ENUMS::ProcessedModeOption::AS_WHOLE) :			{ processResult = WhenAsOne(progress, ENUMS::ProcessedOutputType::ORIGINAL_GEOMETRY, currentTime); } break;
			case static_cast<exint>(ENUMS::ProcessedModeOption::PER_ELEMENT) :		{ processResult = WhenPerElement(progress, ENUMS::ProcessedOutputType::ORIGINAL_GEOMETRY, currentTime); } break;
			case static_cast<exint>(ENUMS::ProcessedModeOption::PER_GROUP) :		{ processResult = WhenPerGroup(progress, ENUMS::ProcessedOutputType::ORIGINAL_GEOMETRY, currentTime); } break;
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
#undef GDP_UTILS
#undef ATTRIB_ACCESS
#undef PRM_ACCESS
#undef UI
#undef COMMON_NAMES

#undef MSS_Selector
#undef SOP_Base_Operator
#undef SOP_Operator