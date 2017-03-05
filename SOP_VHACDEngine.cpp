/*
	This is a place where you should create and register all SOP's, Selectors and their custom states.

	IMPORTANT! ------------------------------------------
	DO NOT MODIFY THIS FILE.
	Doing so may break every extension that uses it as a base or utility.
	Modify it ONLY when you know what you are doing. That means NEVER!

	#define WIP - Comment this directive if you don't want to compile operators that are used as test subjects or are still work in progress
	#define FINAL - Do not touch this. 
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
REGISTRATION SPECIFIC DEFINES                                      |
----------------------------------------------------------------- */

#define FINAL

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

#include <UT/UT_DSOVersion.h>

#include "../hou-hdk-common/NW_MustHave.h"
#include "../hou-hdk-common/Operators/SOP/Base/NW_SOP_Operator.h"
#include "Operator/SOP_VHACDEngine_Operator.h"

#ifdef FINAL
#include "../hou-hdk-common/NW_MustHave.cpp"
#include "../hou-hdk-common/Operators/SOP/Base/NW_SOP_Operator.cpp"
#include "Operator/SOP_VHACDEngine_Operator.cpp"
#endif 

/* -----------------------------------------------------------------
USING                                                              |
----------------------------------------------------------------- */

GET_SOP_NAMESPACE()

/* -----------------------------------------------------------------
SOP REGISTRATION                                                   |
----------------------------------------------------------------- */

auto 
newSopOperator(OP_OperatorTable* table)
-> void
{
	auto success = false;

	auto sopVHACDEngine = new OP_Operator(SOP_VHACDENGINE_OP_SMALLNAME, SOP_VHACDENGINE_OP_BIGNAME, SOP_OPERATOR_NAME(SOP_VHACDENGINE_NAME)::Create_Operator, SOP_OPERATOR_NAME(SOP_VHACDENGINE_NAME)::parametersList, 1, 1, 0, OP_FLAG_GENERATOR, 0, 1, SOP_VHACDENGINE_SUBMENUPATH);				
	success = table->addOperator(sopVHACDEngine);
}