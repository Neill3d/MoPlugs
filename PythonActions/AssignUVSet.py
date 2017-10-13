
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

lSystem = FBSystem()

for lTexture in lSystem.Scene.Textures:
    if lTexture.Selected:
        lProp = lTexture.PropertyList.Find('UVSet')
        if lProp:
            lProp.Data = 'lightmap'