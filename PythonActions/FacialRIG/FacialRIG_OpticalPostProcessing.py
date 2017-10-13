
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#

from pyfbsdk import *

lFilter = FBFilterManager().CreateFilter("Butterworth")

if lFilter:
    
    for prop in lFilter.PropertyList:
        print prop.Name
        print prop.Data
