/*
	Volumetric-Hierarchical Approximate Convex Decomposition.
	Based on https://github.com/kmammou/v-hacd

	IMPORTANT! ------------------------------------------
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

#pragma once
#ifndef ____sop_vhacd_scout_junior_h____
#define ____sop_vhacd_scout_junior_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// SESI
#include <MSS/MSS_ReusableSelector.h>

// hou-hdk-common
#include <Macros/CookMySop.h>
#include <Macros/DescriptionPRM.h>
#include <Macros/Namespace.h>
#include <Macros/UpdateParmsFlags.h>
#include <Enums/MethodProcessResult.h>

// this
#include "SOP_VHACDScout.h"

/* -----------------------------------------------------------------
FORWARDS                                                           |
----------------------------------------------------------------- */

class UT_AutoInterrupt;
class GEO_PrimClassifier;

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define CONTAINERS					GET_Base_Namespace()::Containers
#define ENUMS						GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
DECLARATION                                                        |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

	class SOP_VHACDScoutJunior final : public SOP_VHACDScout
	{
		DECLARE_CookMySop()
		DECLARE_UpdateParmsFlags()

		DECLARE_DescriptionPRM_Callback()

	protected:
		~SOP_VHACDScoutJunior() override;
		SOP_VHACDScoutJunior(OP_Network* network, const char* name, OP_Operator* op);
		const char*					inputLabel(unsigned input) const override;

	public:
		static OP_Node*				CreateMe(OP_Network* network, const char* name, OP_Operator* op);
		static PRM_Template			parametersList[];		

		static int					CallbackGRPPerHull(void* data, int index, float time, const PRM_Template* tmp);
		static int					CallbackPointPerHullCenter(void* data, int index, float time, const PRM_Template* tmp);
	};

DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS

#endif // !____sop_vhacd_scout_junior_h____