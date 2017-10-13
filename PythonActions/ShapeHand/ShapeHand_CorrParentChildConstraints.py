########################################################################
#
# MoPlugs Project
#
# MAKE A GLOBAL ROTATION CONNECTION BETWEEN NODES
# select two meshes, from two different heirarchy
#
# Sergey Solohin (Neill3d) 2016
#   e-mail to: s@neill3d.com
#       www.neill3d.com
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
########################################################################

from pyfbsdk import *

gCorrDict = {
    'palm' : 'Hand', 
    'thumbA' : 'HandThumb1', 
    'thumbB' : 'HandThumb2', 
    'thumbC' : 'HandThumb3', 
    'thumbD' : 'HandThumb4',
    'thumbC_End' : 'HandThumb4',
    'littleA' : 'HandPinky1',
    'littleB' : 'HandPinky2',
    'littleC' : 'HandPinky3',
    'littleD' : 'HandPinky4',
    'littleC_End' : 'HandPinky4',
    'ringA' : 'HandRing1',
    'ringB' : 'HandRing2',
    'ringC' : 'HandRing3',
    'ringD' : 'HandRing4',
    'ringC_End' : 'HandRing4',
    'middleA' : 'HandMiddle1',
    'middleB' : 'HandMiddle2',
    'middleC' : 'HandMiddle3',
    'middleD' : 'HandMiddle4',
    'middleC_End' : 'HandMiddle4',
    'indexA' : 'HandIndex1',
    'indexB' : 'HandIndex2',
    'indexC' : 'HandIndex3',
    'indexD' : 'HandIndex4',
    'indexC_End' : 'HandIndex4'     }

## CONSTRAINTS INDICES

gConstraintManager = FBConstraintManager()
gRelationId = -1
gParentChildId = -1

for i in range(gConstraintManager.TypeGetCount()):
    name = gConstraintManager.TypeGetName(i)
    if name == "Relation":
        gRelationId = i
    elif name == "Parent/Child":
        gParentChildId = i

## FUNCTIONS

def GetRoot(model):
    if model.Name.find('palm') >= 0:
        return model
        
    parent = model.Parent
    if parent == None:
        return model
    else:
        if parent.Name.find('ForeArm') >= 0:
            return model
        else:
            return GetRoot(parent)

def FindChildren(model, nameKey):
    
    keylen = len(nameKey)
    namelen = len(model.Name)
    pos = model.Name.find(nameKey)
    
    # check if we found a key and it's the end of a name string
    #   to distract RightHand from RightHandIndex1
    if pos >= 0 and (pos+keylen == namelen):
        return model
        
    for child in model.Children:
        res = FindChildren(child, nameKey)
        if res != None:
            return res
        
    return None
    
    

def FindCorrespondance(name, parentDst):
    
    dstName = ""
    
    for key in gCorrDict.keys():
        if name.find(key) >= 0:
            dstName = gCorrDict[key]
    
    if dstName != "":
        someModel = FindChildren(parentDst, dstName)
        return someModel
        
    return None
    

def ConstraintBetweenNodes( modelSrc, parentDst ):
    
    # skip end joints
    if len(modelSrc.Children) == 0:
        return
    
    # TODO: try to find a correspondance node in destination heirarchy
    corrModel = FindCorrespondance(modelSrc.Name, parentDst)
    
    if corrModel and gParentChildId > 0:
        newcon = gConstraintManager.TypeCreateConstraint(gParentChildId)
        
        newcon.ReferenceAdd(0, corrModel)
        newcon.ReferenceAdd(1, modelSrc)
        
        newcon.PropertyList.Find('Affect Translation X').Data = False
        newcon.PropertyList.Find('Affect Translation Y').Data = False
        newcon.PropertyList.Find('Affect Translation Z').Data = False
        
        newcon.Name = 'ParentChild_' + modelSrc.Name
        
    for child in modelSrc.Children:
        ConstraintBetweenNodes(child, parentDst)  



## MAIN

modellist = FBModelList()
FBGetSelectedModels(modellist)

if len(modellist) == 2:
    
    modelSrc = modellist[0]
    modelDst = modellist[1]
    
    parentSrc = GetRoot(modelSrc)
    parentDst = GetRoot(modelDst)
    
    if parentDst.Name.find('palm') >= 0:
        temp = parentDst
        parentDst = parentSrc
        parentSrc = temp
    
    print ('src - ' + parentSrc.Name)
    print ('dst - ' + parentDst.Name)

    # DONE: make list of correspondances Parent\Child constraints
    #   make for every node, except endjoints
    
    ConstraintBetweenNodes(parentSrc, parentDst)
    