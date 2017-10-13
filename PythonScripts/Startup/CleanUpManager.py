

#################################################################
#
# Script for cleaning unused and dublicated content in the scene
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
# Author: Sergey Solohin (Neill3d) 2014, e-mail to: s@neill3d.com
#  www.neill3d.com
#
# TODO:
#   - remove layered replaces only diffuse channel in the material
#   - remove dublicates in textures replaces only diffuse channel
#   - CompareMaterials function compares only ambient, diffuse colors and diffuse textures
#
# LOG
#   16.01.2014 - fix for dynamic textures and videos
#   11.05.2014 - check dublicate textures and video clips
#   29.03.2014 * skip deleting video clips that take part in story tracks
#   28.03.2014 - first version of a script
#
#
#################################################################


from pyfbsdk import *
from pyfbsdk_additions import *
from os.path import dirname, basename, splitext, exists, split
import shutil

from StringIO import StringIO
import sys

gDEVELOPMENT = True

CLEANUP_MANAGER_VERSION = 0.61

gApp = FBApplication()
gSystem = FBSystem()

gConsole = FBMemo()
gStrings = FBStringList()

materialTextureTypes = [FBMaterialTextureType.kFBMaterialTextureAmbient, FBMaterialTextureType.kFBMaterialTextureAmbientFactor, 
    FBMaterialTextureType.kFBMaterialTextureBump, FBMaterialTextureType.kFBMaterialTextureDiffuse, FBMaterialTextureType.kFBMaterialTextureDiffuseFactor, 
    FBMaterialTextureType.kFBMaterialTextureEmissive, FBMaterialTextureType.kFBMaterialTextureEmissiveFactor, FBMaterialTextureType.kFBMaterialTextureNormalMap,
    FBMaterialTextureType.kFBMaterialTextureReflection, FBMaterialTextureType.kFBMaterialTextureReflectionFactor, FBMaterialTextureType.kFBMaterialTextureShiness, 
    FBMaterialTextureType.kFBMaterialTextureSpecular, FBMaterialTextureType.kFBMaterialTextureSpecularFactor, FBMaterialTextureType.kFBMaterialTextureTransparent, 
    FBMaterialTextureType.kFBMaterialTextureTransparentFactor]

#/////////////////////////////////////////////////////////////////////////////////////////////////////////
#// COMMON 
        
def cclear():
    global gStrings
    global gConsole
   
    gStrings.Clear()
    gStrings.Add( ">>>" )
    gConsole.SetStrings(gStrings)
    
def cprint(text):
    global gStrings
    global gConsole
    
    gStrings.Add(text)
    gConsole.SetStrings(gStrings)
    gConsole.Refresh()
    

#/////////////////////////////////////////////////////////////////////////////////////////////////////////
#// 

def TextureGetVideoPath(texture):
    
    if isinstance(texture, FBLayeredTexture):
        if len(texture.Layers) > 0:
            return TextureGetVideoPath(texture.Layers[0])
        
    elif isinstance(texture, FBTexture):
        video = texture.Video
        if video != None:
            if isinstance(video, FBVideoMemory):
                return video.Name
            elif isinstance(video, FBVideoClip):
                fname = video.Filename
                return fname
    return ""
        

def CompareTextures(texture1, texture2):
    
    path1 = TextureGetVideoPath(texture1)
    path2 = TextureGetVideoPath(texture2)
    
    if path1 == None: 
        print "FAILED to find path for texture1"
        return False
        
    if path2 == None: 
        print "FAILED to find path for texture2"
        return False
    
    if path1 != path2: return False
    
    return True

def CompareColors(color1, color2):
    if color1[0] != color2[0] or color1[1] != color2[1] or color1[2] != color2[2]:
        return False
        
    return True

def CompareMaterials(mat1, mat2):
    
    #print "compare " + mat1.Name + " and " + mat2.Name
    
    if False == CompareColors(mat1.Ambient, mat2.Ambient): 
        #print "ambient failed"
        return False
    if False == CompareColors(mat1.Diffuse, mat2.Diffuse): 
        #print "diffuse failed"
        return False
    if False == CompareTextures(mat1.GetTexture(FBMaterialTextureType.kFBMaterialTextureDiffuse), mat2.GetTexture(FBMaterialTextureType.kFBMaterialTextureDiffuse)):
        #print "diffuse textures failed"
        return False
    
    #print "THE SAME"
    return True
    
# replace all connections with src material to the dst material
def ReplaceMaterial( srcmat, dstmat ):
    
    result = False
    count = srcmat.GetDstCount()
    
    dstList = []
    for i in range(0, count):
        dstList.append(srcmat.GetDst(i))
    
    for dst in dstList:
        
        if isinstance(dst, FBModel):
            
            temp = []
            for mat in dst.Materials:
                temp.append(mat)
                
                if mat.Name == dstmat.Name:
                    # to avoid material dublication, lets keep several copies of material
                    result = False
                    return result

                
            dst.Materials.removeAll()
            
            for mat in temp:
                if mat.Name == srcmat.Name:
                    print "REPLACE " + srcmat.Name + " with " + dstmat.Name 
                    dst.Materials.append(dstmat)                   
                    result = True
                else:
                    #print "adding - " + mat.Name
                    dst.Materials.append(mat)  
                    
            del (temp)                  
                    
    return result

def ReplaceTexture( textureToReplace, textureToPut ):
    count = textureToReplace.GetDstCount()
    dstArray = []
    for j in range(count):
        dstArray.append( textureToReplace.GetDst(j) )
        
    for dst in dstArray:
     
        if dst.Name == "DiffuseColor" and isinstance(dst, FBProperty):
            owner = dst.GetOwner()
       
            if owner and isinstance(owner, FBMaterial):
                owner.SetTexture(textureToPut, FBMaterialTextureType.kFBMaterialTextureDiffuse)
                
            #
        #

def DoCleanUp():

    cclear()
    cprint( "== CLEANING OPERATION == " )
    cprint( "" )

    global popupControls
    CleanUpMaterials =           (popupControls[0].State == 1)
    CleanUpMaterialsDublicates = (popupControls[1].State == 1)
    CleanUpMaterialsUnUsed =    (popupControls[2].State == 1)
    CleanUpMaterialsDelete =    (popupControls[3].State == 1)
    CleanUpTextures =           (popupControls[4].State == 1)
    CleanUpTexturesDublicates = (popupControls[5].State == 1)
    CleanUpTexturesReplaceLayered = (popupControls[6].State == 1)
    CleanUpTexturesUnUsed =         (popupControls[7].State == 1)
    CleanUpTexturesDelete =         (popupControls[8].State == 1)
    CleanUpVideoClips =             (popupControls[9].State == 1)
    CleanUpVideoClipsDublicates =   (popupControls[10].State == 1)
    CleanUpVideoClipsUnUsed =       (popupControls[11].State == 1)
    CleanUpVideoClipsDelete =       (popupControls[12].State == 1)
    CleanUpShaders =            (popupControls[13].State == 1)
    CleanUpShadersDelete =      (popupControls[14].State == 1)
    numDeletedClips = 0

    #
    # process textures
    #
    modelList = FBModelList()
    FBGetSelectedModels(modelList)
    videoList = []
    videoFlags = []
    if CleanUpVideoClips:
        for lvideo in gSystem.Scene.VideoClips:
            if isinstance(lvideo, FBVideoClip) and not isinstance(lvideo, FBVideoMemory):
                videoList.append(lvideo)
                videoFlags.append(False)
            
    textureList = []
    textureFlags = []
    if CleanUpTextures:
        if len(modelList) > 0:
            for lModel in modelList:
                for lMat in lModel.Materials: 
                    ProcessMaterial(textureList, textureFlags, lMat)
        else:
            for texture in gSystem.Scene.Textures:
                textureList.append(texture)
                textureFlags.append(False)
          
    materialList = []
    materialFlags = []
    if CleanUpMaterials:
        for lMaterial in gSystem.Scene.Materials:
            if lMaterial.Name == "DefaultMaterial":
                continue
                
            materialList.append(lMaterial)     
            materialFlags.append(False)
        # next
    
    shaderList = []
    if CleanUpShaders:
        for lShader in gSystem.Scene.Shaders:

            if lShader.Name != "Default Shader":
                shaderList.append(lShader)

    # process materials (cleanup dublicated)
    numDeletedMaterials = 0
    
    # process materials (cleanup unused)
    #CleanUpMaterialsUnUsed = True
    if CleanUpMaterialsUnUsed:
        for i in range(len(materialList)):
            
            if materialFlags[i] == True:
                continue
                
            lMaterial = materialList[i]
            
            count = lMaterial.GetDstCount()
            IsConnected = False
            for j in range(count):
                dst = lMaterial.GetDst(j)
                
                if isinstance(dst, FBModel):
                    IsConnected = True
                    break
                    
            if IsConnected == False:
                cprint( "Delete unused - " + lMaterial.Name )                
                materialFlags[i] = True

    
    #CleanUpMaterialsDublicates = True
    if CleanUpMaterialsDublicates:
        progress = FBProgress()
        
        progress.Caption = 'CleanUp'
        progress.Text = 'Search for material dublicates'
        progress.ProgressBegin()
        
        for i in range(len(materialList)):
            
            # skip deleted material
            if materialFlags[i] == True:
                continue
            
            # check material in dict (with flag for deleting and skip in comparing operation)
            for j in range(len(materialList)):
                if i == j or materialFlags[j] == True:
                    continue
                    
                # delete j material if it's equeal to i'th material
                if CompareMaterials(materialList[i], materialList[j]) == True:
                    
                    # delete only if this material is unique!
                    if ReplaceMaterial(materialList[j], materialList[i]):
                        materialFlags[j] = True
                progress.Percent = int(1.0 * i / len(materialList))
            
        progress.ProgressDone()
    
    # delete materials after comparing
    for i in range(len(materialList)):
        if materialFlags[i] == True:
            if CleanUpMaterialsDelete:
                materialList[i].FBDelete()
            numDeletedMaterials += 1
    
               
    #
    # process textures (cleanup unused)
    #
    #CleanUpTexturesDelete = True
    numDeletedTextures = 0 
    #print textureFlags                   
    for i in range(0, len(textureList)):
        
        if textureFlags[i] == True:
            continue
        
        texture = textureList[i]
        #print texture.Name
       
        if CleanUpTexturesReplaceLayered and isinstance(texture, FBLayeredTexture):
            #print "layered"
            if len(texture.Layers) == 0:
                
                textureFlags[i] = True
                #print "DELETE - " + str(i) + " - " + texture.Name
                #if CleanUpTexturesDelete:
                #    texture.FBDelete()
                numDeletedTextures += 1
                
            elif len(texture.Layers) == 1:
                #cleanup, no need to continue with layered texture
                ReplaceTexture( texture, texture.Layers[0] )
                texture.Layers.removeAll()
                #print "DELETE - " + str(i) + " - " + texture.Name
                
                textureFlags[i] = True
            else:
                print "MORE THEN ONE LAYER"
    
    #CleanUpTexturesUnUsed = True  
    #print textureFlags  
    if CleanUpTexturesUnUsed:
        for i in range(len(textureList)):
            texture = textureList[i]
            
            if  textureFlags[i] == True:
                continue
            #print texture.Name               
            count = texture.GetDstCount()
            
            IsConnected = False
            for j in range(count):
                dst = texture.GetDst(j)
                  
                if isinstance(dst, FBProperty):
                    IsConnected = True
                    break
            
            # check for dynamic textures (composite master, dynamic mask, etc.)
            count = texture.GetSrcCount()
            for j in range(count):
                src = texture.GetSrc(j)
                if isinstance(src, FBVideoMemory):
                    IsConnected = True
                    break
            
            if IsConnected == False:
                
                if isinstance(texture, FBLayeredTexture):
                    texture.Layers.removeAll()
                #print "DELETE - " + texture.Name
                textureFlags[i] = True
                continue
    
    # check dublicates
    if CleanUpTexturesDublicates:
        count = len(textureList)
        
        for i in range(count):
            iTexture = textureList[i]
            for j in range(count):
                jTexture = textureList[j]
                
                if (i!=j) and (textureFlags[i] == False) and (textureFlags[j] == False):
                    # compate and replace j with i
                    if CompareTextures(iTexture, jTexture):
                        
                        ReplaceTexture(jTexture, iTexture)
                        textureFlags[j] = True
    
    for idx in range(len(textureList)):
        if textureFlags[idx] == True:
            if CleanUpTexturesDelete:
                textureList[idx].FBDelete()
            numDeletedTextures += 1
    
    #
    # try to find unused video clips
    #
    
    # CleanUpVideoClipsDublicates = True
    if CleanUpVideoClipsDublicates:
        count = len(videoList)
        for i in range(count):
            iVideo = videoList[i]
            for j in range(count):
                jVideo = videoList[j]
                if (i!=j) and (videoFlags[i] == False) and (videoFlags[j] == False):
                    # compare and replace j with i
                    if (iVideo.Filename != "") and (iVideo.Filename == jVideo.Filename):
                        
                        dstCount = jVideo.GetDstCount()
                        
                        for k in range(dstCount):
                            dst = jVideo.GetDst(k)
                            
                            if isinstance(dst, FBScene):
                                continue
                            elif isinstance(dst, FBTexture):
                                # replace here
                                dst.Video = iVideo
                                videoFlags[j] = True
                                
                            elif isinstance(dst, FBComponent):
                                dstCount2 = dst.GetDstCount()
                                
                                for l in range(dstCount2):
                                    dst2 = dst.GetDst(l)
                                    
                                    if isinstance(dst2, FBScene):
                                        continue
                                    elif isinstance(dst2, FBTexture):
                                        # replace here
                                        dst2.Video = iVideo
                                        videoFlags[j] = True
    
    # CleanUpVideoClipsUnUsed = True
    if CleanUpVideoClipsUnUsed:
        for idx in range(len(videoList)):
            lVideo = videoList[idx]
            count = lVideo.GetDstCount()
            
            IsConnected = False
            for i in range(count):
                dst = lVideo.GetDst(i)
                
                if isinstance(dst, FBScene):
                    continue
                
                if isinstance(dst, FBComponent):
                    
                    count2 = dst.GetDstCount()
                    for j in range(count2):
                        dst2 = dst.GetDst(j)
                        
                        if isinstance(dst2, FBScene):
                            continue
                        if isinstance(dst2, FBTexture) or isinstance(dst2, FBStoryTrack) or isinstance(dst2, FBVideoClip):
                            IsConnected = True
                            break
                
                if isinstance(dst, FBTexture) or isinstance(dst, FBStoryTrack):
                    IsConnected = True
                    break
                        
            if IsConnected == False:
                videoFlags[idx] = True
            #
            
    for idx in range(len(videoList)):
        if videoFlags[idx] == True:
            if CleanUpVideoClipsDelete:
                videoList[idx].FBDelete()
            numDeletedClips += 1
    #
        
    #
    ##
    #
    numDeletedShaders = 0
    for lShader in shaderList:
        
        IsConnected = False
        count = lShader.GetDstCount()
        for i in range(count):
            dst = lShader.GetDst(i)
            
            if isinstance(dst, FBModel):
                IsConnected = True
                break
        
        if IsConnected == False:
            if CleanUpShadersDelete:
                lShader.FBDelete()
            numDeletedShaders += 1
        
    # print info
    msg = "Deleted materials - " + str(numDeletedMaterials) + ' of ' + str(len(materialList))
    msg = msg + "; \ndeleted textures - " + str(numDeletedTextures) + ' of ' + str(len(textureList))
    msg = msg + "; \ndeleted media clips - " + str(numDeletedClips) + ' of ' + str(len(videoList))
    msg = msg + "; \ndeleted shaders - " + str(numDeletedShaders) + ' of ' + str(len(shaderList))
    
    cprint(msg)
    
    FBMessageBox("Texture Manager", msg, "Ok" )

def ProcessTexture(somelist, flaglist, texture):
    if texture: 
        somelist.append(texture)
        flaglist.append(False)
        if isinstance(texture, FBLayeredTexture):
    
            for ltexture in texture.Layers:
                somelist.append(ltexture)
                flaglist.append(False)


def ProcessMaterial(textureList, flaglist, mat):
    
    for textureType in materialTextureTypes:
        ProcessTexture( textureList, flaglist, mat.GetTexture(textureType) )
    
    ##
         
        
def ButtonAboutClickEvent(control, event):
    FBMessageBox("About", "Version: " + str(CLEANUP_MANAGER_VERSION) + "\n\n Author Sergey Solohin (Neill3d) 2014-2017\n e-mail to:s@neill3d.com\n www.neill3d.com\n\nSpecial for Cam Christiansen", "Ok")
    

def ButtonDoItCallback(control, event):    
    DoCleanUp()

def ButtonDblClickEvent(control, event):
    print "DblClick"

def PopulateVBox(names, justify, box):
    global popupControls
    
    idx = 0
    for name in names:
        b = FBButton()
        b.Caption = name
        b.Style = FBButtonStyle.kFBCheckbox
        b.Look = FBButtonLook.kFBLookColorChange
        b.Hint = "Click to switch the button state"
        #b.Justify = FBTextJustify( justify[idx] )
        
        if justify[idx] == 0:
            b.SetStateColor( FBButtonState.kFBButtonState0, FBColor(0.2, 0.2, 0.2) )
            b.SetStateColor( FBButtonState.kFBButtonState1, FBColor(0.8, 0.2, 0.2) )
            box.Add(b, 35)
        else:
            b.SetStateColor( FBButtonState.kFBButtonState0, FBColor(0.5, 0.5, 0.5) )
            b.SetStateColor( FBButtonState.kFBButtonState1, FBColor(0.8, 0.2, 0.2) )
            b.State = 1
            box.Add(b, 25)
            
        popupControls.append(b)
        
        idx += 1
    #

def PopulateLayout(mainLyt):
    
    global popupControls
    popupControls = []
    
    # create regions
    width = 120
    
    main = FBHBoxLayout()
    
    # Shows how to create a FBVBoxLayout that grows from Top to bottom
    box = FBVBoxLayout(FBAttachType.kFBAttachTop)
    names = ["Process Materials", "Dublicates", "UnUsed", "Delete", ]
    justify = [0,                   2,          2,          2]
    
    PopulateVBox(names, justify, box)
    main.Add(box, 120)
    #
    box = FBVBoxLayout(FBAttachType.kFBAttachTop)
    names = ["Process Textures", "Dublicates", "Replace Layered", "UnUsed", "Delete"]
    justify = [0,                   2,              2,              2,          2]
    
    PopulateVBox(names, justify, box)
    main.Add(box, 120)
    #
    box = FBVBoxLayout(FBAttachType.kFBAttachTop)
    names = ["Process Video Clips", "Dublicate", "UnUsed", "Delete"]
    justify = [ 0,                      2,          2,          2]
    
    PopulateVBox(names, justify, box)
    main.Add(box, 120)
    #
    box = FBVBoxLayout(FBAttachType.kFBAttachTop)
    names = ["Process Shaders", "Delete" ]
    justify = [0,      2]
    
    PopulateVBox(names, justify, box)
    main.Add(box, 120)
    
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(200,FBAttachType.kFBAttachNone,"")
    
    mainLyt.AddRegion("main","main", x, y, w, h)
    mainLyt.SetControl("main",main)
    
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"main")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-40,FBAttachType.kFBAttachBottom,"")
    
    mainLyt.AddRegion("console","console", x, y, w, h)
    mainLyt.SetControl("console",gConsole)
    #gConsole.Style = FBListStyle.kFBVerticalList
    cclear()
    
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(-35,FBAttachType.kFBAttachBottom,"")
    w = FBAddRegionParam(-65,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    mainLyt.AddRegion("doit","doit", x, y, w, h)
    
    # connect controls
    b = FBButton()
    b.Caption = "CleanUp!"
    b.OnClick.Add(ButtonDoItCallback)    
    mainLyt.SetControl("doit", b)

    x = FBAddRegionParam(-60,FBAttachType.kFBAttachRight,"")
    y = FBAddRegionParam(-35,FBAttachType.kFBAttachBottom,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    mainLyt.AddRegion("about","about", x, y, w, h)
    
    # connect controls
    b = FBButton()
    b.Caption = "About"
    b.OnClick.Add(ButtonAboutClickEvent)    
    mainLyt.SetControl("about", b)

        
def CreateTool():    
    
    # Tool creation will serve as the hub for all other controls
    global t
    #t = FBCreateUniqueTool("CleanUp manager " + str(CLEANUP_MANAGER_VERSION) )
    t = FBCreateUniqueTool("CleanUp Manager Tool")
    t.StartSizeX = 530
    t.StartSizeY = 500
    PopulateLayout(t)    
    if gDEVELOPMENT:
        ShowTool(t)


CreateTool()
        
