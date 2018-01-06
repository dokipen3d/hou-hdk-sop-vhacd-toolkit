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

#pragma once
#ifndef ____sop_vhacd_engine_h____
#define ____sop_vhacd_engine_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// 3rdParty
#include <VHACD.h>

// hou-hdk-common
#include <Macros/Namespace.h>
#include <Macros/CookMySop.h>
#include <Macros/DescriptionPRM.h>
#include <Macros/UpdateParmsFlags.h>
#include <Enums/MethodProcessResult.h>

// this
#include "SOP_VHACDNode.h"
#include "UserLogger.h"
#include "UserCallback.h"

/* -----------------------------------------------------------------
FORWARDS                                                           |
----------------------------------------------------------------- */

class UT_AutoInterrupt;

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define ENUMS						GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
TYPEDEFS                                                           |
----------------------------------------------------------------- */

typedef std::vector<int>			VHACDTriangleIndexes;
typedef std::vector<float>			VHACDPointPositions;

/* -----------------------------------------------------------------
OPERATOR                                                           |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

	class SOP_VHACDEngine : public SOP_VHACDNode
	{
		DECLARE_CookMySop()
		DECLARE_DescriptionPRM_Callback()
		DECLARE_UpdateParmsFlags()		

	protected:
		~SOP_VHACDEngine() override;
		SOP_VHACDEngine(OP_Network* network, const char* name, OP_Operator* op);

	public:
		static OP_Node*				CreateMe(OP_Network* network, const char* name, OP_Operator* op);
		static PRM_Template			parametersList[];

	private:
		const char*					inputLabel(unsigned input) const override;

		exint						PullIntPRM(GU_Detail* detail, const PRM_Template& parameter, bool interfaceonly = false, fpreal time = 0);
		fpreal						PullFloatPRM(GU_Detail* detail, const PRM_Template& parameter, bool interfaceonly = false, fpreal time = 0);
		ENUMS::MethodProcessResult	PrepareGeometry(GU_Detail* detail, UT_AutoInterrupt progress, fpreal time);
		void						SetupVHACD(GU_Detail* geometry, fpreal time);
		ENUMS::MethodProcessResult	PrepareDataForVHACD(GU_Detail* detail, UT_AutoInterrupt progress, fpreal time);
		ENUMS::MethodProcessResult	DrawConvexHull(GU_Detail* detail, VHACD::IVHACD::ConvexHull hull, UT_AutoInterrupt progress);
		ENUMS::MethodProcessResult	GenerateConvexHulls(GU_Detail* detail, UT_AutoInterrupt progress);
			
		UserLogger					_loggerVHACD;
		UserCallback				_callbackVHACD;
		VHACD::IVHACD::Parameters	_parametersVHACD;
		VHACD::IVHACD*				_interfaceVHACD;

		VHACDTriangleIndexes		_triangleIndexes;
		VHACDPointPositions			_pointPositions;

		GA_RWAttributeRef			_positionReference;
		GA_RWHandleV3				_positionHandle;
		GA_RWHandleV3				_hullCenterHandle;
		GA_RWHandleD				_hullVolumeHandle;

		bool						_allowParametersOverrideValueState;		
		bool						_showReportValueState;
		int							_reportModeChoiceValueState;
	};

DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS

#endif // !____sop_vhacd_engine_h____