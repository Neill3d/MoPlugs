
#####################################
#
# Script for turning off/on ligts in the scene
#
# Sergey Solohin (Neill3d) 2015
#   e-mail to: s@neill3d.com
#       www.neill3d.com
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
#####################################

from pyfbsdk import *

gSystem = FBSystem()

userChoice = FBMessageBox( "Lights", "What do you want to do with lights?", "Turn On", "Turn Off", "Cancel")

if userChoice == 1:
    for theLight in gSystem.Scene.Lights:
        theLight.CastLightOnObject = True
elif userChoice == 2:
    for theLight in gSystem.Scene.Lights:
        theLight.CastLightOnObject = False