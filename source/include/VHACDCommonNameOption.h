/*
	Enum for common names.

	IMPORTANT! ------------------------------------------
	* this should be synchronized with VHACDCommonName.h
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
#ifndef ____vhacd_common_name_option_h____
#define ____vhacd_common_name_option_h____

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
	enum class VHACDCommonNameOption : exint
	{
		// toolkit global
		TOOLKIT_TABMENU_PATH,
		TOOLKIT_ICONNAME,

		SOP_OUTPUTNAME_CONVEXHULLS,
		SOP_OUTPUTNAME_ORIGINALGEOMETRY,

		// SOP_VHACDDebug only
		SOP_DEBUG_ICONNAME,
		SOP_DEBUG_SMALLNAME,
		SOP_DEBUG_BIGNAME,
		GUI_DEBUG_SMALLNAME,
		GUI_DEBUG_BIGNAME,

		// SOP_VHACDDelete only
		SOP_DELETE_ICONNAME,
		SOP_DELETE_SMALLNAME,
		SOP_DELETE_BIGNAME,	
		SOP_DELETE_GROUP_PRMNAME,
		MSS_DELETE_SMALLNAME,
		MSS_DELETE_BIGNAME,
		MSS_DELETE_PROMPT,
		SOP_DELETE_OTL_SMALLNAME,		// TODO: remove this when selector crash for operator will be solved

		// SOP_VHACDGenerate only
		SOP_GENERATE_ICONNAME,
		SOP_GENERATE_SMALLNAME,
		SOP_GENERATE_BIGNAME,
		SOP_GENERATE_GROUP_PRMNAME,
		MSS_GENERATE_SMALLNAME,
		MSS_GENERATE_BIGNAME,
		MSS_GENERATE_PROMPT,
		SOP_GENERATE_OTL_SMALLNAME,		// TODO: remove this when selector crash for operator will be solved

		// SOP_VHACDScoutJunior only
		SOP_SCOUT_JUNIOR_ICONNAME,
		SOP_SCOUT_JUNIOR_SMALLNAME,
		SOP_SCOUT_JUNIOR_BIGNAME,

		// SOP_VHACDScoutSenior only
		SOP_SCOUT_SENIOR_ICONNAME,
		SOP_SCOUT_SENIOR_SMALLNAME,
		SOP_SCOUT_SENIOR_BIGNAME,

		// SOP_VHACDSetup only
		SOP_SETUP_ICONNAME,
		SOP_SETUP_SMALLNAME,
		SOP_SETUP_BIGNAME,
		SOP_SETUP_GROUP_PRMNAME,
		MSS_SETUP_SMALLNAME,
		MSS_SETUP_BIGNAME,
		MSS_SETUP_PROMPT,

		// SOP_VHACDMerge only
		SOP_MERGE_ICONNAME,
		SOP_MERGE_SMALLNAME,
		SOP_MERGE_BIGNAME,

		// SOP_VHACDTransform only
		SOP_TRANSFORM_ICONNAME,
		SOP_TRANSFORM_SMALLNAME,
		SOP_TRANSFORM_BIGNAME,
		SOP_TRANSFORM_GROUP_PRMNAME,
		MSS_TRANSFORM_SMALLNAME,
		MSS_TRANSFORM_BIGNAME,
		MSS_TRANSFORM_PROMPT,
		SOP_TRANSFORM_OTL_SMALLNAME		// TODO: remove this when selector crash for operator will be solved
	};
}
DECLARE_Base_Namespace_End

#endif // !____vhacd_common_name_option_h____