/*
	This is a place where you should create and register all SOP's, Selectors and their custom states.

	IMPORTANT! ------------------------------------------
	* Macros starting and ending with '____' shouldn't be used anywhere outside of this file.
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
#include <UT/UT_DSOVersion.h>
#include <OP/OP_OperatorTable.h>
#include <BM/BM_ResourceManager.h>

// hou-hdk-common
#include <Macros/GroupMenuPRM.h>

// this
#include "SOP_VHACDEngine.h"
#include "SOP_VHACDSetup.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_TabMenuPath		"Toolkit/V-HACD"
#define MSS_Prompt			"Select geometry. Press <enter> to accept."

/* -----------------------------------------------------------------
REGISTRATION                                                       |
----------------------------------------------------------------- */
void
newSelector(BM_ResourceManager* manager)
{
	auto table = OP_Network::getOperatorTable(SOP_TABLE_NAME);
	auto success = false;

#define SOP_Operator		GET_SOP_Namespace()::SOP_VHACDEngine
#define SOP_SmallName		"vhacd::engine::1.0"
#define SOP_BigName			"Engine (v-hacd)"

	auto sopVHACDEngine = new OP_Operator(SOP_SmallName, SOP_BigName, SOP_Operator::CreateMe, SOP_Operator::parametersList, 1, 1, 0, 0, 0, 1, SOP_TabMenuPath);
	success = table->addOperator(sopVHACDEngine);

#define SOP_Operator		GET_SOP_Namespace()::SOP_VHACDSetup
#define SOP_SmallName		"vhacd::setup::1.1"
#define SOP_BigName			"Setup (v-hacd)"
#define SOP_GroupPRM		CONST_PrimitiveGroupInput0_Name

#define MSS_Selector		GET_SOP_Namespace()::MSS_VHACDSetup
#define MSS_SmallName		"vhacd::setupselector::1.1"
#define MSS_BigName			"Setup (v-hacd selector)"

	auto sopVHACDSetup = new OP_Operator (SOP_SmallName, SOP_BigName, SOP_Operator::CreateMe, SOP_Operator::parametersList, 1, 1, 0, 0, 0, 1, SOP_TabMenuPath);
	success = table->addOperator(sopVHACDSetup);	
	if (success)
	{
		auto selectorVHACDSetup = new PI_SelectorTemplate(MSS_SmallName, MSS_BigName, SOP_TABLE_NAME);

		// setup selector
		selectorVHACDSetup->constructor(static_cast<void*>(&MSS_Selector::CreateMe));
		selectorVHACDSetup->data(OP3DthePrimSelTypes);

		success = manager->registerSelector(selectorVHACDSetup);
		if (!success) return;

		// bind selector		
		success = manager->bindSelector
		(
			sopVHACDSetup,
			MSS_SmallName,
			MSS_BigName,
			MSS_Prompt,
			SOP_GroupPRM,					// Parameter to write group to.
			0,								// Input number to wire up.
			1,								// 1 means this input is required.
			"0x000000ff",					// Prim/point mask selection.
			0,
			0,
			0,
			0,
			false
		);
	}
}

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef SOP_GroupPRM
#undef MSS_BigName
#undef MSS_SmallName
#undef MSS_Selector

#undef SOP_BigName
#undef SOP_SmallName
#undef SOP_Operator

#undef MSS_Prompt
#undef SOP_TabMenuPath