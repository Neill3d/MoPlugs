
###########################################################
#
# Tool for managing composition trees
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
# Author Sergey Solohin (Neill3d) 2016-2017
#  e-mail to: s@neill3d.com
#   www.neill3d.com
#
###########################################################

from pyfbsdk import *
from pyfbsdk_additions import *
import math
import os

gDEVELOPMENT = False
COMPOSITION_TREE_TOOL_VERSION = 0.38

gapp = FBApplication()
gplayer = FBPlayerControl()
gSystem = FBSystem()

# UI controls

gMainIndex = 0
gMainFilter = ''
gMainTreeDict = {}
gMainTree = FBTree()
gEditLayout = FBLayout()
gLayoutCreate = FBScrollBox()
gProps = FBBrowsingProperty()
gView = FBView()
gButtonMove = FBButton()
gButtonCopy = FBButton()
gButtonSyncSelection = FBButton()

gSelectInScene = True

gTreeDragComponent = 0

gAssetCompositionName = ['Composition']
gAssetLayerNames = ['Render Layer', 'Color Layer', 'Shadow Layer']
gAssetFilterNames = ['3d Decal Filter', '3d DOF Filter', '3d Fog Filter', '3d LUT Filter', 'Blur Filter', 'Change Color Filter', 'Color Correction Filter',
'Cross Stitching Filter', 'Crosshatch Filter', 'Film Grain Filter', 'HalfTone Filter',
'Outline Filter', 'Posterization Filter', 'SSAO Filter']

gAssetAllNames = gAssetCompositionName + gAssetLayerNames + gAssetFilterNames

gCompositionTypeInfo = -1
gLayerTypeInfo = -1
gEffectTypeInfo = -1
gMarkerTypeInfo = -1

print gCompositionTypeInfo
print gLayerTypeInfo
print gEffectTypeInfo


######################################################################

def ClearPropertyView():
    for i in range(gProps.ObjectGetCount()):
        gProps.RemoveObject(gProps.ObjectGet(i) )

def AddToPropertyView(comp):
    
    ClearPropertyView()
    gProps.AddObject(comp)

def ClearTreeDict():
    global gMainIndex, gMainTreeDict
    gMainIndex = 1
    gMainTreeDict = {}

'''

 This is a lot of possible information that could be useful
 
 - parent final composition
 - parent source for the effect
 - node
 
'''
def PutIntoTreeDict(treeNode, final, parent, node, nodeIdx):
    global gMainIndex
    
    gMainTreeDict[gMainIndex] = (final, parent, node, nodeIdx)
    treeNode.Reference = gMainIndex
    gMainIndex += 1
    
    return treeNode.Reference


def IsNodeFinal(comp):
    
    if comp.Is(gCompositionTypeInfo):
        return True
        
    return False

def IsNodeMarker(comp):
    return comp.Is(gMarkerTypeInfo)

def IsNodeSource(comp):
    return comp.Is(gLayerTypeInfo)

def IsNodeFilter(comp):
    return comp.Is(gEffectTypeInfo)

def IsNodeSourceOrFilter(comp):
    return IsNodeSource(comp) or IsNodeFilter(comp)

def IsNodeAnyCompositionNode(comp):
    return IsNodeFinal(comp) or IsNodeSource(comp) or IsNodeFilter(comp)

def IsAssetFinalComposition(comp):
    name = ''
    if isinstance(comp, FBComponent):
        if comp.GetSrcCount() > 0 or comp.GetDstCount() > 0:
            return False
        name = comp.Name
    else:
        name = comp
        
    return (name in gAssetCompositionName)

def IsAssetAnyComposition(comp):
    name = ''
    if isinstance(comp, FBComponent):
        if comp.GetSrcCount() > 0 or comp.GetDstCount() > 0:
            return False
        name = comp.Name
    else:
        name = comp
    return (name in gAssetAllNames)

def IsAssetSource(comp):
    name = ''
    if isinstance(comp, FBComponent):
        if comp.GetSrcCount() > 0 or comp.GetDstCount() > 0:
            return False
        name = comp.Name
    else:
        name = comp
    return name in gAssetLayerNames

def IsAssetFilter(comp):
    name = ''
    if isinstance(comp, FBComponent):
        if comp.GetSrcCount() > 0 or comp.GetDstCount() > 0:
            return False
        name = comp.Name
    else:
        name = comp
    return name in gAssetFilterNames

def IsAssetSourceOrFilter(comp):
    name = ''
    if isinstance(comp, FBComponent):
        if comp.GetSrcCount() > 0 or comp.GetDstCount() > 0:
            return False
        name = comp.Name
    else:
        name = comp
    if name in gAssetLayerNames:
        return True
    if name in gAssetFilterNames:
        return True
    return False

def CreateNewAsset(comp):
    assetPath = ''

    if IsAssetFinalComposition(comp):
        assetPath = 'Browsing/Templates/Shading Elements/Composition'
    elif IsAssetSource(comp):
        assetPath = 'Browsing/Templates/Shading Elements/Composition/Layers'
    elif IsAssetFilter(comp):
        assetPath = 'Browsing/Templates/Shading Elements/Composition/Filters'
    
    compName = ''
    if isinstance(comp, FBComponent):
        compName = comp.Name
    else:
        compName = comp
    
    newComp = None
    if assetPath != '':
        newComp = FBCreateObject(assetPath, compName, compName)
    return newComp

#
# TODO: add info about enabled/disabled state and used as main render
#
def ConcatenateCompositionName(comp):
    
    name = comp.Name
    
    openBracket = True
    closeBracket = False
    
    prop = comp.PropertyList.Find('Output To Render')
    if prop != None and prop.Data:
        
        if openBracket:
            name = name + ' ['
            openBracket = False
            
        name = name + ' Render '
        closeBracket = True
        
    prop = comp.PropertyList.Find('Cameras')
    if prop != None:
        for i in range(prop.GetSrcCount() ):
            camera = prop.GetSrc(i)
            
            if openBracket:
                name = name + ' [ '
                openBracket = False
                
            name = name + camera.Name + ' '
            closeBracket = True
    
    if closeBracket:
        name = name + ']'
    
    return name

def PopulateTreeCompositionCameras(treeRoot, final, comp):
    
    count = comp.GetDstCount()
    
    for i in range(count):
        if isinstance(comp.GetDst(i), FBCamera):
            camera = comp.GetDst(i)
            cameraTreeNode = gMainTree.InsertLast(treeRoot, camera.Name)
            
            PutIntoTreeDict(cameraTreeNode, final, comp, camera, i)

def PopulateTreeCompositionLights(treeRoot, final, comp):
    
    count = comp.GetDstCount()
    
    for i in range(count):
        if isinstance(comp.GetDst(i), FBLight):
            light = comp.GetDst(i)
            lightTreeNode = gMainTree.InsertLast(treeRoot, light.Name)
            
            PutIntoTreeDict(lightTreeNode, final, comp, light, i)

def PopulateTreeCompositionGroups(treeRoot, final, comp):
    
    count = comp.GetDstCount()
    
    for i in range(count):
        if isinstance(comp.GetDst(i), FBGroup):
            group = comp.GetDst(i)
            groupTreeNode = gMainTree.InsertLast(treeRoot, group.Name)
            
            PutIntoTreeDict(groupTreeNode, final, comp, group, i)

def PopulateTreeCompositionLayers(treeRoot, final, comp):
    
    count = comp.GetSrcCount()
    i = count-1
    print count
    while i >= 0:
        node = comp.GetSrc(i)
        print node.Name
        if IsNodeSourceOrFilter(node):
            print "insert node"
            
            theNodeName = node.Name
            
            prop = node.PropertyList.Find('Use Composite Mask')
            if prop != None and prop.Data == True:
                theNodeName = theNodeName + " [Masked]"
            nodeTreeNode = gMainTree.InsertLast(treeRoot, theNodeName)
        
            PutIntoTreeDict(nodeTreeNode, final, comp, node, i)
        
            if IsNodeSource(node):
                PopulateTreeCompositionLayers(nodeTreeNode, final, node)
            elif IsNodeFilter(node):
                # add source models for filters
                PopulateTreeCompositionLayers(nodeTreeNode, final, node)
        elif IsNodeMarker(node):
            
            theNodeName = 'Marker - ' + node.Name
            nodeTreeNode = gMainTree.InsertLast(treeRoot, theNodeName)
        
            PutIntoTreeDict(nodeTreeNode, final, comp, node, i)
        
        i = i-1
    
def PopulateTreeCompositionOutput(treeRoot, final, comp):
    
    count = comp.GetDstCount()
    
    for i in range(count):
        if isinstance(comp.GetDst(i), FBVideo):
            node = comp.GetDst(i)
            videoTreeNode = gMainTree.InsertLast(treeRoot, node.Name)
            
            PutIntoTreeDict(videoTreeNode, final, comp, node, i)

def PopulateTreeComposition(treeRoot, comp, compIdx):
    
    compositionName = ConcatenateCompositionName(comp)
    
    compTreeNode = gMainTree.InsertLast(treeRoot, compositionName)
    compTreeNode.Reference = gMainIndex
    
    refIndex = PutIntoTreeDict(compTreeNode, comp, comp, comp, compIdx)
    
    batchTreeNode = gMainTree.InsertLast(compTreeNode, "Batch Processing Textures")
    batchTreeNode.Reference = refIndex
    
    cameraTreeNode = gMainTree.InsertLast(compTreeNode, "Linked Cameras" )
    cameraTreeNode.Reference = refIndex
    PopulateTreeCompositionCameras(cameraTreeNode, comp, comp)

    lightTreeNode = gMainTree.InsertLast(compTreeNode, "Linked Lights" )
    lightTreeNode.Reference = refIndex
    PopulateTreeCompositionLights(lightTreeNode, comp, comp)
    '''
    groupTreeNode = gMainTree.InsertLast(compTreeNode, "Linked Groups" )
    groupTreeNode.Reference = refIndex
    PopulateTreeCompositionGroups(groupTreeNode, comp, comp)
    '''
    # TODO: global filters are part of global layers
    #filtersTreeNode = gMainTree.InsertLast(compTreeNode, "Global Filters" )
    #PopulateTreeCompositionFilters(filtersTreeNode, comp)
    
    layersTreeNode = gMainTree.InsertLast(compTreeNode, "Layers" )
    layersTreeNode.Reference = refIndex
    PopulateTreeCompositionLayers(layersTreeNode, comp, comp)

    outputTreeNode = gMainTree.InsertLast(compTreeNode, "Output" )
    outputTreeNode.Reference = refIndex
    PopulateTreeCompositionOutput(outputTreeNode, comp, comp)

def GetGlobalSettingsObject():
    
    for obj in gSystem.Renderer.RendererCallbacks:
        if obj.ClassName() == 'MoRendererCallback':
            return obj
    
    # if nothing has found, then create a new one
    return None

def PopulateTreeGlobalSettings(treeRoot, obj):
    
    compTreeNode = gMainTree.InsertLast(treeRoot, "Renderer Settings")
    refIndex = PutIntoTreeDict(compTreeNode, obj, obj, obj, -1)

def PopulateTreeRenderOutput(rootNode):

    node = gMainTree.InsertLast(rootNode, "Render Output" )
    gMainTree.InsertLast(node, "Color")
    gMainTree.InsertLast(node, "Depth")
    gMainTree.InsertLast(node, "Normals")
    gMainTree.InsertLast(node, "Mask A")
    gMainTree.InsertLast(node, "Mask B")
    gMainTree.InsertLast(node, "Mask C")
    gMainTree.InsertLast(node, "Mask D")    

def PopulateTree(tree):
    
    ClearTreeDict()
    
    tree.Clear()
    tree.AutoExpandOnDragOver = True
    tree.AllowExpansion = True
    tree.ItemHeight = 35
    tree.Indent = 25
    tree.ShowLines = True
    
    lscene = gSystem.Scene
    
    treeRootNode = tree.GetRoot()
    
    # TODO: this should be a part of preview !
    # PopulateTreeRenderOutput(treeRootNode)
    
    globalSettingsObj = GetGlobalSettingsObject()
    PopulateTreeGlobalSettings(treeRootNode, globalSettingsObj)

    for i in range(len(lscene.UserObjects)):
        comp = lscene.UserObjects[i]

        if IsNodeFinal(comp):        
            if gMainFilter == '' or comp.Name.find(gMainFilter) >= 0:
                PopulateTreeComposition(treeRootNode, comp, i)

def CheckComponentConnectivity(compPlace, dragComp):
    
    # drag an instance of itself
    if compPlace == dragComp:
        return False
    
    if IsNodeFinal(compPlace):
        if isinstance(dragComp, FBCamera) or isinstance(dragComp, FBLight):
            return True
        if IsNodeFinal(dragComp) or IsNodeSourceOrFilter(dragComp):
            return True
            
    if IsNodeSourceOrFilter(compPlace) and IsNodeSourceOrFilter(dragComp):
        return True
    
    return False

def ConnectComponents(comp, dragComp):
    print ' connect ' + comp.Name + ' and ' + dragComp.Name
    
    if IsNodeFinal(comp) and isinstance(dragComp, FBCamera):

        camerasProp = comp.PropertyList.Find('Cameras')
        if camerasProp != None:

            isConnected = False
            for i in range(camerasProp.GetSrcCount() ):
                if dragComp == camerasProp.GetSrc(i):
                    isConnected = True
                    break
            
            if isConnected == False:
                FBConnect(dragComp, camerasProp)
    
    if IsNodeFinal(comp) and isinstance(dragComp, FBLight):
        
        lightsProp = comp.PropertyList.Find('Lights')
        if lightsProp != None:
            
            isConnected = False
            for i in range(lightsProp.GetSrcCount() ):
                if dragComp == lightsProp.GetSrc(i):
                    isConnected = True
                    break
            
            if isConnected == False:
                FBConnect(dragComp, lightsProp)
    
def MoveComponents(parent, compFrom, compIdxFrom, compTo, compIdxTo):
        
    if compIdxFrom >= 0 and compIdxTo >= 0:
        
        # check if we should connect filter to the source instead of swap
        if IsNodeSource(compTo) and IsNodeFilter(compFrom):
            userid = FBMessageBox("Composition Tree", "Please choose an action", "Move", "WrapIn", "Cancel")
            
            if userid == 1:
                parent.MoveSrcAt(compIdxFrom, compIdxTo)
            elif userid == 2:
                parent.DisconnectSrcAt(compIdxFrom)
                FBConnect(compFrom, compTo)
                                        
        else:
            parent.MoveSrcAt(compIdxFrom, compIdxTo)

def CollectNodes(root, nodes):

    for i in range(root.GetSrcCount() ):
        CollectNodes(root.GetSrc(i), nodes)

    if IsNodeFinal(root) or IsNodeSource(root) or IsNodeFilter(root):
        nodes.append(root)

def DeleteAllConnectedNodes(root):
    
    nodes = []
    CollectNodes(root, nodes)
    
    map( FBComponent.FBDelete, nodes )
    del (nodes)

def CopyProperty(src, dst):
    
    if src.IsReadOnly() == True:
        return
    
    propType = src.GetPropertyType()
    
    if propType == FBPropertyType.kFBPT_bool:
        
        dst.Data = src.Data
    elif propType == FBPropertyType.kFBPT_ColorRGB:
        dst.Data = src.Data
    elif propType == FBPropertyType.kFBPT_ColorRGBA:
        #dst.Data = src.Data
        print dst.Name
        print "not possible to copy"
    elif propType == FBPropertyType.kFBPT_double:
        dst.Data = src.Data
    elif propType == FBPropertyType.kFBPT_float:
        dst.Data = src.Data
    elif propType == FBPropertyType.kFBPT_int:
        dst.Data = src.Data
    elif propType == FBPropertyType.kFBPT_Vector2D:
        dst.Data = src.Data
    elif propType == FBPropertyType.kFBPT_Vector3D:
        dst.Data = src.Data
    elif propType == FBPropertyType.kFBPT_Vector4D:
        dst.Data = src.Data    
    else:
        print dst.Name
        print "SKIP"

def CopySources(root, newRoot):

    # copy all properties and values
    for propSrc, propDst in zip(root.PropertyList, newRoot.PropertyList):
        CopyProperty(propSrc, propDst)

    count = root.GetSrcCount()
    for i in range(count):
        comp = root.GetSrc(i)
        if IsNodeSourceOrFilter(comp):
            prop = comp.PropertyList.Find('Asset Name')
            if prop:
                print prop.Data
                newComp = CreateNewAsset( prop.Data )
                print newComp
                if newComp:
                    FBConnect(newComp, newRoot)
                    
                    CopySources(comp, newComp)
          

def CopyComposition(root):
    
    newRoot = CreateNewAsset('Composition')
    
    if newRoot:
        CopySources(root, newRoot)

def MakeACopy(dragComp):
    # create a copy of component
    prop = dragComp.PropertyList.Find('Asset Name')
    if prop:
        print prop.Data
        newComp = CreateNewAsset( prop.Data )
        #parent.ConnectSrcAt(compIdx, newComp)
        
        # copy properties and children
        CopySources(dragComp, newComp)
        
        return newComp
    return None

###########################################################################
##
## Drag And Drop Event
##
###########################################################################

def OnTreeDragAndDropEvent(control, event):
    
    global gTreeDragComponent
    
    #levent = FBEventDragAndDrop(event)
    #print "drag n drop"
    
    doACopy = (gButtonCopy.State == 1)
    
    state = event.State
    if state == FBDragAndDropState.kFBDragAndDropBegin:
        
        gTreeDragComponent = 0
        
        event.Clear()
        
        selection = gMainTree.SelectedNodes
        for sel in selection:
            index = sel.Reference
            gTreeDragComponent = index
            
            final, parent, comp, compIdx = gMainTreeDict.get(index)
            if comp and compIdx >= 0:
                event.Add(comp)
                
                event.Accept()
                
    elif state == FBDragAndDropState.kFBDragAndDropDrag:
        
        # ref is a component under the cursor
        ref = event.Data[0]
        count = event.GetCount()
        
        if ref == 0:
        
            for i in range(count):
                if IsAssetFinalComposition( event.Get(i) ):
                    event.Accept()
                elif IsNodeFinal( event.Get(i) ):
                    # let's do a composition copy
                    event.Accept()
        else:                    
            # information about the component under the cursor
            final, parent, comp, compIdx = gMainTreeDict.get(ref)
            if comp != None:
        
                for i in range(count):                
                    dragComp = event.Get(i)
        
                    if IsAssetSourceOrFilter(dragComp) and IsNodeAnyCompositionNode(comp):
                        event.Accept()
                    elif CheckComponentConnectivity(comp, dragComp):
                        event.Accept()
                        
    elif state == FBDragAndDropState.kFBDragAndDropDrop:
        
        # ref is a component under the cursor
        ref = event.Data[0]
        count = event.GetCount()
        
        if ref == 0:
            
            for i in range(count):
                if IsAssetFinalComposition( event.Get(i) ):
                    print "create a new asset"
                    CreateNewAsset(event.Get(i) )

                elif IsNodeFinal( event.Get(i) ):
                    print "TODO: create a composition copy!"
                    gTreeDragComponent = 0
                    
                    CopyComposition(event.Get(i) )
        else:
            # information about the component under cursor
            final, parent, comp, compIdx = gMainTreeDict.get(ref)
            if comp and compIdx >= 0:

                for i in range(count):
                    # collect information about a drag component
                    dragIndex = gTreeDragComponent
                    dragComp = event.Get(i)
                    
                    print dragIndex
                    
                    dragFinal = None
                    dragParent = None
                    dragComp2 = None
                    dragCompIdx = -1
                    
                    if dragComp.GetDstCount() > 0 or dragComp.GetSrcCount() > 0:
                        check = gMainTreeDict.get(dragIndex)
                        if check != 0 and check != None:
                            dragFinal, dragParent, dragComp2, dragCompIdx = gMainTreeDict.get(dragIndex)
                    
                    #
                    # drop conditions
                    
                    if IsNodeFinal(comp):
                         
                        if IsNodeFinal(dragComp):
                            # swap compositions order
                            pScene = gSystem.Scene
                            
                            compIdx = -1
                            dragCompIdx = -1
                            
                            idx = 0
                            for userObj in pScene.UserObjects:
                                if userObj == comp:
                                    compIdx = idx
                                elif userObj == dragComp:
                                    dragCompIdx = idx
                                    
                                idx += 1
                            
                            if compIdx >= 0 and dragCompIdx >= 0:
                                print "swap"
                                
                                pScene.UserObjects.remove(comp)
                                pScene.UserObjects.remove(dragComp)
                                
                                if compIdx < dragCompIdx:
                                    pScene.UserObjects.append(dragComp)
                                    pScene.UserObjects.append(comp)
                                else:
                                    pScene.UserObjects.append(comp)
                                    pScene.UserObjects.append(dragComp)
                                
                                #pScene.UserObjects.SwapSrc(compIdx, dragCompIdx)
                            
                        elif isinstance(dragComp, FBCamera):
                            camerasProp = final.PropertyList.Find('Cameras')
                            if camerasProp != None:
                                FBConnect(dragComp, camerasProp)
                        elif isinstance(dragComp, FBLight):
                            lightsProp = final.PropertyList.Find('Lights')
                            if lightsProp != None:
                                FBConnect(dragComp, lightsProp)
                                
                        elif IsAssetSourceOrFilter(dragComp):

                            # new component from assets
                            print "create a new asset"
                            newComp = CreateNewAsset(dragComp)
                            if newComp:
                                FBConnect(newComp, comp)
                        
                        elif IsNodeSourceOrFilter(dragComp):
                            
                            if doACopy:
                                newComp = MakeACopy(dragComp)
                                final.ConnectSrcAt(compIdx, newComp)                                
                            else:
                                if dragCompIdx >= 0:
                                    dragParent.DisconnectSrcAt(dragCompIdx)
                                FBConnect(dragComp, comp)
                        #
                    elif IsNodeSource(comp):
                    
                        if IsAssetSource(dragComp):
                            # new component from assets
                            newComp = CreateNewAsset(dragComp)
                            if newComp:
                                final.ConnectSrcAt(compIdx, newComp)
                                
                        elif IsAssetFilter(dragComp):
                            # new component from assets
                            newComp = CreateNewAsset(dragComp)
                            if newComp:
                                userid = FBMessageBox("Composition Tree", "Please choose an action", "Move", "WrapIn", "Cancel")

                                if userid == 1:
                                    final.ConnectSrcAt(compIdx, newComp)
                                elif userid == 2:
                                    FBConnect(newComp, comp)
                                    
                        elif dragCompIdx >=0 and dragParent == parent:
                            
                            if IsNodeSource(dragComp):
                                if doACopy:
                                    newComp = MakeACopy(dragComp)
                                    parent.ConnectSrcAt(compIdx, newComp)
                                else:    
                                    parent.MoveSrcAt(dragCompIdx, compIdx)
                            
                            elif IsNodeFilter(dragComp):
                                userid = FBMessageBox("Composition Tree", "Please choose an action", "Swap", "WrapIn", "Cancel")

                                if userid == 1:
                                    if doACopy:
                                        newComp = MakeACopy(dragComp)
                                        parent.ConnectSrcAt(compIdx, newComp)
                                    else:
                                        parent.MoveSrcAt(dragCompIdx, compIdx)
                                elif userid == 2:
                                    if doACopy:
                                        newComp = MakeACopy(dragComp)
                                        FBConnect(newComp, comp)
                                    else:
                                        dragFinal.DisconnectSrcAt(dragCompIdx)
                                        FBConnect(dragComp, comp)
                        
                        elif dragCompIdx >= 0 and dragParent != parent:
                            
                            if IsNodeSource(dragComp):
                                if doACopy:
                                    newComp = MakeACopy(dragComp)
                                    final.ConnectSrcAt(compIdx, newComp)
                                else:
                                    dragParent.DisconnectSrcAt(dragCompIdx)
                                    final.ConnectSrcAt(compIdx, dragComp)
                            
                            elif IsNodeFilter(dragComp):
                                userid = FBMessageBox("Composition Tree", "Please choose an action", "Move", "WrapIn", "Cancel")

                                if userid == 1:
                                    if doACopy:
                                        newComp = MakeACopy(dragComp)
                                        final.ConnectSrcAt(compIdx, newComp)
                                    else:
                                        dragParent.DisconnectSrcAt(dragCompIdx)
                                        final.ConnectSrcAt(compIdx, dragComp)
                                elif userid == 2:
                                    if doACopy:
                                        newComp = MakeACopy(dragComp)
                                        FBConnect(newComp, comp)
                                    else:
                                        dragParent.DisconnectSrcAt(dragCompIdx)
                                        FBConnect(dragComp, comp)
                        #
                        
                    elif IsNodeFilter(comp):
                        
                        if IsAssetSourceOrFilter(dragComp):
                            # new component from assets
                            newComp = CreateNewAsset(dragComp)
                            if newComp:
                                final.ConnectSrcAt(compIdx, newComp)
                    
                        elif dragCompIdx >=0 and dragParent == parent:
                            
                            if IsNodeSourceOrFilter(dragComp):
                                if doACopy:
                                    newComp = MakeACopy(dragComp)
                                    parent.ConnectSrcAt(compIdx, newComp)
                                else:
                                    parent.MoveSrcAt(dragCompIdx, compIdx)
                        
                        elif dragCompIdx >= 0 and dragParent != parent:
                            
                            if IsNodeSourceOrFilter(dragComp):
                                if doACopy:
                                    newComp = MakeACopy(dragComp)
                                    parent.ConnectSrcAt(compIdx, newComp)
                                else:
                                    dragParent.DisconnectSrcAt(dragCompIdx)
                                    parent.ConnectSrcAt(compIdx, dragComp)
                            
                    ##
                        
                    gTreeDragComponent = 0
                      
        PopulateTree(gMainTree)
        
    elif state == FBDragAndDropState.kFBDragAndDropEnd:
        print "drop end"
        # DONE: check if user want to disconnect or even delete component

        ref = gTreeDragComponent
       
        if ref > 0:
            print ref
            # DONE: store not only comp, but a final and a parent elements as well
            final, parent, comp, compIdx = gMainTreeDict.get(ref)
            print comp
            if final != None and comp != None and compIdx >= 0:

                if isinstance(comp, FBCamera):
                    userid = FBMessageBox("Composition Tree", "Please choose an action for component", "Disconnect", "Delete", "Cancel")

                    if userid <= 2:
                    
                        # DONE: we need to store current composition from selection !!
                        # or store composition in the tree dict as pair values    
                        camerasProp = final.PropertyList.Find('Cameras')
                        if camerasProp != None:
                            FBDisconnect(comp, camerasProp)
                            OnRefreshClick(None, None)
                            
                        if userid == 2 and comp.SystemCamera == False:
                            comp.FBDelete()
                            OnRefreshClick(None, None)
                            
                elif IsNodeSourceOrFilter(comp):
                    userid = FBMessageBox("Composition Tree", 
                    "Please choose an action for component", "Disconnect", "Delete", "Cancel")

                    # TODO: if we have several instances, we should track which one to disconnect, not everything !

                    if userid == 1:
                        parent.DisconnectSrcAt(compIdx)
                        #FBDisconnect(comp, parent)
                        OnRefreshClick(None, None)
                    elif userid == 2:
                        FBDisconnect(comp, parent)
                        comp.FBDelete()  
                        OnRefreshClick(None, None)
                elif IsNodeFinal(comp):
                    userid = FBMessageBox("Composition Tree", 
                    "Do you want to delete the composition?", "All Nodes", "Only comp", "Cancel" )
                    
                    if userid == 1:
                        # delete composition with all nodes
                        DeleteAllConnectedNodes(comp)
                        OnRefreshClick(None, None)
                    elif userid == 2:
                        # delete only composition
                        comp.FBDelete()
                        OnRefreshClick(None, None)
                    
    else:
        print state
    #

def OnTreeSelectEvent(control, event):
    
    treeNode = event.TreeNode
    
    if treeNode:
        ref = treeNode.Reference
        
        final, parent, comp, compIdx = gMainTreeDict[ref]
        
        AddToPropertyView(comp)
        
        if gButtonSyncSelection.State == 1:
            
            FBBeginChangeAllModels()
            
            for elem in gSystem.Scene.Components:
                
                if elem.HasObjectFlags( FBObjectFlag.kFBFlagSystem ):
                    continue
                    
                elem.Selected = False
            
            comp.Selected = True
            
            FBEndChangeAllModels()
                    
    
def OnTreeDblClickEvent(control, event):
    
    treeNode = event.TreeNode
    
    if treeNode:
        ref = treeNode.Reference
        final, parent, comp, compIdx = gMainTreeDict[ref]
       
        if comp.ClassName() == 'MoRendererCallback':
            comp.HardSelect()
        elif comp and compIdx >= 0:
            compName = comp.Name
            lastButtonIsCancel = True      
            userid, value = FBMessageBoxGetUserValue( "Composition Tree", "Enter a new name", compName, FBPopupInputType.kFBPopupString,"Ok","Cancel", None, 1, lastButtonIsCancel )

            if userid == 1:
                comp.Name = value
                PopulateTree(gMainTree)

def OnRefreshClick(control, event):
    PopulateTree(gMainTree)

def SelectSources(elem):
    
    for i in range(elem.GetSrcCount()):
        src = elem.GetSrc(i)
        
        SelectSources(src)
        
    elem.Selected = True

def OnSelectNodesClick(control, event):
   
    nodes = gMainTree.SelectedNodes
    
    if len(nodes) > 0:
        treeNode = nodes[0]
        
        ref = treeNode.Reference
        final, parent, comp, compIdx = gMainTreeDict[ref]
       
        if final:
            SelectSources(final)

def OnEditPanelChange(control, event):
    
    if control.ItemIndex == 0:
        gEditLayout.SetControl("main", gLayoutCreate)
    elif control.ItemIndex == 1:
        gEditLayout.SetControl("main", gProps)

def OnFilterEditChange(control, event):
    
    global gMainFilter
    
    gMainFilter = control.Text
    PopulateTree(gMainTree)

def OnButtonMoveCopyClick (control, event):
    
    if control == gButtonMove:
        gButtonMove.State = 1
        gButtonCopy.State = 0
    elif control == gButtonCopy:
        gButtonMove.State = 0
        gButtonCopy.State = 1

def OnButtonCreateCompositionClick(control, event):
    obj = FBCreateObject( 'Browsing/Templates/Shading Elements/Composition', 'Composition', 'Composition' )    
    PopulateTree(gMainTree)

def OnButtonCreateLayerClick(control, event):

    nodes = gMainTree.SelectedNodes
    
    if len(nodes) > 0:
        treeNode = nodes[0]
        
        ref = treeNode.Reference
        final, parent, comp, compIdx = gMainTreeDict[ref]
       
        if final:
            obj = FBCreateObject( 'Browsing/Templates/Shading Elements/Composition/Layers', control.Caption, control.Caption )    
            FBConnect(obj, final)
            PopulateTree(gMainTree)           

def OnButtonCreateFilterClick(control, event):
    
    nodes = gMainTree.SelectedNodes
    
    if len(nodes) > 0:
        treeNode = nodes[0]
        
        ref = treeNode.Reference
        final, parent, comp, compIdx = gMainTreeDict[ref]
       
        if final:
            obj = FBCreateObject( 'Browsing/Templates/Shading Elements/Composition/Filters', control.Caption, control.Caption )    
            FBConnect(obj, final)
            PopulateTree(gMainTree)            

def EventButtonAboutClick(control, event):
    
    FBMessageBox( "Composition Tree " + str(COMPOSITION_TREE_TOOL_VERSION),
        'Author Sergey Solokhin (Neill3d)\n'
        ' MoPlugs Project 2016-2017\n'
        '  e-mail to: s@neill3d.com\n'
        '   www.neill3d.com', 'Ok' )

def EventFileChange(control, event):
    PopulateTree(gMainTree)
    print "populate"

def InitGlobalVariables():
    
    global gCompositionTypeInfo
    global gLayerTypeInfo
    global gEffectTypeInfo
    global gMarkerTypeInfo
    
    if gCompositionTypeInfo >= 0:
        return
    
    # ObjectCompositeFinal
    obj = FBCreateObject( 'Browsing/Templates/Shading Elements/Composition', 'Composition', 'test composition' )
    if obj != None:
        prop = obj.PropertyList.Find('Type Info Composition')
        if prop: gCompositionTypeInfo = prop.Data
        prop = obj.PropertyList.Find('Type Info Layer')
        if prop: gLayerTypeInfo = prop.Data
        prop = obj.PropertyList.Find('Type Info Filter')
        if prop: gEffectTypeInfo = prop.Data
        prop = obj.PropertyList.Find('Type Info Marker')
        if prop: gMarkerTypeInfo = prop.Data
        
        obj.FBDelete()
        
    print gCompositionTypeInfo
    print gLayerTypeInfo
    print gEffectTypeInfo

def EventShowTool(control, event):
    print "shown"
    if True == event.Shown:
        
        # init global variables only once
        InitGlobalVariables()
        
        #
        #
        PopulateTree(gMainTree)
        gapp.OnFileNewCompleted.Add(EventFileChange)
        gapp.OnFileOpenCompleted.Add(EventFileChange)
        gapp.OnFileMerge.Add(EventFileChange)
    else:
        gapp.OnFileNewCompleted.Remove(EventFileChange)
        gapp.OnFileOpenCompleted.Remove(EventFileChange)
        gapp.OnFileMerge.Remove(EventFileChange)

def PopulateLayoutCreate(scroll):
    
    # Create Main region frame:
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    main = FBVBoxLayout()
   
    # Content property is the scrollbox's layout: create a region in it
    scroll.Content.AddRegion( "content", "content", x, y, w, h )
    
    # set our layout as the content of the scrollbox
    scroll.Content.SetControl("content", main)
    
    # init the scrollbox content size. We will be able to scroll on this size.
    scroll.SetContentSize(250, 800)


    b = FBButton()
    b.Caption = 'Create A New Composition'
    b.OnClick.Add( OnButtonCreateCompositionClick )
    main.Add(b, 35)
    
    b = FBLabel()
    b.Caption = 'Add Layers'
    main.Add(b, 40)
    
    for name in gAssetLayerNames:
        b = FBButton()
        b.Caption = name
        b.OnClick.Add( OnButtonCreateLayerClick )
        main.Add(b, 35)

    b = FBLabel()
    b.Caption = 'Add Filters'
    main.Add(b, 40)

    for name in gAssetFilterNames:
        b = FBButton()
        b.Caption = name
        b.OnClick.Add( OnButtonCreateFilterClick )
        main.Add(b, 35)

def PopulateLayout(mainLyt):
    
    # Create Main region frame:
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    main = FBVBoxLayout()
    mainLyt.AddRegion("main","main", x, y, w, h)
    mainLyt.SetControl("main",main)
    
    toolbar = FBHBoxLayout()
    
    b = FBLabel()
    b.Caption = "Filter: "
    toolbar.Add(b, 40)
    
    b = FBEdit()
    b.OnChange.Add( OnFilterEditChange )
    toolbar.Add(b, 180)
    
    b = FBLabel()
    b.Caption = ""
    toolbar.Add(b, 40)
    
    b = FBButton()
    b.Caption = "Refresh"
    b.OnClick.Add(OnRefreshClick)
    toolbar.Add(b, 120)
    
    b = FBButton()
    b.Caption = "Select Nodes"
    b.OnClick.Add(OnSelectNodesClick)
    toolbar.Add(b, 120)
    
    b = FBButton()
    b.Caption = "About"
    b.OnClick.Add(EventButtonAboutClick)
    toolbar.Add(b, 120)
    
    main.Add(toolbar, 32)
    
    coreLayout = FBHBoxLayout()
    
    treeLayout = FBVBoxLayout()
    
    treeToolbar = FBHBoxLayout()
    b = FBLabel()
    b.Caption = "Drag Action:"
    treeToolbar.Add(b, 75)
    

    gButtonMove.Style = FBButtonStyle.kFBRadioButton
    gButtonMove.Caption = "Move"
    gButtonMove.State = 1
    gButtonMove.OnClick.Add( OnButtonMoveCopyClick )
    treeToolbar.Add(gButtonMove, 55)

    gButtonCopy.Style = FBButtonStyle.kFBRadioButton
    gButtonCopy.Caption = "Copy"
    gButtonCopy.OnClick.Add( OnButtonMoveCopyClick )
    treeToolbar.Add(gButtonCopy, 55)
    
    treeLayout.Add(treeToolbar, 25)
    
    gButtonSyncSelection.Caption = "Sync scene selection"
    gButtonSyncSelection.Style = FBButtonStyle.kFBCheckbox
    gButtonSyncSelection.State = 0
    treeLayout.Add(gButtonSyncSelection, 25)
    
    treeLayout.AddRelative(gMainTree, 0.9)
    
    coreLayout.Add(treeLayout, 200)
    
    viewLayout = FBLayout()
    coreLayout.AddRelative(viewLayout, 0.5)
    viewLayout.AddRegion("main","main", x, y, w, h)
    viewLayout.SetView("main", gView)
    gView.Refresh(True)
    
    editLayout = FBVBoxLayout()
    b = FBTabPanel()
    b.Items.append("Create")
    b.Items.append("Properties")
    b.ItemIndex = 1
    b.OnChange.Add(OnEditPanelChange)
    editLayout.Add(b, 25)
    editLayout.AddRelative(gEditLayout, 0.9)
    
    PopulateLayoutCreate(gLayoutCreate)
    
    gEditLayout.AddRegion("main","main", x, y, w, h)
    gEditLayout.SetControl("main",gProps)
    
    coreLayout.Add(editLayout, 300)
    
    main.AddRelative(coreLayout, 0.9)

    #PopulateTree(gMainTree)
    gMainTree.OnDragAndDrop.Add(OnTreeDragAndDropEvent)
    gMainTree.OnSelect.Add(OnTreeSelectEvent)
    gMainTree.OnDblClick.Add(OnTreeDblClickEvent)
    
    
def CreateTool():    
    print "create tool"
    global t
    
    # Tool creation will serve as the hub for all other controls
    #t = FBCreateUniqueTool("Composition Tree " + str(COMPOSITION_TREE_TOOL_VERSION))
    t = FBCreateUniqueTool("Composition Tree Tool")
    t.OnShow.Add( EventShowTool )
    PopulateLayout(t)
    t.StartSizeX = 800
    t.StartSizeY = 600
    
    if gDEVELOPMENT:
        ShowTool(t)

CreateTool()
