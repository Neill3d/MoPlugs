
# script to create a generic animation story track from selected models

# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

models = FBModelList()
FBGetSelectedModels(models)

def ModelToTrackContent( model, prop, addChildren ):
    
    if addChildren:
        for child in model.Children:
            ModelToTrackContent(child, prop, addChildren)
            
    FBConnect(model, prop)
    
def NewTrackFromModel(model):
    
    newTrack = FBStoryTrack( FBStoryTrackType.kFBStoryTrackAnimation )
    prop = newTrack.PropertyList.Find('Track Content')
    
    if prop != None:
        ModelToTrackContent(model, prop, True)    

if len(models) > 0:
    
    for model in models:
        NewTrackFromModel(model)