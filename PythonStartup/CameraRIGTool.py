

########################################################################
#
# Tool for managing Camera RIG
#
#
# Author: Sergey Solohin (Neill3d) 2015, e-mail to: s@neill3d.com
#  www.neill3d.com
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
# TODO
#
#
#
# LOG
#   30.07.2015
#       
#
#
########################################################################


from pyfbsdk import *
from pyfbsdk_additions import *
import os
import re

gDEVELOPMENT = True

gApp = FBApplication()
gSystem = FBSystem()
gProfiler = FBProfiler()
gPlayer = FBPlayerControl()

gButtonControlCamera = FBButton()
gButtonGrouping = FBButton()
gEditGroupName = FBEdit()

CAMERA_RIG_TOOL_VERSION = 0.11

def GetSelectionCenter():
    
    l = FBModelList()
    FBGetSelectedModels(l)
    
    center = FBVector3d()
    
    for obj in l:
        vmin = FBVector3d()
        vmax = FBVector3d()
        m = FBMatrix()
        
        obj.GetMatrix(m)
        obj.GetBoundingBox(vmin, vmax)
        
        vmin4 = FBVector4d(vmin[0], vmin[1], vmin[2], 1.0)
        vmax4 = FBVector4d(vmax[0], vmax[1], vmax[2], 1.0)
        
        FBVectorMatrixMult(vmin4, m, vmin4)
        FBVectorMatrixMult(vmax4, m, vmax4)
        
        center[0] += (vmin4[0] + 0.5*(vmax4[0]-vmin4[0]))
        center[1] += (vmin4[1] + 0.5*(vmax4[1]-vmin4[1]))
        center[2] += (vmin4[2] + 0.5*(vmax4[2]-vmin4[2]))
                
    count = len(l)
    if (count > 0):
        center[0] /= count
        center[1] /= count
        center[2] /= count
        
    return center

def CreateNull(name, pos, parent, bshow):
                
    newObj = FBModelNull(name)
    newObj.Show = bshow
    newObj.Visible = True
    
    newObj.SetVector(pos)
    if parent:
        newObj.Parent = parent
    
    return newObj
                
def CreateCamera( name, target, parent ):
    
    newObj = FBCamera(name)
    newObj.Show = True
    newObj.Visible = True
    newObj.Roll.SetAnimated(True)
    newObj.PropertyList.Find('Field Of View').SetAnimated(True)
    
    if target != None:
        newObj.Interest = target
    
    if parent != None:
        newObj.Parent = parent

    pos = FBVector3d(0.0, 0.0, 400.0)
    '''
    currCam = gSystem.Renderer.CurrentCamera
    if currCam:
        currCam.GetVector(pos)
    '''
    newObj.SetVector(pos, FBModelTransformationType.kModelInverse_Translation, False)
    
    return newObj
                
def CreateProp(name, obj):
    
    newProp = obj.PropertyCreate(name, FBPropertyType.kFBPT_double, 'Number',True, True, None)
    newProp.SetAnimated(True)
    return newProp

# Define a utility function to look up a named animation node
# under a given parent. Will not recurse in the hierarchy.
def FindAnimationNode( pParent, pName ):
    lResult = None
    for lNode in pParent.Nodes:
        if lNode.Name == pName:
            lResult = lNode
            break
    return lResult

def OnButtonGroupEvent(control, event):
    print "grouping"

def OnCreateEvent(control, event):
    print "create"

    # calculate selection center
    center = GetSelectionCenter()
    
    # create hierarchy of elements with root in the selection center

    if gButtonControlCamera.State == 1:
        targetObj = CreateNull( "CameraRig_root", center, None, True )
        offsetObj = CreateNull( "Offset", center, targetObj, False )
        
        headingObj = CreateNull( "Heading", center, offsetObj, False )
        pitchObj = CreateNull( "Pitch", center, headingObj, False )
        bankObj = CreateNull( "Bank", center, pitchObj, False )
        
        cameraObj = CreateCamera( "Camera", bankObj, offsetObj )
        
    else:
        targetObj = CreateNull( "CameraRig_root", center, None, True )
        offsetObj = CreateNull( "Offset", center, targetObj, False )
        headingObj = CreateNull( "Heading", center, offsetObj, False )
        pitchObj = CreateNull( "Pitch", center, headingObj, False )
        bankObj = CreateNull( "Bank", center, pitchObj, False )
        cameraObj = CreateCamera( "Camera", offsetObj, bankObj )
    
    if gButtonGrouping.State == 1:
        lGroup = FBGroup("CameraRIGGroup")
        #adding objects
        lGroup.ConnectSrc(targetObj)
        lGroup.ConnectSrc(offsetObj)
        lGroup.ConnectSrc(headingObj)
        lGroup.ConnectSrc(pitchObj)
        lGroup.ConnectSrc(bankObj)
        lGroup.ConnectSrc(cameraObj)
    
    
    headingProp = CreateProp("Heading", targetObj)
    pitchProp = CreateProp("Pitch", targetObj)
    bankProp = CreateProp("Bank", targetObj)
    trackProp = CreateProp("Track", targetObj)
    trackProp.Data = 400.0
    trackProp.SetMin(0.0, False)
    trackProp.SetMax(1000.0, False)          
    headingSpeedProp = CreateProp("Heading Speed", targetObj)            
    pitchSpeedProp = CreateProp("Pitch Speed", targetObj)            
    bankSpeedProp = CreateProp("Bank Speed", targetObj)            
    trackSpeedProp = CreateProp("Track Speed", targetObj)            
    xSpeedProp = CreateProp("X Speed", targetObj)            
    ySpeedProp = CreateProp("Y Speed", targetObj)            
    zSpeedProp = CreateProp("Z Speed", targetObj)            
    focalShiftProp = CreateProp("Focal Shift", targetObj)                                    
    
    # connect props in relation constraint
    
    newcon = FBConstraintRelation("RelationCameraRig")
    
    targetBox = newcon.SetAsSource(targetObj)
    offsetBox = newcon.ConstrainObject(offsetObj)
    offsetBox.UseGlobalTransforms = False
    headingBox = newcon.ConstrainObject(headingObj)
    headingBox.UseGlobalTransforms = False
    pitchBox = newcon.ConstrainObject(pitchObj)
    pitchBox.UseGlobalTransforms = False
    cameraBoxSrc = newcon.SetAsSource(cameraObj)
    cameraBox = newcon.ConstrainObject(cameraObj)
    cameraBox.UseGlobalTransforms = False
    bankBox = newcon.ConstrainObject(bankObj)
    bankBox.UseGlobalTransforms = False
    
    localTimeBox = newcon.CreateFunctionBox("System", "Local Time")
    timeToSecondsBox = newcon.CreateFunctionBox("Converters", "Time to seconds")
    multBox = newcon.CreateFunctionBox("Number", "Multiply (a x b)")
    addBox = newcon.CreateFunctionBox("Number", "Add (a + b)")
    numberToVectorBox = newcon.CreateFunctionBox("Converters", "Number to Vector")

    src = FindAnimationNode(localTimeBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(timeToSecondsBox.AnimationNodeInGet(), "Time")
    FBConnect(src, dst)
    
    #
    # connect heading
    
    src = FindAnimationNode(timeToSecondsBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(multBox.AnimationNodeInGet(), "b")
    FBConnect(src, dst)
    
    src = FindAnimationNode(targetBox.AnimationNodeOutGet(), "Heading Speed")
    dst = FindAnimationNode(multBox.AnimationNodeInGet(), "a")
    FBConnect(src, dst)
    
    src = FindAnimationNode(targetBox.AnimationNodeOutGet(), "Heading")
    dst = FindAnimationNode(addBox.AnimationNodeInGet(), "a")
    FBConnect(src, dst)    
    
    src = FindAnimationNode(multBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(addBox.AnimationNodeInGet(), "b")
    FBConnect(src, dst)
    
    src = FindAnimationNode(addBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(numberToVectorBox.AnimationNodeInGet(), "Y")
    FBConnect(src, dst)

    src = FindAnimationNode(numberToVectorBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(headingBox.AnimationNodeInGet(), "Lcl Rotation")
    FBConnect(src, dst)
    
    #
    # connect pitch
    multBox = newcon.CreateFunctionBox("Number", "Multiply (a x b)")
    addBox = newcon.CreateFunctionBox("Number", "Add (a + b)")
    numberToVectorBox = newcon.CreateFunctionBox("Converters", "Number to Vector")
    
    src = FindAnimationNode(timeToSecondsBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(multBox.AnimationNodeInGet(), "b")
    FBConnect(src, dst)
    
    src = FindAnimationNode(targetBox.AnimationNodeOutGet(), "Pitch Speed")
    dst = FindAnimationNode(multBox.AnimationNodeInGet(), "a")
    FBConnect(src, dst)
    
    src = FindAnimationNode(targetBox.AnimationNodeOutGet(), "Pitch")
    dst = FindAnimationNode(addBox.AnimationNodeInGet(), "a")
    FBConnect(src, dst)    
    
    src = FindAnimationNode(multBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(addBox.AnimationNodeInGet(), "b")
    FBConnect(src, dst)
    
    src = FindAnimationNode(addBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(numberToVectorBox.AnimationNodeInGet(), "X")
    FBConnect(src, dst)

    src = FindAnimationNode(numberToVectorBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(pitchBox.AnimationNodeInGet(), "Lcl Rotation")
    FBConnect(src, dst)


    #
    # connect bank (roll)
    
    multBox = newcon.CreateFunctionBox("Number", "Multiply (a x b)")
    addBox = newcon.CreateFunctionBox("Number", "Add (a + b)")
    
    src = FindAnimationNode(timeToSecondsBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(multBox.AnimationNodeInGet(), "b")
    FBConnect(src, dst)
    
    src = FindAnimationNode(targetBox.AnimationNodeOutGet(), "Bank Speed")
    dst = FindAnimationNode(multBox.AnimationNodeInGet(), "a")
    FBConnect(src, dst)
    
    src = FindAnimationNode(targetBox.AnimationNodeOutGet(), "Bank")
    dst = FindAnimationNode(addBox.AnimationNodeInGet(), "a")
    FBConnect(src, dst)    
    
    src = FindAnimationNode(multBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(addBox.AnimationNodeInGet(), "b")
    FBConnect(src, dst)
    
    src = FindAnimationNode(addBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(cameraBox.AnimationNodeInGet(), "Roll")
    FBConnect(src, dst)
    
    #
    # connect track
    
    multBox = newcon.CreateFunctionBox("Number", "Multiply (a x b)")
    addBox = newcon.CreateFunctionBox("Number", "Add (a + b)")
    numberToVectorBox = newcon.CreateFunctionBox("Converters", "Number to Vector")
    
    src = FindAnimationNode(timeToSecondsBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(multBox.AnimationNodeInGet(), "b")
    FBConnect(src, dst)
    
    src = FindAnimationNode(targetBox.AnimationNodeOutGet(), "Track Speed")
    dst = FindAnimationNode(multBox.AnimationNodeInGet(), "a")
    FBConnect(src, dst)
    
    src = FindAnimationNode(targetBox.AnimationNodeOutGet(), "Track")
    dst = FindAnimationNode(addBox.AnimationNodeInGet(), "a")
    FBConnect(src, dst)    
    
    src = FindAnimationNode(multBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(addBox.AnimationNodeInGet(), "b")
    FBConnect(src, dst)
    
    src = FindAnimationNode(addBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(numberToVectorBox.AnimationNodeInGet(), "Z")
    FBConnect(src, dst)

    src = FindAnimationNode(numberToVectorBox.AnimationNodeOutGet(), "Result")
    if gButtonControlCamera.State == 1:
        dst = FindAnimationNode(bankBox.AnimationNodeInGet(), "Lcl Translation")        
    else:
        dst = FindAnimationNode(cameraBox.AnimationNodeInGet(), "Lcl Translation")
    FBConnect(src, dst)
    
    #
    # connect x speed

    numberToVectorBox = newcon.CreateFunctionBox("Converters", "Number to Vector")
    
    multBox = newcon.CreateFunctionBox("Number", "Multiply (a x b)")
    
    src = FindAnimationNode(timeToSecondsBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(multBox.AnimationNodeInGet(), "b")
    FBConnect(src, dst)
    
    src = FindAnimationNode(targetBox.AnimationNodeOutGet(), "X Speed")
    dst = FindAnimationNode(multBox.AnimationNodeInGet(), "a")
    FBConnect(src, dst)
        
    src = FindAnimationNode(multBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(numberToVectorBox.AnimationNodeInGet(), "X")
    FBConnect(src, dst)

    # y
    multBox = newcon.CreateFunctionBox("Number", "Multiply (a x b)")
    
    src = FindAnimationNode(timeToSecondsBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(multBox.AnimationNodeInGet(), "b")
    FBConnect(src, dst)
    
    src = FindAnimationNode(targetBox.AnimationNodeOutGet(), "Y Speed")
    dst = FindAnimationNode(multBox.AnimationNodeInGet(), "a")
    FBConnect(src, dst)
        
    src = FindAnimationNode(multBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(numberToVectorBox.AnimationNodeInGet(), "Y")
    FBConnect(src, dst)

    # z
    multBox = newcon.CreateFunctionBox("Number", "Multiply (a x b)")
    
    src = FindAnimationNode(timeToSecondsBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(multBox.AnimationNodeInGet(), "b")
    FBConnect(src, dst)
    
    src = FindAnimationNode(targetBox.AnimationNodeOutGet(), "Z Speed")
    dst = FindAnimationNode(multBox.AnimationNodeInGet(), "a")
    FBConnect(src, dst)
        
    src = FindAnimationNode(multBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(numberToVectorBox.AnimationNodeInGet(), "Z")
    FBConnect(src, dst)

    # x,y,z speed to lcl translation
    
    src = FindAnimationNode(numberToVectorBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(offsetBox.AnimationNodeInGet(), "Lcl Translation")
    FBConnect(src, dst)
    
    #
    # focal shift (or simple realization - field of view shift)
    
    addBox = newcon.CreateFunctionBox("Number", "Add (a + b)")

    src = FindAnimationNode(targetBox.AnimationNodeOutGet(), "Focal Shift")
    dst = FindAnimationNode(addBox.AnimationNodeInGet(), "a")
    FBConnect(src, dst)    
    
    src = FindAnimationNode(cameraBoxSrc.AnimationNodeOutGet(), "Field Of View")
    dst = FindAnimationNode(addBox.AnimationNodeInGet(), "b")
    FBConnect(src, dst)
    
    src = FindAnimationNode(addBox.AnimationNodeOutGet(), "Result")
    dst = FindAnimationNode(cameraBox.AnimationNodeInGet(), "Field Of View")
    FBConnect(src, dst)

    # 
    #
    newcon.Active = True
    
def OnPlotEvent(control, event):
    print "plot and delete rig"
    
def PopulateLayout(mainLyt):
    
    # Create Main region frame:
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    main = FBVBoxLayout()
    mainLyt.AddRegion("main","main", x, y, w, h)
    mainLyt.SetControl("main",main)
    
    # create horizontal boxes packed in a vbox
    vstripes = FBVBoxLayout()
    
    b = FBButton()
    b.Caption = "Create"
    b.OnClick.Add(OnCreateEvent)

    vstripes.AddRelative(b, 0.3)
    
    gButtonControlCamera.Caption = "Control node is Camera"
    gButtonControlCamera.Style = FBButtonStyle.kFBCheckbox
    gButtonControlCamera.State = 0

    vstripes.AddRelative(gButtonControlCamera, 0.3)
    
    hstripes = FBHBoxLayout()
    
    gButtonGrouping.Style = FBButtonStyle.kFBCheckbox
    gButtonGrouping.Caption = "Grouping"
    gButtonGrouping.OnClick.Add(OnButtonGroupEvent)
    gButtonGrouping.State = 1
    
    hstripes.Add(gButtonGrouping, 100)
    
    gEditGroupName.Text = 'CameraRigGroup'
    hstripes.AddRelative(gEditGroupName, 0.5)
    
    vstripes.AddRelative(hstripes, 0.3)
    
    b = FBButton()
    b.Caption = "Plot and Delete RIG"
    b.OnClick.Add(OnPlotEvent)

    vstripes.AddRelative(b, 0.3)
    
    main.AddRelative(vstripes,1.0)
    
    
def CreateTool():    
    
    # Tool creation will serve as the hub for all other controls
    global t
    t = FBCreateUniqueTool("Camera RIG " + str(CAMERA_RIG_TOOL_VERSION) + " (MoPlugs)" )
    t.StartSizeX = 220
    t.StartSizeY = 220
    PopulateLayout(t)    
    
    if gDEVELOPMENT:
        ShowTool(t)

CreateTool()

