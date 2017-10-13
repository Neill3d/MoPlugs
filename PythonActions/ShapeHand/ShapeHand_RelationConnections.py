
from pyfbsdk import *

# script to rename bvh shapehand skeleton
# first execution will make all needed connections
# next execution will update local offsets between joints

# Sergey Solohin (Neill3d) 2016
#   e-mail to: s@neill3d.com
#       www.neill3d.com

# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

# do correspondence connecton between bvh data and character (control rig) fingers

gRenameLeftDict = {
    'lpalm' : 'LeftHand', 
    'lthumbA' : 'LeftHandThumb1', 
    'lthumbB' : 'LeftHandThumb2', 
    'lthumbC' : 'LeftHandThumb3',
    'lthumbD' : 'lthumbC_End',
    'lthumbC_End' : 'LeftHandThumb4',
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

gRenameRightDict = {
    'rpalm' : 'RightHand', 
    'rthumbA' : 'RightHandThumb1', 
    'rthumbB' : 'RightHandThumb2', 
    'rthumbC' : 'RightHandThumb3', 
    'rthumbD' : 'rthumbC_End',
    'rthumbC_End' : 'RightHandThumb4',
    'rlittleA' : 'RightHandPinky1',
    'rlittleB' : 'RightHandPinky2',
    'rlittleC' : 'RightHandPinky3',
    'rlittleD' : 'rlittleC_End',
    'rlittleC_End' : 'RightHandPinky4',
    'rringA' : 'RightHandRing1',
    'rringB' : 'RightHandRing2',
    'rringC' : 'RightHandRing3',
    'rringD' : 'rringC_End',
    'rringC_End' : 'RightHandRing4',
    'rmiddleA' : 'RightHandMiddle1',
    'rmiddleB' : 'RightHandMiddle2',
    'rmiddleC' : 'RightHandMiddle3',
    'rmiddleD' : 'rmiddleC_End',
    'rmiddleC_End' : 'RightHandMiddle4',
    'rindexA' : 'RightHandIndex1',
    'rindexB' : 'RightHandIndex2',
    'rindexC' : 'RightHandIndex3',
    'rindexD' : 'rindexC_End',
    'rindexC_End' : 'RightHandIndex4'     }

gFingersLeftDict = {
    'lpalm' : FBBodyNodeId.kFBLeftWristNodeId,
    'lthumbA' : FBBodyNodeId.kFBLeftThumbANodeId, 
    'lthumbB' : FBBodyNodeId.kFBLeftThumbBNodeId, 
    'lthumbC' : FBBodyNodeId.kFBLeftThumbCNodeId, 
    'lthumbD' : FBBodyNodeId.kFBLeftThumbDNodeId,
    'llittleA' : FBBodyNodeId.kFBLeftPinkyANodeId,
    'llittleB' : FBBodyNodeId.kFBLeftPinkyBNodeId,
    'llittleC' : FBBodyNodeId.kFBLeftPinkyCNodeId,
    'llittleD' : FBBodyNodeId.kFBLeftPinkyDNodeId,
    'lringA' : FBBodyNodeId.kFBLeftRingANodeId,
    'lringB' : FBBodyNodeId.kFBLeftRingBNodeId,
    'lringC' : FBBodyNodeId.kFBLeftRingCNodeId,
    'lringD' : FBBodyNodeId.kFBLeftRingDNodeId,
    'lmiddleA' : FBBodyNodeId.kFBLeftMiddleANodeId,
    'lmiddleB' : FBBodyNodeId.kFBLeftMiddleBNodeId,
    'lmiddleC' : FBBodyNodeId.kFBLeftMiddleCNodeId,
    'lmiddleD' : FBBodyNodeId.kFBLeftMiddleDNodeId,
    'lindexA' : FBBodyNodeId.kFBLeftIndexANodeId,
    'lindexB' : FBBodyNodeId.kFBLeftIndexBNodeId,
    'lindexC' : FBBodyNodeId.kFBLeftIndexCNodeId,
    'lindexD' : FBBodyNodeId.kFBLeftIndexDNodeId     }

gFingersRightDict = {
    'rpalm' : FBBodyNodeId.kFBRightWristNodeId,
    'rthumbA' : FBBodyNodeId.kFBRightThumbANodeId, 
    'rthumbB' : FBBodyNodeId.kFBRightThumbBNodeId, 
    'rthumbC' : FBBodyNodeId.kFBRightThumbCNodeId, 
    'rthumbD' : FBBodyNodeId.kFBRightThumbDNodeId,
    'rlittleA' : FBBodyNodeId.kFBRightPinkyANodeId,
    'rlittleB' : FBBodyNodeId.kFBRightPinkyBNodeId,
    'rlittleC' : FBBodyNodeId.kFBRightPinkyCNodeId,
    'rlittleD' : FBBodyNodeId.kFBRightPinkyDNodeId,
    'rringA' : FBBodyNodeId.kFBRightRingANodeId,
    'rringB' : FBBodyNodeId.kFBRightRingBNodeId,
    'rringC' : FBBodyNodeId.kFBRightRingCNodeId,
    'rringD' : FBBodyNodeId.kFBRightRingDNodeId,
    'rmiddleA' : FBBodyNodeId.kFBRightMiddleANodeId,
    'rmiddleB' : FBBodyNodeId.kFBRightMiddleBNodeId,
    'rmiddleC' : FBBodyNodeId.kFBRightMiddleCNodeId,
    'rmiddleD' : FBBodyNodeId.kFBRightMiddleDNodeId,
    'rindexA' : FBBodyNodeId.kFBRightIndexANodeId,
    'rindexB' : FBBodyNodeId.kFBRightIndexBNodeId,
    'rindexC' : FBBodyNodeId.kFBRightIndexCNodeId,
    'rindexD' : FBBodyNodeId.kFBRightIndexDNodeId     }

gPrefix = ""

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

def GetRoot(model):
    parent = model.Parent
    if parent == None:
        return model
    else:
        return GetRoot(parent)

def IsNodeInsideControlRig(model):
    root = GetRoot(model)
    
    for child in root.Children:
        prop = child.PropertyList.Find("IK Blend T")
        if prop != None:
            return True
            
    return False
 
def IsNodeACharacterBone(model):
    
    srcCount = model.GetSrcCount()
    for i in range(srcCount):
        plug = model.GetSrc(i)
        
        if plug.ClassName() == "FBCharacter":
            return True
    
    return False

def GetConnectedCharacter(model):
    
    srcCount = model.GetSrcCount()
    for i in range(srcCount):
        plug = model.GetSrc(i)
        
        if plug.ClassName() == "FBCharacter":
            return plug
    
    return None

def FindChildrenNode(model, name):
    
    global gPrefix
    
    modelname = gPrefix + model.Name
    print (modelname + " - " + name)
    if modelname == name:
        return model
    else:
        
        for child in model.Children:
            res = FindChildrenNode(child, name)
            if res != None:
                return res
    
    return None

def FindRootNodeWithAName(model, name1, name2):
    
    global gPrefix
    
    modelname = gPrefix + model.Name
    
    if modelname == name1 or modelname == name2:
        return model
    else:
        
        parent = model.Parent
        if parent:
            res = FindRootNodeWithAName(parent, name1, name2)
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

def ConnectFinger( constraint, boxSrc, boxDst, isThumb ):
    
    # connect local rotation
    #  vector to number -> Scale And Offset -> Number To Vector
    #  Scale And Offset will store initial state,
    #   later we could make a script to update initial state
    
    srcPos = constraint.GetBoxPosition(boxSrc)
    dstPos = constraint.GetBoxPosition(boxDst)

    xPos = srcPos[1]
    yPos = srcPos[2]
    
    vecToNumber = constraint.CreateFunctionBox( "Converters", "Vector To Number" )
    numberToVec = constraint.CreateFunctionBox( "Converters", "Number To Vector" )
    scaleAndOffsetX = constraint.CreateFunctionBox( "Number", "Scale And Offset (Number)" )
    scaleAndOffsetY = constraint.CreateFunctionBox( "Number", "Scale And Offset (Number)" )
    scaleAndOffsetZ = constraint.CreateFunctionBox( "Number", "Scale And Offset (Number)" )
        
    ConnectBoxes( boxSrc, vecToNumber, "Lcl Rotation", "V" )
    ConnectBoxes( numberToVec, boxDst, "Result", "Lcl Rotation" )

    if isThumb:
        ConnectBoxes( vecToNumber, scaleAndOffsetX, "X", "X" )
        ConnectBoxes( vecToNumber, scaleAndOffsetY, "Y", "X" )
        ConnectBoxes( vecToNumber, scaleAndOffsetZ, "Z", "X" )
                
        ConnectBoxes( scaleAndOffsetX, numberToVec, "Result", "Z" )
        ConnectBoxes( scaleAndOffsetY, numberToVec, "Result", "X" )
        ConnectBoxes( scaleAndOffsetZ, numberToVec, "Result", "Y" )        
    else:
        ConnectBoxes( vecToNumber, scaleAndOffsetX, "X", "X" )
        ConnectBoxes( vecToNumber, scaleAndOffsetY, "Y", "X" )
        ConnectBoxes( vecToNumber, scaleAndOffsetZ, "Z", "X" )
                
        ConnectBoxes( scaleAndOffsetX, numberToVec, "Result", "Z" )
        ConnectBoxes( scaleAndOffsetY, numberToVec, "Result", "Y" )
        ConnectBoxes( scaleAndOffsetZ, numberToVec, "Result", "X" )

    ConnectBoxes( boxSrc, scaleAndOffsetX, "MultX", "Scale Factor" )
    ConnectBoxes( boxSrc, scaleAndOffsetY, "MultY", "Scale Factor" )
    ConnectBoxes( boxSrc, scaleAndOffsetZ, "MultZ", "Scale Factor" )
        
    ConnectBoxes( boxSrc, scaleAndOffsetX, "OffsetX", "Offset" )
    ConnectBoxes( boxSrc, scaleAndOffsetY, "OffsetY", "Offset" )
    ConnectBoxes( boxSrc, scaleAndOffsetZ, "OffsetZ", "Offset" )
    
    # positions
    
    constraint.SetBoxPosition(vecToNumber, xPos + 400, yPos)
    constraint.SetBoxPosition(numberToVec, xPos + 700, yPos)
    constraint.SetBoxPosition(scaleAndOffsetX, xPos + 400, yPos-200)
    constraint.SetBoxPosition(scaleAndOffsetY, xPos + 400, yPos+100)
    constraint.SetBoxPosition(scaleAndOffsetZ, xPos + 400, yPos+300)
    
#
#

def MakeConnections(handSrc, currChar, isLeftHand):

    if handSrc == None or currChar == None:
        return False

    gConstraintManager = FBConstraintManager()
    gRelationId = -1
    gParentChildId = -1
    
    for i in range(gConstraintManager.TypeGetCount()):
        name = gConstraintManager.TypeGetName(i)
        if name == "Relation":
            gRelationId = i
        elif name == "Parent/Child":
            gParentChildId = i
    
    thumbIds = []
    tempName = "Relation_ShapeHand"
    fingersDict = 0
    renameDict = 0
    
    if isLeftHand:
        thumbIds.append(FBBodyNodeId.kFBLeftThumbANodeId)
        thumbIds.append(FBBodyNodeId.kFBLeftThumbBNodeId)
        thumbIds.append(FBBodyNodeId.kFBLeftThumbCNodeId)
        thumbIds.append(FBBodyNodeId.kFBLeftThumbDNodeId)
        tempName = tempName + "_Left"
        fingersDict = gFingersLeftDict
        renameDict = gRenameLeftDict
    else:
        thumbIds.append(FBBodyNodeId.kFBRightThumbANodeId)
        thumbIds.append(FBBodyNodeId.kFBRightThumbBNodeId)
        thumbIds.append(FBBodyNodeId.kFBRightThumbCNodeId)
        thumbIds.append(FBBodyNodeId.kFBRightThumbDNodeId)
        tempName = tempName + "_Right"
        fingersDict = gFingersRightDict
        renameDict = gRenameRightDict

    isControlRig = (None != currChar.GetCurrentControlSet())
    
    # new relation constraint
    constraint = gConstraintManager.TypeCreateConstraint(gRelationId)
    constraint.Name = tempName
    
    yPos = 0
    for srcName, dstNodeId in fingersDict.items():
        modelSrc = None
        modelDst = None    
        if isControlRig:
            modelDst = currChar.GetCtrlRigModel( dstNodeId )
            modelSrc = FindChildrenNode( handSrc, srcName )
        else:
            modelDst = currChar.GetModel( dstNodeId )
            modelSrc = FindChildrenNode( handSrc, srcName )
        if modelSrc == None:
            modelSrc = FindChildrenNode( handSrc, renameDict[srcName] )
            
        srcBox = None
        multProp = None
        offsetXProp = None
        offsetYProp = None
        if modelSrc:
            srcBox = constraint.SetAsSource(modelSrc)
            srcBox.UseGlobalTransforms = False
            constraint.SetBoxPosition(srcBox, 0, yPos)
        
            multProp = modelSrc.PropertyList.Find("MultX")
            if multProp == None:
                multProp = modelSrc.PropertyCreate( "MultX", FBPropertyType.kFBPT_double, "Number", True, True, None)            
            multProp.Data = 1.0
            multProp.SetAnimated(True)

            multProp = modelSrc.PropertyList.Find("MultY")
            if multProp == None:
                multProp = modelSrc.PropertyCreate( "MultY", FBPropertyType.kFBPT_double, "Number", True, True, None)            
            multProp.Data = -1.0
            multProp.SetAnimated(True)

            multProp = modelSrc.PropertyList.Find("MultZ")
            if multProp == None:
                multProp = modelSrc.PropertyCreate( "MultZ", FBPropertyType.kFBPT_double, "Number", True, True, None)            
            multProp.Data = 1.0
            multProp.SetAnimated(True)
        
            offsetXProp = modelSrc.PropertyList.Find("OffsetX")
            if offsetXProp == None:
                offsetXProp = modelSrc.PropertyCreate("OffsetX", FBPropertyType.kFBPT_double, "Number", True, True, None)
            offsetXProp.Data = 0.0
            offsetXProp.SetAnimated(True)            
            
            offsetYProp = modelSrc.PropertyList.Find("OffsetY")
            if offsetYProp == None:
                offsetYProp = modelSrc.PropertyCreate("OffsetY", FBPropertyType.kFBPT_double, "Number", True, True, None)
            offsetYProp.Data = 0.0
            offsetYProp.SetAnimated(True)
            
            offsetZProp = modelSrc.PropertyList.Find("OffsetZ")
            if offsetZProp == None:
                offsetZProp = modelSrc.PropertyCreate("OffsetZ", FBPropertyType.kFBPT_double, "Number", True, True, None)
            offsetZProp.Data = 0.0
            offsetZProp.SetAnimated(True)
        
        dstBox = None
        if modelDst:
            dstBox = constraint.ConstrainObject(modelDst)
            dstBox.UseGlobalTransforms = False
            constraint.SetBoxPosition(dstBox, 1000, yPos)
                   
        isThumb = (dstNodeId in thumbIds)
        
        if srcBox and dstBox:
            ConnectFinger(constraint, srcBox, dstBox, isThumb)
            
        '''
        if isThumb and modelSrc and modelDst:
            
            parentConstraint = gConstraintManager.TypeCreateConstraint(gParentChildId)
            
            if parentConstraint:
                parentConstraint.ReferenceAdd(0, modelDst)
                parentConstraint.ReferenceAdd(1, modelSrc)
                
                parentConstraint.PropertyList.Find("Affect Translation X").Data = False
                parentConstraint.PropertyList.Find("Affect Translation Y").Data = False
                parentConstraint.PropertyList.Find("Affect Translation Z").Data = False
                                                        
                parentConstraint.Snap()
                parentConstraint.Active = True
                
                parentConstraint.Name = "ParentChild " + modelDst.Name
        '''
        yPos += 700
    #
    return True


def UpdateOffsets(model):
    
    offsetXProp = model.PropertyList.Find("OffsetX")
    offsetYProp = model.PropertyList.Find("OffsetY")
    offsetZProp = model.PropertyList.Find("OffsetY")
        
    if offsetXProp and offsetYProp:
        v = FBVector3d()
        model.GetVector( v, FBModelTransformationType.kModelRotation, False )
        
        offsetXProp.Data = -v[0]
        offsetYProp.Data = -v[1]
        offsetZProp.Data = -v[2]
            
    for child in model.Children:
        UpdateOffsets(child)
    
    #
    
def IsInsideRelation(model):
    
    lScene = FBSystem().Scene

    for constraint in lScene.Constraints:
        if constraint.ClassName() == "FBConstraintRelation":
            for box in constraint.Boxes:
                if box.ClassName() == "FBModelPlaceHolder":
                    boxModel = box.Model
                    
                    if boxModel.Parent and boxModel.Parent == model:
                        print boxModel.Parent.Name
                        return constraint 
    
    return None
    
# should be selected two models, one source, one output character skeleton
modelList = FBModelList()
FBGetSelectedModels(modelList)

if len(modelList) == 1:
    
    leftHandSrc = None
    rightHandSrc = None
    currChar = FBApplication().CurrentCharacter
    
    gPrefix = ""
    handSrc = FindRootNodeWithAName(modelList[0], 'palm', 'palm')
    if handSrc:
        value = FBMessageBox( "ShapeHand Connections", "Which hand do you want to setup?", "Left", "Right" )
    
        if value == 1:
            gPrefix = "l"
            leftHandSrc = handSrc
        else:
            gPrefix = "r"
            rightHandSrc = handSrc
    else:
        leftHandSrc = FindRootNodeWithAName(modelList[0], 'lpalm', 'LeftHand')
        if leftHandSrc == None:
            rightHandSrc = FindRootNodeWithAName(modelList[0], 'rpalm', 'RightHand')    

    constraint = None
    if leftHandSrc != None:
        constraint = IsInsideRelation( leftHandSrc )
    elif rightHandSrc != None:
        constraint = IsInsideRelation( rightHandSrc )
    
    if constraint != None:
        # only update offsets
        if leftHandSrc:
            UpdateOffsets( leftHandSrc )
        elif rightHandSrc:
            UpdateOffsets( rightHandSrc )
    else:
        # do a new relation setup
        print "test"
        
        if leftHandSrc != None:
            MakeConnections( leftHandSrc, currChar, True )
        elif rightHandSrc != None:
            MakeConnections( rightHandSrc, currChar, False )
        