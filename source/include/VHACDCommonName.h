/*
	Container for common attribute names.

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
#ifndef ____vhacd_common_name_h____
#define ____vhacd_common_name_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// hou-hdk-common
#include <Macros/Namespace.h>
#include <Containers/CommonNameT.h>
#include <Macros/GroupMenuPRM.h>

// this
#include "VHACDCommonNameOption.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define CONTAINERS	GET_Base_Namespace()::Containers
#define ENUMS		GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
DECLARATION                                                        |
----------------------------------------------------------------- */

DECLARE_Base_Namespace_Start()
namespace Containers
{	
	class VHACDCommonName final : public CONTAINERS::CommonNameT<ENUMS::VHACDCommonNameOption>
	{
	public:
		VHACDCommonName() : CommonNameT<ENUMS::VHACDCommonNameOption>()
		{
			// toolkit global
			this->Add(ENUMS::VHACDCommonNameOption::TOOLKIT_TABMENU_PATH, "Toolkit/V-HACD");
			this->Add(ENUMS::VHACDCommonNameOption::TOOLKIT_ICONNAME, "SOP_VHACD.png");			
			
			// SOP_VHACDEngine only
			this->Add(ENUMS::VHACDCommonNameOption::SOP_ENGINE_ICONNAME, this->Get(ENUMS::VHACDCommonNameOption::TOOLKIT_ICONNAME));
			this->Add(ENUMS::VHACDCommonNameOption::SOP_ENGINE_SMALLNAME, "vhacd::engine::1.2");
			this->Add(ENUMS::VHACDCommonNameOption::SOP_ENGINE_BIGNAME, "Engine (v-hacd)");

			// SOP_VHACDSetup only
			this->Add(ENUMS::VHACDCommonNameOption::SOP_SETUP_ICONNAME, this->Get(ENUMS::VHACDCommonNameOption::TOOLKIT_ICONNAME));
			this->Add(ENUMS::VHACDCommonNameOption::SOP_SETUP_SMALLNAME, "vhacd::setup::1.2");
			this->Add(ENUMS::VHACDCommonNameOption::SOP_SETUP_BIGNAME, "Setup (v-hacd)");
			this->Add(ENUMS::VHACDCommonNameOption::SOP_SETUP_GROUP_PRMNAME, CONST_PrimitiveGroupInput0_Name);
			this->Add(ENUMS::VHACDCommonNameOption::MSS_SETUP_SMALLNAME, "vhacd::setupselector::1.2");
			this->Add(ENUMS::VHACDCommonNameOption::MSS_SETUP_BIGNAME, "Setup (v-hacd selector)");
			this->Add(ENUMS::VHACDCommonNameOption::MSS_SETUP_PROMPT, "Select primitives. Press <enter> to accept.");

			// SOP_VHACDMerge 2.0 only
			this->Add(ENUMS::VHACDCommonNameOption::SOP_MERGE_ICONNAME_V2, this->Get(ENUMS::VHACDCommonNameOption::TOOLKIT_ICONNAME));
			this->Add(ENUMS::VHACDCommonNameOption::SOP_MERGE_SMALLNAME_V2, "wip::merge::2.0");
			this->Add(ENUMS::VHACDCommonNameOption::SOP_MERGE_BIGNAME_V2, "Merge v2.0 (v-hacd) (don't use it yet)");
		}
	};
}
DECLARE_Base_Namespace_End

/* -----------------------------------------------------------------
UNDEFINE                                                           |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS

#endif // !____vhacd_common_attribute_name_h____