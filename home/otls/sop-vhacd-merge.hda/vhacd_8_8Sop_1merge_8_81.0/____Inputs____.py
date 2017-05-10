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
# VARIABLES
# ---------------------------------------------------------------------
allowedTypes = ['nodeway::vhacdmerge', 'nodeway::vhacdgenerator']
connectedInputs = {'is0connected' : 0, 'is1connected' : 0, 'is2connected' : 0, 'is3connected' : 0}
connectedTypes = {'0type' : 'None', '1type' : 'None', '2type' : 'None', '3type' : 'None'}
firstNames = ['is0connected', 'is1connected', 0, 1, 'output1', 'output2']
secondNames = ['is2connected', 'is3connected', 2, 3, 'output1', 'output2']
    
# ---------------------------------------------------------------------
# HELPERS
# ---------------------------------------------------------------------
def WhenInput0IsConnected(node):
    connectedInputs['is0connected'] = 1
    connectedTypes['0type'] = node.inputs()[0].type().name()

def WhenInput1IsConnected(node):
    connectedInputs['is1connected'] = 1
    connectedTypes['1type'] = node.inputs()[1].type().name()    
    
def WhenInput2IsConnected(node):
    connectedInputs['is2connected'] = 1
    connectedTypes['2type'] = node.inputs()[2].type().name()    

def WhenInput3IsConnected(node):
    connectedInputs['is3connected'] = 1
    connectedTypes['3type'] = node.inputs()[3].type().name()    
        
def WhenInputIsConnected(node, input):
    ''' 
        Checks and sets connected toggle 
    '''
    if input < len(node.inputLabels()):    
        inputName = 'is{inputNumber}connected'.format(inputNumber = input)
        node.parm(inputName).set(1)        
    
        methodName = 'WhenInput{inputNumber}IsConnected(node)'.format(inputNumber = input)
        exec(methodName)
    
    else:
        message = 'Input number out of bounds. Max {0} (from 0 to {1}) ) inputs allowed.'.format(len(node.inputLabels()), len(node.inputLabels()) - 1)
        hou.ui.displayMessage(message, severity=hou.severityType.Error)
    
def WhenAllInputsAreDisconnected(node):
    ''' 
        Use it to reset all connected toggles and connected dictionary.
    '''
    for i in range(len(node.inputLabels())):
        key = 'is{parmNumber}connected'.format(parmNumber = i)
        # reset toggle and dictionary
        node.parm(key).set(0)        
        connectedInputs[key] = 0
        
    node.parm('errorlevel').set(0)
    
def CheckPairs(node, pair):
    ''' 
        Use to check if input pairs are connected properly.
    '''
    if node.parm(pair[0]).eval() == 1 and node.parm(pair[1]).eval() == 1:
        if connectedTypes['0type'] == connectedTypes['1type']:
            if node.inputs()[pair[2]].name() == node.inputs()[pair[3]].name():
                connections = node.inputConnections()
                
                if connections[pair[2]].inputName() != pair[4]:
                    node.parm('errorlevel').set(1)
                    return                
                if connections[pair[3]].inputName() != pair[5]:
                    node.parm('errorlevel').set(1)
                    return
            else:
                node.parm('errorlevel').set(1)
                return
        else:
            node.parm('errorlevel').set(1)
            return
                    
def SetErrorLevel(node):
    ''' 
        -1 - everything is good
        0 - some input is not connected
        1 - first or second pair is connected to wrong node type or not connected to the same node
    '''
    # make sure everything is connected
    for i in range(len(connectedInputs)):
        key = 'is{parmNumber}connected'.format(parmNumber = i)
        if connectedInputs[key] == 0:
            node.parm('errorlevel').set(0)
            return
        else:
            node.parm('errorlevel').set(-1)
        
    # check if pairs are connected to the same nodes and correct inputs        
    CheckPairs(node, firstNames)
    if node.parm('errorlevel').eval() == -1:        
        CheckPairs(node, secondNames)
   
        
# ---------------------------------------------------------------------
# MAIN
# ---------------------------------------------------------------------
def CheckInputs(node):
    ''' 
        Make sure that we are connected to something.
    '''
    
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
                     
        # set error level based on which input is connected
        SetErrorLevel(node)
                
    # none connected        
    else:
        WhenAllInputsAreDisconnected(node)        
                
    # set expressions
    #if node.parm('errorlevel').eval() == -1:
    #    SetupExpressions(node)
    #else:
    #    CleanParameters(node)