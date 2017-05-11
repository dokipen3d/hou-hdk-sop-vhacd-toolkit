'''
    Author:     NODEWAY (2016)
    
    Email:      nodeway@hotmail.com
    Twitter:    twitter.com/nodeway
    
    ----------------------------------
    IMPORTANT:
    - requires line below to be placed inside of python module: 
    inputs = toolutils.createModuleFromSection('inputs', kwargs['type'], '__2_Inputs__.py')
    
    - requires below lines to be placed inside of OnCreated and OnInputChanged module: 
    this = kwargs['node']
    # make sure that we got required inputs connected
    this.hdaModule().inputs.CheckInputs(this)    
'''

# ---------------------------------------------------------------------
# HELPERS
# ---------------------------------------------------------------------
def WhenOnlyFirstInputIsConnected(node):
    node.parm('is0connected').set(1)
    node.parm('is1connected').set(0)
    
def WhenOnlySecondInputIsConnected(node):
    node.parm('is0connected').set(0)
    node.parm('is1connected').set(1) 
    
def WhenBothInputsAreConnected(node):
    node.parm('is0connected').set(1)
    node.parm('is1connected').set(1)
    
def WhenBothInputsAreDisconnected(node):
    node.parm('is0connected').set(0)
    node.parm('is1connected').set(0)              

# ---------------------------------------------------------------------
# MAIN
# ---------------------------------------------------------------------
def CheckInputs(node):
    ''' Make sure that we are connected to something '''
    
    # check connection state   
    isConnected = node.inputs()

    # are we connected at all?
    if isConnected != ():
        
        # which inputs are connected?
        howManyConnected = len(isConnected)
     
        # if only first connected
        if howManyConnected == 1:
            WhenOnlyFirstInputIsConnected(node)
            
        # if both connected    
        elif howManyConnected == 2: 
        
            # is first really connected?
            if isConnected[0] != None:
                WhenBothInputsAreConnected(node)                
            else:
                WhenOnlySecondInputIsConnected(node)
                                                                                 
    # none connected        
    else:
        WhenBothInputsAreDisconnected(node)