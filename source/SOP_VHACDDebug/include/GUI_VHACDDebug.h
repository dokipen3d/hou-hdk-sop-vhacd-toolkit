/*
	Volumetric-Hierarchical Approximate Convex Decomposition.
	Based on https://github.com/kmammou/v-hacd

	IMPORTANT! ------------------------------------------
	* Macros starting and ending with '____' shouldn't be used anywhere outside of this file.
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
#ifndef ____gui_vhacd_debug_h____
#define ____gui_vhacd_debug_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// SESI
#include <GUI/GUI_PrimitiveHook.h>
#include <GT/GT_DANumeric.h>

// hou-hdk-common
#include <Macros/Namespace.h>
#include "Enums/MethodProcessResult.h"

// this
#include "VHACDCommonAttributeName.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define CONTAINERS			GET_Base_Namespace()::Containers
#define ENUMS				GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
DECLARATION                                                        |
----------------------------------------------------------------- */

DECLARE_GUI_Namespace_Start()

	class GUI_VHACDDebug : public GUI_PrimitiveHook
	{
	public:
		GUI_VHACDDebug();
		~GUI_VHACDDebug();

		/// The main virtual for filtering GT or GEO primitives. If the hook is
		/// interested in the primitive, it should set 'processed' to  GR_PROCESSED,
		/// even if it is not actively modifying the primitive (in which case
		/// return a NULL primitive handle). This ensures that the primitive will
		/// be updated when its display option is toggled. If not interested in the
		/// primitive at all, set 'processed' to GR_NOT_PROCESSED.
		GT_PrimitiveHandle							filterPrimitive(const GT_PrimitiveHandle& primhandle, const GEO_Primitive* primitive, const GR_RenderInfo* info, GR_PrimAcceptResult& processed) override;		
		
	private:
		template<typename BufferValueType>
		static ENUMS::MethodProcessResult			ProcessAttribute(const GT_DataArrayHandle& datahandle, GT_DataArrayHandle& newvertexcolors);

		CONTAINERS::VHACDCommonAttributeName		_vhacdCommonAttributeNames = CONTAINERS::VHACDCommonAttributeName();
	};
	
DECLARE_GUI_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS

#endif // !____gui_vhacd_debug_h____