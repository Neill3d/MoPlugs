
#########################################################
##
## Viewport Grabber Tool script for Autodesk MotionBuilder
##
## Author Sergey Solohin (Neill3d) 2015
##  e-mail to: s@neill3d.com
##        www.neill3d.com
##
## Github repo - https://github.com/Neill3d/MoPlugs
## Licensed under BSD 3-clause
##  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
##
#########################################################

from pyfbsdk import *
from pyfbsdk_additions import *
try:
    import pyskybox
except:
    print "[ERROR Viewport Grabber Tool] pyskybox module is not found!"
    #FBMessageBox( "Viewport Grabber Tool", "pyskybox module is not found!", "Ok" )
import os.path

gToolName = "Viewport Grabber Tool"
#Development? - need to recreate each time!!
gDEVELOPMENT = True

lApp = FBApplication()
lSystem = FBSystem()

lEditPath = FBEdit()
lEditSize = FBEditNumber()
lButtonSeparate = FBButton()

lEditPanoPath = FBEdit()
lEditPanoWidth = FBEditNumber()
lEditPanoHeight = FBEditNumber()

###########################################################

def OnButtonSize512(control, event):
    lEditSize.Value = 512.0

def OnButtonSize1024(control, event):
    lEditSize.Value = 1024.0
    
def OnButtonSize2048(control, event):
    lEditSize.Value = 2048.0

def OnButtonPanoWidth512(control, event):
    lEditPanoWidth.Value = 512.0

def OnButtonPanoWidth1024(control, event):
    lEditPanoWidth.Value = 1024.0
    
def OnButtonPanoWidth2048(control, event):
    lEditPanoWidth.Value = 2048.0

def OnButtonPanoHeight512(control, event):
    lEditPanoHeight.Value = 512.0

def OnButtonPanoHeight1024(control, event):
    lEditPanoHeight.Value = 1024.0
    
def OnButtonPanoHeight2048(control, event):
    lEditPanoHeight.Value = 2048.0

def OnButtonPath(control, event):
    lPopup = FBFilePopup()
    lPopup.Style = FBFilePopupStyle.kFBFilePopupSave
    lPopup.Caption = 'Choose file to save cubemap (*.dds)'
    lPopup.Filter = '*.dds'
    
    if lPopup.Execute():
        lEditPath.Text = lPopup.FullFilename

def OnButtonPanoPath(control, event):
    lPopup = FBFilePopup()
    lPopup.Style = FBFilePopupStyle.kFBFilePopupSave
    lPopup.Caption = 'Choose file to save panorama (*.dds)'
    lPopup.Filter = '*.dds'
    
    if lPopup.Execute():
        lEditPanoPath.Text = lPopup.FullFilename

def OnButtonGrab(control, event):
    
    lPath = lEditPath.Text
    lSize = int(lEditSize.Value)
    lSeparate = (lButtonSeparate.State == 1)
    if lPath != "":
        pyskybox.GrabViewportCubeMap( lPath, lSize, lSeparate )
    
def OnButtonMakeBox(control, event):
    
    lPath = lEditPath.Text
    if (lPath == "") or not os.path.isfile(lPath):
        FBMessageBox( "Viewport Grabber Tool", "CubeMap is not exists", "Ok" )
    else:
        lBox = FBModelCube("skybox_box")
        lBox.Show = True
        lBox.SetVector(FBVector3d(1000.0, 1000.0, 1000.0), FBModelTransformationType.kModelScaling)
                
        lProp = lBox.PropertyList.Find("Culling")
        if lProp:
            lProp.Data = 2
        
        v = FBVector3d(0.0, -90.0, 0.0)
        lBox.SetVector(v, FBModelTransformationType.kModelRotation)
        lCamera = lSystem.Renderer.CurrentCamera
        if lCamera:
            lCamera.GetVector(v)
            lBox.SetVector(v)

        lShader = pyskybox.ORShaderSkyBox("skybox")
        lShader.Type = pyskybox.FBSkyBoxType.kFBSkyBoxTypeCubeMap
        
        lPath = lEditPath.Text
        print lPath
        if False == lShader.LoadCubeMap( lPath ):
            FBMessageBox( "Viewport Grabber Tool", "Failed to load dds cubemap", "Ok" )
            
        FBSystem().Scene.Shaders.append(lShader)
    
        lBox.Shaders.removeAll()
        lBox.Shaders.append(lShader)

def OnButtonMakeSkyDome(control, event):
    
    lPath = lEditPanoPath.Text
    if (lPath == "") or not os.path.isfile(lPath):
        FBMessageBox( "Viewport Grabber Tool", "Panorama is not exists", "Ok" )
    else:    
        lDome = FBCreateObject( "Browsing/Templates/Elements/Primitives", "polySphere", "polySphere" )
        if lDome == None:
            return
        print "skydome"  
        lDome.Name = 'SkyDome Sphere'      
        lDome.Show = True
        lDome.SetVector(FBVector3d(100.0, 100.0, 100.0), FBModelTransformationType.kModelScaling)
        lProp = lDome.PropertyList.Find("Culling")
        if lProp:
            lProp.Data = 2
        
        v = FBVector3d(0.0, -90.0, 0.0)
        lDome.SetVector(v, FBModelTransformationType.kModelRotation)
        lCamera = lSystem.Renderer.CurrentCamera
        if lCamera:
            lCamera.GetVector(v)
            lDome.SetVector(v)
            
        
        texture = FBTexture(lPath)
        material = FBMaterial('SkyDomeMaterial')
        material.SetTexture(texture)
        
        # append to skydome sphere
        lDome.Materials.append(material)
            
        lShader = pyskybox.ORShaderSkyDome("SkyDome")   
        FBSystem().Scene.Shaders.append(lShader)
        lDome.Shaders.removeAll()
        lDome.Shaders.append(lShader)
    
def OnButtonPanoConvert(control, event):

    lPath = lEditPath.Text
    lPanoPath = lEditPanoPath.Text
    
    lWidth = int(lEditPanoWidth.Value)
    lHeight = int(lEditPanoHeight.Value)
    
    if not os.path.isfile(lPath):
        FBMessageBox( "Viewport Grabber Tool", "CubeMap file is not exists", "Ok" )
        return
    
    if False == pyskybox.MakePanorama( lPath, lPanoPath, lWidth, lHeight ):
        FBMessageBox( "Viewport Grabber Tool", "Failed to Convert CubeMap 2 Panorama", "Ok" )

def PopulateCubeMap(t):
    
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")

    t.AddRegion("Border1","CubeMap", x, y, w, h)
    t.SetBorder("Border1",FBBorderStyle.kFBStandardBorder,True, True,1,0,90,0)

    ##
    
    x = FBAddRegionParam(10,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("labelPath","labelPath", x, y, w, h)

    b = FBLabel()
    b.Caption = "Path:"
    t.SetControl( "labelPath", b );

    x = FBAddRegionParam(-40,FBAttachType.kFBAttachRight,"")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-10,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonPath","buttonPath", x, y, w, h)

    b = FBButton()
    b.Caption = "..."
    b.OnClick.Add( OnButtonPath )
    t.SetControl( "buttonPath", b );

    x = FBAddRegionParam(10,FBAttachType.kFBAttachRight,"labelPath")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-10,FBAttachType.kFBAttachLeft,"buttonPath")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("editPath","editPath", x, y, w, h)

    lEditPath.Text = "C:\\cubemap.dds"
    t.SetControl( "editPath", lEditPath );

    ##
    
    x = FBAddRegionParam(10,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachBottom,"labelPath")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("labelSize","labelSize", x, y, w, h)

    b = FBLabel()
    b.Caption = "Texture Size*"
    t.SetControl( "labelSize", b );
    
    x = FBAddRegionParam(-40,FBAttachType.kFBAttachRight,"")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachBottom,"labelPath")
    w = FBAddRegionParam(-10,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonSize2048","buttonSize2048", x, y, w, h)

    b = FBButton()
    b.Caption = "2048"
    b.OnClick.Add( OnButtonSize2048 )
    t.SetControl( "buttonSize2048", b );
    
    x = FBAddRegionParam(-40,FBAttachType.kFBAttachLeft,"buttonSize2048")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachBottom,"labelPath")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachLeft,"buttonSize2048")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonSize1024","buttonSize1024", x, y, w, h)

    b = FBButton()
    b.Caption = "1024"
    b.OnClick.Add( OnButtonSize1024 )
    t.SetControl( "buttonSize1024", b );
    
    x = FBAddRegionParam(-40,FBAttachType.kFBAttachLeft,"buttonSize1024")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachBottom,"labelPath")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachLeft,"buttonSize1024")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonSize512","buttonSize512", x, y, w, h)

    b = FBButton()
    b.Caption = "512"
    b.OnClick.Add( OnButtonSize512 )
    t.SetControl( "buttonSize512", b );
    
    x = FBAddRegionParam(10,FBAttachType.kFBAttachRight,"labelSize")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachBottom,"labelPath")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachLeft,"buttonSize512")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("editSize","editSize", x, y, w, h)

    lEditSize.Value = 512
    lEditSize.Min = 2.0
    lEditSize.Max = 8096.0
    t.SetControl( "editSize", lEditSize );

    ##
    ##
    
    x = FBAddRegionParam(10,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(10,FBAttachType.kFBAttachBottom,"labelSize")
    w = FBAddRegionParam(160,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonSaveSeparate","buttonSaveSeparate", x, y, w, h)

    lButtonSeparate.Caption = "Save separate images"
    lButtonSeparate.Style = FBButtonStyle.kFBCheckbox
    t.SetControl( "buttonSaveSeparate", lButtonSeparate );
    
    ##
    ##
    
    x = FBAddRegionParam(10,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachBottom,"buttonSaveSeparate")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonGrab","buttonGrab", x, y, w, h)

    b = FBButton()
    b.Caption = "Grab"
    b.OnClick.Add( OnButtonGrab )
    t.SetControl( "buttonGrab", b );

    x = FBAddRegionParam(5,FBAttachType.kFBAttachRight,"buttonGrab")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachBottom,"buttonSaveSeparate")
    w = FBAddRegionParam(-10,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonMakeBox","buttonMakeBox", x, y, w, h)

    b = FBButton()
    b.Caption = "Make a box with SkyBox shader"
    b.OnClick.Add( OnButtonMakeBox )
    t.SetControl( "buttonMakeBox", b );

def PopulatePano(t):
    
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")

    t.AddRegion("Border1","Convert CubeMap to Panorama", x, y, w, h)
    t.SetBorder("Border1",FBBorderStyle.kFBStandardBorder,True, True,1,0,90,0)

    ##

    x = FBAddRegionParam(10,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("labelPath","labelPath", x, y, w, h)

    b = FBLabel()
    b.Caption = "Path:"
    t.SetControl( "labelPath", b );

    x = FBAddRegionParam(-40,FBAttachType.kFBAttachRight,"")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-10,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonPath","buttonPath", x, y, w, h)

    b = FBButton()
    b.Caption = "..."
    b.OnClick.Add( OnButtonPanoPath )
    t.SetControl( "buttonPath", b );

    x = FBAddRegionParam(10,FBAttachType.kFBAttachRight,"labelPath")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-10,FBAttachType.kFBAttachLeft,"buttonPath")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("editPath","editPath", x, y, w, h)

    lEditPanoPath.Text = "C:\\panorama.dds"
    t.SetControl( "editPath", lEditPanoPath );

    ##
    
    x = FBAddRegionParam(10,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachBottom,"labelPath")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("labelSize","labelSize", x, y, w, h)

    b = FBLabel()
    b.Caption = "Width *"
    t.SetControl( "labelSize", b );
    
    x = FBAddRegionParam(-40,FBAttachType.kFBAttachRight,"")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachBottom,"labelPath")
    w = FBAddRegionParam(-10,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonSize2048","buttonSize2048", x, y, w, h)

    b = FBButton()
    b.Caption = "2048"
    b.OnClick.Add( OnButtonPanoWidth2048 )
    t.SetControl( "buttonSize2048", b );
    
    x = FBAddRegionParam(-40,FBAttachType.kFBAttachLeft,"buttonSize2048")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachBottom,"labelPath")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachLeft,"buttonSize2048")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonSize1024","buttonSize1024", x, y, w, h)

    b = FBButton()
    b.Caption = "1024"
    b.OnClick.Add( OnButtonPanoWidth1024 )
    t.SetControl( "buttonSize1024", b );
    
    x = FBAddRegionParam(-40,FBAttachType.kFBAttachLeft,"buttonSize1024")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachBottom,"labelPath")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachLeft,"buttonSize1024")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonSize512","buttonSize512", x, y, w, h)

    b = FBButton()
    b.Caption = "512"
    b.OnClick.Add( OnButtonPanoWidth512 )
    t.SetControl( "buttonSize512", b );
    
    x = FBAddRegionParam(10,FBAttachType.kFBAttachRight,"labelSize")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachBottom,"labelPath")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachLeft,"buttonSize512")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("editSize","editSize", x, y, w, h)

    lEditPanoWidth.Value = 512
    lEditPanoWidth.Min = 2.0
    lEditPanoWidth.Max = 8096.0
    t.SetControl( "editSize", lEditPanoWidth );

    ##
    
    x = FBAddRegionParam(10,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachBottom,"labelSize")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("labelHeight","labelHeight", x, y, w, h)

    b = FBLabel()
    b.Caption = "Height *"
    t.SetControl( "labelHeight", b );
    
    x = FBAddRegionParam(-40,FBAttachType.kFBAttachRight,"")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachBottom,"labelSize")
    w = FBAddRegionParam(-10,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonHeight2048","buttonHeight2048", x, y, w, h)

    b = FBButton()
    b.Caption = "2048"
    b.OnClick.Add( OnButtonPanoHeight2048 )
    t.SetControl( "buttonHeight2048", b );
    
    x = FBAddRegionParam(-40,FBAttachType.kFBAttachLeft,"buttonHeight2048")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachBottom,"labelSize")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachLeft,"buttonHeight2048")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonHeight1024","buttonHeight1024", x, y, w, h)

    b = FBButton()
    b.Caption = "1024"
    b.OnClick.Add( OnButtonPanoHeight1024 )
    t.SetControl( "buttonHeight1024", b );
    
    x = FBAddRegionParam(-40,FBAttachType.kFBAttachLeft,"buttonHeight1024")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachBottom,"labelSize")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachLeft,"buttonHeight1024")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonHeight512","buttonHeight512", x, y, w, h)

    b = FBButton()
    b.Caption = "512"
    b.OnClick.Add( OnButtonPanoHeight512 )
    t.SetControl( "buttonHeight512", b );
    
    x = FBAddRegionParam(10,FBAttachType.kFBAttachRight,"labelSize")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachBottom,"labelSize")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachLeft,"buttonHeight512")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("editHeight","editHeight", x, y, w, h)

    lEditPanoHeight.Value = 512
    lEditPanoHeight.Min = 2.0
    lEditPanoHeight.Max = 8096.0
    t.SetControl( "editHeight", lEditPanoHeight );

    x = FBAddRegionParam(10,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachBottom,"labelHeight")
    w = FBAddRegionParam(140,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonPano","buttonPano", x, y, w, h)

    b = FBButton()
    b.Caption = "Convert CubeMap to Pano"
    b.OnClick.Add( OnButtonPanoConvert )
    t.SetControl( "buttonPano", b );

    x = FBAddRegionParam(5,FBAttachType.kFBAttachRight,"buttonPano")
    y = FBAddRegionParam(20,FBAttachType.kFBAttachBottom,"labelHeight")
    w = FBAddRegionParam(-10,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(24,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonPanoShader","buttonPanoShader", x, y, w, h)

    b = FBButton()
    b.Caption = "Make a sphere with SkyDome"
    b.OnClick.Add( OnButtonMakeSkyDome )
    t.SetControl( "buttonPanoShader", b );

def PopulateTool(t):

    # cubemap layout
    lyt = FBLayout()

    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(10,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(205,FBAttachType.kFBAttachNone,"")

    t.AddRegion("Layout","Layout", x, y, w, h)
    t.SetControl("Layout", lyt )

    PopulateCubeMap(lyt)
    
    # panorama layout
    lyt = FBLayout()

    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"Layout")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(205,FBAttachType.kFBAttachNone,"")

    t.AddRegion("LayoutPano","LayoutPano", x, y, w, h)
    t.SetControl("LayoutPano", lyt )

    PopulatePano(lyt)

    ##
    
    x = FBAddRegionParam(25,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"LayoutPano")
    w = FBAddRegionParam(260,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(50,FBAttachType.kFBAttachNone,"")
    t.AddRegion("labelSizeTip","labelSizeTip", x, y, w, h)

    b = FBLabel()
    b.Caption = "* please use power of two sizes\n pano width should be two times greater then height"
    t.SetControl( "labelSizeTip", b );

def CreateTool():
    t = None
    #t = CreateUniqueTool("Viewport Grabber tool (by Neill3d)")
    try:
        t = FBCreateUniqueTool("Viewport Grabber Tool")
    except NameError:
        t = CreateUniqueTool("Viewport Grabber Tool")
        print "supporting MoBu 2010"
        
    if t:
        t.StartSizeX = 320
        t.StartSizeY = 400
        PopulateTool(t)
        ShowTool(t)
        

if gDEVELOPMENT:
    FBDestroyToolByName(gToolName)

if gToolName in FBToolList:
    tool = FBToolList[gToolName]
    ShowTool(tool)
else:
    tool=FBCreateUniqueTool(gToolName)
    if tool:
        tool.StartSizeX = 320
        tool.StartSizeY = 480
        PopulateTool(tool)
        ShowTool(tool)
        
    if gDEVELOPMENT:
        ShowTool(tool)
