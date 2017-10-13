
#
# script for computing acceleration of a model translation
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *
import math

gapp = FBApplication()
gplayer = FBPlayerControl()
gsystem = FBSystem()

def ReadFromVectorNodes(parentNode, time):
    
    result = FBVector3d()
    
    for i in range(3):
        result[i] = parentNode.Nodes[i].FCurve.Evaluate(time)
    return result
    

models = FBModelList()
FBGetSelectedModels(models)


currTime = gplayer.LoopStart
endTime = gplayer.LoopStop
stepTime = FBTime(0,0,0,1)

for mdl in models:
    prop = mdl.PropertyList.Find('accel pos')
    if prop:
        prop.SetAnimated(False)
        prop.SetAnimated(True)
    else:
        prop = mdl.PropertyCreate( "accel pos", FBPropertyType.kFBPT_Vector3D, "Vector3D", True, True, None)
        prop.SetAnimated(True)
    #
    prop = mdl.PropertyList.Find('accel av')
    if prop:
        prop.SetAnimated(False)
        prop.SetAnimated(True)
    else:
        prop = mdl.PropertyCreate( "accel av", FBPropertyType.kFBPT_double, "Number", True, True, None)
        prop.SetAnimated(True)

while currTime < endTime:
    
    nextTime = currTime + stepTime
    
    for mdl in models:
        
        if mdl.Translation.IsAnimated():
            node = mdl.Translation.GetAnimationNode()
        
            currPos = FBVector3d()
            nextPos = FBVector3d()
            
            currPos = ReadFromVectorNodes(node, currTime)
            nextPos = ReadFromVectorNodes(node, nextTime)
            
            prop = mdl.PropertyList.Find('accel pos')
            if prop:
                node = prop.GetAnimationNode()
                for i in range(3):
                    node.Nodes[i].KeyAdd(currTime, nextPos[i]-currPos[i])
    
            #
            prop = mdl.PropertyList.Find('accel av')
            if prop:
                av = math.sqrt((nextPos[0]-currPos[0])*(nextPos[0]-currPos[0]) + (nextPos[1]-currPos[1])*(nextPos[1]-currPos[1]) + (nextPos[2]-currPos[2])*(nextPos[2]-currPos[2]) )
                
                node = prop.GetAnimationNode()
                node.KeyAdd(currTime, av)
    
    currTime = nextTime
