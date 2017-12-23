/*
	Some helper methods for working with primitive groups.

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
#ifndef ____primitive_group_accessing_h____
#define ____primitive_group_accessing_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// SESI
#include <SOP/SOP_Node.h>
#include <GA/GA_EdgeGroup.h>
#include <GOP/GOP_Manager.h>
#include <GU/GU_EdgeUtils.h>

#if _WIN32		
	#include <sys/SYS_Types.h>
#else
	#include <SYS/SYS_Types.h>
#endif

// hou-hdk-common
#include "../Macros/Namespace.h"
#include "../Macros/ProgressEscape.h"
#include "../Enums/NodeErrorLevel.h"
#include "../Utility/AttributeAccessing.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define ATTRIB_ACCESS			GET_Base_Namespace()::Utility::Attribute
#define CONTAINERS				GET_Base_Namespace()::Containers
#define ENUMS					GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
DECLARE & IMPLEMENT                                                |
----------------------------------------------------------------- */

DECLARE_Base_Namespace_Start()
namespace Utility
{
	namespace Groups
	{
		namespace Primitive
		{
			class Break
			{
			public:
				static bool
				PerConnectivity(SOP_Node* node, GU_Detail* detail, UT_AutoInterrupt progress)
				{
					// make sure we have connectivity attribute
					GA_RWHandleI connectivityHandle;
					auto success = ATTRIB_ACCESS::Find::IntATT(node, detail, GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, "connectivity", connectivityHandle);
					if (!success) connectivityHandle = GA_RWHandleI(detail->addIntTuple(GA_AttributeOwner::GA_ATTRIB_PRIMITIVE, GA_AttributeScope::GA_SCOPE_PUBLIC, "connectivity", 1, GA_Defaults(-1)));
					
					if (connectivityHandle.isValid())
					{						
						GA_OffsetArray visitedPointOffsets;
						exint connectivityID = 0;

						visitedPointOffsets.clear();

						for (auto pointIt = GA_Iterator(detail->getPointRange()); !pointIt.atEnd(); pointIt.advance())
						{
							PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_BOOL(node, progress, false)

							if (visitedPointOffsets.find(*pointIt) > exint(-1)) continue;
							visitedPointOffsets.append(*pointIt);

							GA_OffsetArray currPrimOffsets;
							currPrimOffsets.clear();
							detail->getPrimitivesReferencingPoint(currPrimOffsets, *pointIt);
							ProcessPrimitiveOffsets(node, detail, progress, connectivityHandle, connectivityID, currPrimOffsets, visitedPointOffsets);
						}
					}
					else
					{
						node->addError(SOP_MESSAGE, "Failed to create connectivity attribute.");
						return false;
					}

					return true;
				}

			private:
				static void
				ProcessPrimitiveOffsets(SOP_Node* node, GU_Detail* detail, UT_AutoInterrupt progress, GA_RWHandleI& connectivityhandle, exint& connectivityid, GA_OffsetArray& primitiveoffsets, GA_OffsetArray& visitedpointoffsets)
				{
					// check if any offset have connectivity already set
					auto isSet = false;
					exint foundConnectivityValue = -1;
					for (auto offset : primitiveoffsets)
					{
						exint currConnectivity = connectivityhandle.get(offset);
						if (currConnectivity > exint(-1))
						{
							foundConnectivityValue = currConnectivity;
							isSet = true;
							break;
						}
					}

					// set connectivity value
					for (auto offset : primitiveoffsets)
					{
						if (isSet) connectivityhandle.set(offset, foundConnectivityValue);
						else connectivityhandle.set(offset, connectivityid);
					}

					// bump id
					if (!isSet) connectivityid++;

					// process next primitives
					UT_Set<GA_Offset> pointOffsets;
					for (auto offset : primitiveoffsets)
					{
						// get primitive
						auto currPrim = detail->getPrimitive(offset);
						for (auto pointIt = GA_Iterator(currPrim->getPointRange()); !pointIt.atEnd(); pointIt.advance()) pointOffsets.insert(*pointIt);
					}

					// process primitive points
					for (auto offset : pointOffsets)
					{	
						// skip already processed points
						if (visitedpointoffsets.find(offset) > exint(-1)) continue;
						visitedpointoffsets.append(offset);

						GA_OffsetArray currPrimOffsets;
						auto size = detail->getPrimitivesReferencingPoint(currPrimOffsets, offset);

						if (size > 0) ProcessPrimitiveOffsets(node, detail, progress, connectivityhandle, connectivityid, currPrimOffsets, visitedpointoffsets);
					}
				}
			};
		}
	}
}
DECLARE_Base_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS
#undef ATTRIB_ACCESS

#endif // !____primitive_group_accessing_h____