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
#include <UT/UT_DSOVersion.h>
#include <OP/OP_OperatorTable.h>
#include <DM/DM_RenderTable.h>

// this
#include "SOP_VHACDDebug.h"
#include "GUI_VHACDDebug.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator		GET_SOP_Namespace()::SOP_VHACDDebug
#define GUI_Hook			GET_GUI_Namespace()::GUI_VHACDDebug
#define COMMON_NAMES		GET_SOP_Namespace()::COMMON_NAMES
#define ENUMS				GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
OPERATOR                                                           |
----------------------------------------------------------------- */

void 
newSopOperator(OP_OperatorTable* table)
{	
	const auto sop = new OP_Operator
	(
		COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_DEBUG_SMALLNAME),
		COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_DEBUG_BIGNAME),
		SOP_Operator::CreateMe,
		SOP_Operator::parametersList,
		1,								// min inputs 
		2,								// max inputs
		nullptr,
		0,								// type of node OP_FLAG_GENERATOR (BE CAREFUL WITH THIS LITTLE FUCKER)
		nullptr,
		0,								// outputs count
		COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::TOOLKIT_TABMENU_PATH)
	);

	auto success = table->addOperator(sop);
	//table->addOpHidden(sop->getName());
}

/* -----------------------------------------------------------------
HOOK                                                               |
----------------------------------------------------------------- */

void newRenderHook(DM_RenderTable* table)
{
	// the priority only matters if multiple hooks are assigned to the same
	// primitive type. If this is the case, the hook with the highest priority
	// (largest priority value) is processed first.
	const int priority = 0;

	// register the actual hook
	const auto success = table->registerGTHook(new GUI_Hook(), GT_PRIM_POLYGON_MESH, priority, GUI_HOOK_FLAG_PRIM_FILTER);
}

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef COMMON_NAMES
#undef GUI_Hook
#undef SOP_Operator