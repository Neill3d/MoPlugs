
#
# Helper script for manage switch between Stance (active facial retargeting) and Animation (plot on joints)
#
#  Version for MotionBuilder 2014
#
# Sergey Solohin (Neill3d) 2015
#   e-mail to: s@neill3d.com
#  www.neill3d.com
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *
from pyfbsdk_additions import *

gDevelopment = True
gVersion = 0.5
gSystem = FBSystem()

# UI
gButtonActivateConstraintsOnStance = FBButton()
gButtonDeactivateConstraintsOnAnim = FBButton()
gButtonPlotJointsOnAnim = FBButton()

############# PROCEDURES ###############

def SelectNodeExclusive(nodeName):
    
    for comp in gSystem.Scene.Components:
        comp.Selected = False
        
    model = FBFindModelByLabelName(nodeName)
    if model:
        model.Selected = True

def SelectShaderExclusive(nodeName):
    
    for comp in gSystem.Scene.Components:
        comp.Selected = False
        
    for theshader in gSystem.Scene.Shaders:
        if theshader.Name == nodeName:            
            theshader.Selected = True
            theshader.HardSelect()

def FindRetargetingConstraint():
    
    for theconstraint in gSystem.Scene.Constraints:
        if (theconstraint.ClassName() == "ConstraintFacialRetargeting"):
            return theconstraint
    
    return None

def TurnOnFacialConstraints():
    for theitem in gSystem.Scene.Components:
        if isinstance(theitem, FBFolder) and theitem.Name == "RIG_TO_DELETE":
           
            for thechild in theitem.Items:
                thechild.Active = True
    
def TurnOffFacialConstraints():

    for theitem in gSystem.Scene.Components:
        if isinstance(theitem, FBFolder) and theitem.Name == "RIG_TO_DELETE":
           
            for thechild in theitem.Items:
                thechild.Active = False

def PlotHeadCorrectiveShapes():
    '''
    for comp in gSystem.Scene.Components:
        comp.Selected = False
    '''
    for theshader in gSystem.Scene.Shaders:
        if theshader.Name == "Projective HEAD":            

            dstcount = theshader.GetDstCount()
            
            for i in range(0, dstcount):
                model = theshader.GetDst(i)
                
                if isinstance(model, FBModel):
                    model.Selected = True
            '''
            thetake = gSystem.CurrentTake
            if thetake:
                timeStep = FBTime(0,0,0,1)
                thetake.PlotTakeOnSelected(timeStep)
            '''
            
def PlotFacialJoints():
    
    for comp in gSystem.Scene.Components:
        comp.Selected = False
    
    PlotHeadCorrectiveShapes()
    
    for thegroup in gSystem.Scene.Groups:
        if thegroup.Name == "JOINTS_TO_BAKE":
            thegroup.Select(True)
            
            thetake = gSystem.CurrentTake
            if thetake:
                timeStep = FBTime(0,0,0,1)
                thetake.PlotTakeOnSelected(timeStep)


def RemoveFacialConstraints():
    
    for theitem in gSystem.Scene.Components:
        if isinstance(theitem, FBFolder) and theitem.Name == "RIG_TO_DELETE":
            
            count = len(theitem.Items)
            count -= 1
            
            tobeRemoved = (count >= 0)
            
            while count >= 0:
                
                thechild = theitem.Items[count]
                thechild.FBDelete()
                del (thechild)
                
                count -= 1
                
            if tobeRemoved:
                theitem.FBDelete()
                
                
############# EVENTS ####################

def ButtonSelectRightEyeCameraEvent(control, event):
    SelectNodeExclusive("CameraForRightEye")

def ButtonSelectLeftEyeCameraEvent(control, event):
    SelectNodeExclusive("CameraForLeftEye")
    
def ButtonSelectNoseCameraEvent(control, event):
    SelectNodeExclusive("CameraForNose")
    
def ButtonSelectMouthCameraEvent(control, event):
    SelectNodeExclusive("CameraForMouth")

#

def ButtonSelectRightEyeShaderEvent(control, event):
    SelectShaderExclusive("Projective RIGHT EYE")

def ButtonSelectLeftEyeShaderEvent(control, event):
    SelectShaderExclusive("Projective LEFT EYE")
    
def ButtonSelectHeadShaderEvent(control, event):
    SelectShaderExclusive("Projective HEAD")
    
def ButtonSelectMouthShaderEvent(control, event):
    SelectShaderExclusive("Projective MOUTH")

##

def ButtonFocusOnConstraintEvent(control, event):
    theconstraint = FindRetargetingConstraint()    

    if theconstraint:
        theconstraint.HardSelect()

def ButtonBackingEvent(control, event):
    
    theconstraint = FindRetargetingConstraint()
    
    if theconstraint:
        prop = theconstraint.PropertyList.Find("Backing For Head Markers")
        if prop:
            for themodel in prop:
                themodel.Show = not themodel.Show

def ButtonGoToStanceEvent(control, event):

    CurrentChar = FBApplication().CurrentCharacter
    if CurrentChar!=None:
        CurrentChar.GoToStancePose( True, True );
        
        CurrentChar.InputType = FBCharacterInputType.kFBCharacterInputStance
        CurrentChar.ActiveInput = True
        
        if gButtonActivateConstraintsOnStance.State > 0:
            TurnOnFacialConstraints()
        
    else:
        FBMessageBox("Character","There is no current Character in the scene", "Ok")

    
def ButtonGoToAnimEvent(control, event):

    CurrentChar = FBApplication().CurrentCharacter

    if CurrentChar!=None:

        if gButtonPlotJointsOnAnim.State > 0:
            PlotFacialJoints()

        if gButtonDeactivateConstraintsOnAnim.State > 0:
            TurnOffFacialConstraints()

        CurrentChar.InputType = FBCharacterInputType.kFBCharacterInputMarkerSet
        CurrentChar.ActiveInput = True
        
    else:
        FBMessageBox("Character","There is no current Character in the scene", "Ok")


def ButtonCleanupEvent(control, event):
    
    RemoveFacialConstraints()

################ UI ########################

def PopulateCameraLayout(mainLyt):
    
    vlayout = FBVBoxLayout()
    
    comp = FBLabel()
    comp.Caption = "Select Cameras"
    vlayout.AddRelative(comp, 0.1)
    
    hlayout = FBHBoxLayout()
    
    comp = FBButton()
    comp.Caption = "Right Eye"
    comp.OnClick.Add(ButtonSelectRightEyeCameraEvent)
    hlayout.AddRelative(comp, 0.1)

    comp = FBButton()
    comp.Caption = "Left Eye"
    comp.OnClick.Add(ButtonSelectLeftEyeCameraEvent)
    hlayout.AddRelative(comp, 0.1)
    
    vlayout.AddRelative(hlayout, 0.1)

    comp = FBButton()
    comp.Caption = "Nose"
    comp.OnClick.Add(ButtonSelectNoseCameraEvent)
    vlayout.AddRelative(comp, 0.1)
    
    comp = FBButton()
    comp.Caption = "Mouth"
    comp.OnClick.Add(ButtonSelectMouthCameraEvent)
    vlayout.AddRelative(comp, 0.1)
    
    mainLyt.AddRelative(vlayout, 0.33)

def PopulateRetargetingLayout(mainLyt):
    vlayout = FBVBoxLayout()
    
    comp = FBLabel()
    comp.Caption = "Retargeting Controls:"
    vlayout.AddRelative(comp, 0.1)
    
    hlayout = FBHBoxLayout()
    
    comp = FBButton()
    comp.Caption = "Go To Stance"
    comp.OnClick.Add(ButtonGoToStanceEvent)
    hlayout.AddRelative(comp, 0.1)

    comp = FBButton()
    comp.Caption = "Go To Animation"
    comp.OnClick.Add(ButtonGoToAnimEvent)
    hlayout.AddRelative(comp, 0.1)
    
    vlayout.AddRelative(hlayout, 0.2)
    
    comp = FBLabel()
    comp.Caption = "Retargeting options:"
    vlayout.AddRelative(comp, 0.1)
    
    gButtonActivateConstraintsOnStance.Caption = "Activate constraints on stance"
    gButtonActivateConstraintsOnStance.Style = FBButtonStyle.kFBCheckbox
    gButtonActivateConstraintsOnStance.State = 1
    vlayout.AddRelative(gButtonActivateConstraintsOnStance, 0.1)
    
    gButtonDeactivateConstraintsOnAnim.Caption = "DeActivate constraints on anim"
    gButtonDeactivateConstraintsOnAnim.Style = FBButtonStyle.kFBCheckbox
    gButtonDeactivateConstraintsOnAnim.State = 1
    vlayout.AddRelative(gButtonDeactivateConstraintsOnAnim, 0.1)

    gButtonPlotJointsOnAnim.Caption = "Plot joints on anim"
    gButtonPlotJointsOnAnim.Style = FBButtonStyle.kFBCheckbox
    gButtonPlotJointsOnAnim.State = 1
    vlayout.AddRelative(gButtonPlotJointsOnAnim, 0.1)
    
    comp = FBButton()
    comp.Caption = "Focus on constraint"
    comp.OnClick.Add(ButtonFocusOnConstraintEvent)
    vlayout.AddRelative(comp, 0.1)
    
    comp = FBButton()
    comp.Caption = "Show / Hide Backing Surface"
    comp.OnClick.Add(ButtonBackingEvent)
    vlayout.AddRelative(comp, 0.1)
    
    comp = FBButton()
    comp.Caption = "Cleanup..."
    comp.OnClick.Add(ButtonCleanupEvent)
    vlayout.AddRelative(comp, 0.1)
    
    mainLyt.AddRelative(vlayout, 0.33)

def PopulateShadersLayout(mainLyt):
    
    vlayout = FBVBoxLayout()
    
    comp = FBLabel()
    comp.Caption = "Select Shaders"
    vlayout.AddRelative(comp, 0.1)
    
    hlayout = FBHBoxLayout()
    
    comp = FBButton()
    comp.Caption = "Right Eye"
    comp.OnClick.Add(ButtonSelectRightEyeShaderEvent)
    hlayout.AddRelative(comp, 0.1)

    comp = FBButton()
    comp.Caption = "Left Eye"
    comp.OnClick.Add(ButtonSelectLeftEyeShaderEvent)
    hlayout.AddRelative(comp, 0.1)
    
    vlayout.AddRelative(hlayout, 0.1)

    comp = FBButton()
    comp.Caption = "HEAD"
    comp.OnClick.Add(ButtonSelectHeadShaderEvent)
    vlayout.AddRelative(comp, 0.1)
    
    comp = FBButton()
    comp.Caption = "Mouth"
    comp.OnClick.Add(ButtonSelectMouthShaderEvent)
    vlayout.AddRelative(comp, 0.1)
    
    mainLyt.AddRelative(vlayout, 0.33)

def PopulateLayout(mainLyt):
    # Create Main region frame:
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    main = FBHBoxLayout()
    mainLyt.AddRegion("main","main", x, y, w, h)
    mainLyt.SetControl("main",main)

    PopulateCameraLayout(main)    
    PopulateRetargetingLayout(main)
    PopulateShadersLayout(main)
    
def CreateTool():    
    # Tool creation will serve as the hub for all other controls
    t = FBCreateUniqueTool("Facial RIG Helper Tool " + str(gVersion) )
    PopulateLayout(t)
    t.StartSizeX = 640
    t.StartSizeY = 300
    if gDevelopment:
        ShowTool(t)


CreateTool()





