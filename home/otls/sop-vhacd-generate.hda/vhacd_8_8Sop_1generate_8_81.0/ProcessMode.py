'''
    Author:     NODEWAY (2016)
    
    Email:      nodeway@hotmail.com
    Twitter:    twitter.com/nodeway
    
    ----------------------------------
    IMPORTANT:
    - requires line below to be placed inside of python module: 
    processMode = toolutils.createModuleFromSection('processmode', kwargs['type'], 'ProcessMode.py')  
    
    - requires line below to be placed in callback of Description toggle 
    hou.phm().processMode.Callback(kwargs)
'''

# ---------------------------------------------------------------------
# HELPERS
# ---------------------------------------------------------------------
def WhenAsOnePiece(node):
    ''' 
        What happens when processing all parts as one object. 
    '''    
    #node.parm('creategrouppereachhullbundle').revertToDefaults()    
    #node.parm('showbundleid').revertToDefaults()
    
def WhenPerEachPiece(node):
    ''' 
        What happens when processing each part separately. 
    '''    
    #node.parm('creategrouppereachhullbundle').revertToDefaults() 
    #node.parm('showbundleid').revertToDefaults()        
    
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
    parmState = this.parm('processmode').eval()
    
    if not parmState: WhenAsOnePiece(this)
    else: WhenPerEachPiece(this)