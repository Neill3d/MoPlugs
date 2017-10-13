
from pyfbsdk import *

##################################################################################
# Script for selecting effecting character and control rig from model selection
# Author Sergey Solohin (Neill3d) 2014
#   e-mail to: s@neill3d.com
#   www.neill3d.com
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
##################################################################################

lSystem = FBSystem()
lScene = lSystem.Scene

lCharToSelect = None

for lChar in lScene.Characters:
    lSkinModelList = FBModelList()
    lChar.GetSkinModelList(lSkinModelList)
    
    lCharToSelect = None
    for lModel in lSkinModelList:
        
        if lModel.Selected == True:
            
            lCharToSelect = lChar
            break
            
    if lCharToSelect != None:
        lCharToSelect.Selected = True
        
        lControlSet = lCharToSelect.GetCurrentControlSet()
        if lControlSet != None:
            lControlSet.Selected = True
            
        # select ref
        lRefModel = lCharToSelect.GetCtrlRigModel(FBBodyNodeId.kFBReferenceNodeId)
        if lRefModel:
            lRefModel.Selected = True
            
        lCharToSelect.SelectModels(True, True, True, True)
        
        # select all character geometry as well
        for lModel in lSkinModelList:
            lModel.Selected = True
            
            # let's select a geometry root
            if lModel.Parent != None:
                lModel.Parent.Selected = True
            