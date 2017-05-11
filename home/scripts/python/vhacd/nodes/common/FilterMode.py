'''
    Author:     NODEWAY (2016)
    
    Email:      nodeway@hotmail.com
    Twitter:    twitter.com/nodeway
    
    ----------------------------------
    IMPORTANT:
    - requires line below to be placed inside of python module: 
    filterMode = toolutils.createModuleFromSection('filtermode', kwargs['type'], 'FilterMode.py')  
    
    - requires line below to be placed in callback of Description toggle 
    hou.phm().filterMode.Callback(kwargs)
'''

# ---------------------------------------------------------------------
# HELPERS
# ---------------------------------------------------------------------
def SetDefaultState(node):
    ''' 
        What happens when whole section is disabled. 
    '''
    
    node.parm('bundleidpattern').revertToDefaults()
    node.parm('primitivegroupinput0and1').revertToDefaults()
    
# ---------------------------------------------------------------------
# MAIN CALLBACK
# ---------------------------------------------------------------------
def Callback(kwargs):
    ''' 
        For callback script.
        It should be called only by main checkbox of parm group.
    '''  
    
    # standard variables
    this = kwargs['node']
    SetDefaultState(this)