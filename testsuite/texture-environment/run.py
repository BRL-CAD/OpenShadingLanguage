# Copyright Contributors to the Open Shading Language project.
# SPDX-License-Identifier: BSD-3-Clause
# https://github.com/AcademySoftwareFoundation/OpenShadingLanguage
#!/usr/bin/env python 

command += oiiotool("-pattern fill:topleft=0,0,0:topright=1,0,0:bottomleft=0,1,0:bottomright=1,1,1 256x128 3 -d uint8 -oenv ramp.env")
command += testshade("-g 256 128 -od uint8 -o Cout out.tif test")
outputs = [ "out.tif" ]

# A uniform direction has no derivative storage and must behave like explicit zeros.
command += testshade("-g 16 16 -o Cout implicit.tif no_derivs")
command += testshade("-g 16 16 -param explicit_derivs 1 -o Cout explicit.tif no_derivs")
command += oiiotool("implicit.tif explicit.tif --diff")
