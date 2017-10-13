##################################################################################
#
# MoPlugs Project
#
# script to connect current character skeleton fingers to the rig skeleton nodes
# using relation constraint (constraint local rotation)
#
# Sergey Solohin (Neill3d) 2016
#   e-mail to: s@neill3d.com
#       www.neill3d.com
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
##################################################################################

from pyfbsdk import *

gFingersLeftArray = (
    FBBodyNodeId.kFBLeftWristNodeId,
    FBBodyNodeId.kFBLeftThumbANodeId, 
    FBBodyNodeId.kFBLeftThumbBNodeId, 
    FBBodyNodeId.kFBLeftThumbCNodeId, 
    FBBodyNodeId.kFBLeftPinkyANodeId,
    FBBodyNodeId.kFBLeftPinkyBNodeId,
    FBBodyNodeId.kFBLeftPinkyCNodeId,
    FBBodyNodeId.kFBLeftRingANodeId,
    FBBodyNodeId.kFBLeftRingBNodeId,
    FBBodyNodeId.kFBLeftRingCNodeId,
    FBBodyNodeId.kFBLeftMiddleANodeId,
    FBBodyNodeId.kFBLeftMiddleBNodeId,
    FBBodyNodeId.kFBLeftMiddleCNodeId,
    FBBodyNodeId.kFBLeftIndexANodeId,
    FBBodyNodeId.kFBLeftIndexBNodeId,
    FBBodyNodeId.kFBLeftIndexCNodeId )

gFingersRightArray = (
    FBBodyNodeId.kFBRightWristNodeId,
    FBBodyNodeId.kFBRightThumbANodeId, 
    FBBodyNodeId.kFBRightThumbBNodeId, 
    FBBodyNodeId.kFBRightThumbCNodeId, 
    FBBodyNodeId.kFBRightPinkyANodeId,
    FBBodyNodeId.kFBRightPinkyBNodeId,
    FBBodyNodeId.kFBRightPinkyCNodeId,
    FBBodyNodeId.kFBRightRingANodeId,
    FBBodyNodeId.kFBRightRingBNodeId,
    FBBodyNodeId.kFBRightRingCNodeId,
    FBBodyNodeId.kFBRightMiddleANodeId,
    FBBodyNodeId.kFBRightMiddleBNodeId,
    FBBodyNodeId.kFBRightMiddleCNodeId,
    FBBodyNodeId.kFBRightIndexANodeId,
    FBBodyNodeId.kFBRightIndexBNodeId,
    FBBodyNodeId.kFBRightIndexCNodeId  )

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

def GetChild(model, nameKey):
    '''
    keylen = len(nameKey)
    namelen = len(model.Name)
    pos = model.Name.find(nameKey)
    
    # check if we found a key and it's the end of a name string
    #   to distract RightHand from RightHandIndex1
    if pos >= 0 and (pos+keylen == namelen):
        return model
    '''
    if model.Name == nameKey:
        return model
        
    for child in model.Children:
        res = GetChild(child, nameKey)
        if res != None:
            return res
            
    return None

# Define a utility function to look up a named animation node
# under a given parent. Will not recurse in the hierarchy.
def FindAnimationNode( pParent, pName ):
    lResult = None
    for lNode in pParent.Nodes:
        if lNode.Name == pName:
            lResult = lNode
            break
    return lResult

def ConnectBoxes( src, dst, srcName, dstName ):
    
    srcOut = FindAnimationNode(src.AnimationNodeOutGet(), srcName )
    dstIn = FindAnimationNode(dst.AnimationNodeInGet(), dstName )
    
    if srcOut and dstIn:
        FBConnect(srcOut, dstIn)
        return True
        
    return False

def RelationConnectRotations(constraint, modelSrc, modelDst, yPos):
    
    srcBox = constraint.SetAsSource(modelSrc)
    srcBox.UseGlobalTransforms = False
    constraint.SetBoxPosition(srcBox, 0, yPos)
    
    dstBox = constraint.ConstrainObject(modelDst)
    dstBox.UseGlobalTransforms = False
    constraint.SetBoxPosition(dstBox, 500, yPos)
    
    ConnectBoxes( srcBox, dstBox, "Lcl Rotation", "Lcl Rotation" )    
    #

## MAIN

def Main():

    currChar = FBApplication().CurrentCharacter

    if currChar == None:
        FBMessageBox( "ShapeHand Tool Script", "Current Character should be settedUp", "Ok")
        return

    modellist = FBModelList()
    FBGetSelectedModels(modellist)
    
    rigRoot = GetRoot(modellist[0])
    
    if len(modellist) != 1:
        FBMessageBox( "ShapeHand Tool Script", "You should select one of the RIG skeleton joints", "Ok")
        return

    value = FBMessageBox("ShapeHand Tool Script", "What side do you want to connect?", "Left", "Right", "Cancel")
    if value == 3:
        return
    
    gCharFingersArray = gFingersLeftArray
    
    conName = 'LeftHand'
    if value == 2:
        gCharFingersArray = gFingersRightArray
        conName = 'RightHand'

    gSrcFingersArray = gFingersLeftArray
    if rigRoot.Name.find('Right') >= 0:
        gSrcFingersArray = gFingersRightArray
 
    newcon = gConstraintManager.TypeCreateConstraint(gRelationId)
    newcon.Name = 'Relation_' + conName
    yPos = 0
    
    
    for (srcitem, charitem) in zip(gSrcFingersArray, gCharFingersArray):

        modelDst = currChar.GetModel(charitem)
        modelCharForSrc = currChar.GetModel(srcitem)
        modelSrc = GetChild(rigRoot, modelCharForSrc.Name)
        
        print ('modelDst - ' + modelDst.Name + '; modelCharForSrc - ' + modelCharForSrc.Name)
        
        # DONE: find a correspondance model
        
        if modelSrc and modelDst:
            RelationConnectRotations(newcon, modelSrc, modelDst, yPos)
            yPos += 300
        
#
Main()