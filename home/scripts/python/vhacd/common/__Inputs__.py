'''
	IMPORTANT! ------------------------------------------
	* Do not change anything!
	
    * requires line below to be placed inside of python module: 
    inputs = toolutils.createModuleFromSection('inputs', kwargs['type'], '__Inputs__.py')
    
    * requires below lines to be placed inside of OnCreated and OnInputChanged module: 
    this = kwargs['node']
    # make sure that we got required inputs connected
    this.hdaModule().inputs.CheckInputs(this)    
    
    TODO:
    - maybe make this as class, so we could override each input method outside of this file
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