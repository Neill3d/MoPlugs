
# select src then dst model !

# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

def CopyBlendShapeAnimation(srcmodel, dstmodel):
    
    for lSrcProp in srcmodel.PropertyList:
        
        if lSrcProp.IsAnimatable() and lSrcProp.GetDataTypeName().lower()=="shape":
            
            lDstProp = dstmodel.PropertyList.Find( lSrcProp.Name )
            
            if lDstProp:
                lDstProp.SetAnimated(True)
                lDstProp.GetAnimationNode().FCurve.KeyReplaceBy(lSrcProp.GetAnimationNode().FCurve)
    

def main():

    models = FBModelList()
    FBGetSelectedModels(models)
    
    if len(models) == 2:
        
        FBUndoManager().Undo(True)
        
        models2 = FBModelList()
        FBGetSelectedModels(models2)
        
        if len(models2) == 1:
            
            srcmodel = models2[0]
            dstmodel = None
            if srcmodel == models[0]:
                dstmodel = models[1]
            else:
                dstmodel = models[0]
                
            print srcmodel.Name
            print dstmodel.Name
            
            CopyBlendShapeAnimation(srcmodel, dstmodel)
            
#
main()