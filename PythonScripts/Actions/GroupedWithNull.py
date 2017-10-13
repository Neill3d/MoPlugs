
####
#  Script for grouping selected objects with null
#
# DONE: check one hierarchy selected items (don't reparent them)
#
#
# Author Sergey Solohin (Neill3d) 2013
#   e-mail to: s@neill3d.com
#       www.neill3d.com 
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
###


from pyfbsdk import *

modelList = FBModelList()
FBGetSelectedModels(modelList)

if len(modelList) > 0:
    
    lUndoManager = FBUndoManager()
    
    lUndoManager.TransactionBegin("grouping")
    
    null = FBModelNull("group")
    null.Show = True
    null.Selected = True
    
    lUndoManager.TransactionAddObjectDestroy(null)
    
    for model in modelList:
        model.Selected = False
        
        lProp = model.PropertyList.Find("Selected")
        if lProp: lUndoManager.TransactionAddProperty(lProp)
        lProp = model.PropertyList.Find("Parent")
        if lProp: lUndoManager.TransactionAddProperty(lProp)
        
        if model.Parent and model.Parent.Selected == False:
            null.Parent = model.Parent
            model.Parent = null
        
        if not model.Parent:
            model.Parent = null
    #
    
    lUndoManager.TransactionEnd()
#