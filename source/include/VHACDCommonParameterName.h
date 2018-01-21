/*
	Container for common parameter names.

	IMPORTANT! ------------------------------------------
	* this should be synchronized with VHACDCommonParameterNameOption.h
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
#ifndef ____vhacd_common_parameter_name_h____
#define ____vhacd_common_parameter_name_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// hou-hdk-common
#include <Macros/Namespace.h>
#include <Containers/CommonTName.h>
#include <Macros/GroupMenuPRM.h>

// this
#include "VHACDCommonParameterNameOption.h"

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
	class VHACDCommonParameterName final : public CONTAINERS::CommonTName<ENUMS::VHACDCommonParameterNameOption>
	{
	public:
		VHACDCommonParameterName() : CommonTName<ENUMS::VHACDCommonParameterNameOption>()
		{
			// vhacd global
			this->Add(ENUMS::VHACDCommonParameterNameOption::ALPHA,							"alpha");
			this->Add(ENUMS::VHACDCommonParameterNameOption::BETA,							"beta");
			this->Add(ENUMS::VHACDCommonParameterNameOption::CONCAVITY,						"concavity");
			this->Add(ENUMS::VHACDCommonParameterNameOption::CONVEXHULL_APPROXIMATION,		"convexhullapproximation");
			this->Add(ENUMS::VHACDCommonParameterNameOption::CONVEXHULL_DOWNSAMPLING,		"convexhulldownsampling");
			this->Add(ENUMS::VHACDCommonParameterNameOption::MAX_CONVEXHULLS_COUNT,			"maxconvexhullscount");
			this->Add(ENUMS::VHACDCommonParameterNameOption::MAX_TRIANGLE_COUNT,			"maxtrianglecount");
			this->Add(ENUMS::VHACDCommonParameterNameOption::ADAPTIVE_SAMPLING,				"adaptivesampling");
			this->Add(ENUMS::VHACDCommonParameterNameOption::DECOMPOSITION_MODE,			"decompositionmode");
			this->Add(ENUMS::VHACDCommonParameterNameOption::USE_OCL,						"useopencl");
			this->Add(ENUMS::VHACDCommonParameterNameOption::NORMALIZE_MESH,				"normalizemesh");
			this->Add(ENUMS::VHACDCommonParameterNameOption::PLANE_DOWNSAMPLING,			"planedownsampling");
			this->Add(ENUMS::VHACDCommonParameterNameOption::PROJECT_HULL_VERTICES,			"projecthullvertices");
			this->Add(ENUMS::VHACDCommonParameterNameOption::RESOLUTION,					"resolution");

			// decomposition mode only
			this->Add(ENUMS::VHACDCommonParameterNameOption::VOXEL,							"Voxel");
			this->Add(ENUMS::VHACDCommonParameterNameOption::TETRAHEDRON,					"Tetrahedron");
		}
	};
}
DECLARE_Base_Namespace_End

/* -----------------------------------------------------------------
UNDEFINE                                                           |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS

#endif // !____vhacd_common_parameter_name_h____