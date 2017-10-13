
# script to rename ShapeHand bvh import naming
#   into the ShapeHand device format

# BVH:rpalm -> SHAPEHAND_RIGHT:palm

# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

gRenameRightDict = {
    'rpalm' : 'palm', 
    'rthumbA' : 'thumbA', 
    'rthumbB' : 'thumbB', 
    'rthumbC' : 'thumbC', 
    'rthumbD' : 'thumbC_End',
    'rthumbC_End' : 'thumbC_End',
    'rlittleA' : 'littleA',
    'rlittleB' : 'littleB',
    'rlittleC' : 'littleC',
    'rlittleD' : 'littleC_End',
    'rlittleC_End' : 'littleC_End',
    'rringA' : 'ringA',
    'rringB' : 'ringB',
    'rringC' : 'ringC',
    'rringD' : 'ringC_End',
    'rringC_End' : 'ringC_End',
    'rmiddleA' : 'middleA',
    'rmiddleB' : 'middleB',
    'rmiddleC' : 'middleC',
    'rmiddleD' : 'middleC_End',
    'rmiddleC_End' : 'middleC_End',
    'rindexA' : 'indexA',
    'rindexB' : 'indexB',
    'rindexC' : 'indexC',
    'rindexD' : 'indexC_End',
    'rindexC_End' : 'indexC_End'     }

gRenameDict = 0
gNamespaceSearch = "BVH"
gNamespaceReplace = "ShapeHand_RIGHT"

def GetRoot(model):
    if model.Name.find("reference") >= 0:
        return model
        
    parent = model.Parent
    if parent == None:
        return model
    else:
        return GetRoot(parent)

def RenameShapeHandSkeleton( model ):
    
    modelName = model.Name

    if modelName in gRenameDict.keys():
        model.Name = gRenameDict[modelName]      

    if gNamespaceReplace != "":
        model.ProcessObjectNamespace( FBNamespaceAction.kFBReplaceNamespace, gNamespaceSearch, gNamespaceReplace )
    
    for child in model.Children:
        RenameShapeHandSkeleton(child)     
        
models = FBModelList()
FBGetSelectedModels(models)

if len(models) == 1:
    
    model = models[0]
    parent = GetRoot(model)
    
    gNamespaceSearch = "BVH"
    gNamespaceReplace = "ShapeHand_RIGHT"
    
    gRenameDict = gRenameRightDict
    RenameShapeHandSkeleton(parent)

    