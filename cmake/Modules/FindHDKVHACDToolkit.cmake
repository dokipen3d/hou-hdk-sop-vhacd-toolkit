#[[
	IMPORTANT! ------------------------------------------
	* Variables starting and ending with `____` shouldn't be used anywhere outside of this file.
	-----------------------------------------------------

	Author: 	SWANN
	Email:		sebastianswann@outlook.com

	LICENSE ------------------------------------------

	Copyright (c) 2017 SWANN
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	3. The names of the contributors may not be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
]]

#[[-----------------------------------------------------------------
INCLUDES                                                           |
------------------------------------------------------------------]]

Include(FindPackageHandleStandardArgs)
Include(HDKUtils)

#[[-----------------------------------------------------------------
DEFAULTS                                                           |
------------------------------------------------------------------]]

# specify module name
Set(____module_name____ 									"hou-hdk-sop-vhacd-toolkit")

# specify source directory
Set(____module_source_dir____ 								"${CMAKE_CURRENT_SOURCE_DIR}/../source")
Set(____module_3rdparty_dir____ 							"${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty")

#[[-----------------------------------------------------------------
HELPERS                                                            |
------------------------------------------------------------------]]

Macro(HDK_MODULE_GET_3RDPARTY_INCLUDE_FILES _dir)
	File(GLOB_RECURSE ____module_3rdparty_include_files____
		"${_dir}/*.h"
	)
EndMacro()

Macro(HDK_MODULE_GET_3RDPARTY_SOURCE_FILES _dir)
	File(GLOB_RECURSE ____module_3rdparty_source_files____
		"${_dir}/*.cpp" 
		"${_dir}/*.c"
	)
EndMacro()

Macro(HDK_MODULE_GET_3RDPARTY_OPENCL_KERNEL_FILES _dir)
	File(GLOB_RECURSE ____module_3rdparty_opencl_kernel_files____
		"${_dir}/*.cl"
	)
EndMacro()

#[[-----------------------------------------------------------------
MAIN                                                               |
------------------------------------------------------------------]]
	
HDK_MODULE_NAME_TOUPPER()
if(EXISTS ${____module_source_dir____} AND EXISTS ${____module_3rdparty_dir____})

	# find all common header files
	HDK_MODULE_GET_COMMON_INCLUDE_FILES(${____module_source_dir____})
	
	# find 3rdParty header and source files
	HDK_MODULE_GET_3RDPARTY_INCLUDE_FILES(${____module_3rdparty_dir____})
	HDK_MODULE_GET_3RDPARTY_SOURCE_FILES(${____module_3rdparty_dir____})

	# find OpenCL kernel
	HDK_MODULE_GET_3RDPARTY_OPENCL_KERNEL_FILES(${____module_3rdparty_dir____})

	# output data
	Set(HDK_VHACDTOOLKIT_COMMON_INCLUDE_DIR 				"${____module_source_dir____}/include")
	Set(HDK_VHACDTOOLKIT_COMMON_INCLUDE_FILES 				"${____module_common_include_files____}")
	
	Set(HDK_VHACDTOOLKIT_3RDPARTY_INCLUDE_DIR 				"${____module_3rdparty_dir____}/VHACD_Lib/inc" "${____module_3rdparty_dir____}/VHACD_Lib/public")
	Set(HDK_VHACDTOOLKIT_3RDPARTY_INCLUDE_FILES 			"${____module_3rdparty_include_files____}")
	Set(HDK_VHACDTOOLKIT_3RDPARTY_SOURCE_FILES 				"${____module_3rdparty_source_files____}")
	Set(HDK_VHACDTOOLKIT_3RDPARTY_OPENCL_KERNEL_FILES 		"${____module_3rdparty_opencl_kernel_files____}")

#[[-----------------------------------------------------------------
COLLECT NODE PROJECT DATA                                          |
------------------------------------------------------------------]]
		
	HDK_MODULE_GET_NODE_FILES("SOP_VHACDGenerate")
	HDK_MODULE_GET_NODE_FILES("SOP_VHACDMerge")
	HDK_MODULE_GET_NODE_FILES("SOP_VHACDScoutJunior")
	HDK_MODULE_GET_NODE_FILES("SOP_VHACDScoutSenior")
	HDK_MODULE_GET_NODE_FILES("SOP_VHACDSetup")
	
else()
	Message(STATUS "Didn't found ${____module_name_toupper____}")
endif()