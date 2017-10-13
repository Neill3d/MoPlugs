
# script to make a reflection setup for windows
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

gWindowsNames = ['WINDOWS', 'WINDOWS 1', 'WINDOWS 2', 'WINDOWS 3', 'WINDOWS 4', 'WINDOWS 5']
gWindowsModels = []

def FindParent(model):
    
    parent = model.Parent
    
    if None != parent:
        parent = FindParent(parent)
    else:
        parent = model
        
    return parent
    
    # end
    
def LocateWindowModel(parent, windowsList):
    
    for child in parent.Children:
        LocateWindowModel(child, windowsList)

    if parent.Name in gWindowsNames:
        windowsList.append(parent)
    
modelList = FBModelList()
FBGetSelectedModels(modelList)

gWindowsModels = []

if len(modelList) > 0:
    model = modelList[0]
    print model.Name
    
    parent = FindParent(model)
    print parent.Name
    
    LocateWindowModel(parent, gWindowsModels)
    
    print len(gWindowsModels)
    
    if len(gWindowsModels) == len(gWindowsNames):
        # we have found all window models
        
        
        # check for unique material
        isUnique = False
        
        model = gWindowsModels[0]
        if len(model.Materials) > 0:            
            mat = model.Materials[0]
            
            matModels = 0
            count = mat.GetDstCount()
            
            for i in range(count):
                plug = mat.GetDst(i)
                if isinstance(plug, FBModel):
                    matModels += 1
            
            print matModels
            if matModels == len(gWindowsModels):
                print 'same len'
                isUnique = True
        
        mat = None
        if False == isUnique:
            mat = FBMaterial( 'Mat Main Windows' )
        
        #
        # check for shader
        
        isUnique = False
        
        model = gWindowsModels[0]
        if len(model.Shaders) > 0:            
            shader = model.Shaders[0]
            
            shaderModels = 0
            count = shader.GetDstCount()
            
            for i in range(count):
                plug = shader.GetDst(i)
                if isinstance(plug, FBModel):
                    shaderModels += 1
            
            print shaderModels
            if shaderModels == len(gWindowsModels):
                print 'same len'
                isUnique = True
        
        newShader = None
        if False == isUnique:
            newShader = obj = FBCreateObject( 'Browsing/Templates/Shading Elements/Shaders', 'Projective Mapping', 'Projective Main Windows' )
        
        
        #
        #
        
        for model in gWindowsModels:
        
            model.Show = True
            model.Visibility = True
            
            # fix for window 1
            if model.Name == 'WINDOWS 1':
                v = FBVector3d(-14.62, 41.91, -51.91)
                r = FBVector3d(89.99, 90.0, 0.0)
                
                model.SetVector(v, FBModelTransformationType.kModelTranslation, False)
                model.SetVector(r, FBModelTransformationType.kModelRotation, False)
            
            #if len(model.Materials) > 0:
            #    model.Materials.ReplaceSrcAt(0, mat)
            
            if newShader != None:
                model.Shaders.removeAll()
                model.Shaders.append(newShader)
    
    