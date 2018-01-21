/*
	Container for storing OpenCL platform data.

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
#ifndef ____ocl_platform_data_h____
#define ____ocl_platform_data_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// SESI
#include <CL/cl.h>
#include <UT/UT_Array.h>
#include <UT/UT_String.h>

// hou-hdk-common
#include <Macros/Namespace.h>

// this
#include "OCL_DeviceData.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define CONTAINERS							GET_Base_Namespace()::Containers

/* -----------------------------------------------------------------
IMPLEMENTATION													   |
----------------------------------------------------------------- */

DECLARE_Base_Namespace_Start()
namespace Containers
{
	struct OCL_PlatformData
	{
		virtual ~OCL_PlatformData() {}

		OCL_PlatformData(cl_platform_id platformid, cl_device_type devicemask = CL_DEVICE_TYPE_ALL)
		{
			this->_platformID = platformid;
			this->_deviceMask = devicemask;
		}

		void								AddName(const char buffer[]) { this->_platformName = buffer; }
		void								AddVendor(const char buffer[]) { this->_platformVendor = buffer; }
		void								AddVersion(const char buffer[]) { this->_platformVersion = buffer; }
		void								AddProfile(const char buffer[]) { this->_platformProfile = buffer; }
		void								AddExtensions(const char buffer[]) { this->_platformExtensions = buffer; }
		void								AddDevice(CONTAINERS::OCL_DeviceData devicedata) { this->_devices.append(devicedata); }

		cl_platform_id						GetID() const { return this->_platformID; }		
		UT_String							GetName() const { return this->_platformName.c_str(); }
		UT_String							GetVendor() const { return this->_platformVendor.c_str(); }
		UT_String							GetVersion() const { return this->_platformVersion.c_str(); }
		UT_String							GetProfile() const { return this->_platformProfile.c_str(); }
		UT_String							GetExtensions() const { return this->_platformExtensions.c_str(); }		
		const UT_Array<OCL_DeviceData>&		GetDevices() const { return this->_devices; }
		
		exint								DeviceCount() const { return this->_devices.size(); }
		cl_device_type						DeviceMask() const { return this->_deviceMask; }

		virtual void						Clear()
		{
			this->_devices.clear();
		}

		virtual void						Report() const
		{
			if(DeviceCount() == 0) return;

			std::cout << "------------------------------------ Platform Name: " << GetName() << std::endl;
			std::cout << "ID: " << GetID() << std::endl;			
			std::cout << "Vendor: " << GetVendor() << std::endl;
			std::cout << "Version: " << GetVersion() << std::endl;
			std::cout << "Profile: " << GetProfile() << std::endl;
			std::cout << "Extensions: " << GetExtensions() << std::endl;			
			std::cout << "Device Count:" << this->_devices.size() << std::endl;
		}

		/* -------------------------------- RANGE-LOOPS SUPPORT */

		using OCL_DeviceIterator = UT_Array<OCL_DeviceData>::iterator;
		using OCL_DeviceConstIterator = UT_Array<OCL_DeviceData>::const_iterator;

		OCL_DeviceIterator					begin() { return this->_devices.begin(); }
		OCL_DeviceIterator					end() { return this->_devices.end(); }
		OCL_DeviceConstIterator				begin() const { return this->_devices.begin(); }
		OCL_DeviceConstIterator				end() const { return this->_devices.end(); }

	private:
		cl_platform_id						_platformID;
		std::string							_platformName;
		std::string							_platformVendor;
		std::string							_platformVersion;
		std::string							_platformProfile;
		std::string							_platformExtensions;

		UT_Array<OCL_DeviceData>			_devices;
		cl_device_type						_deviceMask;
	};

}
DECLARE_Base_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef CONTAINERS

#endif // !____ocl_platform_data_h____