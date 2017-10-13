
# script for reconnecting joystick to car sim
#
# There is a bug in Mobu, when joystick devices loose control after scene load
#
# Sergey Solohin (2016)
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

gSystem = FBSystem()
gScene = gSystem.Scene

gCon = None

# Define a utility function to look up a named animation node
# under a given parent. Will not recurse in the hierarchy.
def FindAnimationNode( pParent, pName ):
    lResult = None
    for lNode in pParent.Nodes:
        print lNode.Name
        if lNode.Name == pName:
            lResult = lNode
            break
    return lResult

def ConnectBoxes( src, dst, srcName, dstName ):
    
    srcOut = FindAnimationNode(src.AnimationNodeOutGet(), srcName )
    dstIn = FindAnimationNode(dst.AnimationNodeInGet(), dstName )
    
    if srcOut and dstIn:
        FBConnect(srcOut, dstIn)
        return True
        
    return False

def Main():

    for con in gScene.Constraints:
        if con.Selected:
            gCon = con
            break
            
    if gCon == None:
        FBMessageBox( "ReConnect Joystick", "Please select a relation constraint with a car sim macro", "Ok" )
        return 0
        
    gCarMacroBox = None
    for box in gCon.Boxes:

        if box.Name == "Relation CarMacro":
            gCarMacroBox = box

    if gCarMacroBox == None:
        FBMessageBox( "ReConnect Joystick", "couldnt find a Relation CarMacro", "Ok" )
        return 0

    print gCon.Name
    
    # remove all joystick devices in the scene
    devicesToRemove = []
    for device in gScene.Devices:
        
        deviceName = device.Name
        print deviceName
        
        if deviceName.find("Joystick") >= 0:
            print "added"
            devicesToRemove.append(device)

    if len(devicesToRemove) > 0:
        map(FBComponent.FBDelete, devicesToRemove)
        
        
    # create a new joystick device
    
    joyDevice = FBCreateObject( "Browsing/Templates/Devices", "Joystick", "Joystick" )
    print joyDevice
    if joyDevice:
        gScene.Devices.append(joyDevice)
        
        joyDevice.Live = True
        joyDevice.Online = True
        
        
        # TODO: connect to relation
        
        joyBox = gCon.SetAsSource(joyDevice)
        ConnectBoxes( joyBox, gCarMacroBox, "Button 01", "MacroInput3" )
        ConnectBoxes( joyBox, gCarMacroBox, "Button 02", "MacroInput4" )
        ConnectBoxes( joyBox, gCarMacroBox, "Button 03", "MacroInput5" )    
        
        ConnectBoxes( joyBox, gCarMacroBox, "Z", "MacroInput1" )    
        ConnectBoxes( joyBox, gCarMacroBox, "X", "MacroInput2" )    
        ConnectBoxes( joyBox, gCarMacroBox, "Z", "MacroInput0" )    
    return 1
            
Main()