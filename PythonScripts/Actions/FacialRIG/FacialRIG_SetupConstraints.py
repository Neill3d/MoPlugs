
'''
 Script for Autodesk MotionBuilder 2014
 MoPLUGS Project (2015)
 
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

for lIdx in range( lMgr.TypeGetCount() ):

    # User feedback, if the python console is up.
    if "Aim" == lMgr.TypeGetName( lIdx ):
        aimIdx = lIdx
    elif "Position" == lMgr.TypeGetName( lIdx ):
        posIdx = lIdx
    elif "Parent/Child" == lMgr.TypeGetName( lIdx ):
        parentIdx = lIdx


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

# L_crease_1_jnt

PositionBind( "L_crease_1_jnt", "NOSTRIL_OUTER_LEFT", 68.0 )
PositionBind( "R_crease_1_jnt", "NOSTRIL_OUTER_RIGHT", 68.0 )

# L_crease_2_jnt

PositionBind( "L_crease_2_jnt", ["MOUTH_LIP_OUTER_CORNER_LEFT", "NOSTRIL_OUTER_LEFT"], 68.0 )
PositionBind( "R_crease_2_jnt", ["MOUTH_LIP_OUTER_CORNER_RIGHT", "NOSTRIL_OUTER_RIGHT"], 68.0 )

# L_crease_3_jnt

PositionBind( "L_crease_3_jnt", "MOUTH_LIP_OUTER_CORNER_LEFT", 68.0 )
PositionBind( "R_crease_3_jnt", "MOUTH_LIP_OUTER_CORNER_RIGHT", 68.0 )

# L_cheeks_4_jnt

PositionBind( "L_cheeks_3_jnt", "MOUTH_LIP_OUTER_CORNER_LEFT", 52.0 )
PositionBind( "L_cheeks_4_jnt", "MOUTH_LIP_OUTER_CORNER_LEFT", 47.0 )

PositionBind( "R_cheeks_3_jnt", "MOUTH_LIP_OUTER_CORNER_RIGHT", 52.0 )
PositionBind( "R_cheeks_4_jnt", "MOUTH_LIP_OUTER_CORNER_RIGHT", 47.0 )