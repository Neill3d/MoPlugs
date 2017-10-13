
#########################################################
##
## Prepare command line to run a rendering processing from *.bat
##
## Github repo - https://github.com/Neill3d/MoPlugs
## Licensed under BSD 3-clause
##  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
##
## Author Sergey Solokhin (Neill3d) 2017
##  e-mail to: s@neill3d.com
##        www.neill3d.com
##
#########################################################


from pyfbsdk import *
from pyfbsdk_additions import *
import os

gDEVELOPMENT = False
gCmdLineToolVersion = 0.31

lApp = FBApplication()
lSystem = FBSystem()

gMoBuPath = FBEdit()
gButtonAA = FBButton()
gButtonOffscreen = FBButton()
gListCamera = FBList()
gListRenderer = FBList()
gEditStartTime = FBEditNumber()
gEditStopTime = FBEditNumber()
gEditStepTime = FBEditNumber()
gEditPath = FBEdit()
gListPictureFormat = FBList()
gListExt = FBList()
gListCodec = FBList()

gExtensionKeys = [ 'AVI',
                    'MOV',
                    'JPG',
                    'TGA',
                    'TIF',
                    'TIFF',
                    'YUV',
                    'SWF (Flash)' ]
                    
gExtensions = {'AVI' : '.avi',
                'MOV' : '.mov',
                'JPG' : '.jpg',
                'TGA' : '.tga',
                'TIF' : '.tif',
                'TIFF' : '.tif',
                'YUV' : 'yuv',
                'SWF (Flash)' : '.swf'}

gPictureFormats = {
                'From Camera' : 'FromCamera',
                '(720 x 486) D1 NTSC' : 'D1_NTSC',
                '(640 x 480) NTSC' : 'NTSC',
                '(570 x 486) PAL' : 'PAL',
                '(720 x 576) D1 PAL' : 'D1_PAL',
                '(1920 x 1080) HD' : 'HD',
                '(640 x 480) 640x480' : '640x480',
                '(320 x 200) 320x200' : '320x200',
                '(320 x 240) 320x240' : '320x240',
                '(128 x 128) 128x128' : '128x128',
                'Full Screen' : 'FullScreen' }

def OnButtonChooseFile(control, event):
    print "choose file"

    lDialog = FBFilePopup()
    lDialog.Style = FBFilePopupStyle.kFBFilePopupSave
    lDialog.Filter = '*.avi'
    lDialog.FileName = gEditPath.Text
    
    if lDialog.Execute():
        gEditPath.Text = lDialog.FullFilename

def GenerateCmdLine():
    
    fbxscenePath = lApp.FBXFileName
    
    cmdline = 'start /d "' + gMoBuPath.Text + '\\"' + ' motionbuilder.exe'
    cmdline = cmdline + ' -console -suspendMessages --sdk-begin'
    # DONE: choose 'window' or 'offline' render modes
    if gButtonOffscreen.State == 1:
        cmdline = cmdline + ' --r offscreen'
    else:
        cmdline = cmdline + ' --r window'
    
    # antialiasing
    if gButtonAA.State == 1:
        cmdline = cmdline + ' --aa 1'
    else:
        cmdline = cmdline + ' --aa 0'
    
    # choose renderer
    currRendererName = gListRenderer.Items[gListRenderer.ItemIndex]
    cmdline = cmdline + ' --rr \"' + currRendererName + '\"'
    
    # render camera
    if gListCamera.ItemIndex > 0:
        cameraName = gListCamera.Items[gListCamera.ItemIndex]
        cmdline = cmdline + ' --cam \"' + cameraName + '\"'
    else:
        currCamera = lSystem.Renderer.CurrentCamera
        if isinstance(currCamera, FBCameraSwitcher):
            currCamera = currCamera.CurrentCamera
        
        if None != currCamera:
            cmdline = cmdline + ' --cam \"' + currCamera.LongName + '\"'
    
    # render time range
    
    cmdline = cmdline + ' --stt ' + str(int(gEditStartTime.Value))
    cmdline = cmdline + ' --stp ' + str(int(gEditStopTime.Value))
    cmdline = cmdline + ' --step ' + str(int(gEditStepTime.Value))
    
    # output picture format
    
    pictureFormatKey = gListPictureFormat.Items[gListPictureFormat.ItemIndex]
    pictureFormat = gPictureFormats[pictureFormatKey]
    cmdline = cmdline + ' --pctFrm ' + pictureFormat
    
    # output file format
        
    formatKey = gListExt.Items[gListExt.ItemIndex]
    outformat = gExtensions[formatKey]
    
    cmdline = cmdline + ' --oFmt ' + outformat

    codecName = ''
    if gListCodec.ItemIndex >= 0:
        codecName = gListCodec.Items[gListCodec.ItemIndex]
        cmdline = cmdline + ' --c ' + codecName
    
    # output file
    
    cmdline = cmdline + ' --of \"' + gEditPath.Text + '\"'
    
    # fin
    
    cmdline = cmdline + ' --close'
    cmdline = cmdline + ' --sdk-end '
    cmdline = cmdline + fbxscenePath
    
    return cmdline

def OnButtonMakeCmd(control, event):
    print "make cmd"


    lDialog = FBFilePopup()
    lDialog.Style = FBFilePopupStyle.kFBFilePopupSave
    lDialog.Filter = '*.bat'
    
    if lDialog.Execute():

        filePath = lDialog.FullFilename
        
        f = open(filePath, 'wt')
        
        f.write( GenerateCmdLine() )
        
        f.close()

def CheckOutputFileName():
    
    currExtKey = gExtensionKeys[gListExt.ItemIndex]
    currExt = gExtensions[currExtKey]
    
    thisFile = gEditPath.Text

    base = os.path.splitext(thisFile)[0]

    middle = ''
    if gListExt.ItemIndex > 1 and gListExt.ItemIndex < len(gExtensionKeys)-1:
        if len(base) > 4 and base[-4:] == '####':
            middle = ''
        else:
            middle = '####'
    else:
        if len(base) > 4 and base[-4:] == '####':
            middle = ''
            base = base[:-4]        
    
    gEditPath.Text = base + middle + currExt

def UpdateCodecList():
    
    currFormat = gListExt.Items[gListExt.ItemIndex]
    
    codecMan = FBVideoCodecManager()
    codecList = []
    codecList = codecMan.GetCodecIdList(currFormat)
    
    gListCodec.Items.removeAll()
    
    for codec in codecList:
        if codec != '\x0b':
            gListCodec.Items.append(codec)

def OnOutputFormatChange(control, event):
    CheckOutputFileName()
    UpdateCodecList()

def OnOutputFileChange(control, event):
    CheckOutputFileName()

def OnButtonAbout(control, event):
    FBMessageBox( 'CmdLine Rendering Tool', ' Version: ' + str(gCmdLineToolVersion) + '\n\n Author Sergey Solokhin (c) 2017', 'Ok' )

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
    mainOptions = FBHBoxLayout()
    mainTitles = FBVBoxLayout()
    mainValues = FBVBoxLayout()
    
    main.Add(mainOptions, 365)
    mainOptions.AddRelative(mainTitles, 0.25)
    mainOptions.AddRelative(mainValues, 0.75)
    
    b = FBLabel()
    b.Caption = 'MoBu Path:'
    mainTitles.Add(b, 35)
    
    gMoBuPath.Text = lSystem.ApplicationPath # + '\\motionbuilder.exe'
    mainValues.Add(gMoBuPath, 35)

    b = FBLabel()
    b.Caption = ''
    mainTitles.Add(b, 25)

    gButtonAA.Style = FBButtonStyle.kFBCheckbox
    gButtonAA.Caption = 'Antialiasing'
    gButtonAA.State = 0
    mainValues.Add(gButtonAA, 25)

    b = FBLabel()
    b.Caption = ''
    mainTitles.Add(b, 25)

    gButtonOffscreen.Style = FBButtonStyle.kFBCheckbox
    gButtonOffscreen.Caption = 'Offscreen Render'
    gButtonOffscreen.State = 1
    mainValues.Add(gButtonOffscreen, 25)

    # choose camera
    # TODO: we should use current scene camera instead
    
    b = FBLabel()
    b.Caption = 'Camera:'
    mainTitles.Add(b, 35)
    
    gListCamera.Style = FBListStyle.kFBDropDownList
    mainValues.Add(gListCamera, 35)
    
    
    # choose render callback
    b = FBLabel()
    b.Caption = 'Render Using:'
    mainTitles.Add(b, 35)
    
    gListRenderer.Style = FBListStyle.kFBDropDownList
    mainValues.Add(gListRenderer, 35)

    # let's choose a picture format
    b = FBLabel()
    b.Caption = 'Picture Format:'
    mainTitles.Add(b, 35)
    
    gListPictureFormat.Style = FBListStyle.kFBDropDownList
    gListPictureFormat.Items.append( 'From Camera' )
    gListPictureFormat.Items.append( '(720 x 486) D1 NTSC' )
    gListPictureFormat.Items.append( '(640 x 480) NTSC' )
    gListPictureFormat.Items.append( '(570 x 486) PAL' )
    gListPictureFormat.Items.append( '(720 x 576) D1 PAL' )
    gListPictureFormat.Items.append( '(1920 x 1080) HD' )
    gListPictureFormat.Items.append( '(640 x 480) 640x480' )
    gListPictureFormat.Items.append( '(320 x 200) 320x200' )
    gListPictureFormat.Items.append( '(320 x 240) 320x240' )
    gListPictureFormat.Items.append( '(128 x 128) 128x128' )
    gListPictureFormat.Items.append( 'Full Screen' )
    mainValues.Add(gListPictureFormat, 35)

    # output filename and path
    lGrabber = FBVideoGrabber()
    lGrabberOptions = lGrabber.GetOptions()

    b = FBLabel()
    b.Caption = 'Output Format:'
    mainTitles.Add(b, 35)

    gListExt.Style = FBListStyle.kFBDropDownList
    for name in gExtensionKeys:    
        gListExt.Items.append(name)
    gListExt.OnChange.Add( OnOutputFormatChange )
    mainValues.Add(gListExt, 35)

    # output file codec
    
    b = FBLabel()
    b.Caption = 'Output Video Codec:'
    mainTitles.Add(b, 35)

    gListCodec.Style = FBListStyle.kFBDropDownList
    UpdateCodecList()
    mainValues.Add(gListCodec, 35)

    # output filename

    b = FBLabel()
    b.Caption = 'Output File:'
    mainTitles.Add(b, 25)

    gEditPath.Text = lGrabberOptions.OutputFileName
    gEditPath.OnChange.Add( OnOutputFileChange )
    mainValues.Add(gEditPath, 25)

    #
    CheckOutputFileName()

    b = FBButton()
    b.Caption = "Choose an output file..."
    b.OnClick.Add( OnButtonChooseFile )
    mainValues.Add(b, 35)
    
    timeLayout = FBHBoxLayout()
    
    b = FBLabel()
    b.Caption = 'Start:'
    timeLayout.AddRelative(b, 0.1)
    
    gEditStartTime.Value = 0.0
    timeLayout.AddRelative(gEditStartTime, 0.25)
    
    b = FBLabel()
    b.Caption = 'Stop:'
    timeLayout.AddRelative(b, 0.1)
    
    gEditStopTime.Value = 0.0
    timeLayout.AddRelative(gEditStopTime, 0.25)
    
    b = FBLabel()
    b.Caption = 'Step:'
    timeLayout.AddRelative(b, 0.1)
    
    gEditStepTime.Value = 1.0
    timeLayout.AddRelative(gEditStepTime, 0.25)
    
    main.Add(timeLayout, 35)
    
    b = FBLabel()
    b.Caption = ''
    main.Add(b, 25)
    
    toolbox = FBHBoxLayout()
    
    b = FBButton()
    b.Caption = "Make a Command Line"
    b.OnClick.Add( OnButtonMakeCmd )
    
    toolbox.AddRelative(b, 0.75)
    
    b = FBButton()
    b.Caption = "About"
    b.OnClick.Add( OnButtonAbout )
    
    toolbox.AddRelative(b, 0.25)
    
    main.Add(toolbox, 25)

def RefreshUI():

    # list of scene cameras
    
    gListCamera.Items.removeAll()
    gListCamera.Items.append( 'Current Camera' )
    for lCamera in lSystem.Scene.Cameras:
        gListCamera.Items.append( lCamera.LongName )

    # list of renderer callbacks
    
    gListRenderer.Items.removeAll()
    gListRenderer.Items.append( "Default Renderer" )
    
    for lRenderCallback in lSystem.Renderer.RendererCallbacks:
        gListRenderer.Items.append( lRenderCallback.Name )

def EventFileChange(control, event):
    RefreshUI()
    
def EventShowTool(control, event):
    
    if True == event.Shown:
        RefreshUI()
        lApp.OnFileNewCompleted.Add(EventFileChange)
        lApp.OnFileOpenCompleted.Add(EventFileChange)
        lApp.OnFileMerge.Add(EventFileChange)
    else:
        lApp.OnFileNewCompleted.Remove(EventFileChange)
        lApp.OnFileOpenCompleted.Add(EventFileChange)
        lApp.OnFileMerge.Remove(EventFileChange)

def CreateTool():
    t = None
    try:
        t = FBCreateUniqueTool("CmdLine Rendering Tool")
    except NameError:
        t = CreateUniqueTool("CmdLine Rendering Tool")
        print "supporting MoBu 2010"
        
    if t:
        t.StartSizeX = 400
        t.StartSizeY = 500
        t.OnShow.Add( EventShowTool )
        PopulateTool(t)
        if gDEVELOPMENT:
            ShowTool(t)
        
CreateTool()