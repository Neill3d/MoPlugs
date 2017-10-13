

#################################################################
#
# Script for managing textures and video clips
#
# Author: Sergey Solohin (Neill3d) 2013-2015, e-mail to: s@neill3d.com
#  www.neill3d.com
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
# TODO
#   - check Just_d_gate_03 - removing images for layered material
#   - add a function to locate from dds
#
# LOG
#   29.03.2014
#       + add load/save features
#       + huge update, remade most of functions
#   19.09.2013 - add support for deleting selected group of clips 
#
#
#################################################################


from pyfbsdk import *
from pyfbsdk_additions import *
from os.path import dirname, basename, splitext, exists, split, splitdrive, relpath
import shutil
import os
import re

from StringIO import StringIO
import sys

gDEVELOPMENT = False

TEXTURE_MANAGER_VERSION = 0.61

gApp = FBApplication()
gSystem = FBSystem()

gSpread = FBSpread()
gRowCount = 0
gCurrRow = -1

gConsole = FBMemo()
gStrings = FBStringList()
buttonReAssign = FBButton()

materialTextureTypes = [FBMaterialTextureType.kFBMaterialTextureAmbient, FBMaterialTextureType.kFBMaterialTextureAmbientFactor, 
    FBMaterialTextureType.kFBMaterialTextureBump, FBMaterialTextureType.kFBMaterialTextureDiffuse, FBMaterialTextureType.kFBMaterialTextureDiffuseFactor, 
    FBMaterialTextureType.kFBMaterialTextureEmissive, FBMaterialTextureType.kFBMaterialTextureEmissiveFactor, FBMaterialTextureType.kFBMaterialTextureNormalMap,
    FBMaterialTextureType.kFBMaterialTextureReflection, FBMaterialTextureType.kFBMaterialTextureReflectionFactor, FBMaterialTextureType.kFBMaterialTextureShiness, 
    FBMaterialTextureType.kFBMaterialTextureSpecular, FBMaterialTextureType.kFBMaterialTextureSpecularFactor, FBMaterialTextureType.kFBMaterialTextureTransparent, 
    FBMaterialTextureType.kFBMaterialTextureTransparentFactor]

#for property, value in vars(FBMaterialTextureType).iteritems():
#    print property, ": ", value

def ProcessVideoClip(scenePath, videoclip):
    path = dirname(videoclip.Filename)
    #print ("scenePath - " + scenePath)
    #print ("path - " + path)
    
    # compute difference and put it into the relative path
    dif = videoclip.Filename[len(scenePath): ]
    #print dif
    print videoclip.RelativePath
    

def Test():
    scenePath = dirname(gApp.FBXFileName)

    if scenePath == "":
        FBMessageBox("Absolute path to relative script", "You have to save your scene before doing this operation", "Ok")
    else:
        for clip in gSystem.Scene.VideoClips:
            ProcessVideoClip(scenePath, clip)
       
#/////////////////////////////////////////////////////////////////////////////////////////////////////////
#// COMMON 
        
def cclear():
    global gStrings
    global gConsole
   
    gStrings.Clear()
    gStrings.Add( ">>>" )
    gConsole.SetStrings(gStrings)
    
def cprint(text, doRefresh=False):
    global gStrings
    global gConsole
    
    gStrings.Add(text)
    gConsole.SetStrings(gStrings)
    
    if doRefresh:
        gConsole.Refresh()
    
def FilterVideoClip(clip):
    
    return ((clip is not None) and (True == isinstance(clip, FBVideoClip)) and (False == isinstance(clip, FBVideoMemory)))

def CollectTexture( videoClipsList, t ):

    if t:
        if isinstance(t, FBLayeredTexture):
            for layer in t.Layers:
                CollectTexture( videoClipsList, layer)
                
        elif isinstance(t, FBTexture):
            
            videoClip = t.Video

            if FilterVideoClip(videoClip):
                videoClipsList.append(videoClip)
                
        
def CollectMaterial(videoClipsList, mat):
    #
    if mat:
        for textureType in materialTextureTypes:
            CollectTexture( videoClipsList, mat.GetTexture(textureType) )
        

def PrepareVideoClipsList(videoClipsList):
    
    # try to collect textures only for selected models
    modelList = FBModelList()
    FBGetSelectedModels(modelList)
    
    paths = []
    for row in range(0, gRowCount):            
        if gSpread.GetRow(row).RowSelected:                 
            paths.append( gSpread.GetRow(row).Caption )
    
    selectedMaterials = 0
    selectedTextures = 0
    selectedVideoClips = 0
    selectedModels = len(modelList)
    
    for mat in gSystem.Scene.Materials:
        if mat.Selected:
            selectedMaterials += 1
    for texture in gSystem.Scene.Textures:
        if texture.Selected:
            selectedTextures += 1
    for video in gSystem.Scene.VideoClips:
        if video.Selected:
            selectedVideoClips += 1
    
    if len(paths) > 0:
        cprint ("collect from selected rows in spread sheet")
        
        for clip in gSystem.Scene.VideoClips:
            
            if FilterVideoClip(videoClip):
                lpath = dirname(clip.Filename)
        
                for toDelPath in paths:
                    if lpath == toDelPath:
                        
                        videoClipsList.append(clip)
                        break
    
    elif (selectedMaterials > 0) or (selectedTextures > 0) or (selectedVideoClips > 0):
    
        cprint ("collect clips from selected materials, textures, video clips")
    
        for mat in gSystem.Scene.Materials:
            if mat.Selected:
                CollectMaterial(videoClipsList, mat)
            
        for texture in gSystem.Scene.Textures:
            if texture.Selected:
                CollectTexture(videoClipsList, texture)
                                
        for video in gSystem.Scene.VideoClips:
            if FilterVideoClip(video) and video.Selected:
                videoClipsList.append(video)
    
    elif selectedModels == 0:
        cprint ("collect everything")
        
        for clip in gSystem.Scene.VideoClips:
            if FilterVideoClip(clip):
                videoClipsList.append(clip) 
            
    else:
        
        cprint ("collect from selected models")
        
        for obj in modelList:
            for mat in obj.Materials:
                CollectMaterial(videoClipsList, mat)
           
            for t in obj.Textures:
                CollectTexture(videoClipsList, t)
                
        
#/////////////////////////////////////////////////////////////////////////////////////////////////////////
#// RELOCATE OPERATION
        
relocate_totalCount = 0
relocate_processed = 0
relocate_errors = 0
        
def RelocateVideoClip(clip, newPath, method):
    
    global relocate_totalCount
    global relocate_processed
    global relocate_errors
    
    if FilterVideoClip(clip):
        try:
            clipPath = dirname(clip.Filename)
                    
            relocate_totalCount += 1
            
            if method == 2:
                clipName = clip.LongName
                clipName = clipName.replace(":", "_")
                fileName, fileExtension = os.path.splitext(clip.Filename)
                newFilename = newPath + '\\' + clipName + fileExtension
            
            elif method == 1:
            
                dif = clip.Filename[len(clipPath):]
                dif = dif.replace( "/", "\\" )

                if dif[0] != "\\" or dif[0] != "/":
                    dif = "\\" + dif
                newFilename = newPath + dif

            print ( "newFileName - " + newFilename )

            if exists(newFilename):
                
                clip.Filename = newFilename
                relocate_processed += 1
                        
            else:
                
                # renamee is the file getting renamed
                (root, ext) = os.path.splitext(newFilename)
                newFilename = root + ".dds"
                
                if exists(newFilename):
                    clip.Filename = newFilename
                    relocate_processed += 1
                else:
                    raise Exception ("Relocating video clip - Failed to locate " + clip.LongName + " in the new path")
                
        except Exception as e:
            cprint ("> ERROR " + str(e))
            relocate_errors += 1

    return 1

def ButtonRelocateClickEvent(control, event):
    
    global relocate_totalCount
    global relocate_processed
    global relocate_errors
    
    cclear()
    cprint ("> RELOCATE operation")
    
    videoClipsList = []
    PrepareVideoClipsList( videoClipsList )

    if len(videoClipsList) > 0:
        FolderDialog = FBFolderPopup()
        FolderDialog.Caption = 'Choose a path for searching textures'
        FolderDialog.Path = dirname(videoClipsList[0].Filename)
        
        if FolderDialog.Execute():
    
            method = FBMessageBox( "Relocate", "Please choose a relocate method", "Simple", "By Clip Name" )
    
            relocate_errors = 0
            relocate_totalCount = 0
            relocate_processed = 0
            
            # do stuff 
            for clip in videoClipsList:   
                RelocateVideoClip( clip, FolderDialog.Path, method )
                                
            ClearSpread(gSpread)
            PopulateSpread(gSpread)
            
            msg = "Relocated - " + str(relocate_processed) + " of " + str(relocate_totalCount)
            msg = msg + ";\n errors during operation - " + str(relocate_errors)
            
            cprint( "" )
            cprint( msg, True )
            
            FBMessageBox( "Relocate operation finished", msg, "Ok" )


#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#// COLLECT OPERATION

def ButtonCollectClickEvent(control, event):
    
    FolderDialog = FBFolderPopup()
    FolderDialog.Caption = 'Choose a path for putting all image files there'
    
    if FolderDialog.Execute():
      
        #
        method = FBMessageBox( "Collect operation", "Please choose a collect method", "Simple", "By Clip Name" )
        
        cclear()
        
        videoClipsList = []
        PrepareVideoClipsList(videoClipsList)
                
        # if nothing selected, then collect all textures
        
        numOfCollectedTextures = 0
        numOfErrorTextures = 0
        totalCount = len(videoClipsList)
        
        
      
        for clip in videoClipsList:
            if FilterVideoClip(clip):
                
                try:
                
                    fname = clip.Filename
                    if not exists(fname):
                        raise Exception( "file is not exist - " + fname )
                        
                    head, tail = split(fname)
                    newName = FolderDialog.Path + '\\' + tail
                
                    startFrame = clip.StartFrame
                    stopFrame = clip.StopFrame
                    
                    seqSource = []
                    seqFiles = []
                    seqDigits = []
                    
                    if startFrame < stopFrame:
                
                        files = os.listdir(head)
                        sameTxt = ''
                        mo = list(re.finditer('\d+', tail))
                        if len(mo) > 0:
                            txt = mo[len(mo)-1]
                            sameTxt = tail[:txt.start()]
                            #if tail[txt.end()] == '.':
                            #    cprint (txt.group())
                                
                        for theFile in files:
                            
                            sequence = False
                            same = False
                            digits = ''
                            
                            mo = list(re.finditer('\d+', theFile))
                            if len(mo) > 0:
                                txt = mo[len(mo)-1]
                                part = theFile[:txt.start()]
                                if part == sameTxt:
                                    same = True
                                if tail[txt.end()] == '.':
                                    sequence = True
                                    digits = '_' + txt.group()
                                    
                            if sequence == True and same == True:
                                seqFiles.append(theFile)
                                seqDigits.append(digits)
                    else:
                        seqFiles.append(tail)
                        seqDigits.append('')    
                    
                    for i in range(len(seqFiles)):
                        
                        tail = seqFiles[i]
                        digits = seqDigits[i]
                        
                        fname = head + '\\' + tail
                        newName = FolderDialog.Path + '\\' + tail
                        
                        if method == 3:
                            
                            relPath = relpath(head, folderPath)
                            print relPath
                            if (len(relPath) < len(folderPath)):
                                tail = relPath
                            
                            drive, tail = splitdrive(fname)
                            folderPath = FolderDialog.Path
                            print tail
                            
                            
                            newpath = FolderDialog.Path + '\\' + tail
                            head, tail = split(newpath)
                            
                            if not exists(head): os.makedirs(head)
                            print newpath
                            newName = newpath
    
                        elif method == 2:
                            fileName, fileExtension = os.path.splitext(tail)
                            clipName = clip.LongName
                            clipName = clipName.replace( ":", "_" )
                            newName = FolderDialog.Path + '\\' + clipName + digits + fileExtension
                
                        #
                        if fname != newName:
                            shutil.copy2(fname, newName)
                    
                            if buttonReAssign.State == 1:
                                clip.Filename = newName
                            
                            numOfCollectedTextures += 1
                    # NEXT FILE
                    
                except Exception as e:
                    cprint ("> ERROR " + str(e))
                    numOfErrorTextures += 1
            #
        #
        
        ClearSpread(gSpread)
        PopulateSpread(gSpread)
        
        msg = "Collected - " + str(numOfCollectedTextures) + " of " + str(totalCount)
        msg = msg + "\n Errors during operation - " + str(numOfErrorTextures)
        
        cprint( "" )
        cprint( msg, True )
        
        FBMessageBox( "Collect operation finished", msg, "Ok" )
    #

#//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#// ANALYZE

def TempAnalyze():
    FolderDialog = FBFolderPopup()
    FolderDialog.Caption = 'Choose folder for analyze.txt'
    
    if FolderDialog.Execute():
        f = open( FolderDialog.Path + '\\analyze.txt', 'w+' )
    
        for clip in gSystem.Scene.VideoClips:
            if FilterVideoClip(clip):
                
                f.write( 'Clip name - ' + clip.Name + '\n')
                
                fname = clip.Filename
                if not exists(fname):
                    f.write( '\t!clip filename - ' + fname + ' is not exist!\n' )
            #
        #

        f.close()

def ButtonAnalyzeClickEvent(control, event):
    
    cclear()
    
    videoClips = []
    PrepareVideoClipsList(videoClips)
    
    for clip in videoClips:
        if FilterVideoClip(clip):
            cprint(clip.Filename)
    
    cprint( "> Operation Finished", True )

#//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#// DELETE OPERATION

def DeleteVideoClip(paths):
    
    for clip in gSystem.Scene.VideoClips:
        
        if FilterVideoClip(clip):
        
            lpath = dirname(clip.Filename)
            
            for toDelPath in paths:
                
                if lpath == toDelPath:
                    
                    # try to remove connected textures
                    textures = []
                    
                    count = clip.GetDstCount()
                    for i in range(count):
                        textures.append(clip.GetDst(i))
                        
                    for dst in textures:
                        if isinstance(dst, FBTexture):
                            cprint( "texture - " + dst.Name + " deleted" )
                            dst.FBDelete()
                    
                    cprint ("clip - " + clip.Filename + " deleted")
                    clip.FBDelete()
                    break
    
    return 1



def ButtonDeleteClickEvent(control, event):
    
    if FBMessageBox("Texture Manager", "Are you sure you want to delete selected media?", "Yes, I'm sure", "No, thanks" ) == 1:
        
        cclear()

            # do stuff            
        paths = []
        for row in range(0, gRowCount):            
            if gSpread.GetRow(row).RowSelected: 
                
                paths.append( gSpread.GetRow(row).Caption )
            
        if len(paths) == 0:
            cprint( "no path selected in spread sheet" )
                
        try:
            
            DeleteVideoClip(paths)
            
        except Exception as e:
            cprint( "> ERROR " + str(e) )
        
        cprint( "> Operation finished", True )
        
        ClearSpread(gSpread)
        PopulateSpread(gSpread)


#/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#// REFRESH 

def UpdateVideoClip(video):
    fname = video.Filename
    video.Filename = fname

def ButtonRefreshTexturesClickEvent(control, event):        
    print "refresh textures"
    
    cclear()
        
    videoClipsList = []
    PrepareVideoClipsList(videoClipsList)

   
    for clip in videoClipsList:
        if FilterVideoClip(clip):
            cprint( "refresh " + clip.Name )
            UpdateVideoClip(clip)
                    
    
    msg = "Refreshed - " + str(len(videoClipsList))
    
    cprint( "" )
    cprint( msg, True )
    
    FBMessageBox( "Refresh operation finished", msg, "Ok" )

#/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#//


def ButtonReloadClickEvent(control, event):
    ClearSpread(gSpread)
    PopulateSpread(gSpread)

def Texture_TurnOnMipMap(texture):
    lProp = texture.PropertyList.Find("UseMipMap")
    if lProp:
        lProp.Data = True

def ButtonMipMapClickEvent(control, event):        

    cclear()
    cprint( "this operation is under construction" )
    #return

    # if we have selection in the scene, then turn on mipmaps for selected models only
    modelList = FBModelList()
    FBGetSelectedModels(modelList)
    
    if len(modelList) > 0:
    
        for lModel in modelList:
            for lMat in lModel.Materials:
                textureList = []
                CollectTextures(textureList, lMat)
                
                for lTexture in textureList:
                    Texture_TurnOnMipMap(lTexture)
    
    else:
        for lTexture in gSystem.Scene.Textures:
            Texture_TurnOnMipMap(lTexture)            
        
def ButtonAboutClickEvent(control, event):
    FBMessageBox("About", "Version: " + str(TEXTURE_MANAGER_VERSION) + "\n\n Author Sergey Solohin (Neill3d) 2014-2017\n e-mail to:s@neill3d.com\n www.neill3d.com\n\nSpecially for Cam Christiansen", "Ok")
    
def LoadFromFile(filename):
    
    cclear()
    
    videoClips = {}
    
    for video in gSystem.Scene.VideoClips:
        videoClips[video.Name] = video
    
    f = open( filename, 'r' )
    if f:
        lines = f.readlines()
    
        idx = 0
        while idx < len(lines):
            
            clipName = lines[idx].replace("\n", "")
            clipPath = lines[idx+1].replace("\n", "")
            
            if clipName in videoClips:
                cprint ( "key exist - " + clipName )
                videoClips[clipName].Filename = clipPath
            
            idx += 2
        
        f.close()
    
def SaveToFile(filename):

    cclear()
        
    videoClipsList = []
    PrepareVideoClipsList(videoClipsList)

    if len(videoClipsList) > 0:
        f = open( filename, 'w+' )
    
        if f:
            for video in videoClipsList:
                if FilterVideoClip(video):
                
                    f.write( video.Name + '\n' )
                    f.write( video.Filename + '\n' )
                #
            #
            f.close()
        
    cprint( "Save to file operation finished - " + filename )

def ButtonLoadClickEvent(control, event):
    
    lPopup = FBFilePopup()
    lPopup.Caption = "Choose file to load"
    lPopup.Filter = "*.txt"
    
    if lPopup.Execute():
        
        LoadFromFile(lPopup.FullFilename)
        
        ClearSpread(gSpread)
        PopulateSpread(gSpread)
    
def ButtonSaveClickEvent(control, event):
    
    lPopup = FBFilePopup()
    lPopup.Caption = "Choose file to save"
    lPopup.Filter = "*.txt"
    lPopup.Style = FBFilePopupStyle.kFBFilePopupSave
    
    if lPopup.Execute():
        
        SaveToFile(lPopup.FullFilename)

def OnSpreadEvent(control, event):
    global gCurrRow
    gCurrRow = event.Row
    
def ClearSpread(s):
    global gRowCount
    s.Clear()
    gRowCount = 0
    
    s.ColumnAdd("Number of textures")
    s.ColumnAdd("Textures list")
 
    c = s.GetColumn(-1)
    c.Width = 400
    c = s.GetColumn(0)
    c.Width = 120
    c = s.GetColumn(1)
    c.Width = 300
    
def PopulateSpread(s):
    global gRowCount
    
    texDic = {}

    for clip in gSystem.Scene.VideoClips:
        if FilterVideoClip(clip):
            path = dirname(clip.Filename)
            dif = clip.Filename[len(path):]
            
            if path in texDic.keys():
                texDic[path] = texDic[path] + ', ' + dif
            else:
                texDic[path] = dif
        
    i = 0
    for tex in texDic:
        print tex
        s.RowAdd(tex, i)
        gRowCount = i+1
        
        values = texDic[tex]
        l = values.split(',')
        
        s.SetCellValue(i, 0, str(len(l)))
        s.SetCellValue(i, 1, texDic[tex])
        
        i = i + 1
    



def PopulateLayout(mainLyt):

    button_width = 80

    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(button_width,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    mainLyt.AddRegion("button","button", x, y, w, h)

    x = FBAddRegionParam(5,FBAttachType.kFBAttachRight,"button")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(button_width,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    mainLyt.AddRegion("buttonCollect","buttonCollect", x, y, w, h)

    x = FBAddRegionParam(5,FBAttachType.kFBAttachRight,"buttonCollect")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(button_width,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    mainLyt.AddRegion("buttonReAssign","buttonReAssign", x, y, w, h)

    x = FBAddRegionParam(5,FBAttachType.kFBAttachRight,"buttonReAssign")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(button_width,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    mainLyt.AddRegion("buttonAnalyze","buttonAnalyze", x, y, w, h)

    x = FBAddRegionParam(5,FBAttachType.kFBAttachRight,"buttonAnalyze")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(button_width,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    mainLyt.AddRegion("buttonDelete","buttonDelete", x, y, w, h)

    x = FBAddRegionParam(5,FBAttachType.kFBAttachRight,"buttonDelete")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(button_width,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    mainLyt.AddRegion("buttonRefreshTextures","buttonRefreshTextures", x, y, w, h)

    x = FBAddRegionParam(5,FBAttachType.kFBAttachRight,"buttonRefreshTextures")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(button_width,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    mainLyt.AddRegion("buttonMipMaps","buttonMipMaps", x, y, w, h)

    x = FBAddRegionParam(15,FBAttachType.kFBAttachRight,"buttonMipMaps")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(button_width,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    mainLyt.AddRegion("buttonLoad","buttonLoad", x, y, w, h)

    x = FBAddRegionParam(5,FBAttachType.kFBAttachRight,"buttonLoad")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(button_width,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    mainLyt.AddRegion("buttonSave","buttonSave", x, y, w, h)

    x = FBAddRegionParam(-125,FBAttachType.kFBAttachRight,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    mainLyt.AddRegion("about","about", x, y, w, h)

    x = FBAddRegionParam(-125,FBAttachType.kFBAttachLeft,"about")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(120,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    mainLyt.AddRegion("buttonUI","buttonUI", x, y, w, h)

    x = FBAddRegionParam(0,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"button")
    w = FBAddRegionParam(0,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-100,FBAttachType.kFBAttachHeight, "", 0.8)
    mainLyt.AddRegion("main","main", x, y, w, h)

    x = FBAddRegionParam(0,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"main")
    w = FBAddRegionParam(0,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(0,FBAttachType.kFBAttachBottom,"")
    mainLyt.AddRegion("console","console", x, y, w, h)
    
    gSpread.Caption = "Texture path"
    mainLyt.SetControl("main",gSpread)

    mainLyt.SetControl("console", gConsole)
    #gConsole.Style = FBListStyle.kFBVerticalList
    cclear()
    
    b = FBButton()
    b.Caption = "Locate"
    b.Hint = "Select a path in a spread sheet to relocate only specified video clips, otherwise relocate all video clips in the scene"
    b.OnClick.Add(ButtonRelocateClickEvent)
    mainLyt.SetControl("button", b)

    b = FBButton()
    b.Caption = "Collect"
    b.Hint = "If items in the spread sheet selected, collect specified video clips to a new user path, if objects in the scene selected collect only their video clips otherwise collect all video clips in the scene"
    b.OnClick.Add(ButtonCollectClickEvent)
    mainLyt.SetControl("buttonCollect", b)

    #b = FBButton()
    buttonReAssign.Caption = "Reassign path"
    buttonReAssign.Style = FBButtonStyle.kFBCheckbox
    buttonReAssign.Hint = "This options controls Collect operation behaviour. Do you want to assign video clips from the new collected path?"
    buttonReAssign.State = 1
    mainLyt.SetControl("buttonReAssign", buttonReAssign)

    b = FBButton()
    b.Caption = "Analyze"
    b.OnClick.Add(ButtonAnalyzeClickEvent)
    mainLyt.SetControl("buttonAnalyze", b)
    b = FBButton()
    b.Caption = "Refresh UI"
    b.OnClick.Add(ButtonReloadClickEvent)
    b.Hint = "Refresh spread sheet visual control"
    mainLyt.SetControl("buttonUI", b)    
    b = FBButton()
    b.Caption = "Delete"
    b.Hint = "Select some item in spread sheet and use this operation to delete all specified video clips"
    b.OnClick.Add(ButtonDeleteClickEvent)
    mainLyt.SetControl("buttonDelete", b)
    b = FBButton()
    b.Caption = "Refresh"
    b.OnClick.Add(ButtonRefreshTexturesClickEvent)
    mainLyt.SetControl("buttonRefreshTextures", b)    
    b = FBButton()
    b.Caption = "MipMaps"
    b.OnClick.Add(ButtonMipMapClickEvent)
    mainLyt.SetControl("buttonMipMaps", b)   
    b = FBButton()
    b.Caption = "Load..."
    b.OnClick.Add(ButtonLoadClickEvent)
    mainLyt.SetControl("buttonLoad", b) 
    b = FBButton()
    b.Caption = "Save As..."
    b.OnClick.Add(ButtonSaveClickEvent)
    mainLyt.SetControl("buttonSave", b)  
    b = FBButton()
    b.Caption = "About"
    b.Hint = "Show information about author and contact"
    b.OnClick.Add(ButtonAboutClickEvent)
    mainLyt.SetControl("about", b)
    
    ClearSpread(gSpread)
 
    gSpread.MultiSelect = True
    gSpread.OnRowClick.Add(OnSpreadEvent)
 
    PopulateSpread(gSpread)
    
    
    
def CreateTool():    
    
    # Tool creation will serve as the hub for all other controls
    global t
    #t = FBCreateUniqueTool("Textures manager " + str(TEXTURE_MANAGER_VERSION) )
    t = FBCreateUniqueTool("Textures Manager Tool")
    t.StartSizeX = 1200
    t.StartSizeY = 500
    PopulateLayout(t)    
    if gDEVELOPMENT:
        ShowTool(t)


CreateTool()
        
