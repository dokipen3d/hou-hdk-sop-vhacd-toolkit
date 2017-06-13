/*
	Container for common names.

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
#ifndef ____commonname_h____
#define ____commonname_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// SESI
#include <UT/UT_Map.h>

// this
#include "CommonNameOption.h"

/* -----------------------------------------------------------------
DECLARATION                                                        |
----------------------------------------------------------------- */

class CommonName
{
	void									Add(CommonNameOption key, const char* value) { this->_names[key] = value; }
	UT_Map<CommonNameOption, const char*>	_names;

public:
	const char*								Get(CommonNameOption option) { return this->_names[option]; }

	CommonName()
	{
		Add(CommonNameOption::ICON_NAME, "SOP_VHACD.png");

		Add(CommonNameOption::DECOMPOSITION_MODE, "decompositionmode");
		Add(CommonNameOption::RESOLUTION, "resolution");
		Add(CommonNameOption::DEPTH, "depth");
		Add(CommonNameOption::CONCAVITY, "concavity");
		Add(CommonNameOption::PLANE_DOWNSAMPLING, "planedownsampling");
		Add(CommonNameOption::CONVEXHULL_DOWNSAMPLING, "convexhulldownsampling");
		Add(CommonNameOption::ALPHA, "alpha");
		Add(CommonNameOption::BETA, "beta");
		Add(CommonNameOption::GAMMA, "gamma");
		Add(CommonNameOption::MAX_TRIANGLE_COUNT, "maxtrianglecount");
		Add(CommonNameOption::ADAPTIVE_SAMPLING, "adaptivesampling");
		Add(CommonNameOption::NORMALIZE_MESH, "normalizemesh");
		Add(CommonNameOption::USE_OPENCL, "useopencl");

		Add(CommonNameOption::VOXEL, "Voxel");
		Add(CommonNameOption::TETRAHEDRON, "Tetrahedron");
	}	
};

#endif // !____commonname_h____