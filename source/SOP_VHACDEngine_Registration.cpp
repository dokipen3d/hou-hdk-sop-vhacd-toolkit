/*
	This is a place where you should create and register all SOP's, Selectors and their custom states.

	IMPORTANT! ------------------------------------------
	* Macros starting and ending with '____' shouldn't be used anywhere outside of this file.
	* External macros used:
		GET_SOP_Namespace() - comes from "Macros_Namespace.h"
	-----------------------------------------------------

	Author: 	SNOWFLAKE
	Email:		snowflake@outlook.com

	LICENSE ------------------------------------------

	Copyright (c) 2016-2017 SNOWFLAKE
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

#include <UT/UT_DSOVersion.h>
#include <OP/OP_OperatorTable.h>

#include "SOP_VHACDEngine_Operator.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator		GET_SOP_Namespace()::SOP_VHACDEngine_Operator
#define SOP_SmallName		"vhacd::engine::1.0"
#define SOP_BigName			"Engine (v-hacd)"
#define SOP_TabMenuPath		"Convex Decomposition"

/* -----------------------------------------------------------------
REGISTRATION                                                       |
----------------------------------------------------------------- */

auto newSopOperator(OP_OperatorTable* table)
-> void
{
	auto success = false;

	auto sop = new OP_Operator
	(
		SOP_SmallName,
		SOP_BigName,
		SOP_Operator::CreateOperator,
		SOP_Operator::parametersList,
		1, // min inputs 
		1, // max inputs
		0,
		OP_FLAG_GENERATOR, // type of node
		0,
		1, // outputs count
		SOP_TabMenuPath
	);

	success = table->addOperator(sop);
	//table->addOpHidden(sop->getName());	
}

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef SOP_TabMenuPath
#undef SOP_BigName
#undef SOP_SmallName
#undef SOP_Operator