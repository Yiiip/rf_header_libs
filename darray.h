/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SINGLE-HEADER C/++ DYNAMIC ARRAY LIBRARY
	
	---DESCRIPTION---
	
	This library is effectively a replacement for the C++ 
	std::vector. It allows a dynamic array capable of "stretching";
	it allocates memory as it needs to. This was originally a
	learning exercise inspired by Sean Barrett's stretchy buffer 
	library which is effectively the same thing.

	---USAGE---
	
	There are 8 notable functions/macros for usual usage: da_size, 
	da_cap, da_shrink, da_push, da_insert, da_pop, da_erase, and
	da_free. 
	
	*** da_size returns the number of elements currently in the 
		dynamic array as a uint32_t
	
	*** da_cap returns the number of elements the memory can store
		without reallocating as a uint32_t
		
	*** da_shrink shrinks the capacity if possible.
	
	*** da_push pushes an element to the back of the current
		elements
	
	*** da_insert inserts an element at a position
	
	*** da_pop removes an element from the back of the array
	
	*** da_erase removes an element from a particular position in
		the array
	
	*** da_free frees all array memory. Should usually be called
		when done with a dynamic array.
		
	To declare an array suitable for usage with these functions,
	simply create a pointer to the type you want to store,
	initialize it to NULL, and push/pop/insert/erase as you
	please. Remember to call da_free before the pointer goes
	out of scope if you don't want the memory to be allocated
	for the lifetime of the program.
	
	---EXAMPLE---
	
		int *int_array = NULL;
		for(int i = 0; i < 1000; i++) {
			da_push(int_array, i);
		}
		da_free(int_array);
		
	---WARNING---
	
	Memory will be reallocated; therefore, a pointer to an object
	inside of the array will not /necessarily/ stay the same. If
	you need a reference to an object to remain constant, allocate
	an array of pointers. Consider:
		
		````````````````````````````````````````````````````````````````````
		
		int *int_array = NULL;
		int *first_element = NULL;
		
		for(int i = 0; i < 1000; i++) {
			int a = i;
			da_push(int_array, a);
			if(!i) { first_element = &int_array[da_size(int_array) - 1]; }
		}
		
		//use of first_element is probably invalid
		
		````````````````````````````````````````````````````````````````````
		
		int **int_ptr_array = NULL;
		int *first_element = NULL;
		
		for(int i = 0; i < 1000; i++) {
			int *a = (int *)malloc(sizeof(int));
			*a = i;
			if(!i) { first_element = a; }
			da_push(int_ptr_array, a);
		}
		
		//using first_element is valid
		
		````````````````````````````````````````````````````````````````````
	
	LICENSE INFORMATION IS AT THE END OF THE FILE
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

#ifndef _DARRAY_H
#define _DARRAY_H

#include <stdlib.h>
#include <stdint.h>
#include <memory.h>

#define _DARRAY_START_CAP 32

#define da_shrink(a) if(da_size(a) > 0) { _da_shrink((void **)&a, sizeof(a[0])); }
#define da_push(a, e) _da_push((void **)&a, &e, sizeof(e))
#define da_insert(a, e, i) _da_insert((void **)&a, &e, sizeof(e), i)
#define da_pop(a) if(da_size(a)) { _da_pop((void **)&a, sizeof(a[0])); }
#define da_erase(a, i) if(da_size(a)) { _da_erase((void **)&a, sizeof(a[0]), i); }
#define da_free(a) _da_free((void **)&a)

inline uint32_t *_da_raw(void *array) {
    return (uint32_t *)array - 2;
}

inline uint32_t da_size(void *array) {
    if(array) {
        return _da_raw(array)[0];
    }
    return 0;
}

inline uint32_t da_cap(void *array) {
    if(array) {
        return _da_raw(array)[1];
    }
    return 0;
}

inline void _da_free(void **array) {
    if(*array) {
        *array = (void *)(_da_raw(*array));
    }
    free(*array);
    *array = NULL;
}

inline void _da_grow(void **array, size_t element_size) {
    uint32_t new_cap = _DARRAY_START_CAP, size = 0;
    bool need_realloc = true;
    if(*array) {
        size = da_size(*array);
        if(size >= da_cap(*array)) {
            new_cap = da_cap(*array) + (da_cap(*array) / 2);
        }
        else {
            new_cap = da_cap(*array);
            need_realloc = false;
        }
    }

    if(need_realloc) {
        if(*array) {
            *array = (void *)_da_raw(*array);
        }
        *array = realloc(*array, (new_cap * element_size) + (2 * sizeof(uint32_t)));
        *array = (void *)((uint32_t *)(*array) + 2);
        _da_raw(*array)[0] = size;
        _da_raw(*array)[1] = new_cap;
    }
}

inline void _da_shrink(void **array, size_t element_size) {
    if(*array) {
        if(da_size(*array) > 0) {
            if(da_size(*array) <= da_cap(*array) - (da_cap(*array) / 3)) {
                _da_raw(*array)[1] = da_cap(*array) - (da_cap(*array) / 3);

                *array = realloc(_da_raw(*array), (da_cap(*array) * element_size) + (2 * sizeof(uint32_t)));
                *array = (uint32_t *)(*array) + 2;
            }
        }
        else {
            _da_free(array);
        }
    }
}

inline void _da_push(void **array, void *element, size_t element_size) {
    _da_grow(array, element_size);

    memcpy(((uint8_t *)(*array)) + (element_size * da_size(*array)),
           element, element_size);
    _da_raw(*array)[0]++;
}

inline void _da_insert(void **array, void *element, size_t element_size, uint32_t pos) {
    _da_grow(array, element_size);

    memmove(((uint8_t *)(*array)) + (element_size * (pos + 1)),
            ((uint8_t *)(*array)) + (element_size * pos),
            element_size * (da_size(*array) - pos));

    memcpy(((uint8_t *)(*array)) + (element_size * pos),
           element, element_size);
    _da_raw(*array)[0]++;
}

inline void _da_pop(void **array, size_t element_size) {
    if(*array) {
        _da_raw(*array)[0]--;

        if(da_size(*array) < 1) {
            da_free(*array);
            *array = NULL;
        }
    }
}

inline void _da_erase(void **array, size_t element_size, uint32_t pos) {
    if(*array) {
        memmove(((uint8_t *)(*array)) + (element_size * pos),
                ((uint8_t *)(*array)) + (element_size * (pos + 1)),
                element_size * (da_size(*array) - pos - 1));

        _da_raw(*array)[0]--;

        if(da_size(*array) < 1) {
            da_free(*array);
            *array = NULL;
        }
    }
}

#endif

/* 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Copyright (c) 2017 Ryan Fleury

Permission is hereby granted, free of charge, to any person 
obtaining a copy of this software and associated documentation 
files (the "Software"), to deal in the Software without 
restriction, including without limitation the rights to use, copy, 
modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is 
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be 
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS 
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN 
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
THE SOFTWARE.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/