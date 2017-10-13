
# select a root and run a script
# script for the heirarchy from the duplicateAlongACurveConstraint script
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

gSystem = FBSystem()

def SelectChildren(model):

    if isinstance(model, FBModel)and not isinstance(model, FBModelNull):
        model.Selected = True
        
    for child in model.Children:
        SelectChildren(child)


models = FBModelList()
FBGetSelectedModels(models)

for comp in gSystem.Scene.Components:
    comp.Selected = False

for model in models:
    SelectChildren(model)