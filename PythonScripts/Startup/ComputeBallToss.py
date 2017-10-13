####################################################################
##
## ComputeBallToss.py
## Author Sergey <Neill3d> Solohin 2013
## s@neill3d.com
## www.neill3d.com
##
## Github repo - https://github.com/Neill3d/MoPlugs
## Licensed under BSD 3-clause
##  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
##
####################################################################

from pyfbsdk import *
from pyfbsdk_additions import *
import math

gDevelopment = True

gSystem = FBSystem()
gPlayer = FBPlayerControl()

editStart = FBEditNumber()
buttonStart = FBButton()
editStop = FBEditNumber()
buttonStop = FBButton()

editH = FBEditNumber()
buttonAction = FBButton()

editCurrentSpeed = FBEditNumber()
buttonComputeSpeed = FBButton()
editGoalSpeed = FBEditNumber()
buttonComputeGoal = FBButton()

editScaleFactor = FBEditNumber()
buttonComputeScale = FBButton()

def PopulateTool(t):
    #populate regions here

    layout = FBVBoxLayout()
    
    x = FBAddRegionParam(5,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(0,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(-10,FBAttachType.kFBAttachWidth,"")
    h = FBAddRegionParam(0,FBAttachType.kFBAttachHeight,"")
    t.AddRegion("main","main", x, y, w, h)

    t.SetControl("main", layout)

    #
    b = FBLabel()
    b.Caption = "Action Time Range"
    layout.Add(b, 25)
    layout.Add(editStart, 25)
    layout.Add(buttonStart, 25)
    layout.Add(editStop, 25)
    layout.Add(buttonStop, 25)
    
    b = FBLabel()
    b.Caption = "Define Maximum Height and Compute"
    layout.Add(b, 25)
    layout.Add(editH, 25)
    layout.Add(buttonAction, 25)
    
    b = FBLabel()
    b.Caption = "Helpful physics information"
    layout.Add(b, 25)
    layout.Add(editCurrentSpeed, 25)
    layout.Add(buttonComputeSpeed, 25)
    layout.Add(editGoalSpeed, 25)
    layout.Add(buttonComputeGoal, 25)
    
    layout.Add(editScaleFactor, 25)
    layout.Add(buttonComputeScale, 25)
    
    #
    editStart.Value = 1908.0
    editStop.Value = 1948.0
    
    buttonStart.Caption = "Assign Start"
    buttonStart.OnClick.Add(ButtonStartEvent)
  
    buttonStop.Caption = "Assign Stop"
    buttonStop.OnClick.Add(ButtonStopEvent)
    
    editH.Value = 400.0
    
    buttonAction.Caption = "Do it!"
    buttonAction.OnClick.Add(OnButtonActionClick)
    
    buttonComputeSpeed.Caption = "Compute Current Speed"
    buttonComputeSpeed.OnClick.Add(OnButtonComputeSpeedClick)
    
    buttonComputeGoal.Caption = "Compute Goal Speed"
    buttonComputeGoal.OnClick.Add(OnButtonComputeGoalClick)
    
    buttonComputeScale.Caption = "Compute Scale Factor"
    buttonComputeScale.OnClick.Add(OnButtonComputeScaleClick)
    
def ButtonStartEvent(control, event):
    time = FBSystem().LocalTime
    frame = time.GetFrame()
    editStart.Value = frame
    
def ButtonStopEvent(control, event):
    time = FBSystem().LocalTime
    frame = time.GetFrame()
    editStop.Value = frame        
    
def ButtonTimelineEvent(control, event):
    btime = FBPlayerControl().LoopStart
    etime = FBPlayerControl().LoopStop
    editStart.Value = btime.GetFrame()
    editStop.Value = etime.GetFrame()
    
def IsTimeInside(timespan, time):
    if (time > timespan.GetStart() and time < timespan.GetStop() ): return True
    return False
'''
def ProcessFCurve(curve, currTime, startTime, stopTime, blendTime):
    value = curve.Evaluate(currTime)
    count = len(curve.Keys)
    
    startTimeBlend = startTime - blendTime
    stopTimeBlend = stopTime + blendTime
    
    if curve: curve.KeyDeleteByTimeRange(startTimeBlend, stopTimeBlend, True)
    
    startNdx = curve.KeyAdd(startTime, value)
    stopNdx = curve.KeyAdd(stopTime, value)
    
    if startNdx >= 0: 
        curve.Keys[startNdx].TangentMode = FBTangentMode.kFBTangentModeUser
        curve.Keys[startNdx].LeftDerivative = 0.0
        curve.Keys[startNdx].RightDerivative = 0.0

    if stopNdx >= 0: 
        curve.Keys[stopNdx].TangentMode = FBTangentMode.kFBTangentModeUser   
        curve.Keys[stopNdx].LeftDerivative = 0.0
        curve.Keys[stopNdx].RightDerivative = 0.0
    
def ProcessAnimationNode(animNode, currTime, startTime, stopTime, blendTime):
    for node in animNode.Nodes:
        if node.FCurve:
            ProcessFCurve(node.FCurve, currTime, startTime, stopTime, blendTime)
    
def ButtonActionEvent(control, event):
    currTime = FBSystem().LocalTime
    startTime = FBTime(0,0,0, int(editStart.Value) )
    stopTime = FBTime(0,0,0, int(editStop.Value) )
    blendTime = FBTime(0,0,0, int(editBlendTime.Value) )
    
    
    models = FBModelList()
    FBGetSelectedModels(models)
    
    mode = listAction.ItemIndex
    
    for model in models:
        if mode in [0,2,4]:
            animNode = model.Translation.GetAnimationNode()
            if animNode: ProcessAnimationNode(animNode, currTime, startTime, stopTime, blendTime)
        if mode in [1,2,4]:
            animNode = model.Rotation.GetAnimationNode()
            if animNode: ProcessAnimationNode(animNode, currTime, startTime, stopTime, blendTime)
        if mode in [3,4]:
            animNode = model.Scaling.GetAnimationNode()
            if animNode: ProcessAnimationNode(animNode, currTime, startTime, stopTime, blendTime)
'''

def MakeAGap(curve, startTime, stopTime):
    if curve: 
        curve.KeyDeleteByTimeRange(startTime, stopTime, True)
        
        value = curve.Evaluate(startTime)
        ndx = curve.KeyAdd(startTime, value)
        if ndx >= 0: 
            curve.Keys[ndx].TangentMode = FBTangentMode.kFBTangentModeUser
            curve.Keys[ndx].LeftDerivative = 0.0
            curve.Keys[ndx].RightDerivative = 0.0
            
        value = curve.Evaluate(stopTime)
        ndx = curve.KeyAdd(stopTime, value)
        if ndx >= 0: 
            curve.Keys[ndx].TangentMode = FBTangentMode.kFBTangentModeUser
            curve.Keys[ndx].LeftDerivative = 0.0
            curve.Keys[ndx].RightDerivative = 0.0
        
def MakeAToss(curve, startTime, stopTime):
    
    if curve: 
        curve.KeyDeleteByTimeRange(startTime, stopTime, True)

    currTime = FBTime(startTime)
    stepTime = FBTime(0,0,0, 1)
    
    startSecs = startTime.GetSecondDouble()
    stopSecs = stopTime.GetSecondDouble()
    
    g = 9.8
    u0 = (stopSecs - startSecs) * g * 6.0
    h0 = curve.Evaluate(startTime)
    
    hmax = editH.Value
    u0 = math.sqrt(hmax * 2.0 * g)
    
    tmax = u0 / g

    trange = (stopSecs - startSecs)

    tscale = 2.0 * tmax / trange
    print tmax
    print tscale
    
    while currTime <= stopTime:
        
        t = currTime.GetSecondDouble() - startSecs
        t = t * tscale
        h = u0 * t - g * t * t / 2.0
        curve.KeyAdd(currTime, h + h0)
        
        currTime = currTime + stepTime

def ProcessAnimationNode(animNode, startTime, stopTime):
    
    # gap for X and Z
    MakeAGap(animNode.Nodes[0].FCurve, startTime, stopTime)
    MakeAGap(animNode.Nodes[2].FCurve, startTime, stopTime)
    
    MakeAToss(animNode.Nodes[1].FCurve, startTime, stopTime)
    
def OnButtonActionClick(control, event):
    print "action"
    
    startTime = FBTime(0,0,0, int(editStart.Value) )
    stopTime = FBTime(0,0,0, int(editStop.Value) )
    
    models = FBModelList()
    FBGetSelectedModels(models)
    
    if len(models) > 0:
        ball = models[0]
        
        animNode = ball.Translation.GetAnimationNode()
        if animNode: 
            ProcessAnimationNode(animNode, startTime, stopTime)
    
def OnButtonComputeSpeedClick(control, event):
    print "action"
    
    models = FBModelList()
    FBGetSelectedModels(models)
    
    if len(models) > 0:
        ball = models[0]
        
        v1 = FBVector3d()
        v2 = FBVector3d()
        
        gPlayer.Goto( FBTime(0,0,0, int(editStart.Value) ) )
        gSystem.Scene.Evaluate()
        ball.GetVector(v2)
        
        gPlayer.Goto( FBTime(0,0,0, int(editStart.Value)-1) )
        gSystem.Scene.Evaluate()
        ball.GetVector(v1)
        
        mag = FBLength( FBVector4d(v2[0]-v1[0], v2[1]-v1[1], v2[2]-v1[2], 0.0) )
        
        # v = s / t
        # with 30 fps, 1 frame in seconds = 1.0 / 30.0
        t = 1.0 / 30.0
        editCurrentSpeed.Value = mag / t
        
    
def OnButtonComputeGoalClick(control, event):
    print "action"
    
    if editStart.Value == editStop.Value:
        FBMessageBox( "Compute Ball Toss", "You have an empty time range!", "Ok")
        return
    
    startTime = FBTime(0,0,0, int(editStart.Value) )
    stopTime = FBTime(0,0,0, int(editStop.Value) )
    
    startSecs = startTime.GetSecondDouble()
    stopSecs = stopTime.GetSecondDouble()
    
    g = 9.8
    u0 = (stopSecs - startSecs) * g
    
    editGoalSpeed.Value = u0

def OnButtonComputeScaleClick(control, event):
    
    startTime = FBTime(0,0,0, int(editStart.Value) )
    stopTime = FBTime(0,0,0, int(editStop.Value) )
    
    startSecs = startTime.GetSecondDouble()
    stopSecs = stopTime.GetSecondDouble()
    
    g = 9.8
    u0 = (stopSecs - startSecs) * g * 6.0
   
    hmax = editH.Value
    u0 = math.sqrt(hmax * 2.0 * g)
    
    tmax = u0 / g

    trange = (stopSecs - startSecs)

    tscale = 2.0 * tmax / trange
    editScaleFactor.Value = tscale

def CreateTool():
    t = FBCreateUniqueTool("Ball Toss (Sergey <Neill3d> Solokhin 2017)")
    t.StartSizeX = 325
    t.StartSizeY = 480
    PopulateTool(t)
    if gDevelopment:
        ShowTool(t)
CreateTool()
