/*
	Volumetric-Hierarchical Approximate Convex Decomposition. 
	Based on https://github.com/kmammou/v-hacd

	IMPORTANT! ------------------------------------------
	DO NOT MODIFY THIS FILE.
	Doing so may break every extension that uses it as a base or utility.
	Modify it ONLY when you know what you are doing. That means NEVER!
	-----------------------------------------------------

	TODO! -----------------------------------------------
	-----------------------------------------------------

	Author: 	Nodeway (2016)

	Email:		nodeway@hotmail.com
	Vimeo:		https://vimeo.com/nodeway
	Twitter:	https://twitter.com/nodeway
	Github:		https://github.com/nodeway

	LICENSE ------------------------------------------

	Copyright (c) 2016 Nodeway
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

#include "SOP_VHACDEngine_Parameters.h"

/* -----------------------------------------------------------------
USING                                                              |
----------------------------------------------------------------- */

GET_SOP_NAMESPACE()

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define ____unique_sop_title____		SOP_VHACDENGINE_NAME

#define VHACD_LOGGER()					auto VHACD_LOGGER_TITLE()
#define VHACD_CALLBACK()				auto VHACD_CALLBACK_TITLE()

#define TITLE							SOP_PARMS_NAMESPACE_TITLE()::TITLE
#define UI								SOP_PARMS_NAMESPACE_TITLE()::UI
#define AUTHOR							SOP_PARMS_NAMESPACE_TITLE()::AUTHOR

/* -----------------------------------------------------------------
LOGGER                                                             |
----------------------------------------------------------------- */

VHACD_LOGGER_TITLE()::VHACD_LOGGER_TITLE()() { }
VHACD_LOGGER_TITLE()::~VHACD_LOGGER_TITLE()() { }

VHACD_LOGGER()::Log(const char* const msg)
-> void
{
	if (this->_showMsg) cout << msg << endl;;
}

/* -----------------------------------------------------------------
LOGGER CALLBACK                                                    |
----------------------------------------------------------------- */

VHACD_CALLBACK_TITLE()::VHACD_CALLBACK_TITLE()() : _hou(HOM()) { }
VHACD_CALLBACK_TITLE()::~VHACD_CALLBACK_TITLE()() { }

VHACD_CALLBACK()::Update(const double overallProgress, const double stageProgress, const double operationProgress, const char* const stage, const char* const operation)
-> void
{
	if (this->_showOverallProgress) 
	{
		auto info = string("Overall Progress: ") + to_string((int)(overallProgress + 0.5)).c_str() + "%";

		// TODO: fix this shit, it doesn't update properly
		this->_hou.ui().setStatusMessage(info.c_str());
		
		cout << setfill('-') << "Overall Progress: " << std::setw(52) << " " << (int)(overallProgress + 0.5) << "% " << endl;
	}

	if (this->_showStageProgress) cout << stage << ": " << (int) (stageProgress + 0.5) << "% " << endl;
	if (this->_showOperationProgress) cout << operation << ": " << (int) (operationProgress + 0.5) << "% " << endl;
}

/* -----------------------------------------------------------------
PARAMETERS LIST                                                    |
----------------------------------------------------------------- */

SOP_START_PARAMETERLIST()

		TITLE::titleUpper_Parameter,
		TITLE::title_Parameter,
		TITLE::input0NotConnectedErrorMessage_Parameter,
		TITLE::titleLower_Parameter,

		UI::filterSectionSwitcher_Parameter,
		UI::allowParametersOverrideToggle_Parameter,
		UI::allowParametersOverrideSeparator_Parameter,
		UI::polygonizeToggle_Parameter,
		UI::polygonizeSeparator_Parameter,

		UI::mainSectionSwitcher_Parameter,
		UI::modeChoiceMenu_Parameter,
		UI::resolutionInteger_Parameter,
		UI::depthInteger_Parameter,
		UI::concavityFloat_Parameter,
		UI::planeDownsamplingInteger_Parameter,
		UI::convexHullDownsamplingInteger_Parameter,
		UI::alphaFloat_Parameter,
		UI::betaFloat_Parameter,
		UI::gammaFloat_Parameter,				
		UI::maxTriangleCountInteger_Parameter,
		UI::adaptiveSamplingFloat_Parameter,
		UI::normalizeMeshToggle_Parameter,
		UI::normalizeMeshSeparatorSeparator_Parameter,
		UI::useOpenCLToggle_Parameter,
		UI::useOpenCLSeparatorSeparator_Parameter,

		UI::additionalSectionSwitcher_Parameter,
		DESCRIPTION_PARAMETERLIST(),

		UI::debugSectionSwitcher_Parameter,
		UI::consoleReportToggle_Parameter,
		UI::consoleReportSeparator_Parameter,
		UI::reportModeChoiceMenu_Parameter,

		AUTHOR::authorSectionSwitcher_Parameter,
		AUTHOR::basedOnLabel_Parameter,
		AUTHOR::blankSpaceLabel_Parameter,
		AUTHOR::author_Parameter,
		AUTHOR::authorEmailAdress_Parameter,
		AUTHOR::authorTwitterLink_Parameter,

____SWITCH_VISIBILITY()

		TITLE::title_Parameter,
		UI::filterSectionSwitcher_Parameter,
		UI::mainSectionSwitcher_Parameter,
		UI::additionalSectionSwitcher_Parameter,
		UI::debugSectionSwitcher_Parameter,

____SWITCH_INVERT_VISIBILITY()

		TITLE::input0NotConnectedErrorMessage_Parameter,

____SWITCH_BUTDONTFORCE_VISIBILITY()
SOP_END_PARAMETERLIST()

/* -----------------------------------------------------------------
INITIALIZATION                                                     |
----------------------------------------------------------------- */

SOP_OPERATOR_INITIALIZATION(SOP_VHACDENGINE_OP_ICONNAME)
{
	return SOP_VHACDENGINE_OP_INPUTNAME0;
}

/* -----------------------------------------------------------------
DEFAULT IMPLEMENTATIONS                                            |
----------------------------------------------------------------- */

SOP_OPERATOR()::DESCRIPTION_IMPLEMENTATION()

/* -----------------------------------------------------------------
OVERRIDES                                                          |
----------------------------------------------------------------- */

SOP_OPERATOR()::updateParmsFlags()
-> bool
{
	UPDATEPARMSFLAGS_DEFAULTS()

	// is input connected?
	this->is0Connected = this->getInput(0) == NULL ? 0 : 1;

	/* ---------------------------- Set Global Visibility ---------------------------- */

	visibilityState = this->is0Connected ? 1 : 0;

	// update global parameters visibility
	PARAMETERS_VISIBLESTATE_UPDATE(is0Connected)

	// update title visibility
	changed |= setVisibleState(TITLE::title_Parameter.getToken(), visibilityState);
	changed |= setVisibleState(TITLE::input0NotConnectedErrorMessage_Parameter.getToken(), !visibilityState);

	/* ---------------------------- Set States --------------------------------------- */

	// update parameters states
	if (is0Connected)
	{	
		// set output report state
		Get_IntPRM(this->_currentShowReportValueState, UI::consoleReportToggle_Parameter, this->currentTime);
		visibilityState = this->_currentShowReportValueState ? 1 : 0;
		changed |= setVisibleState(UI::reportModeChoiceMenu_Parameter.getToken(), visibilityState);
	}

	// update description active state
	DESCRIPTION_ACTIVESTATE_UPDATE(is0Connected)

	return changed;
}

SOP_OPERATOR()::Tool_BirthDate() 
-> string 
{ return SOP_VHACDENGINE_DATE; }

SOP_OPERATOR()::Setup_Information() 
-> void 
{ SETUP_INFORMATION_DEFAULTS_V1() }

/* -----------------------------------------------------------------
HELPERS                                                            |
----------------------------------------------------------------- */

SOP_OPERATOR()::Pull_IntPRM(GU_Detail* geometry, const PRM_Template& parameter, bool interfaceonly /* = false */, fpreal time /* = 0 */) 
-> exint
{
	exint currentIntValue = 0;

	if (!interfaceonly)
	{
		GA_RWAttributeRef attributeReference;
		GA_RWHandleI attributeHandle;		

		// check is there attribute with the name that matches parameter name
		// if it exist, use it as parm value, otherwise use UI value
		auto success = Find_IntegerPrimitiveAttribute(geometry, parameter.getToken(), attributeReference, attributeHandle, NW_ERROR_LEVEL::None, time);	
		if (success)
		{
			currentIntValue = attributeHandle.get(GA_Offset(0));
			auto currentRangeValue = parameter.getRangePtr();

			if (currentIntValue < currentRangeValue->getParmMin() || currentIntValue > currentRangeValue->getParmMax()) Get_IntPRM(currentIntValue, parameter, time);
		}
		else  Get_IntPRM(currentIntValue, parameter, time);
	}
	else Get_IntPRM(currentIntValue, parameter, time);

	return currentIntValue;
}

SOP_OPERATOR()::Pull_FloatPRM(GU_Detail* geometry, const PRM_Template& parameter, bool interfaceonly /* = false */, fpreal time /* = 0 */) 
-> fpreal
{	
	fpreal currentFloatValue = 0;

	if (!interfaceonly)
	{
		GA_RWAttributeRef attributeReference;
		GA_RWHandleR attributeHandle;		

		// check is there attribute with the name that matches parameter name
		// if it exist, use it as parm value, otherwise use UI value
		auto success = this->Find_FloatPrimitiveAttribute(geometry, parameter.getToken(), attributeReference, attributeHandle, NW_ERROR_LEVEL::None, time);	
		if (success)
		{				
			currentFloatValue = attributeHandle.get(GA_Offset(0));
			auto currentRangeValue = parameter.getRangePtr();

			if (currentFloatValue < currentRangeValue->getParmMin() || currentFloatValue > currentRangeValue->getParmMax()) Get_FloatPRM(currentFloatValue, parameter, time);		
		}
		else Get_FloatPRM(currentFloatValue, parameter, time);
	}
	else Get_FloatPRM(currentFloatValue, parameter, time);

	return currentFloatValue;
}

SOP_OPERATOR()::Prepare_Geometry(UT_AutoInterrupt progress)
-> bool
{
	// triangulate all...
	this->gdp->convex();

	// ... and make sure we really got only triangles
	for (auto primIt : this->gdp->getPrimitiveRange())
	{
		// make sure we can escape the loop
		if (progress.wasInterrupted())
		{
			this->addError(SOP_MESSAGE, "Operation interrupted");
			return false;
		}							
		
		auto currentPoly = (GEO_PrimPoly*) this->gdp->getGEOPrimitive(primIt);

		if (currentPoly->isClosed())
		{
			// maybe we still got polygons with more than 3 vertices?
			auto currentVertexCount = currentPoly->getVertexRange().getEntries();
			if (currentVertexCount > 3)
			{
				this->addError(SOP_MESSAGE, "Internal triangulation failure. Polygons with 4 or more vertices detected.");
				return false;
			}

			// collapse polygons with less than 3 vertices
			if (currentVertexCount < 3)
			{
				auto nonTriPointsGroup = this->gdp->newDetachedPointGroup();
				nonTriPointsGroup->addRange(currentPoly->getPointRange());

				// TODO: distance probably needs to be calculated per polygon, because we may have big non triangles and this will not collapse them
				this->gdp->fastConsolidatePoints(10.0f, nonTriPointsGroup);
			}
		}

		// collapse zero area and really tiny polygons + kill all open polygons
		if (currentPoly->calcArea() <= (0.0f + SOP_VHACDENGINE_OP_SMALLPOLYGONS_TOLERANCE))
		{
			auto polys = this->gdp->newDetachedPrimitiveGroup();
			auto points = this->gdp->newDetachedPointGroup();

			points->addRange(currentPoly->getPointRange());
			this->gdp->fastConsolidatePoints(10.0f, points);

			polys->addOffset(primIt);
			this->gdp->removeZeroAreaFaces(polys, false);
		}
	}	

	// is there anything left after preparation?	
	success = Check_EnoughPrimitives(this->gdp, NW_ERROR_LEVEL::Warning, 1, UT_String("After removing zero area and open polygons there are no other primitives left."));
	if ((success && this->error() >= UT_ERROR_WARNING) || (!success && this->error() >= UT_ERROR_NONE)) return false;
	
	return Check_EnoughPoints(this->gdp, NW_ERROR_LEVEL::Warning, 4, UT_String("Not enough points to create hull."));
}

SOP_OPERATOR()::Setup_VHACD()
-> void
{
	Get_IntPRM(this->_currentAllowParametersOverrideValueState, UI::allowParametersOverrideToggle_Parameter, this->currentTime);

	// main parameters
	this->_parametersVHACD.m_mode = this->Pull_IntPRM(this->gdp, UI::modeChoiceMenu_Parameter, !this->_currentAllowParametersOverrideValueState, this->currentTime);
	this->_parametersVHACD.m_resolution = this->Pull_IntPRM(this->gdp, UI::resolutionInteger_Parameter, !this->_currentAllowParametersOverrideValueState, this->currentTime);
	this->_parametersVHACD.m_depth = this->Pull_IntPRM(this->gdp, UI::depthInteger_Parameter, !this->_currentAllowParametersOverrideValueState, this->currentTime);
	this->_parametersVHACD.m_concavity = this->Pull_FloatPRM(this->gdp, UI::concavityFloat_Parameter, !this->_currentAllowParametersOverrideValueState, this->currentTime);
	this->_parametersVHACD.m_planeDownsampling = this->Pull_IntPRM(this->gdp, UI::planeDownsamplingInteger_Parameter, !this->_currentAllowParametersOverrideValueState, this->currentTime);
	this->_parametersVHACD.m_convexhullDownsampling = this->Pull_IntPRM(this->gdp, UI::convexHullDownsamplingInteger_Parameter, !this->_currentAllowParametersOverrideValueState, this->currentTime);
	this->_parametersVHACD.m_alpha = this->Pull_FloatPRM(this->gdp, UI::alphaFloat_Parameter, !this->_currentAllowParametersOverrideValueState, this->currentTime);
	this->_parametersVHACD.m_beta = this->Pull_FloatPRM(this->gdp, UI::betaFloat_Parameter, !this->_currentAllowParametersOverrideValueState, this->currentTime);
	this->_parametersVHACD.m_gamma = this->Pull_FloatPRM(this->gdp, UI::gammaFloat_Parameter, !this->_currentAllowParametersOverrideValueState, this->currentTime);
	this->_parametersVHACD.m_maxNumVerticesPerCH = this->Pull_IntPRM(this->gdp, UI::maxTriangleCountInteger_Parameter, !this->_currentAllowParametersOverrideValueState, this->currentTime);
	this->_parametersVHACD.m_minVolumePerCH = this->Pull_FloatPRM(this->gdp, UI::adaptiveSamplingFloat_Parameter, !this->_currentAllowParametersOverrideValueState, this->currentTime);
	this->_parametersVHACD.m_pca = this->Pull_IntPRM(this->gdp, UI::normalizeMeshToggle_Parameter, !this->_currentAllowParametersOverrideValueState, this->currentTime);

	Get_IntPRM(this->_parametersVHACD.m_oclAcceleration, UI::useOpenCLToggle_Parameter, this->currentTime);

	// debug parameters
	Get_IntPRM(this->_currentShowReportValueState, UI::consoleReportToggle_Parameter, this->currentTime);
	if (this->_currentShowReportValueState)
	{
		Get_IntPRM(this->_currentReportModeChoiceValueState, UI::reportModeChoiceMenu_Parameter, this->currentTime);
		switch (this->_currentReportModeChoiceValueState)
		{
		case 1:
			{
				this->_loggerVHACD._showMsg =  true;
				this->_callbackVHACD._showOverallProgress = false;
				this->_callbackVHACD._showStageProgress = false;
				this->_callbackVHACD._showOperationProgress = false;
			}
			break;
		case 2:
			{
				this->_loggerVHACD._showMsg =  false;
				this->_callbackVHACD._showOverallProgress = true;
				this->_callbackVHACD._showStageProgress = true;
				this->_callbackVHACD._showOperationProgress = true;
			}
			break;
		default:
			{
				this->_loggerVHACD._showMsg =  true;
				this->_callbackVHACD._showOverallProgress = true;
				this->_callbackVHACD._showStageProgress = true;
				this->_callbackVHACD._showOperationProgress = true;
			}
			break;
		}
	}
	else
	{
		this->_loggerVHACD._showMsg =  false;
		this->_callbackVHACD._showOverallProgress = false;
		this->_callbackVHACD._showStageProgress = false;
		this->_callbackVHACD._showOperationProgress = false;
	}
}

SOP_OPERATOR()::Prepare_DataForVHACD(UT_AutoInterrupt progress)
-> bool
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
	auto success = Find_VectorPointAttribute(this->gdp, "P", this->_positionReference, this->_positionHandle, NW_ERROR_LEVEL::None, this->currentTime);
	if (success && this->error() < UT_ERROR_WARNING) 
	{		
		// store positions, as continuous list from 0 to last, without breaking it per primitive		
		this->_points.reserve(this->gdp->getNumPoints() * 3);

		auto pointIt = GA_Iterator(this->gdp->getPointRange());
		for (pointIt; !pointIt.atEnd(); pointIt.advance())
		{
			// make sure we can escape the loop
			if (progress.wasInterrupted()) 
			{
				this->addWarning(SOP_MESSAGE, "Operation interrupted");
				return false;
			}

			auto currentPosition = this->_positionHandle.get(*pointIt);
			
			this->_points.push_back(currentPosition.x());
			this->_points.push_back(currentPosition.y());
			this->_points.push_back(currentPosition.z());
		}

		// store indexes, as (0, 1, 2) for each triangle
		this->_triangles.reserve(this->gdp->getNumPrimitives() * 3);

		auto polyIt = GA_Iterator(this->gdp->getPrimitiveRange());
		for (polyIt; !polyIt.atEnd(); polyIt.advance())
		{
			// make sure we can escape the loop
			if (progress.wasInterrupted()) 
			{
				this->addWarning(SOP_MESSAGE, "Operation interrupted");
				return false;
			}
						
			auto currPrim = this->gdp->getPrimitive(*polyIt);
			for (auto i = 0; i < currPrim->getVertexCount(); ++ i) this->_triangles.push_back(currPrim->getPointIndex(i));
		}		
	}
	else return false;

	return true;
}

SOP_OPERATOR()::Generate_ConvexHulls(UT_AutoInterrupt progress)
-> OP_ERROR
{	
	// get interface
	this->_interfaceVHACD = VHACD::CreateVHACD();
		
	this->success = this->_interfaceVHACD->Compute(&this->_points[0], 3, this->_points.size() / 3, &this->_triangles[0], 3, this->_triangles.size() / 3, this->_parametersVHACD);
	if (this->success && this->_interfaceVHACD->GetNConvexHulls())
	{	
		auto hullCount = this->_interfaceVHACD->GetNConvexHulls();
		
		// generate hulls
		for (auto id = 0; id < hullCount; ++id)
		{
			// make sure we can escape the loop
			if (progress.wasInterrupted())
			{
				this->_interfaceVHACD->Cancel();
				this->addWarning(SOP_MESSAGE, "Operation interrupted");				
				return this->error();
			}

			// draw hull
			IVHACD::ConvexHull currentHull;
			
			this->_interfaceVHACD->GetConvexHull(id, currentHull);
			if(!currentHull.m_nPoints || !currentHull.m_nTriangles) continue;
			
			this->success = this->Draw_ConvexHull(this->gdp, id, currentHull, progress);
			if (!this->success && this->error() > UT_ERROR_NONE) return this->error();
		}
	}
	else 
	{		
		this->addError(SOP_MESSAGE, "Computation failed");
		return this->error();		
	}
	
	this->_interfaceVHACD->Clean();
	this->_interfaceVHACD->Release();

	return this->error();
}

SOP_OPERATOR()::Draw_ConvexHull(GU_Detail* geometry, int hullid, IVHACD::ConvexHull hull, UT_AutoInterrupt progress)
-> bool
{
	// add amount of points that hull consists
	auto start = geometry->appendPointBlock(hull.m_nPoints);

	// make sure that we have at least 4 points, if we have less, than it's a flat geometry, so ignore it
	if (Check_EnoughPoints(geometry) && this->error() >= UT_ERROR_WARNING) return false;
	
	// set point positions				
	int hullP = 0;
	std::vector<GA_Offset> pOffsets;	
	pOffsets.clear();

	auto pointIt = GA_Iterator(geometry->getPointRangeSlice(this->gdp->pointIndex(start)));
	while (!pointIt.atEnd())
	{
		// make sure we can escape the loop
		if (progress.wasInterrupted())
		{
			this->_interfaceVHACD->Cancel();
			this->addWarning(SOP_MESSAGE, "Operation interrupted");			
			return false;
		}

		auto currentPosition = UT_Vector3R(hull.m_points[hullP], hull.m_points[hullP + 1], hull.m_points[hullP + 2]);		
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
			this->addWarning(SOP_MESSAGE, "Operation interrupted");			
			return false;
		}

		// create triangle
		auto polygon = static_cast<GEO_PrimPoly*>(geometry->appendPrimitive(GEO_PRIMPOLY));		
		polygon->setSize(0);
		
		polygon->appendVertex(pOffsets[hull.m_triangles[t + 0]]);
		polygon->appendVertex(pOffsets[hull.m_triangles[t + 1]]);
		polygon->appendVertex(pOffsets[hull.m_triangles[t + 2]]);

		polygon->close();
	}			

	return true;
}

/* -----------------------------------------------------------------
MAIN                                                               |
----------------------------------------------------------------- */

SOP_OPERATOR()::cookMySop(OP_Context& context) 
-> OP_ERROR
{
	COOKMYSOP_DEFAULTS()
		
	if (duplicateSource(0, context) < UT_ERROR_WARNING && this->error() < UT_ERROR_WARNING) 
	{
		// make sure we got something to work on
		success = Check_EnoughPrimitives(this->gdp, NW_ERROR_LEVEL::Warning, 1, UT_String("Not enough primitives to create hull."));		
		if ((success && this->error() >= UT_ERROR_WARNING) || (!success && this->error() >= UT_ERROR_NONE)) return this->error();

		// do we want only polygons or do we try to convert anything to polygons?
		Get_IntPRM(this->_polygonizeValueState, UI::polygonizeToggle_Parameter, this->currentTime);
		if (this->_polygonizeValueState)
		{ 
			GEO_ConvertParms parms;			
			this->gdp->convert(parms);			
		}
		else
		{
			for (auto primIt : this->gdp->getPrimitiveRange())
			{
				// make sure we can escape the loop
				if (progress.wasInterrupted())
				{
					this->addError(SOP_MESSAGE, "Operation interrupted");
					return this->error();
				}

				// only polygons allowed
				if (this->gdp->getPrimitive(primIt)->getTypeId() != GA_PRIMPOLY)
				{
					this->addError(SOP_MESSAGE, "Non-polygon geometry detected on input.");
					return this->error();
				}
			}
		}
		
		// we need at least 4 points to get up from bed
		success = Check_EnoughPoints(this->gdp, NW_ERROR_LEVEL::Warning, 4, UT_String("Not enough points to create hull."));
		if ((success && this->error() >= UT_ERROR_WARNING) || (!success && this->error() >= UT_ERROR_NONE)) return this->error();

		// we should have only polygons now, but we need to make sure that they are all correct
		success = this->Prepare_Geometry(progress);
		if ((success && this->error() >= UT_ERROR_WARNING) || (!success && this->error() >= UT_ERROR_NONE)) return this->error();

		// get parameters and logger/callback
		this->Setup_VHACD();
		this->_parametersVHACD.m_logger = &this->_loggerVHACD;
		this->_parametersVHACD.m_callback = &this->_callbackVHACD;

		// prepare data for V-HACD
		success = this->Prepare_DataForVHACD(progress);
		if ((success && this->error() >= UT_ERROR_WARNING) || (!success && this->error() >= UT_ERROR_NONE)) return this->error();				

		// lets make some hulls!
		this->gdp->clear();

		if (this->Generate_ConvexHulls(progress) >= UT_ERROR_WARNING && this->error() >= UT_ERROR_WARNING) return this->error();
	}	

	return this->error();	
}

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef AUTHOR
#undef UI
#undef TITLE

#undef VHACD_CALLBACK
#undef VHACD_LOGGER
#undef VHACD_CALLBACK_TITLE
#undef VHACD_LOGGER_TITLE

#undef ____unique_sop_title____