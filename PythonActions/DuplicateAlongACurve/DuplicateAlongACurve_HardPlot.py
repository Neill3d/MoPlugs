
# script to Hard Plot camera switcher into camera

# Author: Sergey Solohin (Neill3d), e-mail to: s@neill3d.com
#  www.neill3d.com
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

l = FBModelList()
FBGetSelectedModels(l)

gSystem = FBSystem()
player = FBPlayerControl()

startFrame = player.LoopStart
stopFrame = player.LoopStop

curFrame = startFrame
stepFrame = FBTime(0,0,0,1)
'''
for obj in l:
    obj.Translation.SetAnimated(False)
    obj.Translation.SetAnimated(True)
'''
v = FBVector3d()

newCam = FBCamera('HardPloted_Camera')
newCam.Show = True

newCam.Translation.SetAnimated(True)
newCam.Rotation.SetAnimated(True)
newCam.Roll.SetAnimated(True)
newCam.FieldOfView.SetAnimated(True)
newCam.FocalLength.SetAnimated(True)

lRenderer = gSystem.Renderer
switcher = FBCameraSwitcher()

while curFrame <= stopFrame:
    
    player.Goto(curFrame)
    gSystem.Scene.Evaluate()
    
    
    currCamera = switcher.CurrentCamera
    #currCamera = lRenderer.CurrentCamera
    #if isinstance(currCamera, FBCameraSwitcher):
    #    currCamera = currCamera.CurrentCamera
        
    print curFrame.GetFrame()
    print currCamera.Name
    
    currCamera.GetVector(v)
    newCam.SetVector(v)
    newCam.GetVector(v, FBModelTransformationType.kModelTranslation, False)
    
    for i in range(0,3):
        newCam.Translation.GetAnimationNode().Nodes[i].KeyAdd(v[i])
    
    currCamera.GetVector(v, FBModelTransformationType.kModelRotation)
    newCam.SetVector(v, FBModelTransformationType.kModelRotation)
    newCam.GetVector(v, FBModelTransformationType.kModelRotation, False)
    
    for i in range(0,3):
        newCam.Rotation.GetAnimationNode().Nodes[i].KeyAdd(v[i])
        
    value = currCamera.Roll.Data
    newCam.Roll.GetAnimationNode().KeyAdd(value)
    
    value = currCamera.FieldOfView.Data
    newCam.FieldOfView.GetAnimationNode().KeyAdd(value)
    
    #value = currCamera.FocalLength.Data
    #newCam.FocalLength.GetAnimationNode().KeyAdd(value)
          
    curFrame += stepFrame