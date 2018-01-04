/*
	Container for common names.

	IMPORTANT! ------------------------------------------
	* this should be synchronized with VHACDCommonNameOption.h
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
#ifndef ____vhacd_common_name_h____
#define ____vhacd_common_name_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// hou-hdk-common
#include <Macros/Namespace.h>
#include <Containers/CommonTName.h>
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
	class VHACDCommonName final : public CONTAINERS::CommonTName<ENUMS::VHACDCommonNameOption>
	{
	public:
		VHACDCommonName() : CommonTName<ENUMS::VHACDCommonNameOption>()
		{
			// toolkit global
			this->Add(ENUMS::VHACDCommonNameOption::TOOLKIT_TABMENU_PATH,				"Toolkit/V-HACD");
			this->Add(ENUMS::VHACDCommonNameOption::TOOLKIT_ICONNAME,					"SOP_VHACD.png");			

			this->Add(ENUMS::VHACDCommonNameOption::SOP_OUTPUTNAME_CONVEXHULLS,			"Convex Hulls");
			this->Add(ENUMS::VHACDCommonNameOption::SOP_OUTPUTNAME_ORIGINALGEOMETRY,	"Original Geometry");

			// SOP_VHACDDelete 2.0 only			
			this->Add(ENUMS::VHACDCommonNameOption::SOP_DELETE_ICONNAME,				this->Get(ENUMS::VHACDCommonNameOption::TOOLKIT_ICONNAME));
			this->Add(ENUMS::VHACDCommonNameOption::SOP_DELETE_SMALLNAME,				"wip::delete::2.0");
			this->Add(ENUMS::VHACDCommonNameOption::SOP_DELETE_BIGNAME,					"Delete (v-hacd)");			
			
			// SOP_VHACDEngine only			
			this->Add(ENUMS::VHACDCommonNameOption::SOP_ENGINE_ICONNAME,				this->Get(ENUMS::VHACDCommonNameOption::TOOLKIT_ICONNAME));
			this->Add(ENUMS::VHACDCommonNameOption::SOP_ENGINE_SMALLNAME,				"vhacd::engine::1.2");
			this->Add(ENUMS::VHACDCommonNameOption::SOP_ENGINE_BIGNAME,					"Engine (v-hacd)");

			// SOP_VHACDGenerate 2.0 only			
			this->Add(ENUMS::VHACDCommonNameOption::SOP_GENERATE_ICONNAME,				this->Get(ENUMS::VHACDCommonNameOption::TOOLKIT_ICONNAME));
			this->Add(ENUMS::VHACDCommonNameOption::SOP_GENERATE_SMALLNAME,				"wip::generate::2.0");
			this->Add(ENUMS::VHACDCommonNameOption::SOP_GENERATE_BIGNAME,				"Generate (v-hacd)");
			this->Add(ENUMS::VHACDCommonNameOption::SOP_GENERATE_GROUP_PRMNAME,			CONST_PrimitiveGroupInput0_Name);
			this->Add(ENUMS::VHACDCommonNameOption::MSS_GENERATE_SMALLNAME,				"wip::generateselector::2.0");
			this->Add(ENUMS::VHACDCommonNameOption::MSS_GENERATE_BIGNAME,				"Generate (v-hacd selector)");
			this->Add(ENUMS::VHACDCommonNameOption::MSS_GENERATE_PROMPT,				"Select primitives. Press <enter> to accept.");

			// SOP_VHACDScoutJunior 1.0 only
			this->Add(ENUMS::VHACDCommonNameOption::SOP_SCOUT_JUNIOR_ICONNAME,			this->Get(ENUMS::VHACDCommonNameOption::TOOLKIT_ICONNAME));
			this->Add(ENUMS::VHACDCommonNameOption::SOP_SCOUT_JUNIOR_SMALLNAME,			"vhacd::scoutjunior::1.0");
			this->Add(ENUMS::VHACDCommonNameOption::SOP_SCOUT_JUNIOR_BIGNAME,			"Scout Jr. (v-hacd)");

			// SOP_VHACDScoutSenior 1.0 only
			this->Add(ENUMS::VHACDCommonNameOption::SOP_SCOUT_SENIOR_ICONNAME,			this->Get(ENUMS::VHACDCommonNameOption::TOOLKIT_ICONNAME));
			this->Add(ENUMS::VHACDCommonNameOption::SOP_SCOUT_SENIOR_SMALLNAME,			"vhacd::scoutsenior::1.0");
			this->Add(ENUMS::VHACDCommonNameOption::SOP_SCOUT_SENIOR_BIGNAME,			"Scout Sr. (v-hacd)");

			// SOP_VHACDSetup 2.0 only			
			this->Add(ENUMS::VHACDCommonNameOption::SOP_SETUP_ICONNAME,					this->Get(ENUMS::VHACDCommonNameOption::TOOLKIT_ICONNAME));
			this->Add(ENUMS::VHACDCommonNameOption::SOP_SETUP_SMALLNAME,				"vhacd::setup::2.0");
			this->Add(ENUMS::VHACDCommonNameOption::SOP_SETUP_BIGNAME,					"Setup (v-hacd)");
			this->Add(ENUMS::VHACDCommonNameOption::SOP_SETUP_GROUP_PRMNAME,			CONST_PrimitiveGroupInput0_Name);
			this->Add(ENUMS::VHACDCommonNameOption::MSS_SETUP_SMALLNAME,				"vhacd::setupselector::2.0");
			this->Add(ENUMS::VHACDCommonNameOption::MSS_SETUP_BIGNAME,					"Setup (v-hacd selector)");
			this->Add(ENUMS::VHACDCommonNameOption::MSS_SETUP_PROMPT,					"Select primitives. Press <enter> to accept.");

			// SOP_VHACDMerge 2.0 only			
			this->Add(ENUMS::VHACDCommonNameOption::SOP_MERGE_ICONNAME,					this->Get(ENUMS::VHACDCommonNameOption::TOOLKIT_ICONNAME));
			this->Add(ENUMS::VHACDCommonNameOption::SOP_MERGE_SMALLNAME,				"vhacd::merge::2.0");
			this->Add(ENUMS::VHACDCommonNameOption::SOP_MERGE_BIGNAME,					"Merge (v-hacd)");

			// SOP_VHACDTransform 2.0 only			
			this->Add(ENUMS::VHACDCommonNameOption::SOP_TRANSFORM_ICONNAME,				this->Get(ENUMS::VHACDCommonNameOption::TOOLKIT_ICONNAME));
			this->Add(ENUMS::VHACDCommonNameOption::SOP_TRANSFORM_SMALLNAME,			"wip::transfrom::2.0");
			this->Add(ENUMS::VHACDCommonNameOption::SOP_TRANSFORM_BIGNAME,				"Transform (v-hacd)");
		}
	};
}
DECLARE_Base_Namespace_End

/* -----------------------------------------------------------------
UNDEFINE                                                           |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS

#endif // !____vhacd_common_name_h____