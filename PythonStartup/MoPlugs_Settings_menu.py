
##################################################################
##
## MoPLUGS Global Settings main menu
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

AboutInfo = "Version from 15.11.2016"

gApp = FBApplication()
gSystem = FBSystem()
gSettings = None

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
                print "executed!"
                return
                
    FBMessageBox( "MoScripts", "The script is missing", "Ok" )

def QueryGlobalSettings():
    
    global gSettings
    
    lScene = gSystem.Scene
    for obj in lScene.UserObjects:
        if obj.ClassName() == 'GraphicsGlobalSettings':
            gSettings = obj
            break

def MenuDepthItemActivate(control, event):
    
    QueryGlobalSettings()
    if gSettings == None:
        return
        
    gSettings.PropertyList.Find( 'Display Depth' ).Data  = event.Id - 1
    #    
    
def MenuSamplingItemActivate(control, event):

    QueryGlobalSettings()
    if gSettings == None:
        return
        
    gSettings.PropertyList.Find( 'Display Quality' ).Data  = event.Id - 1
    #
    
def MenuStageItemActivate(control, event):

    QueryGlobalSettings()
    if gSettings == None:
        return
    
    if event.Id == 1:
        gSettings.PropertyList.Find('Process DynamicMasks').Data = False
        gSettings.PropertyList.Find('Process Compositions').Data = False
        gSettings.PropertyList.Find('Process CubeMap Rendering').Data = False
        gSettings.PropertyList.Find('Process Camera Rendering').Data = False
        gSettings.PropertyList.Find('Process Shadows').Data = False
    elif event.Id == 2:
        gSettings.PropertyList.Find('Process DynamicMasks').Data = True
        gSettings.PropertyList.Find('Process Compositions').Data = True
        gSettings.PropertyList.Find('Process CubeMap Rendering').Data = True
        gSettings.PropertyList.Find('Process Camera Rendering').Data = True
        gSettings.PropertyList.Find('Process Shadows').Data = True
        
    elif event.Id == 11:
        gSettings.PropertyList.Find('Process Shadows').Data = False
    elif event.Id == 12:
        gSettings.PropertyList.Find('Process CubeMap Rendering').Data = False
    elif event.Id == 13:
        gSettings.PropertyList.Find('Process Camera Rendering').Data = False
    elif event.Id == 14:
        gSettings.PropertyList.Find('Process DynamicMasks').Data = False
    elif event.Id == 15:
        gSettings.PropertyList.Find('Process Compositions').Data = False
        
    elif event.Id == 21:
        gSettings.PropertyList.Find('Process Shadows').Data = True
    elif event.Id == 22:
        gSettings.PropertyList.Find('Process CubeMap Rendering').Data = True
    elif event.Id == 23:
        gSettings.PropertyList.Find('Process Camera Rendering').Data = True
    elif event.Id == 24:
        gSettings.PropertyList.Find('Process DynamicMasks').Data = True
    elif event.Id == 25:
        gSettings.PropertyList.Find('Process Compositions').Data = True
    #
    
def MenuResolutionItemActivate(control, event):

    QueryGlobalSettings()
    if gSettings == None:
        return
        
    gSettings.PropertyList.Find( 'Resolution Preset' ).Data  = event.Id - 1
    #
    
def MenuPreviewItemActivate(control, event):

    QueryGlobalSettings()
    if gSettings == None:
        return
    
    if event.Id == 1:
        gSettings.PropertyList.Find('Preview As SubView').Data = False
    elif event.Id == 2:
        gSettings.PropertyList.Find('Preview As SubView').Data = True
    elif event.Id == 3:
        gSettings.PropertyList.Find('Preview On Holding Alt').Data = True
    elif event.Id == 4:
        gSettings.PropertyList.Find('Preview On Play').Data = True
    elif event.Id == 5:
        gSettings.PropertyList.Find('Preview On Holding Alt').Data = False
    elif event.Id == 6:
        gSettings.PropertyList.Find('Preview On Play').Data = False

    #
    
def MenuSettingsItemActivate(control, event):
    
    QueryGlobalSettings()

    if gSettings == None:
        return
    
    if event.Id == 1:
        gSettings.Selected = True
    elif event.Id == 2:
        gSettings.PropertyList.Find('Use Global Resolution').Data = True
    elif event.Id == 3:
        gSettings.PropertyList.Find('Use Global Resolution').Data = False
    elif event.Id == 4:
        gSettings.PropertyList.Find('Show Comp As Preview').Data = False
    elif event.Id == 5:
        gSettings.PropertyList.Find('Show Comp As Preview').Data = True
    elif event.Id == 6:
        webbrowser.open('https://drive.google.com/folderview?id=0B83XZ3TC_S6PT0ZycFlabW9aWmM&usp=sharing')
    elif event.Id == 7:
        FBMessageBox( "MoPlugs Project", AboutInfo, "Ok" )
    
    #
    
def MenuInit():
    
    menuMgr = FBMenuManager()
    
    #
    ## MoSETTINGS Menu
    #
    menu = menuMgr.GetMenu("MoSETTINGS")
    if menu != None:
        menu.FBDelete()
        menu = None
    
    if not menu:
        menu = menuMgr.InsertAfter(None,"Window", "MoSETTINGS").Menu
    
    depthMenu = FBGenericMenu()
    depthMenu.InsertLast( "Linear Depth", 1 )
    depthMenu.InsertLast( "Logarithmic Depth", 2 )
    depthMenu.InsertLast( "Reverse Depth Range (NV)", 3)
    depthMenu.OnMenuActivate.Add(MenuDepthItemActivate)
    
    aaMenu = FBGenericMenu()
    aaMenu.InsertLast( "No Sampling", 1 )
    aaMenu.InsertLast( "Multi Sampling AntiAliasing", 2 )
    aaMenu.InsertLast( "Coverage Sampling AntiAliasing (NV)", 3)
    aaMenu.OnMenuActivate.Add(MenuSamplingItemActivate)
    
    previewMenu = FBGenericMenu()
    previewMenu.InsertLast( "Show Preview As Fullscreen", 1 )
    previewMenu.InsertLast( "Show Preview As SubView", 2 )
    previewMenu.InsertLast( "", 100 )
    previewMenu.InsertLast( "Use Preview On Holding ALT", 3 )
    previewMenu.InsertLast( "Use Preview On Play", 4 )
    previewMenu.InsertLast( "", 101 )
    previewMenu.InsertLast( "Don't Use Preview On Holding ALT", 5 )
    previewMenu.InsertLast( "Don't Use Preview On Play", 6 )
    previewMenu.InsertLast( "", 102 )
    previewMenu.InsertLast( "Disable Preview On Rendering", 7 )
    previewMenu.OnMenuActivate.Add(MenuPreviewItemActivate)
    
    stageMenu = FBGenericMenu()
    stageMenu.InsertLast( "Turn Off All Stages", 1)
    stageMenu.InsertLast( "Turn On All Stages", 2)
    stageMenu.InsertLast( "", 100 )
    stageMenu.InsertLast( "Turn Off Shadows", 11 )
    stageMenu.InsertLast( "Turn Off CubeMaps Rendering", 12 )
    stageMenu.InsertLast( "Turn Off Camera Rendering", 13 )
    stageMenu.InsertLast( "Turn Off Dynamic Masks", 14 )
    stageMenu.InsertLast( "Turn Off Compositions", 15 )
    stageMenu.InsertLast( "", 101 )
    stageMenu.InsertLast( "Turn On Shadows", 21 )
    stageMenu.InsertLast( "Turn On CubeMaps Rendering", 22 )
    stageMenu.InsertLast( "Turn On Camera Rendering", 23 )
    stageMenu.InsertLast( "Turn On Dynamic Masks", 24 )
    stageMenu.InsertLast( "Turn On Compositions", 25 )
    stageMenu.OnMenuActivate.Add(MenuStageItemActivate)
    
    resolutionMenu = FBGenericMenu()
    resolutionMenu.InsertLast( "Custom Resolution", 1 )
    resolutionMenu.InsertLast( "Full HD (1920 x 1080)", 2 )
    resolutionMenu.InsertLast( "2K with 2.35 aspect (2274 x 967)", 3 )
    resolutionMenu.OnMenuActivate.Add(MenuResolutionItemActivate)
    
    # add items to main settings menu
    
    menu.InsertLast( "Select Global Settings", 1 )
    spaceItem = menu.InsertLast( "", 100 )
    
    spaceItem2 = menu.InsertLast( "", 101 )
    
    menu.InsertLast( "", 102 )
    menu.InsertLast( "Use Global Resolution", 2 )
    globalResItem = menu.InsertLast( "Don't Use Global Resolution", 3 )
   
    menu.InsertLast( "", 103 );
    menu.InsertLast( "Composition Full Mode", 4 )
    previewItem = menu.InsertLast( "Composition Preview Mode", 5 )
    
    menu.InsertLast( "", 104 );
    menu.InsertLast( "Documentation About Settings", 6 );
    menu.InsertLast( "&About MoSETTINGS Script", 7 );
    
    menu.InsertAfter(spaceItem2, "Processing Stages", 69, stageMenu )
    depthItem = menu.InsertAfter(spaceItem, "Depth Quality", 67, depthMenu )
    menu.InsertAfter(depthItem, "AntiAliasing Quality", 68, aaMenu )
    menu.InsertAfter(globalResItem, "Global Resolution Preset", 71, resolutionMenu )
    menu.InsertAfter(previewItem, "Composition Preview Options", 72, previewMenu )
    
    menu.OnMenuActivate.Add(MenuSettingsItemActivate)

    
MenuInit()
