#!/usr/bin/env python3

from zencad import *

m = box(100,90,1.5)

r = cylinder(r=1.4, h=1.5)
r += cylinder(r=1.4, h=1.5).move(48, -1)
r += cylinder(r=1.4, h=1.5).move(32, -52, 0)

m = m - r.move(46,60,0)

m += (box(35,30,12) - box(24.8,30,12).moveX(5)).moveY(53)

to_stl(m, "model.stl", 0.01)

disp(m)
show()

