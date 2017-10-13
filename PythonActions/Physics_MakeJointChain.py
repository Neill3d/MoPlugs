
# script for making a physics joints chain
# use all the joints heirarchy
# Steps are:
#   1 - make nulls between joints
#   2 - make marker capsule for joints collision
#   3 - make a physics joint and setup all the connections
# setup physics joints

# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

# TODO: connect joints and joint markers with Parent\Child constraint

from pyfbsdk import*

models = FBModelList()
FBGetSelectedModels(models)

def setupJointPivots(joint, markerRoot):
    
    v = FBVector3d()
    joint.GetVector(v)
    
    null = FBModelNull( "JointNull" )
    null.Show = True
    null.Visible = True
    null.Parent = markerRoot
    
    null.SetVector(v)
    
    if len(joint.Children) > 0:
        setupJointPivots(joint.Children[0], markerRoot)
    

def setupJointMarkers(joint, markerRoot):
    
    if len(joint.Children) != 1:
        return
    
    child = joint.Children[0]
    
    v1 = FBVector3d()
    v2 = FBVector3d()
    markerV = FBVector3d()
    
    joint.GetVector(v1)
    child.GetVector(v2)
    
    # compute joint len
    v = FBVector4d(v2[0]-v1[0], v2[1]-v1[1], v2[2]-v1[2], 1.0)
    vlen = FBLength(v)
    
    markerV[0] = v1[0] + 0.5 * (v2[0]-v1[0])
    markerV[1] = v1[1] + 0.5 * (v2[1]-v1[1])
    markerV[2] = v1[2] + 0.5 * (v2[2]-v1[2])
            
    print vlen
    
    marker = FBCreateObject( 'Browsing/Templates/Elements', 'Marker', 'JointMarker' )
    marker.Show = True
    marker.Visible = True
    marker.Parent = markerRoot
    
    marker.SetVector(markerV)

    rot = FBVector3d(0.0, 90.0, 0.0)
    marker.SetVector(rot, FBModelTransformationType.kModelRotation, False)

    marker.PropertyList.Find('Size').Data = 100.0 * 0.25 * vlen
    marker.PropertyList.Find('Length').Data = 100.0 * 0.4 * vlen
    marker.PropertyList.Find('LookUI').Data = 4

    newConstraint = FBConstraintManager().TypeCreateConstraint("Parent/Child")
    if newConstraint:
        newConstraint.ReferenceAdd(0, joint)
        newConstraint.ReferenceAdd(1, marker)
        newConstraint.Snap()
        newConstraint.Active = True
        
        newConstraint.Name = "Parent/Child " + joint.Name


    # go to next element in the chain
    setupJointMarkers(child, markerRoot)


# Create the necessary relationships within a Joint Physical Property object.
def createJoints(pNullRoot, pMarkerRoot):
    # The 'joint' object maintains the physical property relations between model pairs and
    # their respective points of rotation.
    joint = FBCreateObject( 'Browsing/Templates/Physical Properties', 'Joint', 'Joint' )
    
    # Bind each '(parent, child)' key and its associated 'nullModel' to the joint object.
    for i in range(1, len(pNullRoot.Children)-1):
        nullModel = pNullRoot.Children[i]
        parent = pMarkerRoot.Children[i-1]
        child = pMarkerRoot.Children[i]
        
        # Connect the joint to the nullModel to define the point of rotation for this (parent, child) pair
        FBConnect(joint, nullModel)        
        
        # Connect the parent and child models to the joint to have them use the nullModel
        # as a point of rotation.
        for component in joint.Components:
            connectionProp = None
            if component.Name == nullModel.Name + ' Joint':
                connectionProp = component.PropertyList.Find('Connections')
                if connectionProp != None:
                    # (!!!) Note: The order of the parameters in FBConnect() is 
                    #       important!
                    FBConnect(parent, connectionProp)
                    FBConnect(child, connectionProp)

############################################################################
#
#

if len(models) == 1:
    
    #
    # prepare pivot points for physics joint
    
    nullRoot = FBModelNull("JointNull ROOT")
    nullRoot.Show = True
    nullRoot.Visible = True
    
    setupJointPivots(models[0], nullRoot)
    
    #
    # prepare collision meshes
    
    markerRoot = FBModelNull("JointMarkers ROOT")
    markerRoot.Show = True
    markerRoot.Visible = True
    
    setupJointMarkers(models[0], markerRoot)
    
    #
    # setup the physics joint connections
    
    createJoints(nullRoot, markerRoot)