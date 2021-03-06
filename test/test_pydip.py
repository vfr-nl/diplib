import PyDIP as dip

# These work with the "alternate" (Python-style) indexing
# a = dip.Image((10,20),1)
# a.Fill(3)
# b = a[0:4,4:-1]
# b.Fill(55)
# a[:3,:10] = 100
# a[5:7,10:15] = 200
# a.Show('normal')
#
# m = a>=100
# m.Show()
#
# a[m].Show('normal')
#
# a[m] = 176
# a.Show('normal')

###

import PyDIP as dip
from scipy import misc

f = misc.face()
a = dip.Image(f)
a.Show()

b = dip.Uniform(a,15)
b.Convert(dip.DT_UINT8)
b.Show()

###

import PyDIP as dip
import numpy as np
from scipy import misc
import matplotlib.pyplot as pp
img = dip.Image(misc.face()[:,:,0])
grad = dip.GradientMagnitude(img,5)
grad.Show()

a = dip.Watershed(grad)
a.Show()

seeds = dip.Image(np.random.random(list(reversed(img.Sizes()))))>0.99
b = dip.SeededWatershed(grad,seeds)
b.Show()

h,b = dip.Histogram(img)
pp.clf()
pp.plot(b[0],h)
pp.show(block=False)

###

import PyDIP as dip
from scipy import misc
img = dip.Image(misc.face())
x = dip.Gradient(dip.Norm(img))
x.Show()

y = x*dip.Transpose(x)
y.Show()

###

import PyDIP as dip
a = dip.Image((20,10),3)
dip.FillXCoordinate(a[0])
dip.FillYCoordinate(a[1])
dip.FillColoredNoise(a[2])
a.Show()

a.SetColorSpace('rgb')
b = dip.ColorSpaceManager.Convert(a,'Lab')
b.Show()

b[1].Show()

b,c = dip.Histogram(a)
len(c)
b.Dimensionality() == len(c)

###

import PyDIP as dip
a = dip.ImageReadICS('../../../test/cermet')
b = dip.Label(a < 120)
dip.MeasurementTool.Features()
m = dip.MeasurementTool.Measure(b,a,['Size','Feret','Convexity','Statistics'])
print(m)
m['Feret'][50][2]

b = dip.EuclideanSkeleton(a > 120)
dip.GetEndPixels(b).Show()
dip.GetLinkPixels(b).Show()
dip.GetBranchPixels(b).Show()
