/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
              SINGLE-HEADER C/++ UTILITY DEFINITIONS
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    DESCRIPTION
    
        This is a single-header library that defines a set of
        useful typedefs/macros for the programmer. This is
        effectively just stuff that I have used to make
        programming easier/more fun, but it can be quite
        convenient so I decided to package it together for
        others. It isn't necessarily cumbersome for one to do
        themselves, but it's nice to have it in a single file
        just to throw into a project.
        
        It is dependent on the CRT.
        
    UTILITIES
    
      * fixed-size type typedefs
            Some aliases are provided for fixed-length integer
            and floating-point types that are shorter and
            easier to type. Here's a list of them:
            
            i8  for int8_t
            i16 for int16_t
            i32 for int32_t
            i64 for int64_t
            u8  for uint8_t
            u16 for uint16_t
            u32 for uint32_t
            u64 for uint64_t
            r32 for float
            r64 for double
            bl  for int8_t (used for booleans)
      
      * foreach loops
            foreach loops are a quicker method of looping from 
            0 to some upper-bound. They're usually useful for
            looping through the indices of an array. The
            foreach macros take two arguments: the identifier
            to be used to define the iterator, and the upper-limit
            of the loop (during the last iteration of the loop,
            the iterator will be the upper-limit minus 1).
            
            Here's an example:
            
                int arr[1000];
                foreach(i, 1000) {
                    arr[i] = i;
                }
                
                // is equivalent to:
                
                int arr[1000];
                for(unsigned int i = 0; i < 1000; ++i) {
                    arr[i] = i;
                }
                
            foreach uses a 64-bit unsigned integer by default.
            There are other foreach macros as well, however, 
            that can allow looping with iterators of different
            lengths. For example, foreach8 loops with an
            unsigned byte, foreach16 loops with a 16-bit
            unsigned integer, etc.
            
      * forrng loops
            forrng ("for range") loops are similar to foreach 
            loops, except they are iterated with a signed integer 
            and allow iterating through negative values. They
            take 3 parameters instead of 2: the identifier to
            be used to define the loop's iterator, the starting
            value, and the upper limit of looping.
            
            Here's an example:
                
                printf("I'll print out every number between ");
                printf("-500 and 500 (inclusive)!");
                
                forrng(i, -500, 501) {
                    printf("%i\n", i);
                }
                
            forrng uses a 64-bit signed integer by default.
            There are other forrng macros as well, however,
            similar to foreach, that allow iterators with
            different lengths. For example, forrng8 loops with
            a signed byte, forrng16 loops with a signed 16-bit
            integer, etc.

    LICENSE INFORMATION IS AT THE END OF THE FILE
*/

#ifndef _RF_UTILS_INCLUDED
#define _RF_UTILS_INCLUDED

#include <stdint.h>

#define foreach8(i, lim)    for(u8 i = 0; i < lim; ++i)
#define foreach16(i, lim)   for(u16 i = 0; i < lim; ++i)
#define foreach32(i, lim)   for(u32 i = 0; i < lim; ++i)
#define foreach64(i, lim)   for(u64 i = 0; i < lim; ++i)
#define foreach(i, lim)     for(u64 i = 0; i < lim; ++i)

#define forrng8(i, l, h)    for(i8 i = l; i < h; ++i)
#define forrng16(i, l, h)   for(i16 i = l; i < h; ++i)
#define forrng32(i, l, h)   for(i32 i = l; i < h; ++i)
#define forrng64(i, l, h)   for(i64 i = l; i < h; ++i)
#define forrng(i, l, h)     for(i64 i = l; i < h; ++i)

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    r32;
typedef double   r64;
typedef int8_t   bl;

#endif

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

MIT License

Copyright (c) 2017 Ryan Fleury

Permission is hereby granted, free of charge, to any
person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the
Software without restriction, including without
limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following
conditions: The above copyright notice and this permission
notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
