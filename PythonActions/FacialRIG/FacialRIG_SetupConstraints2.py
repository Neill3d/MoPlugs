
'''
 Script for Autodesk MotionBuilder 2014
 MoPLUGS Project (2013-2016)
 
 Script do several steps to setup face rig in mobu
 
 1 - positions constraint for cheek and eye squeeze joints
 2 - aim constraint to rotate eyes
 3 - relation constraint to connect corrective blendshapes
 
 
 Sergey Solohin (Neill3d)
    e-mail to: s@neill3d.com
    www.neill3d.com
'''

# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

print "script"


lMgr = FBConstraintManager()

# we need Aim and Position constraints

aimIdx = 0
posIdx = 0
parentIdx = 0
relationIdx = 0

for lIdx in range( lMgr.TypeGetCount() ):

    # User feedback, if the python console is up.
    if "Aim" == lMgr.TypeGetName( lIdx ):
        aimIdx = lIdx
    elif "Position" == lMgr.TypeGetName( lIdx ):
        posIdx = lIdx
    elif "Parent/Child" == lMgr.TypeGetName( lIdx ):
        parentIdx = lIdx
    elif "Relation" == lMgr.TypeGetName( lIdx ):
        relationIdx = lIdx


def PositionBind( dstname, srcname, weight ):
        
    if parentIdx == 0:
        return 1
        
    lcon = lMgr.TypeCreateConstraint( parentIdx )
    lcon.Name = "Parent/Child " + dstname
    
    dst = FBFindModelByLabelName( dstname )
    lcon.ReferenceAdd(0, dst)
    
    if isinstance(srcname, list):
        for item in srcname: 
            src = FBFindModelByLabelName( item )
            lcon.ReferenceAdd(1, src)
    else:
        src = FBFindModelByLabelName( srcname )
        lcon.ReferenceAdd(1, src)
    
    lcon.Snap()
    lcon.Active = True 
    lcon.Weight = weight

def CreateOneComponent(srcName, parentComp):
    
    srcComp = FBFindModelByLabelName(srcName)
    dstComp = None
    
    if srcComp:
        name = srcComp.Name
        v = FBVector3d()        
        srcComp.GetVector(v)
        
        dstComp = FBModelNull( name + " Temp" )
        dstComp.Show = False
        
        dstComp.SetVector(v)
        
        if parentComp != None:
            dstComp.Parent = parentComp
        
    return dstComp
    

def CreateTempComponents():

    headRigRoot = FBFindModelByLabelName("HEAD_RIG_ROOT")
        
    tempRoot = FBModelNull( "Root_SecondaryTemp" )
    tempRoot.Show = False
    
    v = FBVector3d()
    
    if headRigRoot:
        headRigRoot.GetVector(v)
        tempRoot.SetVector(v)
        
        tempRoot.Parent = headRigRoot
    
    CreateOneComponent( "L_crease_1_jnt", tempRoot )
    CreateOneComponent( "L_crease_2_jnt", tempRoot )
    CreateOneComponent( "L_crease_3_jnt", tempRoot )
        
    CreateOneComponent( "R_crease_1_jnt", tempRoot )    
    CreateOneComponent( "R_crease_2_jnt", tempRoot )
    CreateOneComponent( "R_crease_3_jnt", tempRoot )

    CreateOneComponent( "L_cheeks_3_jnt", tempRoot )
    CreateOneComponent( "L_cheeks_4_jnt", tempRoot )
    
    CreateOneComponent( "R_cheeks_3_jnt", tempRoot )
    CreateOneComponent( "R_cheeks_4_jnt", tempRoot )
    
    CreateOneComponent( "L_upperCheeks_1_jnt", tempRoot )
    CreateOneComponent( "L_upperCheeks_2_jnt", tempRoot )    
    
    CreateOneComponent( "R_upperCheeks_1_jnt", tempRoot )
    CreateOneComponent( "R_upperCheeks_2_jnt", tempRoot )        

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

def RelationConnectTwoCompPos(con, nameSrc, nameDst, yPos):
    
    modelSrc = FBFindModelByLabelName(nameSrc)
    modelDst = FBFindModelByLabelName(nameDst)
    
    if modelSrc and modelDst:
        
        boxSrc = con.SetAsSource(modelSrc)
        boxDst = con.ConstrainObject(modelDst)
        
        con.SetBoxPosition(boxSrc, 0, yPos)
        con.SetBoxPosition(boxDst, 300, yPos)
        
        ConnectBoxes( boxSrc, boxDst, "Translation", "Translation" )
        
# create temp heirarchy

CreateTempComponents()
    
# L_crease_1_jnt

PositionBind( "L_crease_1_jnt Temp", "NOSTRIL_OUTER_LEFT", 68.0 )
PositionBind( "R_crease_1_jnt Temp", "NOSTRIL_OUTER_RIGHT", 68.0 )

# L_crease_2_jnt

PositionBind( "L_crease_2_jnt Temp", ["MOUTH_LIP_OUTER_CORNER_LEFT", "NOSTRIL_OUTER_LEFT"], 68.0 )
PositionBind( "R_crease_2_jnt Temp", ["MOUTH_LIP_OUTER_CORNER_RIGHT", "NOSTRIL_OUTER_RIGHT"], 68.0 )

# L_crease_3_jnt

PositionBind( "L_crease_3_jnt Temp", "MOUTH_LIP_OUTER_CORNER_LEFT", 68.0 )
PositionBind( "R_crease_3_jnt Temp", "MOUTH_LIP_OUTER_CORNER_RIGHT", 68.0 )

# L_cheeks_4_jnt

PositionBind( "L_cheeks_3_jnt Temp", "MOUTH_LIP_OUTER_CORNER_LEFT", 52.0 )
PositionBind( "L_cheeks_4_jnt Temp", "MOUTH_LIP_OUTER_CORNER_LEFT", 47.0 )

PositionBind( "R_cheeks_3_jnt Temp", "MOUTH_LIP_OUTER_CORNER_RIGHT", 52.0 )
PositionBind( "R_cheeks_4_jnt Temp", "MOUTH_LIP_OUTER_CORNER_RIGHT", 47.0 )

# upperCheeks

PositionBind( "L_upperCheeks_1_jnt Temp", "EYE_CORNER_INNER_LEFT", 50.0 )
PositionBind( "L_upperCheeks_2_jnt Temp", "EYE_BOTTOM_LEFT", 50.0 )

PositionBind( "R_upperCheeks_1_jnt Temp", "EYE_CORNER_INNER_RIGHT", 50.0 )
PositionBind( "R_upperCheeks_2_jnt Temp", "EYE_BOTTOM_RIGHT", 50.0 )


# relation constraint between temproary and origin joints

if relationIdx > 0:

    lcon = lMgr.TypeCreateConstraint( relationIdx )
    lcon.Name = "Relation Secondary Joints"
    
    RelationConnectTwoCompPos( lcon, "L_crease_1_jnt Temp", "L_crease_1_jnt", 0 )
    RelationConnectTwoCompPos( lcon, "L_crease_2_jnt Temp", "L_crease_2_jnt", 100 )    
    RelationConnectTwoCompPos( lcon, "L_crease_3_jnt Temp", "L_crease_3_jnt", 200 ) 
    
    RelationConnectTwoCompPos( lcon, "R_crease_1_jnt Temp", "R_crease_1_jnt", 300 )
    RelationConnectTwoCompPos( lcon, "R_crease_2_jnt Temp", "R_crease_2_jnt", 400 )    
    RelationConnectTwoCompPos( lcon, "R_crease_3_jnt Temp", "R_crease_3_jnt", 500 ) 

    RelationConnectTwoCompPos( lcon, "L_cheeks_3_jnt Temp", "L_cheeks_3_jnt", 600 )    
    RelationConnectTwoCompPos( lcon, "L_cheeks_4_jnt Temp", "L_cheeks_4_jnt", 700 ) 

    RelationConnectTwoCompPos( lcon, "R_cheeks_3_jnt Temp", "R_cheeks_3_jnt", 800 )    
    RelationConnectTwoCompPos( lcon, "R_cheeks_4_jnt Temp", "R_cheeks_4_jnt", 900 )
    
    RelationConnectTwoCompPos( lcon, "L_upperCheeks_1_jnt Temp", "L_upperCheeks_1_jnt", 1000 )    
    RelationConnectTwoCompPos( lcon, "L_upperCheeks_2_jnt Temp", "L_upperCheeks_2_jnt", 1100 )      

    RelationConnectTwoCompPos( lcon, "R_upperCheeks_1_jnt Temp", "R_upperCheeks_1_jnt", 1200 )    
    RelationConnectTwoCompPos( lcon, "R_upperCheeks_2_jnt Temp", "R_upperCheeks_2_jnt", 1300 )          