
# script to fix instances render
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

gSystem = FBSystem()
gScene = gSystem.Scene
gApp = FBApplication()
gPlayer = FBPlayerControl()

from pyfbsdk import *
import math

# select source, then destination. Script creates relation constraint between all joints elements

lMgr = FBConstraintManager()

# we need Aim and Position constraints

constraintId = 0

for lIdx in range( lMgr.TypeGetCount() ):

    # User feedback, if the python console is up.
    if "Wall Bricks Temp" == lMgr.TypeGetName( lIdx ):
        constraintId = lIdx

print constraintId

models = FBModelList()
FBGetSelectedModels(models)

if constraintId > 0 and len(models)>0:
    
    for model in models:
        
        geom = model.Geometry
        if geom != None:
            
            constraint = lMgr.TypeCreateConstraint( constraintId )
                    
            for i in range(0, geom.GetDstCount() ):
                instModel = geom.GetDst(i)
                if isinstance(instModel, FBModel):
                    
                    constraint.ReferenceAdd(0, instModel)
                    
            constraint.Active = True
            constraint.Active = False            
            #constraint.FBDelete()