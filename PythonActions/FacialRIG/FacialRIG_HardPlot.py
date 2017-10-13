
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

l = FBModelList()
FBGetSelectedModels(l)

newModels = []

for model in l:
    newMdl = FBModelNull( model.Name + "_baked" )
    newMdl.Translation.SetAnimated(True)
    newModels.append(newMdl)
    

lApp = FBApplication()
gSystem = FBSystem()
player = FBPlayerControl()

startTime = player.ZoomWindowStart
stopTime = player.ZoomWindowStop

curFrame = startTime
stepTime = FBTime(0,0,0,1)
'''
for obj in l:
    obj.Translation.SetAnimated(False)
    obj.Translation.SetAnimated(True)
'''
v = FBVector3d()

def OnFrameRendering(control, event):
    localTime = gSystem.LocalTime
    print localTime.GetFrame()
    
    for (facialModel, bakeModel) in zip(l, newModels):
       
        if bakeModel.Translation.GetAnimationNode():
            facialModel.GetVector(v, FBModelTransformationType.kModelTranslation, False)
            for i in range(0,3):
                bakeModel.Translation.GetAnimationNode().Nodes[i].KeyAdd(v[i])       
        
        

gSystem.OnVideoFrameRendering.Add(OnFrameRendering)

# Get the default rendering options, which are saved in the FBX file.
lOptions = FBVideoGrabber().GetOptions()
    # Set VideoCodec Option:
VideoManager = FBVideoCodecManager()
VideoManager.VideoCodecMode = FBVideoCodecMode.FBVideoCodecStored

# Set the name of the rendered file.
lOptions.OutputFileName = 'D:\\tempBake\\temp001.jpg'
    # Only windows supports mov.
#if lRenderFileFormat == '.mov' and os.name != 'nt':
#    lOptions.BitsPerPixel = FBVideoRenderDepth.FBVideoRender32Bits

lOptions.TimeSpan = FBTimeSpan(startTime, stopTime)
lOptions.TimeSteps = stepTime

# Do the render. This will always be done in uncompressed mode.
lApp.FileRender( lOptions )

gSystem.OnVideoFrameRendering.Remove(OnFrameRendering)

#
# copy back local marker animation

for (facialModel, bakeModel) in zip(l, newModels):
     
    if facialModel.Translation.GetAnimationNode():
        for i in range(0,3):
            facialModel.Translation.GetAnimationNode().Nodes[i].FCurve.KeyReplaceBy( bakeModel.Translation.GetAnimationNode().Nodes[i].FCurve )
