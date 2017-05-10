'''
    Author:     NODEWAY (2016)
    
    Email:      nodeway@hotmail.com
    Twitter:    twitter.com/nodeway
    
    ----------------------------------
    IMPORTANT:
    - requires line below to be placed inside of python module: 
    description = toolutils.createModuleFromSection('description', kwargs['type'], 'Description.py')  
    
    - requires line below to be placed in callback of Description toggle 
    hou.phm().description.Callback(kwargs)
    
    - requires line below to be placed in callback of Clean button: 
    hou.phm().description.SetDefaultState(hou.pwd())
'''

# ---------------------------------------------------------------------
# HELPERS
# ---------------------------------------------------------------------
def SetDefaultState(node):
    ''' 
        What happens when whole section is disabled. 
    '''
    
    node.parm('descriptionmessage').revertToDefaults()
    
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
    parmState = this.parm('adddescription').eval()
    
    if not parmState: SetDefaultState(this)