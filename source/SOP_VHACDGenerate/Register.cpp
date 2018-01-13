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
#include <BM/BM_ResourceManager.h>

// this
#include "SOP_VHACDGenerate.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator		GET_SOP_Namespace()::SOP_VHACDGenerate
#define MSS_Selector		GET_SOP_Namespace()::MSS_VHACDGenerate
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
		COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_GENERATE_SMALLNAME),
		COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_GENERATE_BIGNAME),
		SOP_Operator::CreateMe,
		SOP_Operator::parametersList,
		1,								// min inputs 
		1,								// max inputs
		nullptr,
		0,								// type of node OP_FLAG_GENERATOR (BE CAREFUL WITH THIS LITTLE FUCKER)
		nullptr,
		2,								// outputs count
		COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::TOOLKIT_TABMENU_PATH)
	);

	auto success = table->addOperator(sop);
	table->addOpHidden(sop->getName());
}

/* -----------------------------------------------------------------
SELECTOR                                                           |
----------------------------------------------------------------- */

void
newSelector(BM_ResourceManager* manager)
{	
	// find operator
	// TODO: temporary assign it to OTL that is using operator under the hood, till I figure out why it's breaking in viewport
	const auto sopOperator = OP_Network::getOperatorTable(SOP_TABLE_NAME)->getOperator(COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_GENERATE_OTL_SMALLNAME)); //SOP_GENERATE_SMALLNAME));
	if (!sopOperator)
	{
		UT_ASSERT(!"Could not find required operator!");
		return;
	}

	// create selector
	auto sopSelector = new PI_SelectorTemplate
	(
		COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::MSS_GENERATE_SMALLNAME),
		COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::MSS_GENERATE_BIGNAME),
		SOP_TABLE_NAME
	);

	if (sopOperator)
	{
		// setup selector
#if _WIN32		
		sopSelector->constructor(static_cast<void*>(&MSS_Selector::CreateMe));
#else
		sopSelector->constructor((void*)&MSS_Selector::CreateMe);
#endif

		sopSelector->data(OP3DthePrimSelTypes);

		auto success = manager->registerSelector(sopSelector);
		if (!success) return;

		// bind selector		
		success = manager->bindSelector
		(
			sopOperator,
			COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::MSS_GENERATE_SMALLNAME),
			COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::MSS_GENERATE_BIGNAME),
			COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::MSS_GENERATE_PROMPT),
			COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::SOP_GENERATE_GROUP_PRMNAME),
			0,																										// Input number to wire up.
			1,																										// 1 means this input is required.
			std::to_string(GEO_PrimTypeCompat::convertToFileFromMask(GEO_PrimTypeCompat::GEOPRIMPOLY)).c_str(),		// selection mask
			0,
			nullptr,
			0,
			nullptr,
			false
		);
	}
}

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef COMMON_NAMES
#undef MSS_Selector
#undef SOP_Operator