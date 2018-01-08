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
#ifndef ____sop_vhacd_generate_h____
#define ____sop_vhacd_generate_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// SESI
#include <MSS/MSS_ReusableSelector.h>

// 3rdParty
#include <VHACD.h>

// hou-hdk-common
#include <Macros/CookMySop.h>
#include <Macros/DescriptionPRM.h>
#include <Macros/Namespace.h>
#include <Macros/UpdateParmsFlags.h>
#include <Enums/MethodProcessResult.h>

// this
#include "SOP_VHACDNode.h"
#include "UserLogger.h"
#include "UserCallback.h"
#include "ProcessedOutputType.h"

/* -----------------------------------------------------------------
FORWARDS                                                           |
----------------------------------------------------------------- */

class UT_AutoInterrupt;

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define CONTAINERS					GET_Base_Namespace()::Containers
#define ENUMS						GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
TYPEDEFS                                                           |
----------------------------------------------------------------- */

typedef std::vector<int>			VHACDTriangleIndexes;
typedef std::vector<float>			VHACDPointPositions;

/* -----------------------------------------------------------------
DECLARATION                                                        |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

	class SOP_VHACDGenerate final : public SOP_VHACDNode
	{
		DECLARE_CookMySop_Multi()
		DECLARE_UpdateParmsFlags()

		DECLARE_DescriptionPRM_Callback()

	protected:
		~SOP_VHACDGenerate() override;
		SOP_VHACDGenerate(OP_Network* network, const char* name, OP_Operator* op);
		const char*					inputLabel(unsigned input) const override;

	public:
		static OP_Node*				CreateMe(OP_Network* network, const char* name, OP_Operator* op);
		OP_ERROR					cookInputGroups(OP_Context& context, int alone = 0) override;

		static PRM_Template			parametersList[];

		static int					CallbackShowProcessReport(void* data, int index, float time, const PRM_Template* tmp);

	private:
		exint						PullIntPRM(GU_Detail* geometry, const PRM_Template& parameter, fpreal time);
		fpreal						PullFloatPRM(GU_Detail* geometry, const PRM_Template& parameter, fpreal time);

		ENUMS::MethodProcessResult	SeparatePrimitiveRange(GU_Detail* detail);
		ENUMS::MethodProcessResult	PrepareGeometry(GU_Detail* detail, UT_AutoInterrupt progress, fpreal time);
		void						SetupParametersVHACD(GU_Detail* detail, fpreal time);
		ENUMS::MethodProcessResult	GatherDataForVHACD(GU_Detail* detail, UT_AutoInterrupt progress, fpreal time);

		ENUMS::MethodProcessResult	DrawConvexHull(GU_Detail* detail, const VHACD::IVHACD::ConvexHull& hull, UT_AutoInterrupt progress);
		ENUMS::MethodProcessResult	GenerateConvexHulls(GU_Detail* detail, UT_AutoInterrupt progress);
		ENUMS::MethodProcessResult	MergeCurrentDetail(const GU_Detail* detail, exint detailscount = 1, exint iteration = 0);
		ENUMS::MethodProcessResult	ProcessCurrentDetail(GU_Detail* detail, UT_AutoInterrupt progress, ENUMS::ProcessedOutputType processedoutputtype, exint iteration, fpreal time);

		ENUMS::MethodProcessResult	WhenAsWhole(UT_AutoInterrupt progress, ENUMS::ProcessedOutputType processedoutputtype, fpreal time);
		ENUMS::MethodProcessResult	WhenPerElement(UT_AutoInterrupt progress, ENUMS::ProcessedOutputType processedoutputtype, fpreal time);
		ENUMS::MethodProcessResult	WhenPerGroup(UT_AutoInterrupt progress, ENUMS::ProcessedOutputType processedoutputtype, fpreal time);

		GroupCreator				_gop;
		GU_Detail*					_inputGDP;
		const GA_PrimitiveGroup*	_primitiveGroupInput0;

		UserLogger					_loggerVHACD;
		UserCallback				_callbackVHACD;
		VHACD::IVHACD::Parameters	_parametersVHACD;
		VHACD::IVHACD*				_interfaceVHACD;

		VHACDTriangleIndexes		_triangleIndexes;
		VHACDPointPositions			_pointPositions;

		GA_RWHandleV3				_positionHandle;		
		GA_RWHandleV3				_hullCenterHandle;		
	};

/* -----------------------------------------------------------------
SELECTOR DECLARATION                                               |
----------------------------------------------------------------- */

	class MSS_VHACDGenerate : public MSS_ReusableSelector
	{
	public:
		virtual ~MSS_VHACDGenerate();
		MSS_VHACDGenerate(OP3D_View& viewer, PI_SelectorTemplate& templ);

		static BM_InputSelector*	CreateMe(BM_View& Viewer, PI_SelectorTemplate& templ);
		const char*					className() const override;
	};

DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS

#endif // !____sop_vhacd_generate_h____