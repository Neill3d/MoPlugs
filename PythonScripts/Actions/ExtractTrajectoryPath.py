
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import*

# options
skipFrames = 100
usePathConstraint = False
useGlobalTransform = True

#
models = FBModelList()
FBGetSelectedModels(models)

if len(models) > 0:
    model = models[0]
    print model.Name
    matrix = FBMatrix()
    matrix.Identity()
    
    if model.Parent:
        model.Parent.GetMatrix(matrix)
        
    animNode = model.Translation.GetAnimationNode()
    
    if animNode:
        keyCount = len(animNode.Nodes[0].FCurve.Keys)
        print keyCount
        
        
        if keyCount > 0:
            newPath = FBModelPath3D("3d path")
            newPath.Show = True
            
            if not useGlobalTransform:
                newPath.SetMatrix(matrix)
                
            point = FBVector4d()
            for i in range(0, keyCount, skipFrames):        

                time = animNode.Nodes[0].FCurve.Keys[i].Time
                point[0] = animNode.Nodes[0].FCurve.Evaluate(time)
                point[1] = animNode.Nodes[1].FCurve.Evaluate(time)
                point[2] = animNode.Nodes[2].FCurve.Evaluate(time)
                
                if useGlobalTransform:          
                    FBVectorMatrixMult(point, matrix, point)
                                        
                newPath.PathKeyEndAdd(point)
                
        
            #PATCH: Remove the two first point, they are unnecessary
            newPath.PathKeyRemove(0);
            newPath.PathKeyRemove(0);
            
            if usePathConstraint:
                
                newConstraint = FBConstraintManager().TypeCreateConstraint("Path")
                if newConstraint:
                    newConstraint.ReferenceAdd(0, model)
                    newConstraint.ReferenceAdd(1, newPath)
                    newConstraint.Active = True
                    newConstraint.PropertyList.Find("FollowPath").Data = True
                
            if not useGlobalTransform:  
                newPath.SetMatrix(matrix)
                    
                