/*
	Volumetric-Hierarchical Approximate Convex Decomposition.
	Based on https://github.com/kmammou/v-hacd

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

// this
#include "SOP_VHACDMerge.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define SOP_Operator		GET_SOP_Namespace()::SOP_VHACDMerge
#define SOP_SmallName		"vhacd::merge::2.0"
#define SOP_BigName			"Merge (v-hacd)"
#define SOP_TabMenuPath		"Toolkit/V-HACD"

/* -----------------------------------------------------------------
OPERATOR                                                           |
----------------------------------------------------------------- */

void 
newSopOperator(OP_OperatorTable* table)
{
	const auto sop = new OP_Operator
	(
		SOP_SmallName,
		SOP_BigName,
		SOP_Operator::CreateMe,
		SOP_Operator::parametersList,
		2,								// min inputs 
		8,								// max inputs
		nullptr,
		0,								// type of node OP_FLAG_GENERATOR (BE CAREFUL WITH THIS LITTLE FUCKER)
		nullptr,
		2,								// outputs count
		SOP_TabMenuPath
	);

	auto success = table->addOperator(sop);
	//table->addOpHidden(sop->getName());	
}

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef SOP_TabMenuPath
#undef SOP_BigName
#undef SOP_SmallName
#undef SOP_Operator