'''
	IMPORTANT! ------------------------------------------
	* Do not change anything!
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
'''

class NameGenerator():
    @staticmethod  
    def __new_unique_name__(name, step, type):
        '''
        '''
        
        uniqueName = ''
        
        if step == 0: uniqueName = name
        else: uniqueName = '{0}{1}'.format(name, step)        
        
        wasSuccess = None
        try: 
            if type == 'pointgroup':
                wasSuccess = geo.findPointGroup(uniqueName) 
            elif type == 'edgegroup':
                wasSuccess = geo.findEdgeGroup(uniqueName) 
            elif type == 'primitivegroup':
                wasSuccess = geo.findPrimGroup(uniqueName) 
            elif type == 'vertexattribute':
                wasSuccess = geo.findVertexAttrib(uniqueName)                
            elif type == 'pointattribute':
                wasSuccess = geo.findPointAttrib(uniqueName)
            elif type == 'primitiveattribute':
                wasSuccess = geo.findPrimAttrib(uniqueName)
            elif type == 'detailattribute':
                wasSuccess = geo.findGlobalAttrib(uniqueName)
                   
        except: wasSuccess = None
        
        if wasSuccess != None:
            step += 1
            uniqueName = NameGenerator.__new_unique_name__(name, step, type)
        
        return uniqueName

    @staticmethod        
    def UniquePointGroupName(name, step = 0):
        return NameGenerator.__new_unique_name__(name, step, 'pointgroup')
    
    @staticmethod        
    def UniqueEdgeGroupName(name, step = 0):
        return NameGenerator.__new_unique_name__(name, step, 'edgegroup')
        
    @staticmethod        
    def UniquePrimitiveGroupName(name, step = 0):
        return NameGenerator.__new_unique_name__(name, step, 'primitivegroup')
        
    @staticmethod        
    def UniqueVertexAttibName(name, step = 0):
        return NameGenerator.__new_unique_name__(name, step, 'vertexattribute')
        
    @staticmethod        
    def UniquePointAttibName(name, step = 0):
        return NameGenerator.__new_unique_name__(name, step, 'pointattribute')

    @staticmethod        
    def UniquePrimitiveAttribName(name, step = 0):
        return NameGenerator.__new_unique_name__(name, step, 'primitiveattribute')
    
    @staticmethod        
    def UniqueDetailAttribName(name, step = 0):
        return NameGenerator.__new_unique_name__(name, step, 'detailattribute')    