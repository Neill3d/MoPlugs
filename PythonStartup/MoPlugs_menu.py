
##################################################################
##
## MoPLUGS main menu
##
## Author Sergey Solohin (Neill3d) 2014-2016
##    e-mail to: s@neill3d.com
##        www.neill3d.com
##
## Github repo - https://github.com/Neill3d/MoPlugs
## Licensed under BSD 3-clause
##  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
##
##################################################################

from pyfbsdk import *
import webbrowser
import os

AboutInfo = "Version from 04.03.2016"

gApp = FBApplication()
gSystem = FBSystem()

gScriptAssignUVSet = 0
gScriptSelChAndCR = 1
gScriptCheckFacialDataVideoPlane = 3
gScriptCopyBlendShapeAnimation = 4
gScriptSaveLocalPositions = 5
gScriptExtractTrajectoryPath = 6
gScriptDuplicateAlongACurveSetup = 7
gScriptDuplicateAlongACurveRotate90 = 70
gScriptDuplicateAlongACurveSelectNulls = 71
gScriptDuplicateAlongACurveSelectModels = 72
gScriptDuplicateAlongACurveDeform = 73
gScriptDuplicateAlongACurveHardPlot = 74
gScriptFixInstancesGeometry = 75
gScriptGroupedWithNull = 8
gScriptInsertParent = 9
gScriptFacialRigConnectJoints = 10
gScriptFacialRigCreateVideoPlane = 11
gScriptFacialRigHardPlot = 12
gScriptFacialRigOpticalPostProcessing = 13
gScriptFacialRigSetupConstraints = 14
gScriptFromLayeredToSingleTexture = 15
gScriptRefreshModelsTextures = 16
gScriptRefreshTextures = 17
gScriptLightsHardTurnOnOff = 18
gScriptLightsTurnOnOff = 19
gScriptMakeCircle = 20

gScripts = { 
    gScriptAssignUVSet : "Assign Lightmap UVSet",
    gScriptSelChAndCR : "Select Char and ControlRig from Models",
    gScriptCheckFacialDataVideoPlane : "CHECK FacialData VideoPlane",
    gScriptCopyBlendShapeAnimation : "Copy BlendShape Animation",
    gScriptSaveLocalPositions : "Save Local Positions",
    gScriptExtractTrajectoryPath : "Extract Trajectory Path",
    gScriptDuplicateAlongACurveSetup : "Duplicate Along a Curve SETUP",
    gScriptDuplicateAlongACurveRotate90 : "Duplicate Along a Curve Rotate By 90",
    gScriptDuplicateAlongACurveSelectNulls : "Duplicate Along a Curve Select Nulls",
    gScriptDuplicateAlongACurveSelectModels : "Duplicate Along a Curve Select Models",
    gScriptDuplicateAlongACurveDeform : "Duplicate Along A Curve Deform Meshes",
    gScriptDuplicateAlongACurveHardPlot : "Models Animation Hard Plot",
    gScriptFixInstancesGeometry : "Fix Instances Geometry",
    gScriptGroupedWithNull : "Grouped With Null",
    gScriptInsertParent : "Insert Parent",
    gScriptFacialRigConnectJoints : "Facial RIG - Connect Joints",
    gScriptFacialRigCreateVideoPlane : "Facial RIG - Create VideoPlane",
    gScriptFacialRigHardPlot : "Facial RIG - Hard Plot",
    gScriptFacialRigOpticalPostProcessing : "Facial RIG - Optical Post Processing",
    gScriptFacialRigSetupConstraints : "Facial RIG - Setup Constraints",
    gScriptFromLayeredToSingleTexture : "From Layered to Single Texture",
    gScriptRefreshModelsTextures : "Refresh Selected Models Textures",
    gScriptRefreshTextures : "Refresh Textures",
    gScriptLightsHardTurnOnOff : "Lights - Hard Turn On/Off",
    gScriptLightsTurnOnOff : "Lights - Turn On/Off",
    gScriptMakeCircle : "Make A Circle" }


def TryToExecuteScript(filename):
    
    paths = gSystem.GetPythonStartupPath()
   
    for path in paths:

        items = path.split("\\")
      
        scriptpath = ""
        if len(items) > 0:
            
            scriptpath = items[0]
            for i in range(1, len(items)-1):
                scriptpath = scriptpath + "\\" + items[i]
            
            scriptpath = scriptpath + "\\PythonActions\\" + filename 
            print scriptpath
            if os.path.isfile( scriptpath ):
                gApp.ExecuteScript(scriptpath)
                return
                
    FBMessageBox( "MoScripts", "The script is missing", "Ok" )

def MenuScriptsItemActivate(control, event):
    print "editing item"
    
    if event.Id == gScriptAssignUVSet:
        TryToExecuteScript( "AssignUVSet.py" )
    elif event.Id == gScriptSelChAndCR:
        TryToExecuteScript( "SelCharAndCR_ByModelSelection.py" )
    elif event.Id == gScriptCheckFacialDataVideoPlane:
        TryToExecuteScript( "CHECK_FacialData_VideoPlane.py" )
    elif event.Id == gScriptCopyBlendShapeAnimation:
        TryToExecuteScript( "CopyBlendShapeAnimation.py" )
    elif event.Id == gScriptSaveLocalPositions:
        TryToExecuteScript( "SaveLocalPositions.py" )
    elif event.Id == gScriptExtractTrajectoryPath:
        TryToExecuteScript( "ExtractTrajectoryPath.py" )
    elif event.Id == gScriptDuplicateAlongACurveSetup:
        TryToExecuteScript( "DuplicateAlongACurve_SETUP.py" )
    elif event.Id == gScriptDuplicateAlongACurveRotate90:
        TryToExecuteScript( "DuplicateAlongACurve_RotateBy90.py" )
    elif event.Id == gScriptDuplicateAlongACurveSelectNulls:
        TryToExecuteScript( "DuplicateAlongACurve_SelectNulls.py" ) 
    elif event.Id == gScriptDuplicateAlongACurveSelectModels:
        TryToExecuteScript( "DuplicateAlongACurve_SelectModels.py" ) 
    elif event.Id == gScriptDuplicateAlongACurveDeform:
        TryToExecuteScript( "DuplicateAlongACurve_DeformMesh.py" )        
    elif event.Id == gScriptDuplicateAlongACurveHardPlot:
        TryToExecuteScript( "DuplicateAlongACurve_HardPlot.py" )
    elif event.Id == gScriptFixInstancesGeometry:
        TryToExecuteScript( "FixInstancesGeometry.py" )
    elif event.Id == gScriptGroupedWithNull:
        TryToExecuteScript( "GroupedWithNull.py" )
    elif event.Id == gScriptInsertParent:
        TryToExecuteScript( "InsertParent.py" )
    elif event.Id == gScriptFacialRigConnectJoints:
        TryToExecuteScript( "FacialRIG_connectJoints.py" )
    elif event.Id == gScriptFacialRigCreateVideoPlane:
        TryToExecuteScript( "FacialRIG_CreateVideoPlane.py" )
    elif event.Id == gScriptFacialRigHardPlot:
        TryToExecuteScript( "FacialRIG_HardPlot.py" )
    elif event.Id == gScriptFacialRigOpticalPostProcessing:
        TryToExecuteScript( "FacialRIG_OpticalPostProcessing.py" )
    elif event.Id == gScriptFacialRigSetupConstraints:
        TryToExecuteScript( "FacialRIG_SetupConstraints.py" )
    elif event.Id == gScriptFromLayeredToSingleTexture:
        TryToExecuteScript( "FromLayeredToSingleTexture.py" )
    elif event.Id == gScriptRefreshModelsTextures:
        TryToExecuteScript( "RefreshModelsTextures.py" )
    elif event.Id == gScriptRefreshTextures:
        TryToExecuteScript( "RefreshTextures.py" )
    elif event.Id == gScriptLightsHardTurnOnOff:
        TryToExecuteScript( "Lights_HardTurnOnOff.py" )
    elif event.Id == gScriptLightsTurnOnOff:
        TryToExecuteScript( "Lights_TurnOnOff.py" )        
    elif event.Id == gScriptMakeCircle:
        TryToExecuteScript( "MakeCircle.py" )   
                
def MenuEditingItemActivate(control, event):
    print "editing item"
    
def MenuToolsItemActivate(control, event):
    if event.Id == 0:
        FBPopNormalTool( "BlendShape Manager" )
    elif event.Id == 1:
        FBPopNormalTool( "Sculpt Brush Tool" )
    elif event.Id == 3:
        FBPopNormalTool( "Composite Master" )
    elif event.Id == 4:
        FBPopNormalTool( "Dynamic Masks" )
    elif event.Id == 5:
        FBPopNormalTool( "Render Layers" )
    elif event.Id == 6:
        FBPopNormalTool( "Render Options" )
    elif event.Id == 8:
        FBPopNormalTool( "Textures Browser" )
    elif event.Id == 20:
        FBPopNormalTool( "MoPlugs Settings Tool" )
    #
    
def MenuHelpItemActivate(control, event):
    
    if event.Id == 1:
        webbrowser.open('http://neill3d.com')
    elif event.Id == 2:
        webbrowser.open('https://drive.google.com/folderview?id=0B83XZ3TC_S6PT0ZycFlabW9aWmM&usp=sharing')
    elif event.Id == 3:
        FBMessageBox( "MoPlugs Project", AboutInfo, "Ok" )
    #
    
def MenuInit():
    menuMgr = FBMenuManager()
    
    #
    ## MoHELP Menu
    #
    menu = menuMgr.GetMenu("MoHELP")
    if not menu:
        menu = menuMgr.InsertAfter(None,"Window", "MoHELP").Menu
    menu.OnMenuActivate.Add(MenuHelpItemActivate)

    menu.InsertLast( "Visit MoPlugs store", 0 );
    menu.InsertLast( "Visit Neill3d blog", 1 );
    menu.InsertLast( "", 100 );
    menu.InsertLast( "Documentation", 2 );
    menu.InsertLast( "", 200 );
    menu.InsertLast( "&About", 3 );
    
    #
    ## MoTOOLS Menu
    #
    
    menu = menuMgr.GetMenu("MoTOOLS")
    if not menu:
        menu = menuMgr.InsertAfter(None,"Window", "MoTOOLS").Menu
    menu.OnMenuActivate.Add(MenuToolsItemActivate)
    
    menu.InsertLast( "Settings", 20 );
    menu.InsertLast( "", 21 );
    menu.InsertLast( "BlendShape Tool", 0 );
    menu.InsertLast( "&Sculpt brush", 1 );
    menu.InsertLast( "", 2 );
    menu.InsertLast( "&Composite Master", 3 );
    menu.InsertLast( "&Dynamic Mask Tool", 4 );
    menu.InsertLast( "&Render Layers Tool", 5 );
    menu.InsertLast( "Render &Layers Options", 6 );
    menu.InsertLast( "", 7 );
    menu.InsertLast( "&Textures Browser", 8 );
    
    #
    ## MoEditing Menu
    #
    
    menu = menuMgr.GetMenu("MoEDITING")
    if not menu:
        menu = menuMgr.InsertAfter(None,"Window", "MoEDITING").Menu
    menu.OnMenuActivate.Add(MenuEditingItemActivate)
    
    menu.InsertLast( "Make Snapshot", 0 )
    menu.InsertLast( "Combine Models", 1)
    menu.InsertLast( "", 100 )
    menu.InsertLast( "Center Pivot", 2)
    menu.InsertLast( "", 200 )
    menu.InsertLast( "ReCompute Normals", 3)
    menu.InsertLast( "Invert Normals", 4)
    menu.InsertLast( "", 300 )
    menu.InsertLast( "Save Blendshapes As...", 5 )
    menu.InsertLast( "Load Blendshapes...", 6)
    menu.InsertLast( "", 400 )
    menu.InsertLast( "Optimize skin weights", 7 )
    
    #
    ## MoEditing Menu
    #
    
    menu = menuMgr.GetMenu("MoSCRIPTS")
    if not menu:
        menu = menuMgr.InsertAfter(None,"Window", "MoSCRIPTS").Menu
    menu.OnMenuActivate.Add(MenuScriptsItemActivate)
    
    menu.InsertLast( gScripts[gScriptAssignUVSet], gScriptAssignUVSet )
    menu.InsertLast( gScripts[gScriptSelChAndCR], gScriptSelChAndCR )
    menu.InsertLast( "", 100 )
    menu.InsertLast( gScripts[gScriptCheckFacialDataVideoPlane], gScriptCheckFacialDataVideoPlane)

    menu.InsertLast( "", 200 )
    menu.InsertLast( gScripts[gScriptCopyBlendShapeAnimation], gScriptCopyBlendShapeAnimation)
    menu.InsertLast( gScripts[gScriptSaveLocalPositions], gScriptSaveLocalPositions )
    menu.InsertLast( gScripts[gScriptExtractTrajectoryPath], gScriptExtractTrajectoryPath)
    menu.InsertLast( "", 300 )
    menu.InsertLast( gScripts[gScriptDuplicateAlongACurveSetup], gScriptDuplicateAlongACurveSetup)
    menu.InsertLast( gScripts[gScriptDuplicateAlongACurveRotate90], gScriptDuplicateAlongACurveRotate90)
    menu.InsertLast( gScripts[gScriptDuplicateAlongACurveSelectNulls], gScriptDuplicateAlongACurveSelectNulls)
    menu.InsertLast( gScripts[gScriptDuplicateAlongACurveSelectModels], gScriptDuplicateAlongACurveSelectModels)
    menu.InsertLast( gScripts[gScriptDuplicateAlongACurveDeform], gScriptDuplicateAlongACurveDeform)
    menu.InsertLast( gScripts[gScriptDuplicateAlongACurveHardPlot], gScriptDuplicateAlongACurveHardPlot)
    menu.InsertLast( "", 350 )
    menu.InsertLast( gScripts[gScriptFixInstancesGeometry], gScriptFixInstancesGeometry)
    menu.InsertLast( gScripts[gScriptGroupedWithNull], gScriptGroupedWithNull )    
    menu.InsertLast( gScripts[gScriptInsertParent], gScriptInsertParent )
    menu.InsertLast( "", 400 )
    menu.InsertLast( gScripts[gScriptFacialRigConnectJoints], gScriptFacialRigConnectJoints )
    menu.InsertLast( gScripts[gScriptFacialRigCreateVideoPlane], gScriptFacialRigCreateVideoPlane)
    menu.InsertLast( gScripts[gScriptFacialRigHardPlot], gScriptFacialRigHardPlot)
    menu.InsertLast( gScripts[gScriptFacialRigOpticalPostProcessing], gScriptFacialRigOpticalPostProcessing)
    menu.InsertLast( gScripts[gScriptFacialRigSetupConstraints], gScriptFacialRigSetupConstraints)
    menu.InsertLast( "", 500 )
    menu.InsertLast( gScripts[gScriptFromLayeredToSingleTexture], gScriptFromLayeredToSingleTexture )
    menu.InsertLast( gScripts[gScriptRefreshModelsTextures], gScriptRefreshModelsTextures )
    menu.InsertLast( gScripts[gScriptRefreshTextures], gScriptRefreshTextures )
    menu.InsertLast( "", 600 )
    menu.InsertLast( gScripts[gScriptLightsHardTurnOnOff], gScriptLightsHardTurnOnOff )
    menu.InsertLast( gScripts[gScriptLightsTurnOnOff], gScriptLightsTurnOnOff )

    menu.InsertLast( "", 700 )
    menu.InsertLast( gScripts[gScriptMakeCircle], gScriptMakeCircle )

    #
    

    
MenuInit()
