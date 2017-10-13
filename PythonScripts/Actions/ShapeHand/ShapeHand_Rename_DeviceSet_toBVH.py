
# rename from ShapeHand device into the bvh import format
# SHAPEHAND_RIGHT:palm -> BVH:rpalm 

# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

gPrefix = ""
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
    if modelName.find("reference") < 0:
        model.Name = gPrefix + modelName

    if gNamespaceReplace != "":
        model.ProcessObjectNamespace( FBNamespaceAction.kFBReplaceNamespace, gNamespaceSearch, gNamespaceReplace )
    
    for child in model.Children:
        RenameShapeHandSkeleton(child)     
        
models = FBModelList()
FBGetSelectedModels(models)

if len(models) == 1:
    
    model = models[0]
    parent = GetRoot(model)
    
    value = FBMessageBox( "Rename to BVH", "Choose a hand side please", "Left", "Right" )
    
    if value == 1:
        gPrefix = "l"
        gNamespaceSearch = "ShapeHand_LEFT"    
    else:
        gPrefix = "r"
        gNamespaceSearch = "ShapeHand_RIGHT"
    
    gNamespaceReplace = "BVH"
    RenameShapeHandSkeleton(parent)

    