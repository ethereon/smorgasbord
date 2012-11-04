#!/usr/bin/env python

import perlin
import dye

def gen_texture(s=256, f=1/16.0):
    fBm = perlin.make_fBm_perlin_2d()
    def pixel_color(x, y):
        v = int(127.5*(1+fBm(f*x, f*y)))
        return (v, v, v, 255)        
    bmp = dye.BitmapContext(s, s)
    bmp.set_pixels(pixel_color)
    return bmp

if __name__=='__main__':
    gen_texture().preview()