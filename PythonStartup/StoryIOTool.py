
#########################################################
##
## Story xml IO script for Autodesk MotionBuilder
##
## Author Sergey Solohin (Neill3d) 2014
##  e-mail to: s@neill3d.com
##        www.neill3d.com
##
## Github repo - https://github.com/Neill3d/MoPlugs
## Licensed under BSD 3-clause
##  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
##
#########################################################

#
# TODO: compare stored and current story information
#

from pyfbsdk import *
from pyfbsdk_additions import *
from xml.dom import minidom

gDevMode = False

lApp = FBApplication()
lSystem = FBSystem()
lStory = FBStory()
lConsole = FBMemo()

def StoreProperty(newdoc, top_element, prop):
    
    elem = newdoc.createElement("property")
    elem.setAttribute( "Name", prop.Name )
    
    propType = "None"
    value = ""
    try:
        data = prop.Data
        
        if type(data) is int:
            propType = "int"
            value = str(data)
        elif type(data) is float:
            propType = "float"
            value = str(data)
        elif type(data) is str:
            propType = "str"
            value = data
        elif type(data) is bool:
            propType = "bool"
            value = str(data)
        elif type(data) is FBTime:
            propType = "FBTime"
            value = data.GetTimeString()
        elif type(data) is FBColor:
            propType = "FBColor"
            value = str(data)
        elif type(data) is FBVector3d:
            propType = "FBVector3d"
            value = str(data)
        else:        
            value = str(data)
            
    except NotImplementedError:
        value = "undefined"

    elem.setAttribute( "Type", propType )
    elem.setAttribute( "Value", value )
    top_element.appendChild(elem)

# TODO: store clip resource path
def StoreStoryClip(newdoc, top_element, clip):
    
    elem = newdoc.createElement("clip")
    elem.setAttribute( "Name", clip.Name )

    resourcePath = ""
    
    if clip.PropertyList.Find("ClipVideoPath"):
        resourcePath = clip.PropertyList.Find("ClipVideoPath").Data
    elif clip.PropertyList.Find("ClipAudioPath"):
        resourcePath = clip.PropertyList.Find("ClipAudioPath").Data
    elif clip.PropertyList.Find("ClipAnimationPath"):
        resourcePath = clip.PropertyList.Find("ClipAnimationPath").Data
        
    elem.setAttribute( "Path", resourcePath )

    for prop in clip.PropertyList:
        StoreProperty(newdoc, elem, prop)
        
    top_element.appendChild(elem)

# TODO: store track type !!!
def StoreStoryTrack(newdoc, top_element, track):
    
    elem = newdoc.createElement("track")
    elem.setAttribute( "Name", track.Name )
    elem.setAttribute( "Type", str(track.Type) )
    
    for prop in track.PropertyList:
        StoreProperty(newdoc, elem, prop)
    
    for child in track.SubTracks:
        StoreStoryTrack(newdoc, elem, child)

    for child in track.Clips:
        StoreStoryClip(newdoc, elem, child)

    top_element.appendChild(elem)

def StoreStoryFolder(newdoc, top_element, folder):

    elem = newdoc.createElement("folder")
    elem.setAttribute( "Name", folder.Name )

    for prop in folder.PropertyList:
        StoreProperty(newdoc, elem, prop)

    for child in folder.Childs:
        StoreStoryFolder(newdoc, elem, child)
        
    for track in folder.Tracks:
        StoreStoryTrack(newdoc, elem, track)

    top_element.appendChild(elem)

def OnButtonSave(control, event):
    print "save story"

    impl = minidom.getDOMImplementation()
    
    newdoc = impl.createDocument(None, "Story", None)
    top_element = newdoc.documentElement
    text = newdoc.createTextNode(lApp.FBXFileName)
    top_element.appendChild(text)
        
    StoreStoryFolder(newdoc, top_element, lStory.RootFolder)

    
    res = open("C:\\base.xml", "w")
    res.writelines(newdoc.toprettyxml())
    res.close()

def FindStoryClipInScene(clipName):
    for comp in lSystem.Scene.Components:
        if (type(comp) is FBStoryClip) and (comp.Name == clipName):
            return comp
    return None

def OnButtonMerge(control, event):
    print "merge"
    
    xmldoc = minidom.parse("C:\\base.xml")
    allClipElems = xmldoc.getElementsByTagName("clip")
    for clipElem in allClipElems:
        clipName = clipElem.getAttribute("Name")
        clipPath = str(clipElem.getAttribute("Path"))

        clipComp = FindStoryClipInScene(clipName)
        
        if clipComp:
            RetrieveStoryClip(clipElem, clipComp)
            RetrieveStoryClip(clipElem, clipComp)
            print clipName
        else:
            print ("Failed to find - " + clipName)
        
        
def RetrieveProperty(propAttr, comp):
   
    name = str(propAttr.getAttribute("Name"))
    propType = propAttr.getAttribute("Type")
    value = str(propAttr.getAttribute("Value"))
    
    lTime = FBTime(0)
    
  
    lProperty = comp.PropertyList.Find(name)
    if lProperty:
        propValue = ""
        try:
            data = lProperty.Data
            propValue = str(data)
        except NotImplementedError:
            propValue = "undefined"
            
        if propValue != value:
            
            if propType == "int":
                lProperty.Data = int(value)
            elif propType == "float":
                lProperty.Data = float(value)
            elif propType == "bool":
                lProperty.Data = (value == "True")
            elif propType == "str":
                lProperty.Data = value
            elif propType == "FBTime":
                lTime.SetTimeString(value)
                lProperty.Data = lTime
                
            elif propType == "FBColor":
                lProperty.Data = FBColor(1.0, 1.0, 1.0)
            elif propType == "FBVector3d":
                lProperty.Data = FBVector3d(0.0, 0.0, 0.0)
            else:
                lProperty.Data = value
    
        
def strToTrackType(strTrackType):
    trackType = FBStoryTrackType.kFBStoryTrackAnimation
    if strTrackType == "kFBStoryTrackCamera":
        trackType = FBStoryTrackType.kFBStoryTrackCamera
    elif strTrackType == "kFBStoryTrackCharacter":
        trackType = FBStoryTrackType.kFBStoryTrackCharacter
    elif strTrackType == "kFBStoryTrackConstraint":
        trackType = FBStoryTrackType.kFBStoryTrackConstraint
    elif strTrackType == "kFBStoryTrackCommand":
        trackType = FBStoryTrackType.kFBStoryTrackCommand
    elif strTrackType == "kFBStoryTrackShot":
        trackType = FBStoryTrackType.kFBStoryTrackShot
    elif strTrackType == "kFBStoryTrackAudio":
        trackType = FBStoryTrackType.kFBStoryTrackAudio
    elif strTrackType == "kFBStoryTrackVideo":
        trackType = FBStoryTrackType.kFBStoryTrackVideo
        
    return trackType
        
def RetrieveStoryClip(top_element, parentClip):
    
    clipElem = top_element.firstChild
    while clipElem != None:
        
        if clipElem.nodeName == "property":
            RetrieveProperty(clipElem, parentClip)
        
        clipElem = clipElem.nextSibling
        
def FindVideoByPath(path):
    
    for lVideo in lSystem.Scene.VideoClips:
        if lVideo.Filename == path:
            return lVideo
    return None
        
def RetrieveStoryTrack(top_element, parentTrack):
    
    trackElem = top_element.firstChild
    while trackElem != None:
        
        if trackElem.nodeName == "property":
            RetrieveProperty(trackElem, parentTrack)   
            
        elif trackElem.nodeName == "track":
            strTrackType = trackElem.getAttribute("Type")
            trackName = trackElem.getAttribute("Name")
        
            trackType = strToTrackType(strTrackType)
                
            newTrack = FBStoryTrack(trackType, parentFolder)
        
            parentTrack.SubTracks.append(newTrack)
            newTrack.ParentTrack = newTrack
        
            RetrieveStoryTrack(trackElem, parentFolder, newTrack)
        
        elif trackElem.nodeName == "clip":
            clipName = trackElem.getAttribute("Name")
            clipPath = str(trackElem.getAttribute("Path"))
        
            # TODO: find media resource already defined in the scene
            lVideo = FindVideoByPath(clipPath)
            newClip = None
            if lVideo: 
                newClip = FBStoryClip(lVideo, parentTrack, FBTime(0)) 
            else: 
                newClip = FBStoryClip(clipPath, parentTrack, FBTime(0))
        
            RetrieveStoryClip(trackElem, newClip)
            RetrieveStoryClip(trackElem, newClip)

        
        trackElem = trackElem.nextSibling
                
def RetrieveStoryFolder(top_element, parentFolder):
    
    folderElem = top_element.firstChild
    while (folderElem != None):
    
        if folderElem.nodeName == "property":
            RetrieveProperty(folderElem, parentFolder)  
        
        elif folderElem.nodeName == "folder":
            folderName = str(folderElem.getAttribute("Name"))
        
            newFolder = FBStoryFolder(parentFolder)
            RetrieveStoryFolder(folderElem, newFolder)
            
        elif folderElem.nodeName == "track":
            strTrackType = folderElem.getAttribute("Type")
            trackName = folderElem.getAttribute("Name")
        
            trackType = strToTrackType(strTrackType)
        
            newTrack = FBStoryTrack(trackType, parentFolder)
            RetrieveStoryTrack(folderElem, newTrack)
            
        folderElem = folderElem.nextSibling
        
def OnButtonLoad(control, event):
    print "load story"

    xmldoc = minidom.parse("C:\\base.xml")
    allStoryElems = xmldoc.getElementsByTagName("Story")
    if len(allStoryElems) > 0:
        storyElem = allStoryElems[0]
        
        allFolderElems = storyElem.getElementsByTagName("folder")
        if len(allFolderElems) > 0:
            RetrieveStoryFolder(allFolderElems[0], lStory.RootFolder)

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
    b = FBButton()
    b.Caption = "Save Story As..."
    b.OnClick.Add(OnButtonSave)
    main.Add(b, 35)

    #
    b = FBButton()
    b.Caption = "Merge Story..."
    b.OnClick.Add(OnButtonMerge)
    main.Add(b, 35)

    #
    b = FBButton()
    b.Caption = "Load Story..."
    b.OnClick.Add(OnButtonLoad)
    main.Add(b, 35)

    #
    main.AddRelative(lConsole, 80)



def CreateTool():
    t = None
    try:
        t = FBCreateUniqueTool("Story IO Tool")
    except NameError:
        t = CreateUniqueTool("Story IO Tool")
        print "supporting MoBu 2010"
        
    if t:
        t.StartSizeX = 200
        t.StartSizeY = 400
        PopulateTool(t)
        
        if True == gDevMode:
            ShowTool(t)
        
CreateTool()