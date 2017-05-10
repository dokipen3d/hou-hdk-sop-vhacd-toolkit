'''
    Author:     NODEWAY (2016)
    
    Email:      nodeway@hotmail.com
    Twitter:    twitter.com/nodeway
    
    ----------------------------------
    IMPORTANT:
    - requires lines below to be placed inside of Parameter Descriptions, Menu Script field: 
    from nodeway.hda.common import GroupMenu
    reload(GroupMenu)

    return GroupMenu.PrimitiveGroupMenu(kwargs)
    or
    return GroupMenu.PointGroupMenu(kwargs)
'''

def PrimitiveGroupMenu(kwargs):
    # standard variables
    this = kwargs["node"]
    menuItems = [] 
    
    try:
        # get child input 0 node
        inputNode = this.inputs()[0]
    
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
        # get child input 0 node
        inputNode = this.inputs()[0]
    
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