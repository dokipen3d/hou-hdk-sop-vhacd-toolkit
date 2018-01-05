/*
	Container for common attribute names.

	IMPORTANT! ------------------------------------------
	* this should be synchronized with VHACDCommonAttributeNameOption.h
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
#ifndef ____vhacd_common_attribute_name_h____
#define ____vhacd_common_attribute_name_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// SESI
#include <UT/UT_Map.h>

// hou-hdk-common
#include <Macros/Namespace.h>
#include <Containers/CommonTName.h>

// this
#include "VHACDCommonAttributeNameOption.h"

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
	class VHACDCommonAttributeName final : public CONTAINERS::CommonTName<ENUMS::VHACDCommonAttributeNameOption>
	{
	public:
		VHACDCommonAttributeName() : CommonTName<ENUMS::VHACDCommonAttributeNameOption>()
		{
			this->Add(ENUMS::VHACDCommonAttributeNameOption::HULL_COUNT,			"hull_count");
			this->Add(ENUMS::VHACDCommonAttributeNameOption::HULL_ID,				"hull_id");
			this->Add(ENUMS::VHACDCommonAttributeNameOption::HULL_VOLUME,			"hull_volume");
			this->Add(ENUMS::VHACDCommonAttributeNameOption::HULL_CENTER,			"hull_center");
			this->Add(ENUMS::VHACDCommonAttributeNameOption::HULL_BUNDLE_COUNT,		"bundle_count");
			this->Add(ENUMS::VHACDCommonAttributeNameOption::HULL_BUNDLE_ID,		"bundle_id");

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