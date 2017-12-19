/*
	Clean template of SOP operator with selector and group input support.

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
#include <Utility/ParameterAccessing.h>

// this
#include "Parameters.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator			GET_SOP_Namespace()::SOP_Scout
#define SOP_Input_Name_0		"Geometry"
#define SOP_Base_Operator		SOP_VHACDNode
#define MSS_Selector			GET_SOP_Namespace()::MSS_ScoutSelector

// very important
#define SOP_GroupFieldIndex_0	1

#define COMMON_NAMES			GET_SOP_Namespace()::COMMON_NAMES
#define UI						GET_SOP_Namespace()::UI
#define PRM_ACCESS				GET_Base_Namespace()::Utility::PRM
#define ENUMS					GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
PARAMETERS                                                         |
----------------------------------------------------------------- */

PARAMETERLIST_Start(SOP_Operator)

	UI::filterSectionSwitcher_Parameter,

#if ____GROUP_MODE____ == 0
	UI::input0PointGroup_Parameter,
#elif ____GROUP_MODE____ == 1
	UI::input0EdgeGroup_Parameter,
#elif ____GROUP_MODE____ == 2
	UI::input0PrimitiveGroup_Parameter,
#else
	// do nothing
#endif // ____GROUP_MODE____		

	UI::mainSectionSwitcher_Parameter,

	UI::additionalSectionSwitcher_Parameter,
	PARAMETERLIST_DescriptionPRM(UI),

	UI::debugSectionSwitcher_Parameter,

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

	// update description active state
	UPDATE_DescriptionPRM_ActiveState(this, UI)

	return changed;
}

IMPLEMENT_DescriptionPRM_Callback(SOP_Operator, UI)

/* -----------------------------------------------------------------
OPERATOR INITIALIZATION                                            |
----------------------------------------------------------------- */

SOP_Operator::~SOP_Scout() { }

SOP_Operator::SOP_Scout(OP_Network* network, const char* name, OP_Operator* op) : SOP_Base_Operator(network, name, op)
{ op->setIconName(COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_SCOUT_ICONNAME_V2)); }

OP_Node* 
SOP_Operator::CreateMe(OP_Network* network, const char* name, OP_Operator* op) 
{ return new SOP_Operator(network, name, op); }

const char* 
SOP_Operator::inputLabel(unsigned input) const 
{ return SOP_Input_Name_0; }

OP_ERROR
SOP_Operator::cookInputGroups(OP_Context &context, int alone)
{
	const auto isOrdered = true;

#if ____GROUP_MODE____ == 0
	return cookInputPointGroups(context, this->_pointGroupInput0, alone, true, SOP_GroupFieldIndex_0, -1, true, isOrdered, true, 0);
#elif ____GROUP_MODE____ == 1
	return cookInputEdgeGroups(context, this->_edgeGroupInput0, alone, true, SOP_GroupFieldIndex_0, -1, true, 0);
#elif ____GROUP_MODE____ == 2
	return	cookInputPrimitiveGroups(context, this->_primitiveGroupInput0, alone, true, SOP_GroupFieldIndex_0, -1, true, isOrdered, true, 0);
#else
	return error();
#endif // ____GROUP_MODE____		
}

/* -----------------------------------------------------------------
HELPERS                                                            |
----------------------------------------------------------------- */

// YOUR CODE GOES HERE...

/* -----------------------------------------------------------------
MAIN                                                               |
----------------------------------------------------------------- */

#include <GEO/GEO_Closure.h>
#include <GEO/GEO_PointTree.h>
#include <GU/GU_PrimGroup.h>
#include <GA/GA_PrimitiveTypes.h>
#include <chrono>
#include <Containers/GA_PrimitiveIsland.h>
OP_ERROR 
SOP_Operator::cookMySop(OP_Context& context)
{	
	DEFAULTS_CookMySop()
	
	if (duplicateSource(0, context) < OP_ERROR::UT_ERROR_WARNING && error() < OP_ERROR::UT_ERROR_WARNING && cookInputGroups(context) < OP_ERROR::UT_ERROR_WARNING)
	{
		auto success = false;

		// find open primitive islands
		// find closed primitive islands
		
		GA_OffsetArray						startOffsets;
		GA_OffsetArray						endOffsets;				
		UT_Map<GA_Offset, GA_OffsetArray>	primitiveIslands;

		startOffsets.clear();
		endOffsets.clear();
		primitiveIslands.clear();
		
		exint								currIter = 0;

		for (auto primIt = GA_Iterator(this->gdp->getPrimitiveRange()); !primIt.atEnd(); primIt.advance())
		{
			if (currIter == 0) endOffsets.append(*primIt);

			auto currPrim = this->gdp->getPrimitive(*primIt);
			currPrim->forEachPoint([this, &startOffsets, &endOffsets](GA_Offset pointIt)
			{
				GA_OffsetArray pointPrims;
				auto currSize = this->gdp->getPrimitivesReferencingPoint(pointPrims, pointIt);
				for (GA_Offset currOffset : pointPrims)
				{					
					auto success = endOffsets.find(currOffset);
					if (success > exint(-1)) continue;
					else endOffsets.append(currOffset);
				}
			});
			
			currIter++;
		}

		/*
		auto								closure = GEO_Closure(*this->gdp);
		for (auto pointIt = GA_Iterator(this->gdp->getPointRange()); !pointIt.atEnd(); pointIt.advance())
		{
			UT_IntArray prims;

			if (currIter == 0)
			{				
				closure.findPrimsUsingPoint(*pointIt, prims);

				for (auto currOffset : prims) startOffsets.append(currOffset);
			}
		}

		//auto started = std::chrono::high_resolution_clock::now();
		//auto done = std::chrono::high_resolution_clock::now();
		//std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(done - started).count() << std::endl;
		*/

#if ____GROUP_MODE____ == 0
		// first we need to get all points selected by user
		success = this->_pointGroupInput0 && !this->_pointGroupInput0->isEmpty();
		if ((success && error() >= OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return error();
#elif ____GROUP_MODE____ == 1
		// first we need to get all edges selected by user
		success = this->_edgeGroupInput0 && !this->_edgeGroupInput0->isEmpty();
		if ((success && error() >= OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return error();
#elif ____GROUP_MODE____ == 2
		// first we need to get all primitives selected by user
		success = this->_primitiveGroupInput0 && !this->_primitiveGroupInput0->isEmpty();
		if ((success && error() >= OP_ERROR::UT_ERROR_WARNING) || (!success && error() >= OP_ERROR::UT_ERROR_NONE)) return error();
#else
		// do nothing
#endif // ____GROUP_MODE____	
		
	}

	return error();
}

/* -----------------------------------------------------------------
SELECTOR IMPLEMENTATION                                            |
----------------------------------------------------------------- */

#if ____GROUP_MODE____ > -1
MSS_Selector::~MSS_ScoutSelector() { }

MSS_Selector::MSS_ScoutSelector(OP3D_View& viewer, PI_SelectorTemplate& templ) : MSS_ReusableSelector(viewer, templ, COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_SCOUT_SMALLNAME_V2), COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_SCOUT_GROUP_PRMNAME_V2), nullptr, true)
{ this->setAllowUseExistingSelection(false); }

BM_InputSelector*
MSS_Selector::CreateMe(BM_View& viewer, PI_SelectorTemplate& templ)
{ return new MSS_Selector(reinterpret_cast<OP3D_View&>(viewer), templ); }

const char*
MSS_Selector::className() const
{ return "MSS_ScoutSelector"; }
#endif // ____GROUP_MODE____

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef PRM_ACCESS
#undef UI
#undef COMMON_NAMES

#undef SOP_GroupFieldIndex_0

#undef MSS_Selector
#undef SOP_Base_Operator
#undef SOP_Input_Name_0
#undef SOP_Operator