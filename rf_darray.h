/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
       SINGLE-HEADER C/++ DYNAMIC ARRAY LIBRARY
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    DESCRIPTION
    
        This library provides functionality that
        allows easy creation/usage of dynamically
        sizing arrays. It follows a similar pattern
        to Sean Barrett's stretchy buffer library.
        It stretches and reallocates memory as needed
        depending on how many objects it needs to
        store.
        
        It uses the CRT by default, but you can change
        this (see below).

    USAGE
    
        There are a few notable functions for usual
        usage:
        
        * rf_da_size     returns number of elements
        
        * rf_da_cap      returns number of elements
                         allowed without reallocation.
                         if this is surpassed, the array
                         will require reallocation of
                         memory.
        
        * rf_da_push     pushes an element to the back of
                         the array
        
        * rf_da_insert   inserts an element to a position
                         in the array
        
        * rf_da_pop      removes the last element of the
                         array
        
        * rf_da_erase    removes an element at a specified
                         position in the array
                         
        * rf_da_concat   concatenates two darrays
        
        * rf_da_clear    resets size to 0. does not free any
                         memory.
        
        * rf_da_free     frees all memory associated with
                         an array
        
        To declare an array ready to use with these
        functions, simply create a pointer to a type of
        your choice and point it to NULL. Remember to
        call rf_da_free before the pointer goes out of
        scope if you don't want the memory to be
        allocated for the lifetime of the program.
        
    CUSTOMIZATION
    
        There are a few customizations you're able to do with
        the preprocessor:
        
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        
        1. Overriding CRT realloc usage
        
           #define RF_DARRAY_REALLOC to be the identifier of a
           function of your choosing. For safe usage, your
           function must have the form:
           
           void *realloc_func(void *data, size_t n)
           
           Similar to the CRT, your realloc function should
           support NULL being passed as the existing block
           of memory, and all memory that existed in the
           previously allocated block should be copied. Your
           function should free the previously used memory and
           return a newly allocated block of n bytes.
           
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        
        2. Overriding CRT memmove usage
        
           #define RF_DARRAY_MEMMOVE to be the identifier of a function
           of your choosing. For safe usage, your function must
           have the form:
           
           void *memmove_func(void *dest, const void *src, size_t n)
           
           Similar to the CRT, your memmove function should support
           overlapping regions of memory and always perform a move
           of n bytes.
        
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        
        3. Overriding CRT memcpy usage
        
           #define RF_DARRAY_MEMCPY to be the indentifier of a function
           of your choosing. For safe usage, your function must have
           the form:
           
           void *memcpy_func(void *dest, const void *src, size_t n)
           
           Similar to the CRT, you can assume that dest/src are not
           overlapping, you should not check for any null-termination,
           and you should always perform a direct binary copy of
           n bytes.
        
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        
        4. Using something other than uint32_t for size/cap
           storage
        
           #define RF_DARRAY_SIZE_T to be a size type of your choice;
           this will be used as the type to store the 
           size/capacity of arrays.
        
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    EXAMPLE
    
        int *int_array = NULL;
        for(int i = 0; i < 100; i++) {
            rf_da_push(int_array, i);
            printf("%i\n", int_array[i]);
        }
        rf_da_free(int_array);

    WARNING
    
        Memory might be reallocated. Therefore, don't
        expect a pointer to an element to remain
        constant. If you need pointers to elements
        to remain consistent, consider creating an
        array of pointers instead. Consider:
        
            int *int_array = NULL,
                *first_element = NULL;
            for(int i = 0; i < 100; i++) {
                rf_da_push(int_array, i);
                if(!i) first_element = int_array;
            }
            //first_element probably dangling
            
        vs.
        
            int **int_array = NULL,
                *first_element = NULL;
            for(int i = 0; i < 100; i++) {
                int *int_ptr = (int *)malloc(sizeof(int));
                *int_ptr = i;
                rf_da_push(int_array, int_ptr);
                if(!i) first_element = int_ptr;
            }
            //first_element is not dangling
            
        Also, do not try to insert something at a
        position greater than the length of the
        array.

    LICENSE INFORMATION IS AT THE END OF THE FILE
*/

#ifndef _RF_DARRAY_H
#define _RF_DARRAY_H

#ifndef RF_DARRAY_REALLOC
#include <stdlib.h>
#define RF_DARRAY_REALLOC realloc
#endif

#ifndef RF_DARRAY_SIZE_T
#include <stdint.h>
#define RF_DARRAY_SIZE_T uint32_t
#endif

#ifndef RF_DARRAY_MEMMOVE
#include <string.h>
#define RF_DARRAY_MEMMOVE memmove
#endif

#ifndef RF_DARRAY_MEMCPY
#define RF_DARRAY_MEMCPY memcpy
#endif

#define _RF_DARRAY_START_CAP 32

#define _rf__da_raw(a)                     ((a) ? ((uint32_t *)(a)) - 2 : NULL)
#define rf_da_size(a)                      ((a) ? _rf__da_raw(a)[0] : 0)
#define rf_da_cap(a)                       ((a) ? _rf__da_raw(a)[1] : 0)

#define rf_da_push(a, e)                   _rf__da_insert((void **)&(a), &e, sizeof(e), rf_da_size(a))
#define rf_da_insert(a, e, i)              _rf__da_insert((void **)&(a), &e, sizeof(e), i)
#define rf_da_pop(a)                       if(rf_da_size(a)) { _rf__da_erase((void **)&(a), sizeof((a)[0]), rf_da_size(a) - 1); }
#define rf_da_erase(a, i)                  if(rf_da_size(a)) { _rf__da_erase((void **)&(a), sizeof((a)[0]), i); }
#define rf_da_concat(a, b)                   if(rf_da_size(b)) { _rf__da_concat((void **)&(a), (void **)&(b), sizeof(b[0])); }

#define rf_da_clear(a)                     { if(rf_da_size(a)) { _rf__da_raw(a)[0] = 0; } }
#define rf_da_free(a)                      { if(a) { free(_rf__da_raw(a)); (a) = NULL; } }

inline void _rf__da_grow(void **array, size_t element_size, uint32_t required_elements) {
    uint32_t new_cap = (required_elements > _RF_DARRAY_START_CAP ? required_elements : _RF_DARRAY_START_CAP)-1;
    new_cap |= new_cap >> 1;
    new_cap |= new_cap >> 2;
    new_cap |= new_cap >> 4;
    new_cap |= new_cap >> 8;
    new_cap |= new_cap >> 16;
    ++new_cap;
    
    *array = (void *)((uint32_t *)RF_DARRAY_REALLOC(_rf__da_raw(*array), new_cap*element_size + 2*sizeof(uint32_t)) + 2);
    *(_rf__da_raw(*array) + 1) = new_cap;
}

inline void _rf__da_insert(void **array, void *element, size_t element_size, uint32_t pos) {
    int8_t array_null = !*array;
    
    if(rf_da_cap(*array) < rf_da_size(*array) + 1) {
        _rf__da_grow(array, element_size, rf_da_size(*array) + 1);
    }
    
    if(array_null) {
        _rf__da_raw(*array)[0] = 0;
    }

    if(rf_da_size(*array) - pos > 0) {
        RF_DARRAY_MEMMOVE(((uint8_t *)(*array)) + (element_size * (pos + 1)),
                          ((uint8_t *)(*array)) + (element_size * pos),
                          element_size * (rf_da_size(*array) - pos));
    }

    RF_DARRAY_MEMCPY(((uint8_t *)(*array)) + (element_size * pos),
                     element, element_size);

    ++_rf__da_raw(*array)[0];
}

inline void _rf__da_concat(void **dest, void **src, size_t element_size) {
    _rf__da_grow(dest, element_size, rf_da_size(*dest) + rf_da_size(*src));
    RF_DARRAY_MEMCPY((uint8_t *)(*dest) + element_size*rf_da_size(*dest), *src, element_size*rf_da_size(*src));
    _rf__da_raw(*dest)[0] += rf_da_size(*src);
}

inline void _rf__da_erase(void **array, size_t element_size, uint32_t pos) {
    RF_DARRAY_MEMMOVE(((uint8_t *)(*array)) + (element_size * pos),
                      ((uint8_t *)(*array)) + (element_size * (pos + 1)),
                      element_size * (rf_da_size(*array) - pos - 1));

    --_rf__da_raw(*array)[0];
}

#endif

/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
