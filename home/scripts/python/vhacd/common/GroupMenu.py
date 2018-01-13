'''
	IMPORTANT! ------------------------------------------
	* Do not change anything!
	
    * requires lines below to be placed inside of Parameter Descriptions, Menu Script field: 
    from nodeway.hda.common import GroupMenu
    reload(GroupMenu)

    return GroupMenu.PrimitiveGroupMenu(kwargs)
    or
    return GroupMenu.PointGroupMenu(kwargs)
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

def PrimitiveGroupMenu(kwargs):
    # standard variables
    this = kwargs["node"]
    menuItems = []     
	
    try:
	inputs = this.inputs()
	if len(inputs) < 1:
	    return menuItems
			
        # get child input 0 node
        inputNode = inputs[0]
    
        # get groups
        geo = inputNode.geometry()
        primGroups = geo.primGroups()
    
        # create menu
        for group in primGroups:
            menuItems.append(group.name())
            menuItems.append(group.name())
    
    except AttributeError:
        pass
    
    #finish
    return menuItems

def PointGroupMenu(kwargs):
    # standard variables
    this = kwargs["node"]
    menuItems = [] 
    
    try:
	inputs = this.inputs()
	if len(inputs) < 1:
	    return menuItems
	
        # get child input 0 node
        inputNode = inputs[0]
    
        # get groups
        geo = inputNode.geometry()
        pointGroups = geo.pointGroups()
    
        # create menu
        for group in pointGroups:
            menuItems.append(group.name())
            menuItems.append(group.name())
    
    except AttributeError:
        pass
    
    #finish
    return menuItems