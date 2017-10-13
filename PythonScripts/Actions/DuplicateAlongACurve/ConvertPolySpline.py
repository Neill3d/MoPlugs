
## convert polyspline into 3d path

# Author: Sergey Solohin (Neill3d), e-mail to: s@neill3d.com
#  www.neill3d.com
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *
import pyediting

def ConvertPolySpline(polySpline, skipCount):

    m = FBMatrix()
    polySpline.GetMatrix(m)

    scalingM = FBMatrix()
    polySpline.GetMatrix(scalingM, FBModelTransformationType.kModelScaling)

    vertices = pyediting.GetVertexArray(polySpline, False)
    print len(vertices)

    curve = FBModelPath3D("Spline_" + polySpline.Name)
    curve.Show = True
    curve.Visible = True
    
    skip = 0
    p2 = FBVertex()
    
    for point in vertices:    
        if skip == 0:
            FBVertexMatrixMult(p2, scalingM, point)
            curve.PathKeyEndAdd( FBVector4d(p2[0], p2[1], p2[2], 1.0) )

        skip += 1
        if skip > skipCount:
            skip = 0    
    
    #PATCH: Remove the two first point, they are unnecessary
    curve.PathKeyRemove(0)
    curve.PathKeyRemove(0)
    
    # tangents
    
    count = curve.PathKeyGetCount()
    
    for i in range(count):
        curve.PathKeySetXYZDerivative(i, FBVector4d(0.0, 0.0, 0.0, 1.0), False)
    
    curve.SetMatrix(m)
    curve.SetVector( FBVector3d(1.0, 1.0, 1.0), FBModelTransformationType.kModelScaling )
    
    # END

models = FBModelList()
FBGetSelectedModels(models)

btn, value = FBMessageBoxGetUserValue("Convert PolySpline", "Number Of Points To Skip", 10, FBPopupInputType.kFBPopupInt,"Ok")

for model in models:
    ConvertPolySpline(model, value)

