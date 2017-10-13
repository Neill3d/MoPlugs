
#################################################################
#
# Script for reloading models textures and updating in the viewport
#
# Author: Sergey Solohin (Neill3d) 2013, e-mail to: s@neill3d.com
#  www.neill3d.com
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
#################################################################

from pyfbsdk import *

gSystem = FBSystem()


def UpdateTexture(t):
    if t and isinstance(t.Video, FBVideoClip):
        fname = t.Video.Filename
        t.Video.Filename = fname
        
def UpdateMaterial(mat):
    #
    if mat:
        UpdateTexture( mat.GetTexture(FBMaterialTextureType.kFBMaterialTextureEmissive) )
        UpdateTexture( mat.GetTexture(FBMaterialTextureType.kFBMaterialTextureEmissiveFactor) )
        UpdateTexture( mat.GetTexture(FBMaterialTextureType.kFBMaterialTextureAmbient) )
        UpdateTexture( mat.GetTexture(FBMaterialTextureType.kFBMaterialTextureAmbientFactor) )
        UpdateTexture( mat.GetTexture(FBMaterialTextureType.kFBMaterialTextureDiffuse) )
        UpdateTexture( mat.GetTexture(FBMaterialTextureType.kFBMaterialTextureDiffuseFactor) )
        UpdateTexture( mat.GetTexture(FBMaterialTextureType.kFBMaterialTextureSpecular) )
        UpdateTexture( mat.GetTexture(FBMaterialTextureType.kFBMaterialTextureSpecularFactor) )
        UpdateTexture( mat.GetTexture(FBMaterialTextureType.kFBMaterialTextureShiness) )
        UpdateTexture( mat.GetTexture(FBMaterialTextureType.kFBMaterialTextureBump) )
        UpdateTexture( mat.GetTexture(FBMaterialTextureType.kFBMaterialTextureNormalMap) )
        UpdateTexture( mat.GetTexture(FBMaterialTextureType.kFBMaterialTextureTransparent) )
        UpdateTexture( mat.GetTexture(FBMaterialTextureType.kFBMaterialTextureTransparentFactor) )
        UpdateTexture( mat.GetTexture(FBMaterialTextureType.kFBMaterialTextureReflection) )
        UpdateTexture( mat.GetTexture(FBMaterialTextureType.kFBMaterialTextureReflectionFactor) )
        #UpdateTexture( mat.GetTexture(FBMaterialTextureType.kFBMaterialTextureDisplacementColor) )

l = FBModelList()
FBGetSelectedModels(l)

for obj in l:
    for mat in obj.Materials:
        UpdateMaterial(mat)
       
    for t in obj.Textures:
        UpdateTexture(t)
