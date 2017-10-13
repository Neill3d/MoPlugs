###############################################################################
#
# script for the heirarchy from the duplicateAlongACurveConstraint script
#
# Sergey Solohin (Neill3d) 2016
#   e-mail to: s@neill3d.com    
#       www.neill3d.com
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
###############################################################################


from pyfbsdk import *

def RotateChildren(model):

    if isinstance(model, FBModel)and not isinstance(model, FBModelNull):
        
        # fix 1 - zero the rotation pivot
        
        lprop = model.PropertyList.Find('Rotation Pivot (Auto Offset)')
        if lprop:
            lprop.Data = FBVector3d(0.0, 0.0, 0.0)
        
        
        # do rotation
        angles = FBVector3d()
        model.GetVector( angles, FBModelTransformationType.kModelRotation, False )
        angles[1] += 90.0
        model.SetVector( angles, FBModelTransformationType.kModelRotation, False )

    for child in model.Children:
        RotateChildren(child)


models = FBModelList()
FBGetSelectedModels(models)


for model in models:
    RotateChildren(model)