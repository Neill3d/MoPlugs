
#
# script to replace media file for all track clips (for selected track in Story Editor)
#
#   Author Sergey Solokhin (Neill3d) May 2017
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *
from xml.dom import minidom

lApp = FBApplication()
lStory = FBStory()
lRootFolder = lStory.RootFolder

lNewMediaPath = ""

lClipPropNames = ['ImageSequence',
        'UseSystemFrameRate',
        'FrameRate',
        'Start',
        'Stop',
        'ExtractStart',
        'ExtractStop',
        'FirstLoopMarkIn',
        'LastLoopMarkOut',
        'Speed',
        'ClipVideoPath']

########################### FUNCTIONS

def StoreProperty(newdoc, top_element, prop):
    
    elem = newdoc.createElement("Property")
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
            tempValue = data.GetTimeString()
            # fix negative value issue, remove spaces
            value = tempValue.replace(" ", "")
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

def RetrieveProperty(propAttr, comp):
   
    name = str(propAttr.getAttribute("Name"))
  
    if name == 'ClipVideoPath':
        return
  
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
                print lTime
            elif propType == "FBColor":
                lProperty.Data = FBColor(1.0, 1.0, 1.0)
            elif propType == "FBVector3d":
                lProperty.Data = FBVector3d(0.0, 0.0, 0.0)
            else:
                lProperty.Data = value

def ChangeVideoClipImage(clipImage):
    
    print clipImage.Filename
    clipImage.Filename = lNewMediaPath

    
def CheckClip(clip, outClips):
    
    newValue = []
    
    for propName in lClipPropNames:
        newValue.append( clip.PropertyList.Find(propName).Data )
    
    outClips.append(newValue)
    
    '''
    for i in range(0, clip.GetSrcCount()):
        lSrc = clip.GetSrc(i)
        
        for j in range(0, lSrc.GetSrcCount() ):
            lVideo = lSrc.GetSrc(j)
            
            
            if lVideo.ClassName() == 'FBVideoClipImage':
                ChangeVideoClipImage(lVideo)
    '''

def SaveClip(newdoc, top_element, clip):
    
    elem = newdoc.createElement("Clip")
    elem.setAttribute( "Name", clip.Name )
    
    for propName in lClipPropNames:
        prop = clip.PropertyList.Find(propName)
        StoreProperty(newdoc, elem, prop) 

    top_element.appendChild(elem)

def SaveTrack(newdoc, top_element, track):
                
    for subtrack in track.SubTracks:
        SaveTrack(newdoc, top_element, subtrack)

    if track.Selected == True and track.Type == FBStoryTrackType.kFBStoryTrackVideo:

        elem = newdoc.createElement("Track")
        elem.setAttribute( "Name", track.Name )

        print track.Name        
        
        for clip in track.Clips:
            SaveClip(newdoc, elem, clip)    
            
        top_element.appendChild(elem)

def IsTrackExist(track, searchName):
    
    for subtrack in track.SubTracks:

        res = IsTrackExist(subtrack, searchName)
        if res != None:
            return res
            
    if track.Selected == True and track.Type == FBStoryTrackType.kFBStoryTrackVideo:

        if track.Name == searchName:
            return track
    
    return None

def CheckTrack(track):
                
    for subtrack in track.SubTracks:
        CheckTrack(subtrack)

    if track.Selected == True and track.Type == FBStoryTrackType.kFBStoryTrackVideo:

        print track.Name        
        lClips = []
        clipsToDelete = []    
        
        for clip in track.Clips:
            CheckClip(clip, lClips)    
            
            clipsToDelete.append(clip)

        # remove track clips

        map( FBComponent.FBDelete, clipsToDelete )
        del (clipsToDelete)
        
        # add new clips with new media path
        
        for clipData in lClips:
            newClip = FBStoryClip( lNewMediaPath, track, clipData[3] )
            
            for i in range(0, len(lClipPropNames)):
                
                propName = lClipPropNames[i]
                prop = newClip.PropertyList.Find(propName)
                
                if prop != None:
                    prop.Data = clipData[i]
    
def StoreSelectedTracks():

    lDialog = FBFilePopup()
    lDialog.Style = FBFilePopupStyle.kFBFilePopupSave
    lDialog.Filter = '*.xml'
    
    # grab new image filename
    if lDialog.Execute():
        #lNewMediaPath = lDialog.FullFilename
    
        lXmlPath = lDialog.FullFilename
        
        impl = minidom.getDOMImplementation()
            
        newdoc = impl.createDocument(None, "Story", None)
        top_element = newdoc.documentElement
        text = newdoc.createTextNode(lApp.FBXFileName)
        top_element.appendChild(text)

    
        for child in lRootFolder.Childs:
            
            for track in child.Tracks:
                SaveTrack(newdoc, top_element, track)
                
        for track in lRootFolder.Tracks:
            SaveTrack(newdoc, top_element, track) 
    
        res = open(lXmlPath, "w")
        res.writelines(newdoc.toprettyxml())
        res.close()
    
# search for track names and replace media clips

def ReStoreClip(storyTrack, mediaPath, top_element):
    
    newClip = FBStoryClip(mediaPath, storyTrack, FBTime(0) )
    
    elem = top_element.firstChild
    while elem != None:
        
        if elem.nodeName == "Property":
            RetrieveProperty(elem, newClip)
        
        elem = elem.nextSibling
    

def ReStoreTrack(storyTrack, trackElem):
    
    lDialog = FBFilePopup()
    lDialog.Caption = storyTrack.Name
    lDialog.Style = FBFilePopupStyle.kFBFilePopupOpen
    
    if lDialog.Execute():
        
        lMediaPath = lDialog.FullFilename
        
        # remove clips
        
        clipsToDelete = []
        for clip in storyTrack.Clips:
            clipsToDelete.append(clip)
            
        map( FBComponent.FBDelete, clipsToDelete)
        del( clipsToDelete )
        
        # load the same clips but with a specified media file
    
        elem = trackElem.firstChild    
        while elem != None:
            
            if elem.nodeName == "Clip":
                ReStoreClip(storyTrack, lMediaPath, elem)
            
            elem = elem.nextSibling

def ReStoreStoryTracks(top_element):
    
    trackElem = top_element # top_element.firstChild
    while trackElem != None:
        
        if trackElem.nodeName == "Track":
        
            trackName = trackElem.getAttribute("Name")

            # search if there is a track with that name
            lStoryTrack = None
            
            for child in lRootFolder.Childs:
              
                
                for track in child.Tracks:
                    res = IsTrackExist(track, trackName)
                    if res != None:
                        lStoryTrack = res
                        break
                
                if lStoryTrack != None:
                    break
                
            for track in lRootFolder.Tracks:
                res = IsTrackExist(track, trackName)
                if res != None:
                    lStoryTrack = res
                    break 
            
            if lStoryTrack != None:
                print "we have found a track to merge"
                print lStoryTrack.Name
        
                ReStoreTrack( lStoryTrack, trackElem )
        
        trackElem = trackElem.nextSibling

def ReStoreTracks():
    
    lDialog = FBFilePopup()
    lDialog.Style = FBFilePopupStyle.kFBFilePopupOpen
    lDialog.Filter = '*.xml'
    
    # grab new image filename
    if lDialog.Execute():
        
        lXmlPath = lDialog.FullFilename
        
        xmldoc = minidom.parse(lXmlPath)
        allStoryElems = xmldoc.getElementsByTagName("Track")

        if len(allStoryElems) > 0:
            ReStoreStoryTracks(allStoryElems[0])
    
####################### MAIN

'''
lTime = FBTime(0,0,0,-1710)


strValue = lTime.GetTimeString()
print strValue

lTime2 = FBTime(0)
lTime2.SetTimeString(strValue)

frameIdx = lTime2.GetFrame()
print frameIdx

'''
value = FBMessageBox( 'Story Clips IO', 'Please choose an action to do', 'Store Selected Tracks', 'Restore Selected Tracks', 'Cancel')


if value == 1:
    StoreSelectedTracks()
elif value == 2:
    ReStoreTracks()    
    
