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

#pragma once
#ifndef ____sop_vhacdmerge_h____
#define ____sop_vhacdmerge_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// hou-hdk-common
#include <Macros/CookMySop.h>
#include <Macros/DescriptionPRM.h>
#include <Macros/Namespace.h>
#include <Macros/UpdateParmsFlags.h>

// this
#include "SOP_VHACDNode.h"
#include "ProcessedInputType.h"
#include "MismachErrorModeOption.h"

/* -----------------------------------------------------------------
FORWARDS                                                           |
----------------------------------------------------------------- */

class UT_AutoInterrupt;

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define CONTAINERS				GET_Base_Namespace()::Containers
#define ENUMS					GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
DECLARATION                                                        |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

	class SOP_VHACDMerge : public SOP_VHACDNode
	{
		DECLARE_CookMySop_Multi()
		DECLARE_UpdateParmsFlags()

		DECLARE_DescriptionPRM_Callback()

	protected:
		~SOP_VHACDMerge() override;
		SOP_VHACDMerge(OP_Network* network, const char* name, OP_Operator* op);
		const char*				inputLabel(unsigned input) const override;

	public:
		static OP_Node*			CreateMe(OP_Network* network, const char* name, OP_Operator* op);
		static PRM_Template		parametersList[];

		static int				CallbackAttributeMismatchErrorModeChoiceMenu(void* data, int index, float time, const PRM_Template* tmp);

	private:
		bool					GetAllDetailsOfType(UT_AutoInterrupt progress, UT_Array<const GU_Detail*>& details, ENUMS::ProcessedInputType processedinput, fpreal time);
		void					WhenSpecificAttributeMismatch(const PRM_Template& parameter, UT_String attributename, fpreal time);
		void					WhenOverrideAttributeMismatch(UT_AutoInterrupt progress, UT_Array<const GU_Detail*>& details, ENUMS::MismatchErrorModeOption mismatchoption, ENUMS::ProcessedInputType processedinput, fpreal time);
		void					HandleAttributesMismatch(UT_AutoInterrupt progress, UT_Array<const GU_Detail*>& details, ENUMS::ProcessedInputType processedinput, fpreal time);
		bool					AddFoundVHACDAttributes(UT_AutoInterrupt progress, UT_Array<const GU_Detail*>& details, ENUMS::ProcessedInputType processedinput, fpreal time);
		void					ShiftCurrentDetailPrimitiveAttributes(GU_Detail* currentdetail, UT_AutoInterrupt progress, exint iteration, exint& hullshiftvalue, exint& bundleshiftvalue, ENUMS::ProcessedInputType processedinput);
		bool					MergeCurrentDetail(const GU_Detail* currentdetail, exint iteration, exint detailscount);
		void					MergeAllInputDetails(UT_AutoInterrupt progress, UT_Array<const GU_Detail*>& details, ENUMS::ProcessedInputType processedinput, fpreal time);
		
		bool					_createHullCount;
		bool					_createHullID;
		bool					_createBundleCount;
		bool					_createBundleID;
	};

DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS

#endif // !____sop_vhacdmerge_h____