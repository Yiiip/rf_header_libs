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
		
        Its only dependency is the CRT.

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
        
		* rf_da_clear    resets size to 0. does not free any
                         memory.
        
		* rf_da_free     frees all memory associated with
                         an array
        
		To declare an array ready to use with these
        functions, simply create a pointer to a type of
        your choice and point it to NULL. Remember to
        call da_free before the pointer goes out of
        scope if you don't want the memory to be
        allocated for the lifetime of the program.

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

#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <string.h>

#define _RF_DARRAY_START_CAP 32

#define _rf__da_raw(a)                     ((a) ? ((uint32_t *)(a)) - 2 : NULL)
#define rf_da_size(a)                      ((a) ? _rf__da_raw(a)[0] : 0)
#define rf_da_cap(a)                       ((a) ? _rf__da_raw(a)[1] : 0)

#define rf_da_push(a, e)                   _rf__da_insert((void **)&(a), &e, sizeof(e), rf_da_size(a))
#define rf_da_insert(a, e, i)              _rf__da_insert((void **)&(a), &e, sizeof(e), i)
#define rf_da_pop(a)                       if(rf_da_size(a)) { _rf__da_erase((void **)&(a), sizeof((a)[0]), rf_da_size(a) - 1); }
#define rf_da_erase(a, i)                  if(rf_da_size(a)) { _rf__da_erase((void **)&(a), sizeof((a)[0]), i); }

#define rf_da_clear(a)                     { if(rf_da_size(a)) { _rf__da_raw(a)[0] = 0; } }
#define rf_da_free(a)                      { if(a) { free(_rf__da_raw(a)); (a) = NULL; } }

inline void _rf__da_grow(void **array, size_t element_size, uint32_t required_elements) {
    if(rf_da_cap(*array) < required_elements) {
        uint32_t new_cap = rf_da_cap(*array) > _RF_DARRAY_START_CAP ? rf_da_cap(*array) : _RF_DARRAY_START_CAP;
        while(required_elements >= new_cap) {
            new_cap = 3 * (new_cap / 2);
        }
        *array = (void *)((uint32_t *)realloc(_rf__da_raw(*array), new_cap*element_size + 2*sizeof(uint32_t)) + 2);

        *(_rf__da_raw(*array) + 1) = new_cap;
    }
}

inline void _rf__da_insert(void **array, void *element, size_t element_size, uint32_t pos) {
    int8_t array_null = !*array;

    _rf__da_grow(array, element_size, rf_da_size(*array) + 1);

    if(array_null) {
        _rf__da_raw(*array)[0] = 0;
    }

    memmove(((uint8_t *)(*array)) + (element_size * (pos + 1)),
            ((uint8_t *)(*array)) + (element_size * pos),
            element_size * (rf_da_size(*array) - pos));

    memcpy(((uint8_t *)(*array)) + (element_size * pos),
           element, element_size);

    _rf__da_raw(*array)[0]++;
}

inline void _rf__da_erase(void **array, size_t element_size, uint32_t pos) {
    if(*array) {
        memmove(((uint8_t *)(*array)) + (element_size * pos),
                ((uint8_t *)(*array)) + (element_size * (pos + 1)),
                element_size * (rf_da_size(*array) - pos - 1));

        _rf__da_raw(*array)[0]--;

        if(rf_da_size(*array) < 1) {
            rf_da_free(*array);
            *array = NULL;
        }
    }
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
