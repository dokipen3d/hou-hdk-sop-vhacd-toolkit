'''
	IMPORTANT! ------------------------------------------
	* Do not change anything!
	
    * requires line below to be placed inside of python module: 
    selectors = toolutils.createModuleFromSection('selectors', kwargs['type'], '__Selectors__.py') 
    
    * requires line below to be put inside callback field of GroupType:
    hou.phm().selectors.Callback(kwargs)
    
    * requires line below to be placed inside Tools:
    type = hou.sopNodeTypeCategory().nodeTypes()['$HDA_NAME']
    
    * requires lines below to be placed inside correct tool version in Tools:
    type.hdaModule().selectors.SetupEdgeSelector(type)
    type.hdaModule().selectors.SetupPointSelector(type)
    type.hdaModule().selectors.SetupPrimitiveSelector(type)
    
    node = soptoolutils.genericTool(kwargs, '$HDA_NAME')
    type.hdaModule().selectors.SetupEvents(kwargs, node) 
    
    * requires line below to be placed inside ActionButton:
    this = kwargs['node']
    this.hdaModule().selectors.Reselect(kwargs)
    
    * requires line below to be placed inside ActionButton Help field:
    Select geometry from an available viewport.
    
    * requires line below to be placed inside ActionButton Icon field:
    BUTTONS_reselect
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
# ---------------------------------------------------------------------
# IMPORT
# ---------------------------------------------------------------------

import hou
import soputils

# ---------------------------------------------------------------------
# HELPERS
# ---------------------------------------------------------------------

def __destroy_selectors(type):
    '''
        Shouldn't be used directly.
    '''
    allSelectors = type.selectors()
    for s in allSelectors:
        s.destroy()
    
def SetupPointSelector(type):
    '''
        Should be placed inside Tools:    
        type.hdaModule().selectors.SetupPointSelector(type)        
    '''
    __destroy_selectors(type)
    type.addSelector('pointSelector', 
                     'points', 
                     'Specify points to create polygon. Press <Enter> to complete', 
                     (hou.primType.Polygon, ), 
                     'pointgroupinput0', 
                     '', 
                     0, 
                     True)
                 
def SetupEdgeSelector(type):
    '''
        Should be placed inside Tools:
        type.hdaModule().selectors.SetupEdgeSelector(type)        
    '''
    __destroy_selectors(type)
    type.addSelector('edgeSelector', 
                     'edges', 
                     'Specify edges to create polygon. Press <Enter> to complete', 
                     (hou.primType.Polygon, ), 
                     'edgegroupinput0', 
                     '', 
                     0, 
                     True)   
                     
def SetupPrimitiveSelector(type):
    '''
        Should be placed inside Tools:
        type.hdaModule().selectors.SetupPrimitiveSelector(type)        
    '''
    __destroy_selectors(type)
    type.addSelector('primSelector', 
                     'prims', 
                     'Specify primitives. Press <Enter> to complete', 
                     (hou.primType.Polygon, ), 
                     'primitivegroupinput0', 
                     '', 
                     0, 
                     True)

# ---------------------------------------------------------------------
# EVENTS
# ---------------------------------------------------------------------  
 
def SetupEvents(kwargs, node = None):
    ''' 
        Should be placed inside Tools:
        type.hdaModule().selectors.SetupEvents(kwargs, node) 
    '''
    if node == None:
        node = kwargs['node']
        
    # what happens when ALT is pressed   
    if ('altclick' in kwargs and kwargs['altclick']) or ('alt' in kwargs and kwargs['alt']): 
        pass
    else: 
        pass 

    # what happens when CTRL is pressed
    if ('ctrlclick' in kwargs and kwargs['ctrlclick']) or ('ctrl' in kwargs and kwargs['ctrl']):
        pass
    else:
        pass

# ---------------------------------------------------------------------
# MAIN
# ---------------------------------------------------------------------  
                   
def Callback(kwargs):
    '''
        Should be placed inside callback field of GroupType:
        hou.phm().selectors.Callback(kwargs)    
    '''
    node = kwargs['node']
    grouptype = node.parm('grouptype').eval()

    if grouptype == 0:
        SetupPointSelector(node.type())
    elif grouptype == 1:
        SetupEdgeSelector(node.type())
    else:
        SetupPrimitiveSelector(node.type())  
        
def Reselect(kwargs, grouptype = None):                    
    '''
        Should be placed inside ActionButton field:
        this = kwargs['node']
        this.hdaModule().selectors.Reselect(kwargs)        
    '''
    if grouptype == None:   
        grouptype = kwargs['node'].parm('grouptype').eval()
    
    if grouptype == 0:
        kwargs['geometrytype'] = hou.geometryType.Points
    elif grouptype == 1:
        kwargs['geometrytype'] = hou.geometryType.Edges
    else:
        kwargs['geometrytype'] = hou.geometryType.Primitives
        
    kwargs['inputindex'] = 0
    soputils.selectGroupParm(kwargs)
    
    # setup events when modifiers are pressed
    SetupEvents(kwargs)     