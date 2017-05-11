'''
    Author:     NODEWAY (2016)
    
    Email:      nodeway@hotmail.com
    Twitter:    twitter.com/nodeway
    
    ----------------------------------
    IMPORTANT:
    - requires line below to be placed inside of python module: 
    resolution = toolutils.createModuleFromSection('resolution', kwargs['type'], 'Resolution.py')  
    
    - requires line below to be placed in callback of Description toggle 
    hou.phm().resolution.Callback(kwargs)
'''

# ---------------------------------------------------------------------
# HELPERS
# ---------------------------------------------------------------------
def SetDefaultState(node, parameter):
    ''' 
        What happens when whole section is disabled. 
    '''
    node.parm(parameter).revertToDefaults()
    node.parm('fallback{0}'.format(parameter)).revertToDefaults()
    
# ---------------------------------------------------------------------
# MAIN CALLBACK
# ---------------------------------------------------------------------
def Callback(kwargs, parameter):
    ''' 
        For callback script.
        It should be called only by main checkbox of parm group.
    '''  
    
    # standard variables
    this = kwargs['node']
    SetDefaultState(this, parameter)