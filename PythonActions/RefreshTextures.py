
#################################################################
#
# Script for reloading textures and updating in the viewport
#
# Author: Sergey Solohin (Neill3d) 2013, e-mail to: s@neill3d.com
#  www.neill3d.com
#
#################################################################

from pyfbsdk import *

gSystem = FBSystem()

c = 0

# try to refresh only selected
for clip in gSystem.Scene.VideoClips:
    if isinstance(clip, FBVideoClip) and clip.Selected:
        fname = clip.Filename
        clip.Filename = fname
        c = c + 1
        
# without selection - refresh all clips
if c == 0:
    print "Refresh all clips!"
    for clip in gSystem.Scene.VideoClips:
        if isinstance(clip, FBVideoClip):
            fname = clip.Filename
            clip.Filename = fname