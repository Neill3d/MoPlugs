
#
# Script description:
# Specify files to be rendered.
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
# Author Sergey Solohin (Neill3d) 2013
#  e-mail to: s@neill3d.com
#   www.neill3d.com

import os
import re
from pyfbsdk import *
from pyfbsdk_additions import *
import string

gDevelopment = True

gSystem = FBSystem()
gApp = FBApplication()

listFiles = FBList()
buttonRender = FBButton()
buttonAdd = FBButton()
buttonAddDir = FBButton()
buttonRemove = FBButton()
buttonClear = FBButton()
buttonAbout = FBButton()

def EventButtonAdd(control, event):

    lFp = FBFilePopup()
    lFp.Caption = "Choose an FBX file to add"
    # Set the default path. Good for a PC only... will have to be different for Mac.
    lFp.Path = r"C:\\"
    lFp.Filter = "*.fbx"
    
    # Get the GUI to show.
    lRes = lFp.Execute()
    
    # If you select a folder, show its name, otherwise indicate that the selection was canceled.
    if not lRes:
        FBMessageBox( "Warning:", "Operation canceled, cannot continue!", "OK" )
    
    else:
        # do something
        listFiles.Items.append(lFp.FullFilename)

def EventButtonAddDir(control, event):
   
    # Creating all the Constructors
    lFp = FBFolderPopup()
    
    # Select directory you would like to load your FBX files in from 
    # Create the popup and set necessary initial values.
    lFp.Caption = "Scene Files: Select the folder containing the files you would like to render"
    
    # Set the default path. Good for a PC only... will have to be different for Mac.
    lFp.Path = r"C:\\"
    
    # Get the GUI to show.
    lRes = lFp.Execute()
    
    # If you select a folder, show its name, otherwise indicate that the selection was canceled.
    if not lRes:
        FBMessageBox( "Warning:", "Selection canceled, cannot continue!", "OK" )
    
    else:
        # Getting the names of the files in your previously selected folder
        # Using os to get the file names from the specified folder (above) and storing names of files in a list
        fileList = os.listdir(lFp.Path)
        print "fileList", fileList
        # Setting the regular expression to only look for .fbx extenstion
        fbxRE = re.compile('^\w+.fbx$', re.I)
        
        # Removing any files that do not have an .fbx extenstion
        for fname in fileList:
            mo = fbxRE.search(fname)
            if fname.find('.fbx') >= 0:
                listFiles.Items.append(lFp.Path + '\\' + fname)
    
def EventButtonRemove(control, event):
    
    newItems = []
    
    for i in range(0, len(listFiles.Items)):
        if listFiles.IsSelected(i) == False: newItems.append(listFiles.Items[i])
    
    listFiles.Items.removeAll()
    for item in newItems:
        listFiles.Items.append(item)
    
    del(newItems)
    
def EventButtonClear(control, event):       
    listFiles.Items.removeAll()

def EventButtonAbout(control, event):
    FBMessageBox( "About Batch Render Tool", "Author Sergey Solohin (Neill3d) 2013\n\te-mail to: s@neill3d.com\n\t\twww.neill3d.com", "OK" )

def EventButtonBatch(control, event):
    # Creating all the Constructors
    lScene = gSystem.Scene

    id = FBMessageBox( "Batch Render Tool", "Do you want to start batch?", "Ok", "Cancel" )
    if id == 1:    
           
        # Exporting items in the file one at a time
        for fname in listFiles.Items:
            
            # Opening the file in MotionBuilder, this replaces the current scene
            gApp.FileOpen(fname)
                    
            #
            # Do the render
            #
            # Get the default rendering options, which are saved in the FBX file.
                 
            #lRenderFileFormat = '.avi'
            lOptions = FBVideoGrabber().GetOptions()
                                        
            # Set VideoCodec Option:
            VideoManager = FBVideoCodecManager()
            VideoManager.VideoCodecMode = FBVideoCodecMode.FBVideoCodecStored
            
            #codeclist = FBStringList()
            #codellist = VideoManager.GetCodecIdList('AVI');
            #for item in codeclist:
            #    if item.find('XVID') >= 0: VideoManager.SetDefaultCodec('AVI', item)
            
            # Set the name of the rendered file.
            #lDstFileName = "c:\\" + newFileName
            #base = os.path.splitext(lDstFileName)[0]
            #lDstFileName = base + ".avi"
            #lOptions.OutputFileName = lDstFileName
            #lOptions.CameraResolution = FBCameraResolutionMode.kFBResolutionHD
            #lOptions.AntiAliasing = False
            # for 60 fps lets white video in half frame rate
            #lOptions.TimeSpan = FBTimeSpan(FBTime(0,0,0,0), FBTime(0,0,0,0))
            #lOptions.TimeSteps = FBTime(0,0,0,2) 
            
            # On Mac OSX, QuickTime renders need to be in 32 bits.
            #if lRenderFileFormat == '.mov' and platform == 'darwin':
            #    lOptions.BitsPerPixel = FBVideoRenderDepth.FBVideoRender32Bits
            
            # Do the render. This will always be done in uncompressed mode.
            gApp.FileRender( lOptions )
    
            # Closing the current file, not really necessarily needed sine the FBApplication::FileOpne replaces the current scene
            gApp.FileNew()    

          

def PopulateLayout(mainLyt):
    
    offset = -85
    
    # Create Main region frame:
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    main = FBLayout()
    mainLyt.AddRegion("main","main", x, y, w, h)
    mainLyt.SetControl("main",main)
    
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(offset,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    listFiles.Style = FBListStyle.kFBVerticalList
    listFiles.MultiSelect = True
    main.AddRegion("list", "list", x, y, w, h)
    main.SetControl("list", listFiles)

    x = FBAddRegionParam(5,FBAttachType.kFBAttachRight,"list")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    buttonAdd.Caption = "Add"
    buttonAdd.OnClick.Add(EventButtonAdd)    
    main.AddRegion("buttonAdd", "buttonAdd", x, y, w, h)
    main.SetControl("buttonAdd", buttonAdd)

    x = FBAddRegionParam(5,FBAttachType.kFBAttachRight,"list")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"buttonAdd")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    buttonAddDir.Caption = "Add from dir"
    buttonAddDir.OnClick.Add(EventButtonAddDir)    
    main.AddRegion("buttonAddDir", "buttonAddDir", x, y, w, h)
    main.SetControl("buttonAddDir", buttonAddDir)

    x = FBAddRegionParam(5,FBAttachType.kFBAttachRight,"list")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"buttonAddDir")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    buttonRemove.Caption = "Remove"
    buttonRemove.OnClick.Add(EventButtonRemove)    
    main.AddRegion("buttonRemove", "buttonRemove", x, y, w, h)
    main.SetControl("buttonRemove", buttonRemove)


    x = FBAddRegionParam(5,FBAttachType.kFBAttachRight,"list")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"buttonRemove")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    buttonClear.Caption = "Clear"
    buttonClear.OnClick.Add(EventButtonClear)
    main.AddRegion("buttonClear", "buttonClear", x, y, w, h)
    main.SetControl("buttonClear", buttonClear)
    
    x = FBAddRegionParam(5,FBAttachType.kFBAttachRight,"list")
    y = FBAddRegionParam(15,FBAttachType.kFBAttachBottom,"buttonClear")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    buttonRender.Caption = "Render!"
    buttonRender.OnClick.Add(EventButtonBatch)
    main.AddRegion("buttonRender", "buttonRender", x, y, w, h)
    main.SetControl("buttonRender", buttonRender)

    x = FBAddRegionParam(5,FBAttachType.kFBAttachRight,"list")
    y = FBAddRegionParam(15,FBAttachType.kFBAttachBottom,"buttonRender")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    buttonAbout.Caption = "About"
    buttonAbout.OnClick.Add(EventButtonAbout)
    main.AddRegion("buttonAbout", "buttonAbout", x, y, w, h)
    main.SetControl("buttonAbout", buttonAbout)   
    
def CreateTool():    
    # Tool creation will serve as the hub for all other controls
    t = FBCreateUniqueTool("Batch render tool")
    PopulateLayout(t)
    t.StartSizeX = 500
    t.StartSizeY = 350
    
    if gDevelopment:
        ShowTool(t)

CreateTool()





