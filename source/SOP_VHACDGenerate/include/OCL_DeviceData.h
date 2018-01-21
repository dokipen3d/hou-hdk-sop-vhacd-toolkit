/*
	Container for storing OpenCL device data.

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
#ifndef ____ocl_device_data_h____
#define ____ocl_device_data_h____

/* -----------------------------------------------------------------
INCLUDES                                                           |
----------------------------------------------------------------- */

// SESI
#include <CL/cl.h>
#include <UT/UT_String.h>

// hou-hdk-common
#include <Macros/Namespace.h>

// this
#include "OCL_VersionDataType.h"

/* -----------------------------------------------------------------
DEFINES                                                            |
----------------------------------------------------------------- */

#define ENUMS										GET_Base_Namespace()::Enums

/* -----------------------------------------------------------------
IMPLEMENTATION													   |
----------------------------------------------------------------- */

DECLARE_Base_Namespace_Start()
namespace Containers
{
	class OCL_DeviceData
	{
	public:
		virtual ~OCL_DeviceData() {}

		OCL_DeviceData(cl_device_id deviceid) : 
		_deviceType(0),		                                        
		_maxComputeUnits(0)
		{
			this->_deviceID = deviceid;
		}

		void										AddName(const char buffer[]) { this->_deviceName = buffer; }
		void										AddType(cl_device_type devicetype) { this->_deviceType = devicetype; }
		void										AddVendor(const char buffer[]) { this->_deviceVendor = buffer; }
		void										AddVersion(const char buffer[], ENUMS::OCL_VersionDataType versiontype)
		{
			switch(versiontype)
			{
				default: { this->_deviceVersion = buffer; } break;
				case ENUMS::OCL_VersionDataType::DRIVER: { this->_driverVersion = buffer; } break;
				case ENUMS::OCL_VersionDataType::OPENCL: { this->_openclVersion = buffer; } break;
			}
		}
		void										AddMaxComputUnits(cl_uint maxcomputeunits) { this->_maxComputeUnits = maxcomputeunits; }
		
		cl_device_id								GetID() const { return this->_deviceID; }
		UT_String									GetName() const { return this->_deviceName.c_str(); }
		cl_device_type								GetType() const { return this->_deviceType; }
		UT_String									GetVendor() const { return this->_deviceVendor.c_str(); }
		UT_String									GetVersion(ENUMS::OCL_VersionDataType versiontype) const
		{
			switch (versiontype)
			{
				default: { return this->_deviceVersion.c_str(); } break;
				case ENUMS::OCL_VersionDataType::DRIVER: { return this->_driverVersion.c_str(); } break;
				case ENUMS::OCL_VersionDataType::OPENCL: { return this->_openclVersion.c_str(); } break;
			}				
		}
		cl_uint										GetMaxComputeUnits() const { return this->_maxComputeUnits; }

		virtual void								Report() const
		{
			std::cout << "--------- Device Name: " << GetName() << std::endl;
			std::cout << "ID: " << GetID() << std::endl;			
			std::cout << "Type: ";			
			switch(GetType())
			{
				default: { std::cout << "Default" << std::endl; } break;
				case CL_DEVICE_TYPE_CPU: { std::cout << "CPU" << std::endl; } break;
				case CL_DEVICE_TYPE_GPU: { std::cout << "GPU" << std::endl; } break;
				case CL_DEVICE_TYPE_ACCELERATOR: { std::cout << "Accelerator" << std::endl; } break;
				case CL_DEVICE_TYPE_CUSTOM: { std::cout << "Custom" << std::endl; } break;
				case CL_DEVICE_TYPE_ALL: { std::cout << "All" << std::endl; } break;
			}
			std::cout << "Vendor: " << GetVendor() << std::endl;
			std::cout << "Version: " << GetVersion(ENUMS::OCL_VersionDataType::DEVICE) << std::endl;
			std::cout << "Driver: " << GetVersion(ENUMS::OCL_VersionDataType::DRIVER) << std::endl;
			std::cout << "OpenCL: " << GetVersion(ENUMS::OCL_VersionDataType::OPENCL) << std::endl;
			std::cout << "Compute Units: " << GetMaxComputeUnits() << std::endl;
		}

	private:
		cl_device_id								_deviceID;
		std::string									_deviceName;
		cl_device_type								_deviceType;
		std::string									_deviceVendor;
		std::string									_deviceVersion;
		std::string									_driverVersion;
		std::string									_openclVersion;
		cl_uint										_maxComputeUnits;
	};
}
DECLARE_Base_Namespace_End

/* -----------------------------------------------------------------
UNDEFINES                                                          |
----------------------------------------------------------------- */

#undef ENUMS

#endif // !____ocl_device_data_h____