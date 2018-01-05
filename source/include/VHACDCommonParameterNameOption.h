/*
	Enum for common parameter names.

	IMPORTANT! ------------------------------------------
	* this should be synchronized with VHACDCommonParameterName.h
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
#ifndef ____vhacd_common_parameter_name_option_h____
#define ____vhacd_common_parameter_name_option_h____

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
	enum class VHACDCommonParameterNameOption : exint
	{	
		// vhacd global
		ALPHA,
		BETA,
		CONCAVITY,
		CONVEXHULL_APPROXIMATION,
		CONVEXHULL_DOWNSAMPLING,
		MAX_CONVEXHULLS_COUNT,
		MAX_TRIANGLE_COUNT,
		ADAPTIVE_SAMPLING,
		DECOMPOSITION_MODE,
		USE_OPENCL,
		NORMALIZE_MESH,
		PLANE_DOWNSAMPLING,
		PROJECT_HULL_VERTICES,
		RESOLUTION,

		// decomposition mode only		
		VOXEL,
		TETRAHEDRON
	};
}
DECLARE_Base_Namespace_End

#endif // !____vhacd_common_parameter_name_option_h____