/*
	Container for storing hull data that can be preserved in center point.

	IMPORTANT! ------------------------------------------
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
#ifndef ____hull_center_data_h____
#define ____hull_center_data_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// SESI
#include <GA/GA_Types.h>
#include <UT/UT_VectorTypes.h>

// hou-hdk-common
#include "Macros/Namespace.h"

/* -----------------------------------------------------------------
GA_PointOffsetInfo                                                 |
----------------------------------------------------------------- */

DECLARE_Base_Namespace_Start()
namespace Containers
{
	struct HullCenterData
	{
		HullCenterData() { }
		HullCenterData(UT_Vector3 center, exint hullid = -1, exint bundleid = -1) : HullCenterData()
		{
			this->_center = center;
			this->_hullID = hullid;
			this->_bundleID = bundleid;
		}

		UT_Vector3		GetCenter() const { return this->_center; }
		exint			GetHullID() const { return this->_hullID; }
		exint			GetBundleID() const { return this->_bundleID; }

	private:
		UT_Vector3		_center;
		exint			_hullID;
		exint			_bundleID;
	};
}
DECLARE_Base_Namespace_End

#endif // !____hull_center_data_h____