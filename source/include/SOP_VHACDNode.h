/*
	Base node class for VHACD toolkit.

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
#ifndef ____sop_vhacd_node_h____
#define ____sop_vhacd_node_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// SESI
#include <SOP/SOP_Node.h>

// hou-hdk-common
#include <Macros/Namespace.h>
#include <Macros/ProgressEscape.h>

// this
#include "VHACDCommonName.h"
#include "VHACDCommonParameterName.h"
#include "VHACDCommonAttributeName.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define CONTAINERS				GET_Base_Namespace()::Containers
#define ENUMS					GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
DEFAULT VARIABLES                                                  |
----------------------------------------------------------------- */

DECLARE_SOP_Namespace_Start()

// set here to be easily available for everything out there, by just accessing namespace
static CONTAINERS::VHACDCommonName				COMMON_NAMES = CONTAINERS::VHACDCommonName();
static CONTAINERS::VHACDCommonParameterName		COMMON_PRM_NAMES = CONTAINERS::VHACDCommonParameterName();

/* -----------------------------------------------------------------
DECLARATION                                                        |
----------------------------------------------------------------- */

class SOP_VHACDNode : public SOP_Node
{
protected:
	SOP_VHACDNode(OP_Network* network, const char* name, OP_Operator* op) : SOP_Node(network, name, op)
	{
		// defaut node color
		this->setColor(UT_Color(UT_ColorType::UT_RGB, 0.0, 0.0, 0.0));

		// default node icon
		op->setIconName(COMMON_NAMES.Get(ENUMS::VHACDCommonNameOption::TOOLKIT_ICONNAME));
	}

	CONTAINERS::VHACDCommonAttributeName		_commonAttributeNames = CONTAINERS::VHACDCommonAttributeName();
	
	GA_ROHandleI								_convexBundleIDHandle;
	GA_ROHandleI								_originalBundleIDHandle;

	GA_RWHandleI								_hullCountHandle;
	GA_RWHandleI								_hullIDHandle;
	GA_RWHandleD								_hullVolumeHandle;

	GA_RWHandleI								_bundleCountHandle;
	GA_RWHandleI								_bundleIDHandle;
};

DECLARE_SOP_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS

#endif // !____sop_vhacd_node_h____