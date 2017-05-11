'''
    Author:     NODEWAY (2016)
    
    Email:      nodeway@hotmail.com
    Twitter:    twitter.com/nodeway
    
    ----------------------------------
    IMPORTANT:
    - requires line below to be placed inside of python module: 
    selectors = toolutils.createModuleFromSection('selectors', kwargs['type'], '__Selectors__.py') 
    
    - requires line below to be put inside callback field of GroupType:
    hou.phm().selectors.Callback(kwargs)
    
    - requires line below to be placed inside Tools:
    type = hou.sopNodeTypeCategory().nodeTypes()['$HDA_NAME']
    
    - requires lines below to be placed inside correct tool version in Tools:
    type.hdaModule().selectors.SetupEdgeSelector(type)
    type.hdaModule().selectors.SetupPointSelector(type)
    type.hdaModule().selectors.SetupPrimitiveSelector(type)
    
    node = soptoolutils.genericTool(kwargs, '$HDA_NAME')
    type.hdaModule().selectors.SetupEvents(kwargs, node) 
    
    - requires line below to be placed inside ActionButton:
    this = kwargs['node']
    this.hdaModule().selectors.Reselect(kwargs)
    
    - requires line below to be placed inside ActionButton Help field:
    Select geometry from an available viewport.
    
    - requires line below to be placed inside ActionButton Icon field:
    BUTTONS_reselect
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