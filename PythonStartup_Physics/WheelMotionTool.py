
###########################################################
#
# Tool for computing a wheel motion (rolling and steering)
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
# Author Sergey Solohin (Neill3d) 2015
#  e-mail to: s@neill3d.com
#   www.neill3d.com
#
###########################################################

from pyfbsdk import *
from pyfbsdk_additions import *
import math
import os

gDevelopment = True

WHEEL_MOTION_TOOL_VERSION = 0.52

gapp = FBApplication()
gplayer = FBPlayerControl()
gsystem = FBSystem()

gButtonSteering = FBButton()
gButtonRolling = FBButton()

# steering options
gEditLimit = FBEditNumber()
gEditOffset = FBEditNumber()
gEditFrames = FBEditNumber()
gButtonInvertX = FBButton()
gButtonInvertZ = FBButton()
gButtonAxeX = FBButton()
gButtonAxeY = FBButton()
gButtonAxeZ = FBButton()
gButtonUseParentMotion = FBButton()

# rolling options
gRollingInvert = FBButton()
gRollingParent = FBButton()
gRollingRadiusFromChildren = FBButton()

gRollingAxeX = FBButton()
gRollingAxeY = FBButton()
gRollingAxeZ = FBButton()


def ClearCurves(animnode):
    
    for pnode in animnode.Nodes:
        ClearCurves(pnode)
    
    fcurve = animnode.FCurve
    if fcurve:
        fcurve.EditClear()
       
def Test():
    
    for con in gsystem.Scene.Constraints:
        if con.LongName == "STEERING_LIVE:Test":
            for box in con.Boxes:
                box.Name = "STEERING_LIVE:" + box.Name

        
def ComputeNextDirection(model, numberOfFrames):
    
    ltime = gsystem.LocalTime
    
    result = FBVector3d(0.0, 0.0, 0.0)
    
    
    for i in range(numberOfFrames):
    
        gplayer.StepForward()
        gsystem.Scene.Evaluate()
        nextpos = FBVector3d()
        model.GetVector(nextpos)
        nextpos[1] = 0.0
        
        result[0] += nextpos[0]
        result[2] += nextpos[2]
    
    gplayer.Goto(ltime)
  
    result[0] /= numberOfFrames
    result[2] /= numberOfFrames
    
    return result

###########################################
# variable should be 
#  angle offset and difDir axes reflection
# rotateAxes - X=0, Y=1, Z=2
def ComputeWheelSteering(offsetValue, anglelimit, advanceFrames, invertX, invertZ, useParent, rotateAxes):
    
    models = FBModelList()
    FBGetSelectedModels(models)
    
    if len(models) > 0:
        model = models[0]
        print model.Name
        
        gplayer.GotoStart()
        
        pos = FBVector3d()
        model.GetVector(pos)
        prevpos = FBVector3d(pos)
        nextpos = FBVector3d(pos)
                
        animnode = model.Rotation.GetAnimationNode()
        if animnode:
            animnode = animnode.Nodes[rotateAxes]
            ClearCurves(animnode)
            animnode.KeyAdd( 0.0 )
              
        currTime = gsystem.LocalTime
        endTime = gplayer.LoopStop
        
        direction = FBVector3d(0.0, 0.0, -1.0)
        angle = 0.0
        gsystem.Scene.Evaluate()
        rot = FBVector3d()
        model.GetVector(rot, FBModelTransformationType.kModelRotation, True)
        rot[0] = 0.0
        angle = rot[1]
        print angle
        rot[2] = 0.0
        
        parent = model.Parent
        
        lastangle = angle
        
        while currTime < endTime:
            
            gplayer.StepForward()
            gsystem.Scene.Evaluate()
            
            if parent and useParent:
                parent.GetVector(pos)
            else:
                model.GetVector(pos)
            pos[1] = 0.0
           
            #            
            if parent and useParent:
                nextpos = ComputeNextDirection(parent, int(advanceFrames))
            else:
                nextpos = ComputeNextDirection(model, int(advanceFrames))
                
            difDir = FBVector3d(nextpos[0]-pos[0], 0.0, nextpos[2]-pos[2])
            
            ltime = gsystem.LocalTime
          
            if difDir.Length() > 0.1:
                
                difDir.Normalize()
                if invertX:
                    difDir[0] = -1.0 * difDir[0]
                if invertZ:
                    difDir[2] = -1.0 * difDir[2]
                    
                radians = math.atan2(difDir[2], difDir[0]);
                degrees = math.degrees(radians)
                print degrees
                angle = degrees - offsetValue
              
                   
            if animnode:
                
                childMatrix = FBMatrix()
                rv = FBVector3d(0.0, angle, 0.0)
                FBRotationToMatrix( childMatrix, rv )
                
                if parent:
                    parentMatrix = FBMatrix()
                    parentMatrix.Identity()
                    
                    parent.GetMatrix(parentMatrix)
                    FBGetLocalMatrix(childMatrix, parentMatrix, childMatrix)
                
                FBMatrixToRotation( rv, childMatrix )
                
                if anglelimit > 0.0:
                    if rv[1] < -anglelimit:
                        rv[1] = -anglelimit
                    elif rv[1] > anglelimit:
                        rv[1] = anglelimit
                    
                animnode.KeyAdd( rv[1] )
            
                lastangle = angle
            
            prevpos = FBVector3d(pos)
            currTime = gsystem.LocalTime    

#ComputeWheelSteering()
        
##################################################################################
# where axes is X-0, Y-1, Z-2
def ComputeRadius(model):
    
    bmin = FBVector3d()
    bmax = FBVector3d()
    model.GetBoundingBox(bmin, bmax)
        
    scale = FBVector3d()
    model.GetVector(scale, FBModelTransformationType.kModelScaling, True)
    print scale
        
    bmin = bmin * scale
    bmax = bmax * scale
    
    R = 0.5 * (bmax[1] - bmin[1])
    return R

def ComputeWheelRotation(reverseRotation, computeBBFromChildren, useParent, axes):

    models = FBModelList()
    
    FBGetSelectedModels(models)
    
    if len(models) > 0:
        model = models[0]
        print model.Name
        
        R = 0.0
        if computeBBFromChildren and len(model.Children) > 0:
            
            for child in model.Children:
                localR = ComputeRadius(child)
                if localR > R:
                    R = localR
        else:
            R = ComputeRadius(model)
        
        if R <= 0.0: R=1.0
        print R
        L = 2.0 * 3.1415 * R
        print L
        
        gplayer.GotoStart()
        
        pos = FBVector3d()
        
        parent = model.Parent
       
        if parent:
            parent.GetVector(pos)
        else:
            model.GetVector(pos)
        lastpos = FBVector3d(pos)
        
        animnode = model.Rotation.GetAnimationNode()
        if animnode:
            animnode = animnode.Nodes[axes]
            ClearCurves(animnode)
            animnode.KeyAdd( 0.0 )
            
        currTime = gsystem.LocalTime
        endTime = gplayer.LoopStop
        
        angle = 0.0
        
        while currTime < endTime:
            
            gplayer.StepForward()
            gsystem.Scene.Evaluate()
            
            if parent and useParent:
                parent.GetVector(pos)
            else:
                model.GetVector(pos)
            
            dif = FBVector3d(pos[0]-lastpos[0], pos[1]-lastpos[1], pos[2]-lastpos[2])
            S = dif.Length()
            X = 360.0 / L * S
            #print S
            
            if reverseRotation:
                angle -= X
            else:
                angle += X
            
            if animnode:
                animnode.KeyAdd( angle )
            
            lastpos = FBVector3d(pos)
            currTime = gsystem.LocalTime


#ComputeWheelRotation(0)

# Define a utility function to look up a named animation node
# under a given parent. Will not recurse in the hierarchy.
def FindAnimationNode( pParent, pName ):
    lResult = None
    for lNode in pParent.Nodes:
        if lNode.Name == pName:
            lResult = lNode
            break
    return lResult

def LookForConstraintFile(filename):

    pathlist = gsystem.GetPluginPath()
    
    for path in pathlist:
        fullfilename = path + "\\" + filename
        if os.path.isfile(fullfilename):
            return fullfilename

    pathlist = gsystem.GetPythonStartupPath()
    
    for path in pathlist:
        fullfilename = path + "\\" + filename
        if os.path.isfile(fullfilename):
            return fullfilename
            
    return ""
    

def RunLiveTest():
         
    models = FBModelList()
    FBGetSelectedModels(models)
    
    filePath = LookForConstraintFile("SteeringConstraint.fbx")
    print filePath
    if len(models) > 0 and filePath != "":
        model = models[0]
        print model.Name
        model.Selected = False
        
        pos = FBVector3d()
        model.GetVector(pos)
        
        localtime = gsystem.LocalTime
        
        # merge scene with a constraint for testing
        if True == gapp.FileMerge( filePath ):
        
            gplayer.Goto(localtime)
        
            lconn = None
            
            for con in gsystem.Scene.Constraints:
                if con.LongName == "STEERING_LIVE:Test":
                    lconn = con
                    
            if lconn:
            
                dstbox = lconn.ConstrainObject( model )
                srcmodel = lconn.SetAsSource( model )
                for box in lconn.Boxes:
                    if box.LongName == "STEERING_LIVE:Number to Vector":
                        
                        src = FindAnimationNode(box.AnimationNodeOutGet(), "Result")
                        dst = FindAnimationNode(dstbox.AnimationNodeInGet(), "Rotation")
                        
                        if src and dst:
                            FBConnect(src, dst)
                            print "connect"
                        break
            
                # create a null object to control a test
                null = FBModelNull( "Null steering test")
                null.Translation = pos
                null.Show = True
                null.Selected = True
                
                srcbox = lconn.SetAsSource( null )
                
                for box in lconn.Boxes:
                    if box.LongName == "STEERING_LIVE:Subtract (V1 - V2)":
                        src = FindAnimationNode(srcbox.AnimationNodeOutGet(), "Translation")
                        dst = FindAnimationNode(box.AnimationNodeInGet(), "V1")
                        
                        if src and dst:
                            FBConnect(src, dst)
                            print "connected V1"
                            
                        src = FindAnimationNode(srcmodel.AnimationNodeOutGet(), "Translation")
                        dst = FindAnimationNode(box.AnimationNodeInGet(), "V2")
                        
                        if src and dst:
                            FBConnect(src, dst)
                            print "connected V2"
                        break
                
                lconn.Active = True

def StopLiveTest():
    for con in gsystem.Scene.Constraints:
        if con.LongName == "STEERING_LIVE:Test":
            con.Active = False
            con.FBDelete()
            break
            
    controlModel = FBFindModelByLabelName("Null steering test") 
    if controlModel:
        controlModel.FBDelete()
    
def EventRunSteeringLiveTest(control, event):
    
    if (control.State > 0):
        RunLiveTest()
    else:
        StopLiveTest()

def EventSteeringChange(control, event):
    
    if control == gButtonAxeX:
        gButtonAxeX.State = 1
        gButtonAxeY.State = 0
        gButtonAxeZ.State = 0
    elif control == gButtonAxeY:
        gButtonAxeY.State = 1
        gButtonAxeX.State = 0
        gButtonAxeZ.State = 0
    elif control == gButtonAxeZ:
        gButtonAxeZ.State = 1
        gButtonAxeX.State = 0
        gButtonAxeY.State = 0

    lconn = None
    for con in gsystem.Scene.Constraints:
        if con.LongName == "STEERING_LIVE:Test":
            lconn = con
            
    if lconn:
        for box in lconn.Boxes:
          
            if box.LongName == "STEERING_LIVE:Multiply (a x b) 5":
                src = FindAnimationNode(box.AnimationNodeInGet(), "b")
                if src:
                    value = 1.0
                    if gButtonInvertZ.State > 0:
                        value = -1.0
                    src.WriteData( [value] )
                    
            elif box.LongName == "STEERING_LIVE:Multiply (a x b) 4":
                src = FindAnimationNode(box.AnimationNodeInGet(), "b")
                if src:
                    value = 1.0
                    if gButtonInvertX.State > 0:
                        value = -1.0
                    src.WriteData( [value] )
                    
            elif box.LongName == "STEERING_LIVE:Subtract (a - b) 2":
               
                src = FindAnimationNode(box.AnimationNodeInGet(), "b")
                if src:
                    value = gEditOffset.Value
                    src.WriteData( [value] )
                    
                # specify an axes
                
                for box2 in lconn.Boxes:
                    if box2.LongName == "STEERING_LIVE:Number to Vector":
                        
                        src = FindAnimationNode(box.AnimationNodeOutGet(), "Result")
                        dstX = FindAnimationNode(box2.AnimationNodeInGet(), "X")
                        dstY = FindAnimationNode(box2.AnimationNodeInGet(), "Y")
                        dstZ = FindAnimationNode(box2.AnimationNodeInGet(), "Z")
                        
                        if src and dstX and dstY and dstZ:
                            FBDisconnect(src, dstX)
                            FBDisconnect(src, dstY)
                            FBDisconnect(src, dstZ)
                            
                            if gButtonAxeX.State > 0:
                                FBConnect(src, dstX)
                            elif gButtonAxeY.State > 0:
                                FBConnect(src, dstY)
                            else:
                                FBConnect(src, dstZ)
                        
                        break
                    
def EventComputeButton(control, event):
    
    if gButtonSteering.State > 0:
        
        axes = 0
        if gButtonAxeY.State > 0:
            axes = 1
        elif gButtonAxeZ.State > 0:
            axes = 2
        
        ComputeWheelSteering(gEditOffset.Value, gEditLimit.Value, gEditFrames.Value, gButtonInvertX.State, gButtonInvertZ.State, gButtonUseParentMotion.State, axes)

    if gButtonRolling.State > 0:
        
        axes = 0
        if gRollingAxeY.State > 0:
            axes = 1
        elif gRollingAxeZ.State > 0:
            axes = 2
            
        ComputeWheelRotation(gRollingInvert.State, gRollingRadiusFromChildren.State, gRollingParent.State, axes)

def EventRollingAxes(control, event):
    
    if control == gRollingAxeX:
        gRollingAxeX.State = 1
        gRollingAxeY.State = 0
        gRollingAxeZ.State = 0
    elif control == gRollingAxeY:
        gRollingAxeY.State = 1
        gRollingAxeX.State = 0
        gRollingAxeZ.State = 0
    elif control == gRollingAxeZ:
        gRollingAxeZ.State = 1
        gRollingAxeX.State = 0
        gRollingAxeY.State = 0

# for that we have to select two object, first source and then destination
def EventCopyRotation(control, event):
    
    models = FBModelList()
    FBGetSelectedModels(models)
    
    if len(models) == 2:
        
        FBUndoManager().Undo(True)
        
        models2 = FBModelList()
        FBGetSelectedModels(models2)
        
        if len(models2) == 1:
            
            srcmodel = models2[0]
            dstmodel = None
            if srcmodel == models[0]:
                dstmodel = models[1]
            else:
                dstmodel = models[0]
                
            print srcmodel.Name
            print dstmodel.Name
            
            conn = FBConstraintManager().TypeCreateConstraint("Parent/Child")
            if conn:
                conn.ReferenceAdd(0, dstmodel)
                conn.ReferenceAdd(1, srcmodel)
                conn.PropertyList.Find('Affect Translation X').Data = False
                conn.PropertyList.Find('Affect Translation Y').Data = False
                conn.PropertyList.Find('Affect Translation Z').Data = False
                conn.Snap()
                
                dstmodel.Selected = True
                srcmodel.Selected = False
                
                options = FBPlotOptions()
                options.mPlotAllTakes = False
                options.mPlotPeriod = FBTime(0,0,0,1)
                options.mRotationFilterToApply = FBRotationFilter.kFBRotationFilterGimbleKiller
                options.mUseConstantKeyReducer = False
                
                gsystem.CurrentTake.PlotTakeOnSelected(options)
                
                conn.Active = False
                conn.FBDelete()
        

def PopulateLayout(mainLyt):
    
    # Create Main region frame:
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    main = FBVBoxLayout()
    mainLyt.AddRegion("main","main", x, y, w, h)
    mainLyt.SetControl("main",main)
    
    hbox = FBHBoxLayout()
    
    leftbox = FBVBoxLayout()
    
    gButtonSteering.Caption = "Compute Steering"
    gButtonSteering.Look = FBButtonLook.kFBLookColorChange
    gButtonSteering.Style = FBButtonStyle.kFBCheckbox
    leftbox.Add(gButtonSteering, 25)
    
    b = FBLabel()
    b.Caption = "Test and options:"
    leftbox.Add(b, 25)
    
    b = FBButton()
    b.Caption = "Run Live Test!"
    b.Style = FBButtonStyle.kFB2States
    b.OnClick.Add( EventRunSteeringLiveTest )
    leftbox.Add(b, 25)

    #
    propbox = FBHBoxLayout()
    
    b = FBLabel()
    b.Caption = "Offset: "
    propbox.AddRelative(b, 0.4)
    
    gEditOffset.Value = 90.0
    gEditOffset.OnChange.Add( EventSteeringChange )
    propbox.AddRelative(gEditOffset, 0.6)
    
    leftbox.Add(propbox, 25)
    
    #
    propbox = FBHBoxLayout()
    
    b = FBLabel()
    b.Caption = "Local Angle Limit: "
    propbox.AddRelative(b, 0.6)
    
    gEditLimit.Value = 35.0
    propbox.AddRelative(gEditLimit, 0.4)
    
    leftbox.Add(propbox, 25)
    
    #
    propbox = FBHBoxLayout()
    
    b = FBLabel()
    b.Caption = "Adv. Frames:"
    propbox.AddRelative(b, 0.6)
    
    gEditFrames.Value = 2.0
    gEditFrames.Precision = 1.0
    propbox.AddRelative(gEditFrames, 0.4)
    
    leftbox.Add(propbox, 25)
    
    #
    gButtonInvertX.Caption = "Invert X"
    gButtonInvertX.Style = FBButtonStyle.kFBCheckbox
    gButtonInvertX.OnClick.Add( EventSteeringChange )
    leftbox.Add(gButtonInvertX, 25)
    
    gButtonInvertZ.Caption = "Invert Z"
    gButtonInvertZ.Style = FBButtonStyle.kFBCheckbox
    gButtonInvertZ.OnClick.Add( EventSteeringChange )
    gButtonInvertZ.State = 1
    leftbox.Add(gButtonInvertZ, 25)
    
    gButtonUseParentMotion.Caption = "Use Parent Motion"
    gButtonUseParentMotion.Style = FBButtonStyle.kFBCheckbox
    gButtonUseParentMotion.OnClick.Add( EventSteeringChange )
    gButtonUseParentMotion.State = 1
    leftbox.Add(gButtonUseParentMotion, 25)
    
    propbox = FBHBoxLayout()
    
    b = FBLabel()
    b.Caption = "Axes: "
    propbox.AddRelative(b, 0.25)
    
    names = ["X", "Y", "Z"]
    controls = [gButtonAxeX, gButtonAxeY, gButtonAxeZ]
    for name, control in zip(names, controls):
        control.Caption = name
        control.Style = FBButtonStyle.kFBRadioButton
        control.OnClick.Add( EventSteeringChange )
        propbox.AddRelative(control, 0.25)
        
    gButtonAxeY.State = 1
    
    leftbox.Add(propbox, 25)
    
    rightbox = FBVBoxLayout()
    
    ##
    gButtonRolling.Caption = "Compute Rolling"
    gButtonRolling.Justify = FBTextJustify.kFBTextJustifyCenter
    gButtonRolling.Look = FBButtonLook.kFBLookColorChange
    gButtonRolling.Style = FBButtonStyle.kFBCheckbox
    rightbox.Add(gButtonRolling, 25)
    
    #
    gRollingInvert.Caption = "Invert Direction"
    gRollingInvert.Style = FBButtonStyle.kFBCheckbox
    gRollingInvert.State = 1
    rightbox.Add(gRollingInvert, 25)
    
    gRollingRadiusFromChildren.Caption = "Radius From Children"
    gRollingRadiusFromChildren.Style = FBButtonStyle.kFBCheckbox
    gRollingRadiusFromChildren.State = 0
    rightbox.Add(gRollingRadiusFromChildren, 25)
    
    gRollingParent.Caption = "Use Parent Motion"
    gRollingParent.Style = FBButtonStyle.kFBCheckbox
    gRollingParent.State = 1
    rightbox.Add(gRollingParent, 25)
    
    #
    propbox = FBHBoxLayout()
    names = ["X", "Y", "Z"]
    controls = [gRollingAxeX, gRollingAxeY, gRollingAxeZ]
    for name, control in zip(names, controls):
        control.Caption = name
        control.Style = FBButtonStyle.kFBRadioButton
        control.OnClick.Add( EventRollingAxes )
        propbox.AddRelative(control, 0.25)
        
    rightbox.Add(propbox, 25)
        
    gRollingAxeX.State = 1
    
    ###
    hbox.AddRelative(leftbox, 0.5)
    hbox.AddRelative(rightbox, 0.5)
    
    main.AddRelative(hbox, 0.7)
    
    #
    propbox = FBHBoxLayout()
    
    b = FBButton()
    b.Caption = "Compute"
    b.OnClick.Add( EventComputeButton )
    propbox.AddRelative(b, 0.6)
    
    b = FBButton()
    b.Caption = "Copy Rotation"
    b.OnClick.Add( EventCopyRotation )
    propbox.AddRelative(b, 0.4)
    
    main.Add(propbox, 25)
    
def CreateTool():    
    # Tool creation will serve as the hub for all other controls
    t = FBCreateUniqueTool("Wheel Motion Tool " + str(WHEEL_MOTION_TOOL_VERSION))
    PopulateLayout(t)
    t.StartSizeX = 400
    t.StartSizeY = 390

    if gDevelopment:
        ShowTool(t)

CreateTool()
