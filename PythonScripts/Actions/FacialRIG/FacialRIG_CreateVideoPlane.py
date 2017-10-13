
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

newPlane = FBModelPlane("VideoPlane")

newPlane.Show = True

newPlane.Translation = FBVector3d(50.0, 50.0, 0.0)
newPlane.Rotation = FBVector3d(90.0, 0.0, 0.0)
newPlane.Scaling = FBVector3d(0.5, 0.5, 0.5)

# link to group
newGroup = FBGroup("VIDEO_PLANE")
newGroup.Items.append(newPlane)

# parent to Optical data

opticalModel = FBFindModelByLabelName("FaceFeaturesOptical")

if opticalModel:
    newPlane.Parent = opticalModel

