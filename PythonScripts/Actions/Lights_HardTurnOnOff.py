
###########################################################
#   REMOVE LIGHTS FROM THE SCENE AND RECOVER THEM BACK  
# script to convert lights into template objects and do a back convertation
#
# Author Sergey Solohin (Neill3d) 2015
#   e-mail to: s@neill3d.com
#       www.neill3d.com
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
###########################################################

from pyfbsdk import *

gSystem = FBSystem()

gTemplateRootName = "TemplateLightCache"

############## PROCEDURES

def CopyAnimationNode(src, dst):
    
    if src.FCurve and dst.FCurve:
        print ("copy " + src.Name)

        dst.FCurve.KeyReplaceBy(src.FCurve)

def CopyCommonAnimationNodes(srcAnimNode, dstObj):
    
    if srcAnimNode:
        
        dstIndices = []
        currNdx = 0
        for i in range(0, len(srcAnimNode.Nodes)):
            node = srcAnimNode.Nodes[i]
            nodeName = node.Name
            print nodeName
            
            dstIndices.append(-1)
            
            if nodeName == "Lcl Translation":
                dstObj.Translation.SetAnimated(True)
                dstIndices[i] = currNdx
                currNdx += 1
            elif nodeName == "Lcl Rotation":
                dstObj.Rotation.SetAnimated(True)
                dstIndices[i] = currNdx
                currNdx += 1
            elif nodeName == "Lcl Scaling":
                dstObj.Scaling.SetAnimated(True)
                dstIndices[i] = currNdx
                currNdx += 1
            elif nodeName == "Visibility":
                dstObj.Visibility.SetAnimated(True)
                dstIndices[i] = currNdx
                currNdx += 1
                        
        for i in range(0, len(srcAnimNode.Nodes)):
            
            srcnode = srcAnimNode.Nodes[i]
            
            if dstIndices[i] >= 0:
                ndx = dstIndices[i]
                dstnode = dstObj.AnimationNode.Nodes[ndx]
    
                if len(srcnode.Nodes) > 0:
                    for j in range(0, len(srcnode.Nodes)):
                        CopyAnimationNode(srcnode.Nodes[j], dstnode.Nodes[j])
                else:
                    CopyAnimationNode(srcnode, dstnode)


def CopyPropertyColor(srcProp, dstProp):
    
    dstProp.Data = srcProp.Data
    
    if srcProp.GetAnimationNode():
        dstProp.SetAnimated(True)
        
        srcNode = srcProp.GetAnimationNode()
        dstNode = dstProp.GetAnimationNode()
        
        for i in range(0, 3):
            CopyAnimationNode(srcNode.Nodes[i], dstNode.Nodes[i])
        
def CopyPropertyDouble(srcProp, dstProp):

    dstProp.Data = srcProp.Data
    
    if srcProp.GetAnimationNode():
        dstProp.SetAnimated(True)
        
        srcNode = srcProp.GetAnimationNode()
        dstNode = dstProp.GetAnimationNode()
        
        CopyAnimationNode(srcNode, dstNode)

def LightToNull(light):
    
    obj = FBModelNull(light.Name + "_template")

    obj.Translation.Data = light.Translation.Data
    obj.Rotation.Data = light.Rotation.Data
    obj.Scaling.Data = light.Scaling.Data
    obj.Visibility.Data = light.Visibility.Data

    CopyCommonAnimationNodes(light.AnimationNode, obj)
    
    # light type
    prop = obj.PropertyCreate('LightType', FBPropertyType.kFBPT_int, 'Integer', False, True, None)
    prop.Data = int(light.LightType)
    
    # cast light on object
    prop = obj.PropertyCreate('CastLightOnObject', FBPropertyType.kFBPT_bool, 'Bool', False, True, None)
    prop.Data = light.CastLightOnObject
    
    # color
    prop = obj.PropertyCreate('DiffuseColor', FBPropertyType.kFBPT_ColorRGB, 'Color', True, True, None)
    CopyPropertyColor(light.DiffuseColor, prop)
    
    # intensity
    prop = obj.PropertyCreate('Intensity', FBPropertyType.kFBPT_double, 'Number', True, True, None)
    CopyPropertyDouble(light.Intensity, prop)
    
    # outer angle
    prop = obj.PropertyCreate('ConeAngle', FBPropertyType.kFBPT_double, 'Number', True, True, None)
    CopyPropertyDouble(light.ConeAngle, prop)

    prop = obj.PropertyCreate('InnerAngle', FBPropertyType.kFBPT_double, 'Number', True, True, None)
    CopyPropertyDouble(light.InnerAngle, prop)
    
    prop = obj.PropertyCreate('OuterAngle', FBPropertyType.kFBPT_double, 'Number', True, True, None)
    CopyPropertyDouble(light.OuterAngle, prop)
    
    
    return obj
    
    
def NullToLight(null):
    
    name = null.Name
    name = name.replace("_template", "")
    
    light = FBLight(name)
    light.Show = True
    
    light.Translation.Data = null.Translation.Data
    light.Rotation.Data = null.Rotation.Data
    light.Scaling.Data = null.Scaling.Data
    light.Visibility.Data = null.Visibility.Data
    
    CopyCommonAnimationNodes(null.AnimationNode, light)
    
    light.LightType = FBLightType( null.PropertyList.Find('LightType').Data )
    light.CastLightOnObject = null.PropertyList.Find('CastLightOnObject').Data
    CopyPropertyColor( null.PropertyList.Find('DiffuseColor'), light.DiffuseColor )
    CopyPropertyDouble( null.PropertyList.Find('Intensity'), light.Intensity )
    CopyPropertyDouble( null.PropertyList.Find('ConeAngle'), light.ConeAngle )
    CopyPropertyDouble( null.PropertyList.Find('InnerAngle'), light.ConeAngle )
    CopyPropertyDouble( null.PropertyList.Find('OuterAngle'), light.ConeAngle )    
    
    return light

############## MAIN

gLightCache = FBFindModelByLabelName(gTemplateRootName)

if gLightCache != None:
    # restore cache or clear
    
    objects = []
    
    for child in gLightCache.Children:
        NullToLight(child)
        objects.append(child)
    
    map (FBComponent.FBDelete, objects)
    gLightCache.FBDelete()
        
else:
    
    newRoot = FBModelNull(gTemplateRootName) 

    lights = []    
    for light in gSystem.Scene.Lights:
        newNull = LightToNull(light)
        newNull.Parent = newRoot

        lights.append(light)    
    

    map( FBComponent.FBDelete, lights )
    