
#########################################################
##
## Character Selection Tool script for Autodesk MotionBuilder
##
## Github repo - https://github.com/Neill3d/MoPlugs
## Licensed under BSD 3-clause
##  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
##
## Author Sergey Solohin (Neill3d) 2014
##  e-mail to: s@neill3d.com
##        www.neill3d.com
##
#########################################################

#
# TODO: select character dynamic mask (if projectors are used)
#

from pyfbsdk import *
from pyfbsdk_additions import *

gDevelopment = True

lApp = FBApplication()
lSystem = FBSystem()

def SetCurrentCharacterFromModel(control, event):

    lScene = lSystem.Scene
    lSkinModelList = FBModelList()
    
    for lChar in lScene.Characters:
        
        lChar.GetSkinModelList(lSkinModelList)
        for lModel in lSkinModelList:        
            if lModel.Selected == True:            
                lApp.CurrentCharacter = lChar
                break
    ##

def SelectCharacterElementsFromModel(control, event):

    lScene = lSystem.Scene
    lSkinModelList = FBModelList()
    lCharToSelect = None

    for lChar in lScene.Characters:
        
        lChar.GetSkinModelList(lSkinModelList)
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
    

def PopulateTool(t):
    
    # Create Main region frame:
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    main = FBVBoxLayout()
    t.AddRegion("main","main", x, y, w, h)
    t.SetControl("main",main)

    #
    b = FBButton()
    b.Caption = "Choose character from selection"
    b.OnClick.Add(SetCurrentCharacterFromModel)
    main.Add(b, 35)

    #
    b = FBButton()
    b.Caption = "Select character from selection"
    b.OnClick.Add(SelectCharacterElementsFromModel)
    main.Add(b, 35)

def CreateTool():
    t = None
    try:
        t = FBCreateUniqueTool("Character Selection Tool")
    except NameError:
        t = CreateUniqueTool("Character Selection Tool")
        print "supporting MoBu 2010"
        
    if t:
        t.StartSizeX = 200
        t.StartSizeY = 200
        PopulateTool(t)
        if gDevelopment:
            ShowTool(t)
        
CreateTool()