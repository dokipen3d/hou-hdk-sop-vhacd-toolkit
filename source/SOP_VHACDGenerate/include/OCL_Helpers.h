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
#ifndef ____ocl_helpers_h____
#define ____ocl_helpers_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// SESI
#include <CL/cl.h>
#include <SOP/SOP_Node.h>
#include <UT/UT_Interrupt.h>

// hou-hdk-common
#include <Macros/Namespace.h>
#include <Macros/ProgressEscape.h>

// this
#include "OCL_PlatformData.h"
#include "OCL_DeviceData.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define BUFFER_SIZE						1024

#define CONTAINERS						GET_Base_Namespace()::Containers
#define ENUMS							GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
TYPEDEFS                                                           |
----------------------------------------------------------------- */

typedef std::vector<cl_platform_id>		OCLPlatformIDs;
typedef std::vector<cl_device_id>		OCLDeviceIDs;

/* -----------------------------------------------------------------
DECLARATION                                                        |
----------------------------------------------------------------- */

DECLARE_Base_Namespace_Start()
namespace Utility
{
	class OCL_Helpers final
	{
	public:
		static ENUMS::MethodProcessResult 
		QuerryPlatforms(SOP_Node* node, UT_AutoInterrupt progress, UT_Array<CONTAINERS::OCL_PlatformData>& platformquerries, cl_device_type devicemask = CL_DEVICE_TYPE_ALL)
		{
			// check amount of available platforms
			cl_uint platformCount;			
			auto errorCode = clGetPlatformIDs(0, nullptr, &platformCount);
			if (errorCode != CL_SUCCESS)
			{
				node->addError(SOP_MESSAGE, "Failed to querry OpenCL platforms count.");
				return ENUMS::MethodProcessResult::FAILURE;
			}

			// get platform IDs
			OCLPlatformIDs platformIDs(platformCount);
			platformIDs.clear();

			errorCode = clGetPlatformIDs(platformCount, platformIDs.data(), nullptr);
			if (errorCode != CL_SUCCESS)
			{
				node->addError(SOP_MESSAGE, "Failed to querry OpenCL platform IDs.");
				return ENUMS::MethodProcessResult::FAILURE;
			}		

			// for each platform
			for (exint i = 0; i < platformCount; i++)
			{
				PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(node, progress, ENUMS::MethodProcessResult::INTERRUPT)
			
				// collect platform data
				auto currPlatformData = CONTAINERS::OCL_PlatformData(platformIDs[i], devicemask);
				currPlatformData.Clear();

				const auto processResult = QuerryPlatform(node, progress, currPlatformData);
				if (processResult != ENUMS::MethodProcessResult::SUCCESS && processResult != ENUMS::MethodProcessResult::INTERRUPT)
				{
					platformquerries.clear();
					return processResult;
				}
				
				platformquerries.append(currPlatformData);			
			}

			return ENUMS::MethodProcessResult::SUCCESS;
		}

		static ENUMS::MethodProcessResult
		QuerryPlatform(SOP_Node* node, UT_AutoInterrupt progress, CONTAINERS::OCL_PlatformData& platform)
		{
			char nameBuffer[BUFFER_SIZE];
			auto errorCode = clGetPlatformInfo(platform.GetID(), CL_PLATFORM_NAME, BUFFER_SIZE, &nameBuffer, nullptr);
			if (errorCode == CL_SUCCESS) platform.AddName(nameBuffer);

			char vendorBuffer[BUFFER_SIZE];
			errorCode = clGetPlatformInfo(platform.GetID(), CL_PLATFORM_VENDOR, BUFFER_SIZE, &vendorBuffer, nullptr);
			if (errorCode == CL_SUCCESS) platform.AddVendor(vendorBuffer);
			
			char versionBuffer[BUFFER_SIZE];
			errorCode = clGetPlatformInfo(platform.GetID(), CL_PLATFORM_VERSION, BUFFER_SIZE, &versionBuffer, nullptr);
			if (errorCode == CL_SUCCESS) platform.AddVersion(versionBuffer);
			
			char profileBuffer[BUFFER_SIZE];
			errorCode = clGetPlatformInfo(platform.GetID(), CL_PLATFORM_PROFILE, BUFFER_SIZE, &profileBuffer, nullptr);
			if (errorCode == CL_SUCCESS) platform.AddProfile(profileBuffer);
			
			char extensionsBuffer[BUFFER_SIZE];
			errorCode = clGetPlatformInfo(platform.GetID(), CL_PLATFORM_EXTENSIONS, BUFFER_SIZE, &extensionsBuffer, nullptr);
			if (errorCode == CL_SUCCESS) platform.AddExtensions(extensionsBuffer);
			
			// check how many devices platform have			
			return QuerryDevices(node, progress, platform);
		}

		static ENUMS::MethodProcessResult 
		QuerryDevices(SOP_Node* node, UT_AutoInterrupt progress,  CONTAINERS::OCL_PlatformData& platform)
		{
			cl_uint deviceCount;
			auto errorCode = clGetDeviceIDs(platform.GetID(), platform.DeviceMask(), 0, nullptr, &deviceCount);
			if (errorCode != CL_SUCCESS) return ENUMS::MethodProcessResult::INTERRUPT;
			
			OCLDeviceIDs deviceIDs(deviceCount);
			errorCode = clGetDeviceIDs(platform.GetID(), platform.DeviceMask(), deviceCount, deviceIDs.data(), nullptr);
			if (errorCode != CL_SUCCESS)
			{
				node->addError(SOP_MESSAGE, "Failed to querry OpenCL devices IDs.");
				return ENUMS::MethodProcessResult::FAILURE;
			}

			// for each device
			for (exint i = 0; i < deviceCount; i++)
			{
				PROGRESS_WAS_INTERRUPTED_WITH_ERROR_AND_OBJECT(node, progress, ENUMS::MethodProcessResult::INTERRUPT)
				
				// collect device data
				auto currDeviceData = CONTAINERS::OCL_DeviceData(deviceIDs[i]);

				const auto processResult = QuerryDevice(node, progress, currDeviceData);
				if (processResult != ENUMS::MethodProcessResult::SUCCESS)
				{
					node->addError(SOP_MESSAGE, "Failed to querry platform devices.");
					return processResult;
				}

				platform.AddDevice(currDeviceData);
			}

			return ENUMS::MethodProcessResult::SUCCESS;
		}

		static ENUMS::MethodProcessResult
		QuerryDevice(SOP_Node* node, UT_AutoInterrupt progress, CONTAINERS::OCL_DeviceData& device)
		{
			char nameBuffer[BUFFER_SIZE];
			auto errorCode = clGetDeviceInfo(device.GetID(), CL_DEVICE_NAME, BUFFER_SIZE, nameBuffer, nullptr);
			if (errorCode == CL_SUCCESS) device.AddName(nameBuffer);

			cl_device_type typeBuffer;
			errorCode = clGetDeviceInfo(device.GetID(), CL_DEVICE_TYPE, sizeof(typeBuffer), &typeBuffer, nullptr);
			if (errorCode == CL_SUCCESS) device.AddType(typeBuffer);			

			char deviceVendorBuffer[BUFFER_SIZE];
			errorCode = clGetDeviceInfo(device.GetID(), CL_DEVICE_VENDOR, BUFFER_SIZE, deviceVendorBuffer, nullptr);
			if (errorCode == CL_SUCCESS) device.AddVendor(deviceVendorBuffer);

			char deviceVersionBuffer[BUFFER_SIZE];
			errorCode = clGetDeviceInfo(device.GetID(), CL_DEVICE_VERSION, BUFFER_SIZE, deviceVersionBuffer, nullptr);
			if (errorCode == CL_SUCCESS) device.AddVersion(deviceVersionBuffer, ENUMS::OCL_VersionDataType::DEVICE);

			char driverVersionBuffer[BUFFER_SIZE];
			clGetDeviceInfo(device.GetID(), CL_DRIVER_VERSION, BUFFER_SIZE, driverVersionBuffer, nullptr);
			if (errorCode == CL_SUCCESS) device.AddVersion(driverVersionBuffer, ENUMS::OCL_VersionDataType::DRIVER);

			char openclVersionBuffer[BUFFER_SIZE];
			clGetDeviceInfo(device.GetID(), CL_DEVICE_OPENCL_C_VERSION, BUFFER_SIZE, openclVersionBuffer, nullptr);
			if (errorCode == CL_SUCCESS) device.AddVersion(openclVersionBuffer, ENUMS::OCL_VersionDataType::OPENCL);

			cl_uint maxComputeUnitsBuffer;
			clGetDeviceInfo(device.GetID(), CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(maxComputeUnitsBuffer), &maxComputeUnitsBuffer, nullptr);
			if (errorCode == CL_SUCCESS) device.AddMaxComputUnits(maxComputeUnitsBuffer);

			return ENUMS::MethodProcessResult::SUCCESS;
		}
	};
}
DECLARE_Base_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS
#undef CONTAINERS

#undef BUFFER_SIZE

#endif // !____ocl_helpers_h____