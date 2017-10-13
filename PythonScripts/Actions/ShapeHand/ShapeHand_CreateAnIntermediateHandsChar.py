
# create an intermediate character for hands motion transfering

# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

lSkeletonNodeCharacterizationMapping = ( 
        "ReferenceLink",
        "HipsLink", 
        "LeftUpLegLink",
        "LeftUpLegRollLink",
        "LeftLegLink",
        "LeftLegRollLink",
        "LeftFootLink",
        "LeftToeBaseLink",
        "LeftFootIndex1Link",
        "LeftFootIndex2Link",
        "RightUpLegLink",
        "RightUpLegRollLink",
        "RightLegLink",
        "RightLegRollLink",
        "RightFootLink",
        "RightToeBaseLink",
        "RightFootIndex1Link",
        "RightFootIndex2Link",
        "SpineLink",
        "Spine1Link",
        "Spine2Link",
        "LeftShoulderLink",
        "LeftArmLink",
        "LeftArmRollLink",
        "LeftForeArmLink",
        "LeftForeArmRollLink",
        "LeftHandLink",
        "RightShoulderLink",
        "RightArmLink",
        "RightArmRollLink",
        "RightForeArmLink",
        "RightForeArmRollLink",
        "RightHandLink",
        "NeckLink",
        "HeadLink",
        "LeftHandThumb1Link",
        "LeftHandThumb2Link",
        "LeftHandThumb3Link",
        "LeftHandThumb4Link",
        "LeftHandIndex1Link",
        "LeftHandIndex2Link",
        "LeftHandIndex3Link",
        "LeftHandIndex4Link",
        "LeftHandMiddle1Link",
        "LeftHandMiddle2Link",
        "LeftHandMiddle3Link",
        "LeftHandMiddle4Link",
        "LeftHandRing1Link",
        "LeftHandRing2Link",
        "LeftHandRing3Link",
        "LeftHandRing4Link",
        "LeftHandPinky1Link",
        "LeftHandPinky2Link",
        "LeftHandPinky3Link",
        "LeftHandPinky4Link",
        "RightHandThumb1Link",
        "RightHandThumb2Link",
        "RightHandThumb3Link",
        "RightHandThumb4Link",
        "RightHandIndex1Link",
        "RightHandIndex2Link",
        "RightHandIndex3Link",
        "RightHandIndex4Link",
        "RightHandMiddle1Link",
        "RightHandMiddle2Link",
        "RightHandMiddle3Link",
        "RightHandMiddle4Link",
        "RightHandRing1Link",
        "RightHandRing2Link",
        "RightHandRing3Link",
        "RightHandRing4Link",
        "RightHandPinky1Link",
        "RightHandPinky2Link",
        "RightHandPinky3Link",
        "RightHandPinky4Link"
    );

#
## dict that constaints the mapping between source character models and dest
#
gOldToNewNodesDict = {}

def IsValidSkeletonNodeId(pIndex):
    return pIndex >= 0 and pIndex < FBSkeletonNodeId.kFBSkeletonLastIndex

def IsValidBodyNodeId(pIndex):
    return pIndex >= 0 and pIndex < FBBodyNodeId.kFBLastNodeId

def GetOrCreateSkeletonNode(srcCharacter, mappingPropName,pPrefix,pCreateIfNotExisting=True):

    global gOldToNewNodesDict

    lMappingProp = srcCharacter.PropertyList.Find(mappingPropName, False)

    srcSkeletonNode = None
    if lMappingProp != None:
        for i in range(lMappingProp.GetSrcCount() ):
            src = lMappingProp.GetSrc(i)
            if isinstance(src, FBModel):
                srcSkeletonNode = src
                break

    if srcSkeletonNode == None:
        return None
        
    lName = srcSkeletonNode.Name
    
    lSkeletonNode = srcSkeletonNode.Clone()
    lSkeletonNode.LongName = 'HandsRIG:' + lName
    lSkeletonNode.Show = True
    
    gOldToNewNodesDict[srcSkeletonNode] = lSkeletonNode
    
    return lSkeletonNode

# Map skeleton model to character characterization slot
def MapSkeletonBone(pCharacter, srcCharacter, mappingPropName, pPrefix):
    lModel = GetOrCreateSkeletonNode(srcCharacter, mappingPropName,pPrefix)
    
    if lModel != None and pCharacter != None:

        lMappingProp = pCharacter.PropertyList.Find(mappingPropName,False)
        if lMappingProp != None:
            lMappingProp.ConnectSrc(lModel)
        else:
            print 'Could not characterize: ' + lModel.Name + '; MappingName: ' + lMappingPropName


def RecoverHeirarchy(handsChar, srcChar):
    
    for enum in FBBodyNodeId.values:
        
        if not IsValidBodyNodeId(enum):
            continue
        
        srcModel = srcChar.GetModel( FBBodyNodeId(enum) )
        if srcModel == None:
            continue
            
        srcParent = srcModel.Parent
        
        if srcParent:
            if isinstance(srcParent, FBModelNull):
                parentEnum = FBBodyNodeId.kFBReferenceNodeId
            else:
                parentEnum = srcChar.GetIndexByModel( srcParent )
            
            if parentEnum != FBBodyNodeId.kFBInvalidNodeId:
                handsModel = handsChar.GetModel(FBBodyNodeId(enum))
                handsParent = handsChar.GetModel(parentEnum)
                
                if handsModel and handsParent:
                    m = FBMatrix()
                    srcModel.GetMatrix(m)
                    handsModel.SetMatrix(m)
                    
                    srcParent.GetMatrix(m)
                    handsParent.SetMatrix(m)
                    
                    handsModel.Parent = handsParent


def RecoverHeirarchy2():
    
    # using dict gOldToNewNodesDict
    
    m = FBMatrix()
    
    for srcModel in gOldToNewNodesDict.keys():
        dstModel = gOldToNewNodesDict[srcModel]
        
        srcParent = srcModel.Parent
        if srcParent:
            dstParent = gOldToNewNodesDict.get(srcParent, None)
            if dstParent == None:
                print ('srcParent is not a key! - ' + srcParent.Name)
            else:                
                srcParent.GetMatrix(m)
                dstParent.SetMatrix(m)
                
                dstModel.Parent = dstParent
        
        srcModel.GetMatrix(m)
        dstModel.SetMatrix(m)                
##

gApp = FBApplication()


def Main():

    currChar = gApp.CurrentCharacter
    if currChar == None:
        FBMessageBox('HandShape Scripts', 'Please choose a current character', 'Ok')
        return
    
    pPrefix = 'HandsRIG_'
    handsChar = FBCharacter(currChar.Name + '_Hands')
    handsChar.LongName = 'HandsRIG:' + currChar.Name
    
    for propName in lSkeletonNodeCharacterizationMapping:
        MapSkeletonBone(handsChar, currChar, propName, pPrefix)
    
    # DONE: recover heirarchy
    # DONE: recover transformation
    RecoverHeirarchy2()
    
    #handsChar.SetCharacterizeOn(True)
    FBSystem().Scene.Evaluate()
    '''
    refModel = handsChar.GetModel( FBBodyNodeId.kFBReferenceNodeId )
    if refModel:
        v = FBVector3d()
        refModel.GetVector(v)
        v[2] -= 100.0
        refModel.SetVector(v)
    '''
    #
        
Main()
