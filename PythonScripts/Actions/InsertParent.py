
#############################################################################
#
# insert parent for each selected model
#   uses current frame model transformation to put parent in place
#   if model is animated, script will recalculate trajectory into a local one
#
# Author Sergey Solohin (2015) Neill3d
#   e-mail to: s@neill3d.com
#   www.neill3d.com
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
##############################################################################

from pyfbsdk import *


models = FBModelList()
FBGetSelectedModels(models)

animatedModels = []

gSystem = FBSystem()

source = []
nulls = []
constraints = []

# prepare constraint type
lPosIdx = -1

lMgr = FBConstraintManager()
for lIdx in range( lMgr.TypeGetCount() ):
    if lMgr.TypeGetName( lIdx ) == 'Parent/Child':
        lPosIdx = lIdx


def storeTrajectories(models):
    
    global source
    global nulls
    global constraints
    
    source[:] = []
    nulls[:] = []
    constraints[:] = []
    
    for elem in gSystem.Scene.Components:
        elem.Selected = False
    
    for obj in models:
        newObj = FBModelNull( obj.Name + '_Null' )
        newObj.Show = True
        newObj.Selected = True
        obj.Selected = False
        
        source.append(obj.Name)
        nulls.append(newObj)
        
        if lPosIdx >= 0:
            lConstraint = lMgr.TypeCreateConstraint(lPosIdx)
            lConstraint.ReferenceAdd(0, newObj)
            lConstraint.ReferenceAdd(1, obj)    
            lConstraint.Active = True
                
            constraints.append(lConstraint)
            
        
    # plot all new nulls    
    take = gSystem.CurrentTake
    lPeriod = FBTime(0,0,0,1)
    if take: take.PlotTakeOnSelected(lPeriod)
            
    # switch off the constraints            
    for lConstraint in constraints:
        lConstraint.Active = False
        
        objA = lConstraint.ReferenceGet(0, 0)
        objB = lConstraint.ReferenceGet(1, 0)
        
        lConstraint.ReferenceRemove(0, objA)
        lConstraint.ReferenceRemove(1, objB)
                
        lConstraint.ReferenceAdd(0, objB)
        lConstraint.ReferenceAdd(1, objA)

def fetchTrajectories():

    # activate constraints
    i=0
    for lConstraint in constraints:
        lConstraint.Active = True
        
        objA = lConstraint.ReferenceGet(0,0)
        if objA == None:
            objA = FBFindModelByLabelName(source[i])
            lConstraint.ReferenceAdd(0,objA)
            
        if objA != None: objA.Selected = True
        
        i = i + 1

    # plot animations
    take = gSystem.CurrentTake
    lPeriod = FBTime(0,0,0,1)
    if take: take.PlotTakeOnSelected(lPeriod)
    
    # remove unneeded objects and constraints
    for lConstraint in constraints:
        lConstraint.Active = False
        
        objB = lConstraint.ReferenceGet(1,0)
        
        objB.FBDelete()
        lConstraint.FBDelete()


#############################################################################
## MAIN CODE

for model in models:
    
    #if model has animation on TM, we should store it's motion, otherwise simple reparent
    
    isAnimated = False
    
    if model.Translation.IsAnimated():        
        if model.Translation.GetAnimationNode().KeyCount > 0:
            isAnimated = True

    if model.Rotation.IsAnimated():
        if model.Rotation.GetAnimationNode().KeyCount > 0:
            isAnimated = True
            
    if model.Scaling.IsAnimated():
        if model.Scaling.GetAnimationNode().KeyCount > 0:
            isAnimated = True

    if not isAnimated:
        newParent = FBModelNull( model.Name + "_parent" )
        newParent.Show = True
        
        m = FBMatrix()
        model.GetMatrix(m)
        newParent.SetMatrix(m)
        
        modelParent = model.Parent
        model.Parent = newParent
        newParent.Parent = modelParent
        
    else:
        animatedModels.append(model)
        
        
# hold motion for animatedModels

if len(animatedModels) > 0:
    storeTrajectories(animatedModels)
    
    gSystem.Scene.Evaluate()
    
    for model in animatedModels:
        newParent = FBModelNull( model.Name + "_parent" )
        newParent.Show = True
        
        m = FBMatrix()
        model.GetMatrix(m)
        newParent.SetMatrix(m)
        
        modelParent = model.Parent
        model.Parent = newParent
        newParent.Parent = modelParent
    
    fetchTrajectories()

# restore models selection

for elem in gSystem.Scene.Components:
    elem.Selected = False
    
for model in models:
    model.Selected = True
