
#
# Script description: Prepare facial starter scene
#
#  Version for MotionBuilder 2014
#   Special For Cam Christiansen
#
# Sergey Solohin (Neill3d) 2016
#   e-mail to: s@neill3d.com
#  www.neill3d.com
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

import os
import sys
import re
from pyfbsdk import *
from pyfbsdk_additions import *
import string

gDevelopment = True
gScriptVersion = 0.53

gApp = FBApplication ()
gSystem = FBSystem()

gButtonChooseList = FBButton()
gButtonChooseCustom = FBButton()

gListScenes = FBList()

gEditBase = FBEdit()

def EventButtonChooseBase(control, event):
    
    dialog = FBFilePopup()
    dialog.Style = FBFilePopupStyle.kFBFilePopupOpen
    dialog.Filter = "*.fbx"
    
    if dialog.Execute():
        gEditBase.Text = dialog.FullFilename

def EventButtonUseCurrentScene(control, event):
    
    fileName = gApp.FBXFileName
    gEditBase.Text = fileName

def EventButtonChooseList(control, event):
    gButtonChooseList.State = 1
    gButtonChooseCustom.State = 0
    
def EventButtonChooseCustom(control, event):
    gButtonChooseList.State = 0
    gButtonChooseCustom.State = 1

def FindVideoTrack(rootFolder):
    
    for folder in rootFolder.Childs:
        return FindVideoTrack(folder)
        
    for track in rootFolder.Tracks:
        if track.Type == FBStoryTrackType.kFBStoryTrackVideo and track.Name.find("Facial Video") >= 0:
            return track
            
    return None

def AssignVideoClip(videoPlane):
                    
    if videoPlane:

        clip = None
        if len(videoPlane.Materials) > 0:
            tex = videoPlane.Materials[0].GetTexture()
            
            if tex:
                clip = tex.Video
    
        if clip:
            print clip.Filename
            
            lstory = FBStory()
            track = FindVideoTrack(lstory.RootFolder)

            if not track:
                track = FBStoryTrack(FBStoryTrackType.kFBStoryTrackVideo)
                track.Name = "Facial Video Track"

            if track:
                
                # import to remove all previous clips
                trackClips = []
                for theTrackClip in track.Clips:
                    trackClips.append(theTrackClip)
                map( FBComponent.FBDelete, trackClips )
                
                newTrackClip = FBStoryClip(clip.Filename, track, FBTime(0) )

def GetBaseSceneFileName():
    
    if gButtonChooseList.State > 0 and len(gListScenes.Items) > 0:
        return gListScenes.Items[gListScenes.ItemIndex]
    else:
        return gEditBase.Text
        
    return ""

def EventButtonDoIt(control, event):
    
    #
    baseFileName = GetBaseSceneFileName()
    if not os.path.isfile(baseFileName):
        FBMessageBox( "Prepare Starter Scene", "Failed to locate a base scene file!", "Ok" )
        return
    
    # enter an optical clip
    
    dialog = FBFilePopup()
    dialog.Style = FBFilePopupStyle.kFBFilePopupOpen
    dialog.Filter = "*.fbx"
    
    strOpticalClipPath = ""
    
    
    if dialog.Execute():
        strOpticalClipPath = dialog.FullFilename
    
        #
        gApp.FileNew()
        gApp.FileOpen(baseFileName)
    
        #
        defRootPos = FBVector3d(21.09, 165.36, 0.0)
        opticalRoot = FBFindModelByLabelName("DATA:FaceFeaturesOptical")
        if opticalRoot:        
            defRootPos = opticalRoot.Translation.Data
    
        # merge clip
        
        options = FBFbxOptions(True)
        options.NamespaceList = "DATA"
        options.SetAll( FBElementAction.kFBElementActionMerge, True )
        gApp.FileMerge(strOpticalClipPath, False, options)
    
        # move optical root
        
        opticalRoot = FBFindModelByLabelName("DATA:FaceFeaturesOptical")
        if opticalRoot:
            opticalRoot.Scaling = FBVector3d(0.25, 0.25, 0.25)
            opticalRoot.Translation = defRootPos
        
            # assign a new clip to the video story track
            
            videoPlane = None
            
            for child in opticalRoot.Children:
                
                if child.ClassName() == "FBModel":
                    videoPlane = child
                    break
            print videoPlane
            AssignVideoClip(videoPlane)

def LoadBaseScenes():
    #gListScenes.Items.SetString( "David Hare~Driver~Tracy" )
    
    configName = "FacialRIG_PrepStarter.config.txt"
    
    paths = gSystem.GetPythonStartupPath()
    for path in paths:
        fname = path + "\\" + configName

        if os.path.isfile( fname ):
            f = open(fname)
            if f:
                print fname
                content = f.readlines()
                
                for line in content:
                    if line.find('#') < 0:
                        filename = line.rstrip()
                        gListScenes.Items.append(filename)
                
                f.close()
                return
    
def OnRevealClick(control, event):
    print "reveal in browser"
    
    filename = ""
    
    if len(gListScenes.Items) > 0 and gListScenes.ItemIndex >= 0:
        filename = gListScenes.Items[gListScenes.ItemIndex]
   
    if filename != "":
        import subprocess
        subprocess.Popen(r'explorer /select,'+filename)
    
def OnLoadClick(control, event):
    print "open scene in mobu"

    filename = ""
    
    if len(gListScenes.Items) > 0 and gListScenes.ItemIndex >= 0:
        filename = gListScenes.Items[gListScenes.ItemIndex]
    print filename
    if filename != "":  
        gApp.FileNew()
        gApp.FileOpen(filename)
    
def PopulateLayout(mainLyt):
    
    # Create Main region frame:
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    main = FBVBoxLayout()
    mainLyt.AddRegion("main","main", x, y, w, h)
    mainLyt.SetControl("main",main)

    hlayout = FBHBoxLayout()

    gButtonChooseList.Style = FBButtonStyle.kFBRadioButton
    gButtonChooseList.State = 1
    gButtonChooseList.Caption = "Choose base scene from the preset list:"
    gButtonChooseList.OnClick.Add( EventButtonChooseList )

    hlayout.AddRelative( gButtonChooseList, 0.8 )
    
    b = FBButton()
    b.Caption = "Reveal in explorer"
    b.OnClick.Add( OnRevealClick )
    hlayout.Add( b, 120 )
    
    b = FBButton()
    b.Caption = "Load scene"
    b.OnClick.Add( OnLoadClick )
    hlayout.Add( b, 120 )

    main.Add( hlayout, 25 )

    gListScenes.Style = FBListStyle.kFBDropDownList
    LoadBaseScenes()

    main.Add( gListScenes, 25 )

    gButtonChooseCustom.Style = FBButtonStyle.kFBRadioButton
    gButtonChooseCustom.State = 0
    gButtonChooseCustom.Caption = "Use a custom character base scene:"
    gButtonChooseCustom.OnClick.Add( EventButtonChooseCustom )

    main.Add( gButtonChooseCustom, 25 )

    hlayout = FBHBoxLayout()

    gEditBase.Text = "base.fbx"
    hlayout.AddRelative( gEditBase, 0.9 )
    
    b = FBButton()
    b.Caption = "<->"
    b.OnClick.Add(EventButtonUseCurrentScene)
    hlayout.Add(b, 25)
    
    b = FBButton()
    b.Caption = "..."
    b.OnClick.Add(EventButtonChooseBase)
    hlayout.Add(b, 25)
    
    main.Add( hlayout, 25 )

    #
  
    b = FBLayout()
    main.Add(b, 35)    

    b = FBLabel()
    b.Caption = "* Press button and enter optical facial clip"
    main.Add( b, 25 )

    b = FBLabel()
    b.Caption = "   Story video clip will be added"
    main.Add( b, 25 )
    
    b = FBButton()
    b.Caption = "Do It!"
    b.OnClick.Add( EventButtonDoIt )
    
    main.Add(b, 30)
    
def CreateTool():    
    # Tool creation will serve as the hub for all other controls
    t = FBCreateUniqueTool("Facial RIG Prepare Starter v." + str(gScriptVersion))
    PopulateLayout(t)
    t.StartSizeX = 1024
    t.StartSizeY = 300
    if gDevelopment:
        ShowTool(t)


CreateTool()





