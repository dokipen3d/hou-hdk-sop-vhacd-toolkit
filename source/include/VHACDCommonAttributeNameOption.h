/*
	Enum for common attribute names.

	IMPORTANT! ------------------------------------------
	* this should be synchronized with VHACDCommonAttributeName.h
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
#ifndef ____vhacd_common_attribute_name_option_h____
#define ____vhacd_common_attribute_name_option_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// SESI
#if _WIN32		
	#include <sys/SYS_Types.h>
#else
	#include <SYS/SYS_Types.h>
#endif

// hou-hdk-common
#include "Macros/Namespace.h"

/* -----------------------------------------------------------------
ENUM                                                               |
----------------------------------------------------------------- */

DECLARE_Base_Namespace_Start()
namespace Enums
{
	enum class VHACDCommonAttributeNameOption : exint
	{
		HULL_COUNT,
		HULL_ID,
		HULL_VOLUME,

		BUNDLE_COUNT,
		BUNDLE_ID,

		MAX_ENUM_SIZE
	};
}
DECLARE_Base_Namespace_End

#endif // !____vhacd_common_attribute_name_option_h____