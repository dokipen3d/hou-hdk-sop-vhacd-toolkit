/*
	Volumetric-Hierarchical Approximate Convex Decomposition.
	Based on https://github.com/kmammou/v-hacd

	IMPORTANT! ------------------------------------------
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
#ifndef ____sop_vhacdsetup_h____
#define ____sop_vhacdsetup_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// SESI
#include <SOP/SOP_Node.h>
#include <MSS/MSS_ReusableSelector.h>

// hou-hdk-common
#include <Macros/CookMySop.h>
#include <Macros/DescriptionPRM.h>
#include <Macros/Namespace.h>
#include <Macros/UpdateParmsFlags.h>

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

//#define DEBUG_THIS

/* -----------------------------------------------------------------
DECLARATION                                                        |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

	class SOP_VHACDSetup : public SOP_Node
	{
		DECLARE_CookMySop()
		DECLARE_UpdateParmsFlags()

		DECLARE_DescriptionPRM_Callback()

	protected:
		virtual ~SOP_VHACDSetup() override;
		SOP_VHACDSetup(OP_Network* network, const char* name, OP_Operator* op);
		virtual const char*			inputLabel(unsigned input) const override;

	public:
		static OP_Node*				CreateMe(OP_Network* network, const char* name, OP_Operator* op);
		virtual OP_ERROR			cookInputGroups(OP_Context& context, int alone = 0);
		static PRM_Template			parametersList[];

		static int					CallbackProcessModeChoiceMenu(void* data, int index, float time, const PRM_Template* tmp);
		static int					CallbackAddDecompositionModeATT(void* data, int index, float time, const PRM_Template* tmp);
		static int					CallbackAddResolutionATT(void* data, int index, float time, const PRM_Template* tmp);
		static int					CallbackAddDepthATT(void* data, int index, float time, const PRM_Template* tmp);
		static int					CallbackAddConcavityATT(void* data, int index, float time, const PRM_Template* tmp);
		static int					CallbackAddPlaneDownsamplingATT(void* data, int index, float time, const PRM_Template* tmp);
		static int					CallbackAddConvexHullDownsamplingATT(void* data, int index, float time, const PRM_Template* tmp);
		static int					CallbackAddAlphaATT(void* data, int index, float time, const PRM_Template* tmp);
		static int					CallbackAddBetaATT(void* data, int index, float time, const PRM_Template* tmp);
		static int					CallbackAddGammaATT(void* data, int index, float time, const PRM_Template* tmp);
		static int					CallbackAddMaxTriangleCountATT(void* data, int index, float time, const PRM_Template* tmp);
		static int					CallbackAddAdaptiveSamplingATT(void* data, int index, float time, const PRM_Template* tmp);
		static int					CallbackAddNormalizeMeshATT(void* data, int index, float time, const PRM_Template* tmp);		

	private:		
		OP_ERROR					ProcessWholeGeometry(UT_AutoInterrupt progress, fpreal time);
		OP_ERROR					ProcessSpecifiedGeometry(UT_AutoInterrupt progress, fpreal time);

		void						AddIntATT(GA_RWHandleI& attributeHandle, const PRM_Template& valueparameter, const PRM_Template& fallbackparameter, const char* attributename, fpreal time);
		void						AddFloatATT(GA_RWHandleR& attributeHandle, const PRM_Template& valueparameter, const PRM_Template& fallbackparameter, const char* attributename, fpreal time);
		void						UpdateIntATT(GA_RWHandleI& attributeHandle, GA_Offset offset, const PRM_Template& valueparameter, fpreal time);
		void						UpdateFloatATT(GA_RWHandleR& attributeHandle, GA_Offset offset, const PRM_Template& valueparameter, fpreal time);
		OP_ERROR					SetupAttributes(GA_Iterator primit, UT_AutoInterrupt progress, fpreal time);

		const GA_PrimitiveGroup*	_primitiveGroupInput0;		
	};

DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
SELECTOR DECLARATION                                               |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

class MSS_VHACDSetup : public MSS_ReusableSelector
{
public:
	virtual ~MSS_VHACDSetup();
	MSS_VHACDSetup(OP3D_View& viewer, PI_SelectorTemplate& templ);

	static BM_InputSelector*		CreateMe(BM_View& Viewer, PI_SelectorTemplate& templ);
	virtual const char*				className() const;
};

DECLARE_SOP_Namespace_End

#endif // !____sop_vhacdsetup_h____