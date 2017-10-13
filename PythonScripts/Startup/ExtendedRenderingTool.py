
#########################################################
##
## Extended camera and cubemap rendering tool
##
## Tool that make it easier to work with the CameraRendering and CubeMapRendering
##  user objects.
##      MoPlugs Project.
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

EXTENDED_RENDERING_TOOL_VERSION = 0.36

gDEVELOPMENT = False

lApp = FBApplication()
lSystem = FBSystem()

TreeRendering = FBTree()
PropRendering = FBBrowsingProperty()

TreeHash = {}
TreeHashCount = 0

TreeLastSelected = None

###########################################################

def FindProperty(obj, propName):
    
    for lProp in obj.PropertyList:
        if lProp.Name == propName:
            print "true"
            return lProp

    print "false"    
    return None

def ClearTreeHash():
    global TreeHash, TreeHashCount
    
    TreeHash = {}
    TreeHashCount = 0

def AddToTreeHash(lComp):
    global TreeHash, TreeHashCount

    TreeHash[TreeHashCount] = lComp
    TreeHashCount += 1
    return (TreeHashCount-1)

def PropertiesClearAll():
    
    plugs = []
    count = PropRendering.ObjectGetCount()
    
    for i in range(count):
        plugs.append( PropRendering.ObjectGet(i) )
        
    for lPlug in plugs:
        PropRendering.RemoveObject(lPlug)

def PropertiesAddExclusive(plug):
    PropertiesClearAll()
    PropRendering.AddObject(plug)

def PropertiesAdd(plug):
    PropRendering.AddObject(plug)

def RefreshConnectionUI(obj, tree, parentItem):

    global TreeHash, TreeHashCount

    #for lProp in obj.PropertyList:
    #    print lProp.Name

    lProp = obj.PropertyList.Find( 'Camera' )
    if lProp != None:
    
        # get camera model name (if assigned)
        lCamera = None
        lCameraName = "None"

        if len(lProp) > 0:
            lCamera = lProp[0]
            lCameraName = lCamera.Name
        
        newNode = tree.InsertLast(parentItem, 'Camera - ' + lCameraName )
        newNode.Reference = AddToTreeHash(lCamera)

    lProp = obj.PropertyList.Find('Video Output')
    if lProp != None:
        
        # get texture model (if assigned)
        lVideo = None
        lVideoName = "None"
        if len(lProp) > 0:
            lVideo = lProp[0]
            lVideoName = lVideo.Name
        
        newNode = tree.InsertLast(parentItem, 'Video Output - ' + lVideoName)
        newNode.Reference = AddToTreeHash(lVideo)
        
        if lVideo != None:
            for i in range(lVideo.GetDstCount()):
                lComp = lVideo.GetDst(i)
                
                if isinstance(lComp, FBTexture):
                    lTextureName = lComp.Name
                    newNode = tree.InsertLast(parentItem, "Texture Output - " + lTextureName)
                    newNode.Reference = AddToTreeHash(lComp)


def RefreshTreeUI():
    
    ClearTreeHash()
    
    TreeRendering.Clear()
    
    lRoot = TreeRendering.GetRoot()
    
    for obj in lSystem.Scene.Components:
        if obj.ClassName() == 'CameraRendering':
            newItem = TreeRendering.InsertLast(lRoot, obj.Name)
            if newItem:
                newItem.Reference = AddToTreeHash(obj)
                
                RefreshConnectionUI(obj, TreeRendering, newItem)
        elif obj.ClassName() == 'CubeMapRendering':
            newItem = TreeRendering.InsertLast(lRoot, obj.Name)
            if newItem:
                newItem.Reference = AddToTreeHash(obj)
                
                RefreshConnectionUI(obj, TreeRendering, newItem)
            
def OnTreeSelect(control, event):

    global TreeLastSelected
    TreeLastSelected = event.TreeNode

    if event.TreeNode != None:
        print event.TreeNode
        ref = event.TreeNode.Reference

        if ref in TreeHash:
            lComp = TreeHash[ref]
            if lComp != None:
                PropertiesAddExclusive(lComp)
            
def OnTreeDrop(control, event):
    
    if event.State == FBDragAndDropState.kFBDragAndDropBegin:
        
        ref = event.Data[0]
        if ref in TreeHash:
            lComp = TreeHash[ref]
            
            if lComp!=None:
                if lComp.ClassName() == 'CameraRendering' or lComp.ClassName() == 'CubeMapRendering':
                    lProp = lComp.PropertyList.Find('Video Output')
                    if lProp != None:
                        
                        lVideo = None
                        if len(lProp) > 0:
                            lVideo = lProp[0]
                        
                        if lVideo != None:
                            for i in range(lVideo.GetDstCount()):
                                
                                lCompTexture = lVideo.GetDst(i)
                                if isinstance(lCompTexture, FBTexture):
                                    event.Accept()
                                    event.Clear()
                                    event.Add( lCompTexture )        
                                    break
            
                elif isinstance(lComp, FBCamera) or isinstance(lComp, FBVideo) or isinstance(lComp, FBTexture):
                    event.Accept()
                    event.Clear()
                    event.Add( lComp )

    elif event.State == FBDragAndDropState.kFBDragAndDropDrag:
        
        if event.GetCount() > 0 and isinstance(event.Get(0), FBCamera):
            event.Accept()
    
    elif event.State == FBDragAndDropState.kFBDragAndDropDrop:

        if event.GetCount() > 0 and isinstance(event.Get(0), FBCamera):
            lCamera = event.Get(0)
            print lCamera.Name
            ref = event.Data[0]
            if ref in TreeHash:
                lComp = TreeHash[ref]
                if lComp != None and (lComp.ClassName() == 'CameraRendering' or lComp.ClassName() == 'CubeMapRendering'):
                    lProp = lComp.PropertyList.Find('Camera')
                    if lProp != None:
                        
                        lProp.append(lCamera)
                        
                        PropertiesClearAll()
                        RefreshTreeUI()
        
            
def OnButtonFocus(control, event):
    
    if TreeRendering.SelectedCount > 0:
        if TreeLastSelected != None:
            ref = TreeLastSelected.Reference
            
            if ref in TreeHash:
                lComp = TreeHash[ref]
                if lComp != None:
                    lComp.HardSelect()
            
def OnAddCam(control, event):
    
    #newNode = pygraphics.CameraRendering("Camera Rendering")
    newNode = FBCreateObject( "FbxStorable/User", "CameraRendering", "CameraRendering" ) ;
    
    RefreshTreeUI()

def OnAddCubeMap(control, event):
    
    #newNode = pygraphics.CubeMapRendering("CubeMap Rendering")
    newNode = FBCreateObject( "FbxStorable/Model", "CubeMapRendering", "CubeMapRendering" ) ;
    if newNode != None:
        newNode.Show = True
        
    RefreshTreeUI()

def RemoveRenderingElement(comp):
    
    PropertiesClearAll()
    
    # try to remove connected dynamic texture and video
    
    if comp.ClassName() == 'CameraRendering' or comp.ClassName() == 'CubeMapRendering':
        lProp = comp.PropertyList.Find('Video Output')
        if lProp != None:
            
            lVideo = None
            if len(lProp) > 0:
                lVideo = lProp[0]
            
            if lVideo != None:
                for i in range(lVideo.GetDstCount()):
                    
                    lCompTexture = lVideo.GetDst(i)
                    if isinstance(lCompTexture, FBTexture):
                        
                        lCompTexture.FBDelete()
                        break
                        
                lVideo.FBDelete()
    
    # remove element
    comp.FBDelete()
    comp = None
    
    RefreshTreeUI()
    
def OnRemove(control, event):
    
    if 1==FBMessageBox("Extended Rendering Tool", "Are you sure you want to remove a selected element?", "Ok", "Cancel"):
        if TreeRendering.SelectedCount > 0:
            if TreeLastSelected != None:
                ref = TreeLastSelected.Reference
                
                if ref in TreeHash:
                    lComp = TreeHash[ref]
                    if lComp != None:
                        if lComp.ClassName() == 'CameraRendering' or lComp.ClassName() == 'CubeMapRendering':
                            RemoveRenderingElement(lComp)
    
def OnButtonRefresh(control, event):
    
    PropertiesClearAll()
    RefreshTreeUI()
    
def OnClear(control, event):
    FBMessageBox( "Extended Rendering Tool", "This function is in progress", "Ok" )
    
def OnAbout(control, event):
    FBMessageBox( "Extended Rendering Tool", "Version: " + str(EXTENDED_RENDERING_TOOL_VERSION) +  "\n MoPlugs Project (C) 2015-2017", "Ok" )

def EventFileChange(control, event):
    OnButtonRefresh(None, None)
    print "populate"
    
def EventShowTool(control, event):
    print "shown"
    if True == event.Shown:
        OnButtonRefresh(None, None)
        lApp.OnFileNewCompleted.Add(EventFileChange)
        lApp.OnFileOpenCompleted.Add(EventFileChange)
        lApp.OnFileMerge.Add(EventFileChange)
    else:
        lApp.OnFileNewCompleted.Remove(EventFileChange)
        lApp.OnFileOpenCompleted.Add(EventFileChange)
        lApp.OnFileMerge.Remove(EventFileChange)

def PopulateTool(mainLyt):

    # Create Main region frame:
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    main = FBVBoxLayout()
    mainLyt.AddRegion("main","main", x, y, w, h)
    mainLyt.SetControl("main",main)

    # create horizontal boxes packed in a vbox
    hstripes = FBVBoxLayout()
    
    # Shows how to create a FBHBoxLayout that grows from left to right
    box = FBHBoxLayout(FBAttachType.kFBAttachLeft)
    names = ["Add Cam", "Add CubeMap", "Focus", "Refresh", "Remove", "Clear All", "About"]
    events = [ OnAddCam, OnAddCubeMap, OnButtonFocus, OnButtonRefresh, OnRemove, OnClear, OnAbout ]
    
    i = 0
    itemSize = 1.0 / len(names)
    for name in names:
        b = FBButton()
        b.Caption = name
        b.OnClick.Add(events[i])
        
        box.AddRelative(b, itemSize)
        
        i += 1
        
    hstripes.Add(box, 35)

    # add list titles
    box = FBHBoxLayout(FBAttachType.kFBAttachLeft)
    names = ["Rendering Objects:", "Properties:"]
    for name in names:
        b = FBLabel()
        b.Caption = name
        box.AddRelative(b, 0.5)
    hstripes.Add(box, 35)

    # add lists
    box = FBHBoxLayout(FBAttachType.kFBAttachLeft)

    TreeRendering.OnSelect.Add(OnTreeSelect)
    TreeRendering.OnDragAndDrop.Add(OnTreeDrop)

    box.AddRelative(TreeRendering, 0.5)
    box.AddRelative(PropRendering, 0.5)


    hstripes.AddRelative(box, 1.0)

    main.AddRelative(hstripes,1.0)

    #
    #
    #RefreshTreeUI()
        
def CreateTool(): 
    
    global t
       
    # Tool creation will serve as the hub for all other controls
    t = FBCreateUniqueTool("Extended Rendering Tool")
    t.OnShow.Add( EventShowTool )
    PopulateTool(t)
    t.StartSizeX = 550
    t.StartSizeY = 600
    
    if gDEVELOPMENT:
        ShowTool(t)

CreateTool()
