
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

lModels = FBModelList()
FBGetSelectedModels(lModels)

def SetupHierarchy(pModel, pParentIdx, pNextIndex):
    lIdx = pNextIndex
    pNextIndex += 1
    lT = FBVector3d()
    lR = FBVector3d()
    pModel.GetVector(lT,FBModelTransformationType.kModelTranslation)
    pModel.GetVector(lR,FBModelTransformationType.kModelRotation)
    print '//'+str(lIdx)+':'+pModel.Name
    print 'mChannel['+str(lIdx)+'].mName = "' + pModel.Name + '";'
    print 'mChannel['+str(lIdx)+'].mParentChannel = ' +  str(pParentIdx) + ';'
    print 'mChannel['+str(lIdx)+'].mT[0] = ' +  str(lT[0]) + ';'
    print 'mChannel['+str(lIdx)+'].mT[1] = ' +  str(lT[1]) + ';'
    print 'mChannel['+str(lIdx)+'].mT[2] = ' +  str(lT[2]) + ';'
    print 'mChannel['+str(lIdx)+'].mR[0] = ' +  str(lR[0]) + ';'
    print 'mChannel['+str(lIdx)+'].mR[1] = ' +  str(lR[1]) + ';'
    print 'mChannel['+str(lIdx)+'].mR[2] = ' +  str(lR[2]) + ';'
    print ''
    for lChild in pModel.Children:
        pNextIndex = SetupHierarchy(lChild,lIdx,pNextIndex)
    return pNextIndex

if len(lModels) == 1:
    SetupHierarchy(lModels[0],-1,0)