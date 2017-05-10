'''
    Author:     NODEWAY (2016)
    
    Email:      nodeway@hotmail.com
    Twitter:    twitter.com/nodeway
    
    ----------------------------------
    IMPORTANT:
    - requires line below to be placed inside of python module: 
    inputs = toolutils.createModuleFromSection('inputs', kwargs['type'], '__Inputs__.py')
    
    - requires below lines to be placed inside of OnCreated and OnInputChanged module: 
    this = kwargs['node']
    # make sure that we got required inputs connected
    this.hdaModule().inputs.CheckInputs(this)    
    
    TODO:
    - maybe make this as class, so we could override each input method outside of this file
'''
# ---------------------------------------------------------------------
# IMPORT
# ---------------------------------------------------------------------
import hou

# ---------------------------------------------------------------------
# HELPERS
# ---------------------------------------------------------------------
def WhenInput0IsConnected(node):
    pass

def WhenInput1IsConnected(node):
    pass
    
def WhenInput2IsConnected(node):
    pass

def WhenInput3IsConnected(node):
    pass
    
def WhenInput4IsConnected(node):
    pass
    
def WhenInput5IsConnected(node):
    pass
    
def WhenInput6IsConnected(node):
    pass
    
def WhenInput7IsConnected(node):
    pass
    
def WhenInput8IsConnected(node):
    pass
    
def WhenInput9IsConnected(node):
    pass
    
def WhenInputIsConnected(node, input):
    if input < 10:    
        inputName = 'is{inputNumber}connected'.format(inputNumber = input)
        node.parm(inputName).set(1)        
    
        methodName = 'WhenInput{inputNumber}IsConnected(node)'.format(inputNumber = input)
        exec(methodName)
    
    else:
        hou.ui.displayMessage('Input number out of bounds. Max 10 (from 0 to 9) ) inputs allowed.', severity=hou.severityType.Error)
    
def WhenAllInputsAreDisconnected(node):
    for i in range(10):
        node.parm('is{parmNumber}connected'.format(parmNumber = i)).set(0)
    
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
        
        # reset all before setting them
        WhenAllInputsAreDisconnected(node)
        
        # set what is connected
        for i in range(howManyConnected):
        
            if isConnected[i] != None:
                WhenInputIsConnected(node, i)
                
    # none connected        
    else:
        WhenAllInputsAreDisconnected(node)