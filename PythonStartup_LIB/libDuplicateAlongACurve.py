
##############################################################
#
# script uses local X axis to compute element Length and snapshot it along a curve
# 21.02.2016
# Sergey Solohin (Neill3d) 2016
#   e-mail to: s@neill3d.com
#       www.neill3d.com
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
############################################################

# select source, then destination. Script creates relation constraint between all joints elements


from pyfbsdk import *
import math


class Singleton(object):
    obj = None                           # 
    def __new__(cls,*dt,**mp):           # 
       if cls.obj is None:               # 
          cls.obj = object.__new__(cls,*dt,**mp) # 
       return cls.obj                    # 

    

class CDublicateAlongACurve():

    tempFileName = "\\tempCopy.fbx"
    
    mApp = FBApplication()
    mSystem = FBSystem()
    
    mWallBricksId = 0
    mPathWrapId = 0
    
    # store last elements for the unit test access
    mLastOffsetRoot = None
    mLastOffsets = []
    mLastModels = []
    
    mLastConstraint = None
    
    def __init__(self):
        
        self.mWallBricksId, self.mPathWrapId = self.CheckForConstraints()
        
        print self.mWallBricksId
        print self.mPathWrapId  
    
    def GetLastConstraint(self):
        return self.mLastConstraint
    
    def GetLastOffsetRoot(self):
        return self.mLastOffsetRoot
        
    def GetLastOffsets(self):
        return self.mLastOffsets
    
    def GetLastModels(self):
        return self.mLastModels
    
    @staticmethod
    def CheckForConstraints():
        
        lMgr = FBConstraintManager()

        # we need Aim and Position constraints
        
        lWallBricksId = 0
        lPathWrapId = 0
        
        for lIdx in range( lMgr.TypeGetCount() ):
        
            # User feedback, if the python console is up.
            if "Wall Bricks Along a curve" == lMgr.TypeGetName( lIdx ):
                lWallBricksId = lIdx
            elif "Path Wrap" == lMgr.TypeGetName( lIdx ):
                lPathWrapId = lIdx
        
        return [lWallBricksId, lPathWrapId]

    @staticmethod
    def SetupWallBricksConstraint(constraintId, curve, newoffsets):

        lMgr = FBConstraintManager()
        constraint = lMgr.TypeCreateConstraint( constraintId )
        #constraint.PropertyList.Find("Brick Side Length").Data = cubeLength
        
        constraint.ReferenceAdd( 0, curve )
        for offset in newoffsets:
            constraint.ReferenceAdd( 1, offset )
    
        return constraint
    
    @staticmethod
    def SetupPathWrapConstraint(constraintId, curve, components):

        lMgr = FBConstraintManager()        
        constraint = lMgr.TypeCreateConstraint( constraintId )
        constraint.ReferenceAdd( 0, curve )
        for comp in components:
            constraint.ReferenceAdd( 1, comp )

        return constraint

    @staticmethod
    def normal(vector):
        v = FBVector4d(vector)
        length = FBLength(v)
        if length > 0.0:
            FBMult(v, v, 1.0/length)
        
        return v
        
    @staticmethod    
    def LookAtMatrix(Eye, At, Up):
        
        zaxis = CDublicateAlongACurve.normal( FBVector4d(At[0]-Eye[0], At[1]-Eye[1], At[2]-Eye[2], 1.0) )
        temp = FBVector4d()
        # cross product
        FBMult(temp, Up, zaxis) 
        xaxis = CDublicateAlongACurve.normal(temp)
        yaxis = FBVector4d()
        FBMult( yaxis, zaxis, xaxis )
        
        m = FBMatrix()
        m.Identity()
        
        m[0] = xaxis[0]
        m[1] = yaxis[0]
        m[2] = zaxis[0]
        
        m[4] = xaxis[1]
        m[5] = yaxis[1]
        m[6] = zaxis[1]
    
        m[8] = xaxis[2]
        m[9] = yaxis[2]
        m[10] = zaxis[2]
        
        m[12] = -FBDot(xaxis, Eye)
        m[13] = -FBDot(yaxis, Eye)
        m[14] = -FBDot(zaxis, Eye)
    
        FBMatrixTranspose(m, m)
        return m
        
    @staticmethod
    def NormalToMatrix(normal):
        
        # Find a vector in the plane
        tangent0 = FBVector4d()
        tangent1 = FBVector4d()
        
        FBMult( tangent0, normal, FBVector4d(1.0, 0.0, 0.0, 1.0) );
    
        if (FBDot(tangent0, tangent0) < 0.001):
            FBMult(tangent0, normal, FBVector4d(0.0, 1.0, 0.0, 1.0))
    
        FBMult( tangent0, tangent0, 1.0 / FBLength(tangent0) )
        # Find another vector in the plane
        FBMult( tangent1, normal, tangent0 )
        FBMult( tangent1, tangent1, 1.0 / FBLength(tangent1) )
        
        # Construct a 3x3 matrix by storing three vectors in the columns of the matrix
        m = FBMatrix()
        m.Identity()
        m[0] = tangent0[0]
        m[1] = tangent0[1]
        m[2] = tangent0[2]
        m[4] = tangent1[0]
        m[5] = tangent1[1]
        m[6] = tangent1[2]
        m[8] = normal[0]
        m[9] = normal[1]
        m[10] = normal[2]
        
        return m
        
    @staticmethod
    def GetSelectedModels():
        models = FBModelList()
        FBGetSelectedModels(models)
        
        if len(models) != 2:
            return [None, None]
        
        curve = None
        brick = None
        
        if isinstance(models[0], FBModelPath3D):
            curve = models[0]
            brick = models[1]
        elif isinstance(models[1], FBModelPath3D):
            curve = models[1]
            brick = models[0]
        
        if curve == None:
            return [None, None]

        return [curve, brick]
        
    #############################################################
    ## DuplicateAlongCurve
    #
    # curve - 3d path model
    # brick - model to do a snapshot along a curve
    # silentMode - True/False if you want to avoid any messages raise
    # percentOfLength - usage % of a brick length (along the +Z axis) value [0; 100]
    # duplicateAxis - 1 means +X, 2 means +Y, 3 means +Z
    # duplicateType - 1 means Instances, 2 means Copies
    # duplicateAlignType - 1 means Up Vector, 2 means lookAt next element
    # setupConstraint - 1 means Wall Bricks, 2 means Path Deform, other values means no constraint needed
    #
    def __call__(self, curve, brick, silentMode, percentOfLength, duplicateAxis, duplicateType, duplicateAlignType, setupConstraint):
        
        if curve == None and brick == None:
            [curve, brick] = self.GetSelectedModels()
        
        if curve == None or brick == None:
            return 0
        
        vmin = FBVector3d()
        vmax = FBVector3d()
        mat = FBMatrix()
        globalMat = FBMatrix()
        
        brick.GetMatrix(mat, FBModelTransformationType.kModelTransformation_Geometry, False)
        brick.GetMatrix(globalMat)
        mat = globalMat
        
        brick.GetBoundingBox(vmin, vmax)
        
        vmin4 = FBVector4d()
        vmax4 = FBVector4d()
        
        FBVectorMatrixMult(vmin4, mat, FBVector4d(vmin[0], vmin[1], vmin[2], 1.0))
        FBVectorMatrixMult(vmax4, mat, FBVector4d(vmax[0], vmax[1], vmax[2], 1.0))
        
        if not silentMode:
            btn, value = FBMessageBoxGetUserValue("Duplicate Along A Curve","Use % of length:", 90.0, FBPopupInputType.kFBPopupDouble,"Ok")
            percentOfLength = value
        
        # we use +X in world coord system
        
        if duplicateAxis == 1:    
            cubeLength = math.fabs(vmax4[0] - vmin4[0])
        elif duplicateAxis == 2:
            cubeLength = math.fabs(vmax4[1] - vmin4[1])
        elif duplicateAxis == 3:
            cubeLength = math.fabs(vmax4[2] - vmin4[2])
            
        cubeLength = 0.01 * percentOfLength * cubeLength
        
        FBVectorMatrixMult(vmin4, globalMat, FBVector4d(vmin[0], vmin[1], vmin[2], 1.0))
        FBVectorMatrixMult(vmax4, globalMat, FBVector4d(vmax[0], vmax[1], vmax[2], 1.0))
        
        heightOffset = 0.5 * math.fabs(vmax4[1] - vmin4[1])
        print cubeLength
        totalLength = curve.PathLength
        
        numberOfCopies = (int) (totalLength / cubeLength)
        print numberOfCopies
        
        curvemat = FBMatrix()
        curve.GetMatrix(curvemat)
        
        cubepercent = 100 * cubeLength / totalLength
        f = 0.5 * cubepercent
        
        #undoman = FBUndoManager()
        #undoman.TransactionBegin("curveSnapshot")
        
        newbricks = []
        newoffsets = []
        
        parentOffset = FBModelNull( "OffsetRoot" )
        parentOffset.Show = True
        
        self.mLastOffsetRoot = parentOffset
        
        UserConfigPath = self.mSystem.UserConfigPath
        
        #
        tempName = brick.LongName
        elementName = "someTempName"
        brick.LongName = elementName
        
        # prepare a copy
        if duplicateType == 2:
            
            curve.Selected = False
            
            lOptions = FBFbxOptions(False)
            lOptions.SaveSelectedModelsOnly = True
            
            takeCount = lOptions.GetTakeCount()
            for nTake in range(takeCount):
                lOptions.SetTakeSelect(nTake, False)
            
            
            self.mApp.FileSave( UserConfigPath + self.tempFileName, lOptions )
        
            brick.LongName = tempName
        
        for i in range(0, numberOfCopies):
        
            pos = curve.Total_GlobalPathEvaluate( f )
            nextpos = curve.Total_GlobalPathEvaluate( f + cubepercent )
            der = curve.Total_GlobalPathEvaluateDerivative( f )
            FBAdd( nextpos, pos, der )
            
            angle = 90.0 + 180.0 / 3.1415 * math.atan2(der[0], der[2])
            
            # TODO: convert der into rotation
            #rotMatrix = NormalToMatrix(der)
            
            rotMatrix = FBMatrix()
            
            if duplicateAlignType == 1:
                FBRotationToMatrix(rotMatrix, FBVector3d(0.0, angle, 0.0) )            
            else:
                rotMatrix = CDublicateAlongACurve.LookAtMatrix(pos, nextpos, FBVector4d(0.0, 1.0, 0.0, 1.0) )
            
            newOffset = FBModelNull("offset")
            newOffset.Show = True
            newOffset.Parent = parentOffset
            newOffset.SetVector( FBVector3d(pos[0], pos[1], pos[2]) )
            
            # TODO: add creation percent for debug
            
            lProp = newOffset.PropertyCreate("CreationPercent", FBPropertyType.kFBPT_double, "Number", False, False, None)
            
            if lProp:
                lProp.Data = f
    
            lProp = newOffset.PropertyCreate("CreationSize", FBPropertyType.kFBPT_double, "Number", False, False, None)
            
            if lProp:
                lProp.Data = cubepercent
            
            if duplicateType == 1:
                newBrick = brick.Clone()
            else:
                lOptions = FBFbxOptions(True)
                lOptions.SetAll( FBElementAction.kFBElementActionMerge, True )
        
                self.mApp.FileAppend( UserConfigPath+self.tempFileName, False, lOptions )
                
                newBrick = FBFindModelByLabelName( elementName )
                
                if newBrick == None:
                    print "missing element!"
                    continue
                    
                newBrick.LongName = tempName + "_Clone"
                
            newBrick.Show = True
            newBrick.Parent = newOffset
            #newBrick.SetVector( FBVector3d(pos[0], pos[1]+heightOffset, pos[2]) )
            newBrick.SetVector( FBVector3d(0.0, heightOffset, 0.0), FBModelTransformationType.kModelTranslation, False )
            #newBrick.SetVector( FBVector3d(0.0, 0.0, 0.0), FBModelTransformationType.kModelRotation, False )
            
            #newOffset.SetVector( FBVector3d(0.0, angle, 0.0), FBModelTransformationType.kModelRotation )
            newOffset.SetMatrix( rotMatrix, FBModelTransformationType.kModelRotation )
                
            #undoman.TransactionAddObjectDestroy(newBrick)
            
            newbricks.append(newBrick)
            newoffsets.append(newOffset)
            
            f = f + cubepercent
            
            # END FOR
        
        
        self.mLastOffsets = newoffsets
        self.mLastModels = newbricks
        
        if not silentMode:
            FBMessageBox( "Duplicate Along A Curve", str(numberOfCopies) + " elements populated", "Ok" )
        
        #undoman.TransactionEnd()
        
        #
        #
        
        mLastConstraint = None
        if setupConstraint == 1 and self.mWallBricksId > 0:
            self.mLastConstraint = self.SetupWallBricksConstraint(self.mWallBricksId, curve, newoffsets)
        elif setupConstraint == 2 and self.mPathWrapId > 0:
            self.mLastConstraint = self.SetupPathWrapConstraint(self.mPathWrapId, curve, newbricks)
        else:
            return 0
        
        return 1
#
    