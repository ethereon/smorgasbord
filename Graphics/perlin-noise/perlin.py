'''
    Perlin noise in pure python
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    The implementations below are based on the following references:
        1. "Texturing and Modeling - A Procedural Approach" by Perlin, Musgrave, et. al.
        2. "Simplex Noise Demystified" by Stefan Gustavson.

    :copyright: Copyright (c) 2012 Saumitro Dasgupta
    :license: MIT License [http://opensource.org/licenses/MIT]
'''

from math import cos, sin, pi, floor
import random

# The original version used 3t^2 - 2t^3 as the interpolating polynomial.
# The improved version described by Perlin in 2002 uses the current polynomial.
# Its first and second derivatives are zero at t=0 and t=1.
blend = lambda t: 6*(t**5) - 15*(t**4) + 10*(t**3)

def make_perlin_2d(period=256):
    # Create the permutation table
    perms = range(period)
    random.shuffle(perms)
    # Repeat it so we can index in the range (0, 2*period] without modulo.
    perms *= 2 
    # Generate the gradient table: distributed around a unit circle
    phi = 2.0*pi/period
    grads = [(cos(phi*x), sin(phi*x)) for x in xrange(period)]
    def v_at(x, y, gX, gY):
        hashIdx = perms[perms[int(gX)%period] + (int(gY)%period)]
        return grads[hashIdx][0]*(x-gX) + grads[hashIdx][1]*(y-gY)
    def noise(x, y):
        # Get the grid cell for (x,y)
        i, j = floor(x), floor(y)
        # Calculate the noise contribution at the four corners.
        # Interpolate and combine.
        fU, fV = blend(x-i), blend(y-j)
        return ((v_at(x, y, i, j  )*(1-fU) + v_at(x, y, i+1, j  )*fU)*(1-fV) +
                (v_at(x, y, i, j+1)*(1-fU) + v_at(x, y, i+1, j+1)*fU)*fV)
    return noise

def make_fBm_2d(noise, octaves, lacunarity=2, H=1):
    def fBm(x, y):
        value = 0.0
        for i in xrange(octaves):
            value += noise(x, y)*lacunarity**(-H*i) 
            x *= lacunarity
            y *= lacunarity
        return value
    return fBm

def make_fBm_perlin_2d(period=256, octaves=4):
    return make_fBm_2d(make_perlin_2d(period=period), octaves=octaves)
