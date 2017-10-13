
from pyfbsdk import *

# script to rename bvh shapehand skeleton

# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

gRenameDict = {
    "lpalm" : "LeftHand", 
    "lthumbA" : "LeftHandThumb1", 
    "lthumbB" : "LeftHandThumb2", 
    "lthumbC" : "LeftHandThumb3", 
    "lthumbC_End" : "LeftHandThumb4",
    'llittleA' : 'LeftHandPinky1',
    'llittleB' : 'LeftHandPinky2',
    'llittleC' : 'LeftHandPinky3',
    'llittleC_End' : 'LeftHandPinky4',
    'lringA' : 'LeftHandRing1',
    'lringB' : 'LeftHandRing2',
    'lringC' : 'LeftHandRing3',
    'lringC_End' : 'LeftHandRing4',
    'lmiddleA' : 'LeftHandMiddle1',
    'lmiddleB' : 'LeftHandMiddle2',
    'lmiddleC' : 'LeftHandMiddle3',
    'lmiddleC_End' : 'LeftHandMiddle4',
    'lindexA' : 'LeftHandIndex1',
    'lindexB' : 'LeftHandIndex2',
    'lindexC' : 'LeftHandIndex3',
    'lindexC_End' : 'LeftHandIndex4'     }
    
def RenameModel( oldname, newname ):
    model = FBFindModelByLabelName( oldname )
    if model:
        model.Name = newname

def RenameShapeHandSkeleton( model ):
    
    modelName = model.Name

    if modelName in gRenameDict.keys():
        model.Name = gRenameDict[modelName]      
    
    for child in model.Children:
        RenameShapeHandSkeleton(child)
        
#
modelList = FBModelList()
FBGetSelectedModels(modelList)

if len(modelList) > 0:
    RenameShapeHandSkeleton( modelList[0] )