
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

lSystem = FBSystem()

lDevice = None

for theDevice in lSystem.Scene.Devices:
    lDevice = theDevice

if lDevice:

    lprop = lDevice.PropertyList.Find("Time Code")
    if lprop:
        newModel = FBModelCube("newCube")
        trackerAttr = newModel.PropertyCreate("myProp", FBPropertyType.kFBPT_Reference, "", True, True, lprop)