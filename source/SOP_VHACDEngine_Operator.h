/*
	Volumetric-Hierarchical Approximate Convex Decomposition.
	Based on https://github.com/kmammou/v-hacd

	IMPORTANT! ------------------------------------------
	* Macros starting and ending with '____' shouldn't be used anywhere outside of this file.
	* External macros used:
		DECLARE_SOP_Namespace_Start()		- comes from "Macros_Namespace.h"
		DECLARE_SOP_Namespace_End			- comes from "Macros_Namespace.h"	

		DECLARE_DescriptionPRM_Callback()	- comes from "Macros_DescriptionPRM.h"
		DECLARE_UpdateParmsFlags()			- comes from "Macros_UpdateParmsFlags.h"
		DECLARE_Switch_VisibilityState()	- comes from "Macros_UpdateParmsFlags.h"
		DECLARE_CookMySop()					- comes from "Macros_CookMySop.h"
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

#pragma once

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

#include "../../hou-hdk-common/source/SOP/SOP_Base_Operator.h"
#include "../../hou-hdk-common/source/SOP/Macros_CookMySop.h"
#include "../../hou-hdk-common/source/SOP/Macros_DescriptionPRM.h"
#include "../../hou-hdk-common/source/SOP/Macros_ParameterList.h"
#include "../../hou-hdk-common/source/SOP/Macros_UpdateParmsFlags.h"

#include <vector>
#include <string>
#include <iomanip>

#include "../3rdParty/include/VHACD.h"

/* -----------------------------------------------------------------
FORWARDS                                                           |
----------------------------------------------------------------- */

class UT_AutoInterrupt;

/* -----------------------------------------------------------------
DECLARATION                                                        |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

	class SOP_VHACDEngine_Logger : public VHACD::IVHACD::IUserLogger
	{
	public:
		SOP_VHACDEngine_Logger();
		~SOP_VHACDEngine_Logger() override;

		void						Log(const char* const msg) override;

		bool						_showMsg;
	};

	class SOP_VHACDEngine_Callback : public VHACD::IVHACD::IUserCallback
	{
	public:
		SOP_VHACDEngine_Callback();
		~SOP_VHACDEngine_Callback() override;

		void						Update(const double overallProgress, const double stageProgress, const double operationProgress, const char* const stage, const char* const operation) override;

		bool						_showOverallProgress;
		bool						_showStageProgress;
		bool						_showOperationProgress;
	};

	class SOP_VHACDEngine_Operator : public SOP_Base_Operator
	{
		DECLARE_DescriptionPRM_Callback()		
		DECLARE_UpdateParmsFlags()
		DECLARE_CookMySop()

		// TODO: Do I still need this?
		DECLARE_Switch_VisibilityState()

	protected:
		SOP_VHACDEngine_Operator(OP_Network* network, const char* name, OP_Operator* op);
		virtual ~SOP_VHACDEngine_Operator() override;

	public:
		static OP_Node*				CreateOperator(OP_Network* network, const char* name, OP_Operator* op);

		static PRM_Template parametersList[];

	private:
		virtual const char*			inputLabel(unsigned input) const override;

		exint						Pull_IntPRM(GU_Detail* geometry, const PRM_Template& parameter, bool interfaceonly = false, fpreal time = 0);
		fpreal						Pull_FloatPRM(GU_Detail* geometry, const PRM_Template& parameter, bool interfaceonly = false, fpreal time = 0);
		bool						Prepare_Geometry(GU_Detail* geometry, UT_AutoInterrupt progress);
		void						Setup_VHACD(GU_Detail* geometry, fpreal time);
		bool						Prepare_DataForVHACD(GU_Detail* geometry, UT_AutoInterrupt progress, fpreal time);
		OP_ERROR					Generate_ConvexHulls(GU_Detail* geometry, UT_AutoInterrupt progress);
		bool						Draw_ConvexHull(GU_Detail* geometry, int hullid, VHACD::IVHACD::ConvexHull hull, UT_AutoInterrupt progress);
			
		VHACD::IVHACD::Parameters	_parametersVHACD;
		VHACD::IVHACD*				_interfaceVHACD;
		std::vector<int>			_triangles;
		std::vector<float>			_points;

		GA_RWAttributeRef			_positionReference;
		GA_RWHandleV3				_positionHandle;

		bool						_currentAllowParametersOverrideValueState;
		bool						_polygonizeValueState;
		bool						_currentShowReportValueState;
		int							_currentReportModeChoiceValueState;	

		SOP_VHACDEngine_Logger		_loggerVHACD;
		SOP_VHACDEngine_Callback	_callbackVHACD;
	};

DECLARE_SOP_Namespace_End