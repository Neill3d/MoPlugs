
#########################################################
##
## Google Street View Grabber Tool script for Autodesk MotionBuilder
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
    print "[ERROR Google Street View Grabber Tool] pyskybox module is not found!"
    #FBMessageBox( "Google Street View Grabber Tool", "pyskybox module is not found!", "Ok" )

#Development? - need to recreate each time!!
gDEVELOPMENT = True

editPath = FBEdit()
editLat = FBEditNumber()
editLng = FBEditNumber()
editZoom = FBEditNumber()

def EventButtonBrowse(control, event):

    dialog = FBFolderPopup()
    dialog.Path = editPath.Text
    dialog.Caption = "Please choose a path for saving panorama images"
    
    if dialog.Execute():
        editPath.Text = dialog.Path
    
    
def EventButtonGrab(control, event):
    
    path = editPath.Text
    
    lat = editLat.Value
    lng = editLng.Value
    
    if path != '':
        pyskybox.GrabStreetView(path, lat, lng, int(editZoom.Value) )
    

def PopulateTool(t):

    # cubemap layout
    lyt = FBLayout()

    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(10,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(205,FBAttachType.kFBAttachNone,"")

    t.AddRegion("Layout","Layout", x, y, w, h)
    t.SetControl("Layout", lyt )


    vbox = FBVBoxLayout()
    
    hbox = FBHBoxLayout()
    
    b = FBLabel()
    b.Caption = "Output Path:"
    hbox.Add(b, 60)
    
    hbox.AddRelative(editPath, 0.5)
    
    b = FBButton()
    b.Caption = "..."
    b.OnClick.Add( EventButtonBrowse )
    hbox.Add(b, 25)
    
    vbox.Add(hbox, 25)
    
    #
    hbox = FBHBoxLayout()
    
    b = FBLabel()
    b.Caption = "Latitude"
    hbox.Add(b, 60)
    
    editLat.Value = 44.964453
    hbox.AddRelative(editLat, 0.6)
    
    vbox.Add(hbox, 25)

    #
    hbox = FBHBoxLayout()
    
    b = FBLabel()
    b.Caption = "Longitude"
    hbox.Add(b, 60)
    
    editLng.Value = 34.100317
    hbox.AddRelative(editLng, 0.6)
    
    vbox.Add(hbox, 25)
    
    #
    hbox = FBHBoxLayout()
    
    b = FBLabel()
    b.Caption = "Zoom"
    hbox.Add(b, 60)
    
    editZoom.Value = 1.0
    editZoom.Max = 5.0
    editZoom.Min = 1.0
    editZoom.Precision = 1.0
    editZoom.LargeStep = 1.0
    #sliderZoom.Orientation = FBOrientation.kFBHorizontal
    hbox.AddRelative(editZoom, 0.6)
    
    vbox.Add(hbox, 25)
    
    #
    b = FBLayout()
    vbox.Add(b, 25)
    
    b = FBButton()
    b.Caption = "Grab"
    b.OnClick.Add( EventButtonGrab )
    vbox.Add(b, 25)
    
    t.SetControl("Layout", vbox)


gToolName = "Google Street View Grabber Tool"


if gDEVELOPMENT:
    FBDestroyToolByName(gToolName)

if gToolName in FBToolList:
    tool = FBToolList[gToolName]
    ShowTool(tool)
else:
    tool=FBCreateUniqueTool(gToolName)
    if tool:
        tool.StartSizeX = 320
        tool.StartSizeY = 220
        PopulateTool(tool)
        ShowTool(tool)
        
    if gDEVELOPMENT:
        ShowTool(tool)