
# script for instacing selected models through roots

# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *
from pyfbsdk_additions import *
import random

DUPLICATE_TOOL_VERSION = 0.01

gModelLabel= FBLabel()
gModelList = FBModelList()

'''
    
 OVERVIEW:     
     grab selected roots
     duplicate for each root and restore local matrix

'''

def ButtonGrabEvent(control, event):
    print "grab"
    FBGetSelectedModels(gModelList)
    
    #
    text = "Models to duplicate: "
    
    for i in range(len(gModelList)):
        text = text + " " + gModelList[i].Name
        if i < len(gModelList)-1:
            text = text + "; "

    gModelLabel.Caption = text
    
def DoModelClone(root, model, pos, rot, scl, newGroup):
    
    #  
    fps = 30.0
    playOffset = 0.0
    speed = 1.0
               
    if model.PropertyList.Find("SpriteSheet FPS"):
        fps = model.PropertyList.Find("SpriteSheet FPS").Data
        playOffset = model.PropertyList.Find("SpriteSheet PlayOffset").Data
        speed = model.PropertyList.Find("SpriteSheet Speed").Data
    
    #
    newModel = model.Clone()
    newModel.Name = model.Name + "_under_" + root.Name
    newModel.Parent = root
    
    newModel.SetVector(pos, FBModelTransformationType.kModelTranslation, False)
    newModel.SetVector(rot, FBModelTransformationType.kModelRotation, False)
    newModel.SetVector(scl, FBModelTransformationType.kModelScaling, False) 
    
    # assign common sprite properties
    
    if newModel.PropertyList.Find("SpriteSheet FPS"):
        newModel.PropertyList.Find("SpriteSheet FPS").Data = fps
        newModel.PropertyList.Find("SpriteSheet PlayOffset").Data = playOffset
        newModel.PropertyList.Find("SpriteSheet Speed").Data = speed
        newModel.PropertyList.Find("SpriteSheet LocalPlay").Data = True
        newModel.PropertyList.Find("SpriteSheet TriggerOnScaling").Data = True
    
    newGroup.Items.append(newModel)
    
    return newModel
    
def DoDuplicateUnderRoot(modelList, rootList, isRandomMode):
    
    posList = []
    rotList = []
    sclList = []
    
    
    for model in modelList:
        
        pos = FBVector3d()
        rot = FBVector3d()
        scl = FBVector3d()
        
        model.GetVector(pos, FBModelTransformationType.kModelTranslation, False)
        model.GetVector(rot, FBModelTransformationType.kModelRotation, False)
        model.GetVector(scl, FBModelTransformationType.kModelScaling, False)
        
        posList.append(pos)
        rotList.append(rot)
        sclList.append(scl)
        
    
    # choose mode - random or normal
    newGroup = FBGroup("newEffectGroup")
              
    for root in rootList:
        
        if isRandomMode:
            
            index = random.randint(0, len(modelList)-1 )
        
            model = modelList[index]
            
            pos = posList[index]
            rot = rotList[index]
            scl = sclList[index]
            
            DoModelClone( root, model, pos, rot, scl, newGroup )
        
        else:
            
            for i in range(len(modelList)):
                
                model = modelList[index]
                DoModelClone(root, model, posList[i], rotList[i], sclList[i], newGroup)
               
    #
    return 1
    
    
def ButtonDuplicateEvent(control, event):
    
    rootList = FBModelList()
    FBGetSelectedModels(rootList)
    
    if len(rootList) != 0 and len(gModelList) != 0:   
        
        value = FBMessageBox("Duplicate Under Root", "Do you want to scatter models randomly?", "Yes", "No", "Cancel")

        randomMode = (value == 1)
         
        if value < 3:
            DoDuplicateUnderRoot(gModelList, rootList, randomMode)
            
    else:
        FBMessageBox( "Duplicate Under Root", "Please grab models and then select roots", "Ok" )

    
def PopulateLayout(mainLyt):
    
    # Create Main region frame:
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    main = FBVBoxLayout()
    mainLyt.AddRegion("main","main", x, y, w, h)
    mainLyt.SetControl("main",main)
    
    vbox = FBVBoxLayout()
    
    
    gModelLabel.Caption = 'Models to duplicate: Empty List'
    vbox.Add(gModelLabel, 25)
    
    b = FBButton()
    b.Caption = 'Grab models'
    b.OnClick.Add(ButtonGrabEvent)
    vbox.Add(b, 25)
    
    b = FBButton()
    b.Caption = 'Duplicate models'
    b.OnClick.Add(ButtonDuplicateEvent)
    vbox.Add(b, 25)
    
    main.AddRelative(vbox, 1.0)
    
def CreateTool():    
    # Tool creation will serve as the hub for all other controls
    t = FBCreateUniqueTool("Duplicate Under The Root Tool " + str(DUPLICATE_TOOL_VERSION))
    PopulateLayout(t)
    t.StartSizeX = 400
    t.StartSizeY = 390
    ShowTool(t)


CreateTool()