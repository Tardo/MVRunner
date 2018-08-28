#!/usr/bin/python
# Alexandre Díaz - 2018 - GPL
# RGBA -> INT
#
# Working Directory: Any
# Usage:
#   calc_color_int.py <R:G:B:A>
#   calc_color_int.py <Color Int>
#
import sys

def calc_color_int(r, g, b, a):
    return ((int(r)&255)<<24) + ((int(g)&255)<<16) + ((int(b)&255)<<8) + (int(a)&255)

def calc_color_rgba(color):
    return ((color>>24) & 255, (color>>16) & 255, (color>>8) & 255, color & 255);


if __name__ == '__main__':
    colors = sys.argv[1].split(':')
    if len(colors) > 1:
        print(calc_color_int(*colors))
    else:
        print(calc_color_rgba(int(colors[0])))
