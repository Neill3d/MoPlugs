
# script to test deformation along a curve

# Author: Sergey Solohin (Neill3d), e-mail to: s@neill3d.com
#  www.neill3d.com
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *
import math

mdl = FBFindModelByLabelName( "pPlane1" )
curve = FBFindModelByLabelName( "3D Curve" )

# calculate object width
vmin = FBVector3d()
vmax = FBVector3d()
mat = FBMatrix()
scl = FBVector3d()

mdl.GetMatrix(mat, FBModelTransformationType.kModelScaling)
mdl.GetBoundingBox(vmin, vmax)

vmin4 = FBVector4d()
vmax4 = FBVector4d()

FBVectorMatrixMult(vmin4, mat, FBVector4d(vmin[0], vmin[1], vmin[2], 1.0))
FBVectorMatrixMult(vmax4, mat, FBVector4d(vmax[0], vmax[1], vmax[2], 1.0))

mdlLen = vmax4[0] - vmin4[0]

# calculate curve length
curveLen = curve.PathLength

# calculate position of a mesh on a curve

f = 100.0 * 0.5 * mdlLen / curveLen


pivotPos = curve.Total_GlobalPathEvaluate( f )
der = curve.Total_GlobalPathEvaluateDerivative( f )

angle = 90.0 + 180.0 / 3.1415 * math.atan2(der[0], der[2])

centerTM = FBMatrix()
FBTRSToMatrix( centerTM, pivotPos, FBVector3d(0.0, angle, 0.0), FBSVector(1.0, 1.0, 1.0) )

mdl.SetVector( FBVector3d(pivotPos[0], pivotPos[1], pivotPos[2]) )
mdl.SetVector( FBVector3d(0.0, angle, 0.0), FBModelTransformationType.kModelRotation )


# Begin modify geometry 
lGeometry = mdl.Geometry
lGeometry.GeometryBegin()

posList = lGeometry.GetPositionsArray()
numberOfPoints = len(posList)
print numberOfPoints
# Add shape 
lShape0Idx = lGeometry.ShapeAdd("Shape 7")

# Init Shape
lGeometry.ShapeInit(lShape0Idx, numberOfPoints, False)

mdlMatrix = FBMatrix()
mdl.GetMatrix(mdlMatrix)
invMatrix = FBMatrix(mdlMatrix)
invMatrix.Inverse()

# Set shape diff point
for i in range(0, numberOfPoints):    
    
    vertex = FBVertex( lGeometry.VertexGet(i) )
    #vertex[0] = 0.0
    
    # apply only scaling, but still not translation and rotation
    transformedVertex = FBVertex()
    FBVertexMatrixMult( transformedVertex, mat, vertex )

    # this is when we have direction in -X    
    vertexLen = mdlLen - (transformedVertex[0] - vmin4[0])
    vertexF = 100.0 * vertexLen / curveLen
    
    transformedVertex[0] = 0.0
    
    pos = curve.Total_GlobalPathEvaluate( vertexF )
    der = curve.Total_GlobalPathEvaluateDerivative( vertexF )
    
    angle = 90.0 + 180.0 / 3.1415 * math.atan2(der[0], der[2])
    
    curTM = FBMatrix()
    FBRotationToMatrix( curTM, FBVector3d(0.0, angle, 0.0) )
    
    rotatedVertex = FBVertex()
    FBVertexMatrixMult( rotatedVertex, curTM, transformedVertex )
    
    newVertexPos = FBVector4d( pos[0]+rotatedVertex[0], pos[1]+rotatedVertex[1], pos[2]+rotatedVertex[2], 1.0 )
    FBVectorMatrixMult( newVertexPos, invMatrix, newVertexPos )
    
    diff = FBVertex(newVertexPos[0]-vertex[0], newVertexPos[1]-vertex[1], newVertexPos[2]-vertex[2], 0.0)
    lGeometry.ShapeSetDiffPoint(lShape0Idx, i, i, diff)
#lGeometry.ShapeSetDiffPoint(lShape0Idx, 1, 2, FBVertex(0.0, 100.0, 10.0, 0.0));

# End modify geometry, notify for synchronization
lGeometry.GeometryEnd()