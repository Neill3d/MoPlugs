
#################################################################
#
# Script for removing layered textures for diffuse channel (export fbx issue)
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

   
def UpdateMaterial(mat):
    #
    if mat:
        lTexture = mat.GetTexture(FBMaterialTextureType.kFBMaterialTextureDiffuse)
        if lTexture and isinstance(lTexture, FBLayeredTexture):
            if len(lTexture.Layers) > 0:
                mat.SetTexture(lTexture.Layers[0], FBMaterialTextureType.kFBMaterialTextureDiffuse)    
        #
    #
        
l = FBModelList()
FBGetSelectedModels(l)

for obj in l:
    for mat in obj.Materials:
        UpdateMaterial(mat)
       
    for t in obj.Textures:
        UpdateTexture(t)
