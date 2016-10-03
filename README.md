
# NAME

imgtxtenh - Tool for enhancing noisy scanned text images


# DESCRIPTION

The function of 'imgtxtenh' is to clean/enhance noisy scanned text images,
which could be printed or handwritten text. It takes as input an image and
generates as output another image of the same size. It supports all the
input/output image formats that the linked ImageMagick library supports.

Currently only two techniques are implemented, which are the method of Sauvola
[1], and a simple modification [2] of the method of Sauvola that generates a
grayscale image instead of binary foreground-background decisions. For
explanation of the usage, execute:

    ./imgtxtenh -h


# USAGE REQUIREMENT

If this software is used in research, any publication resulting from it is
required to cite the reference [2] below.


# COMPILATION

The procedure for compiling is:

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make


# REFERENCES

[1] Sauvola, J. and Pietikainen, M. Adaptive document image
    binarization. Pattern Recognition, 33(2):225–236, 2000.

[2] M. Villegas, V. Romero, and J. A. Sánchez. On the Modification of
    Binarization Algorithms to Retain Grayscale Information for
    Handwritten Text Recognition. In IbPria 2015.


# COPYRIGHT

The MIT License (MIT)

Copyright (c) 2012-present, Mauricio Villegas <mauvilsa@upv.es>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
