/*
	Clean template of SOP operator with selector and group input support.

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
#ifndef ____sop_vhacdscout_h____
#define ____sop_vhacdscout_h____

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

// this
#include "SOP_VHACDNode.h"

/* -----------------------------------------------------------------
FORWARDS                                                           |
----------------------------------------------------------------- */

class UT_AutoInterrupt;

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

/*
	0 = GA_PointGroup
	1 = GA_EdgeGroup
	2 = GA_PrimitiveGroup
	no number means no group support
*/
#define ____GROUP_MODE____ 0

#define CONTAINERS					GET_Base_Namespace()::Containers
#define ENUMS						GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
DECLARATION                                                        |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

	class SOP_Scout : public SOP_VHACDNode
	{
		DECLARE_CookMySop()
		DECLARE_UpdateParmsFlags()

		DECLARE_DescriptionPRM_Callback()

	protected:
		~SOP_Scout() override;
		SOP_Scout(OP_Network* network, const char* name, OP_Operator* op);
		const char*					inputLabel(unsigned input) const override;

	public:
		static OP_Node*				CreateMe(OP_Network* network, const char* name, OP_Operator* op);
		OP_ERROR					cookInputGroups(OP_Context& context, int alone = 0) override;

		static PRM_Template			parametersList[];

	private:	

#if ____GROUP_MODE____ == 0
		const GA_PointGroup*		_pointGroupInput0;
#elif ____GROUP_MODE____ == 1
		const GA_EdgeGroup*			_edgeGroupInput0;
#elif ____GROUP_MODE____ == 2
		const GA_PrimitiveGroup*	_primitiveGroupInput0;
#else
		// do nothing
#endif // ____GROUP_MODE____

	};

/* -----------------------------------------------------------------
SELECTOR DECLARATION                                               |
----------------------------------------------------------------- */

class MSS_ScoutSelector : public MSS_ReusableSelector
{
public:
	virtual ~MSS_ScoutSelector();
	MSS_ScoutSelector(OP3D_View& viewer, PI_SelectorTemplate& templ);

	static BM_InputSelector*			CreateMe(BM_View& Viewer, PI_SelectorTemplate& templ);
	const char*							className() const override;
};

DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS

#endif // !____sop_vhacdscout_h____