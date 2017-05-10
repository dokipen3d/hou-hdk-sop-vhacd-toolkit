'''
    Author:     NODEWAY (2016)
    
    Email:      nodeway@hotmail.com
    Twitter:    twitter.com/nodeway
    
    ----------------------------------
    IMPORTANT:
    - requires line below to be placed inside of python module: 
    input = toolutils.createModuleFromSection('input', kwargs['type'], '__1_Input__.py')
    
    - requires below lines to be placed inside of OnCreated and OnInputChanged module: 
    this = kwargs['node']
    # make sure that we got required inputs connected
    this.hdaModule().input.CheckInput(this)    
'''

# ---------------------------------------------------------------------
# HELPERS
# ---------------------------------------------------------------------
def WhenInputIsConnected(node):
    node.parm('is0connected').set(1)

def WhenInputIsDisconnected(node):
    node.parm('is0connected').set(0)    
    
# ---------------------------------------------------------------------
# MAIN
# ---------------------------------------------------------------------
def CheckInput(node):    
        
    # get input node
    is0Connected = node.inputs()

    if is0Connected == ():
        WhenInputIsDisconnected(node)        
    else:
        WhenInputIsConnected(node)