/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
       SINGLE-HEADER C/++ C-STRING HELPER LIBRARY
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    DESCRIPTION

        This library provides some nice functions
        that make C-strings much easier to work
        with. It has functionality for appending
        multiple types (other strings, chars,
        ints, and floating point types), inserting
        said types, and erasing. It stretches and
        reallocates memory as necessary. It is
        implemented very much in the same way as
        the darray lib (and by extension the
        stretchy buffer library by Sean Barrett),
        and is effectively a special case of that.

        Its only dependency is the CRT. Strings
        produced by the library's functionality
        can be used as C-strings as they are
        null terminated.

    USAGE

        There are a few notable functions for usual
        usage:

        * ds_size     returns number of chars including
                      null terminator

        * ds_cap      returns number of chars allowed
                      with current allocation. if this
                      is exceeded, reallocation will
                      be necessary.

        * ds_length   returns number of chars without
                      null terminator

        * ds_new      creates a new string- flags can be
                      passed and will be inserted into
                      the string. this uses sprintf so
                      flag functionality will match that.

        * ds_add_s    appends a string to a given string
         (aka ds_add_string)

        * ds_add_c    appends a char to a given string
         (aka ds_add_char)

        * ds_add_i    appends an int to a given string
         (aka ds_add_int)

        * ds_add_f    appends a floating point value to
                      a given string
         (aka ds_add_float)

        * ds_insert_s inserts a string to a given string
                      at a given position
         (aka ds_insert_string)

        * ds_insert_c inserts a char to a given string
                      at a given position
         (aka ds_insert_char)

        * ds_insert_i inserts an int to a given string
                      at a given position
         (aka ds_insert_int)

        * ds_insert_f inserts a floating point value to
                      a given string at a given position
         (aka ds_insert_float)

        * ds_free     frees all memory associated with
                      a string.

        To declare a string ready to use with these
        functions, simply create a char * and point it
        to NULL (You can also use the dstring typedef
        if you please). Remember to call ds_free before
        the pointer goes out of scope if you don't want
        the memory to be allocated for the lifetime of
        the program.

    EXAMPLE

        dstring str = NULL;
        str = ds_new("This is a number: %i", 123);
        ds_add_string(str, "\nHere's another number: ");
        ds_add_int(str, 321);
        ds_insert_float(str, 123.45, 12);
        ds_free(str);

    WARNING

        Memory might be reallocated. Therefore, don't
        expect a pointer to a character to remain
        constant.

        Also, do not try to insert something at a
        position greater than the length of the
        string. Consider:

        dstring test = ds_new("Hello");
        ds_insert_s(test, "This will crash maybe", 1000);
        //the string we're trying to append states
        //information that is probably correct

        dstring test = ds_new("Hello");
        ds_insert_s(test, "This won't crash I bet", 5);
        //this is safe

    LICENSE INFORMATION IS AT THE END OF THE FILE
*/

#ifndef _DSTRING_H
#define _DSTRING_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#define _DSTRING_START_CAP 32

typedef char *dstring;

#define ds_raw(a)                       (a ? ((uint32_t *)a) - 2 : NULL)
#define ds_size(a)                      (a ? ds_raw(a)[0] : 0)
#define ds_cap(a)                       (a ? ds_raw(a)[1] : 0)

#define ds_length(a)                    (ds_size(a) - 1)
#define ds_len(a)                       ds_length(a)

#define ds_add_s(a, s)                  { a = _ds_insert_s(a, s, ds_length(a)); }
#define ds_add_c(a, c)                  { char _ds_c_str[2] = { c, 0 }; ds_add_s(a, _ds_c_str); }
#define ds_add_i(a, i)                  { char _ds_i_str[16] = { 0 }; sprintf(_ds_i_str, "%i", i); ds_add_s(a, _ds_i_str); }
#define ds_add_f(a, f)                  { char _ds_f_str[24] = { 0 }; sprintf(_ds_f_str, "%f", f); ds_add_s(a, _ds_f_str); }

#define ds_insert_s(a, s, pos)          { a = _ds_insert_s(a, s, pos); }
#define ds_insert_c(a, c, pos)          { char _ds_c_str[2] = { c, 0 }; ds_insert_s(a, _ds_c_str, pos); }
#define ds_insert_i(a, i, pos)          { char _ds_i_str[16] = { 0 }; sprintf(_ds_i_str, "%i", i); ds_insert_s(a, _ds_i_str, pos); }
#define ds_insert_f(a, f, pos)          { char _ds_f_str[24] = { 0 }; sprintf(_ds_f_str, "%f", f); ds_insert_s(a, _ds_f_str, pos); }

#define ds_add_string(a, s)             ds_add_s(a, s)
#define ds_add_char(a, c)               ds_add_c(a, c)
#define ds_add_int(a, i)                ds_add_i(a, i)
#define ds_add_float(a, f)              ds_add_f(a, f)

#define ds_insert_string(a, s, pos)     ds_insert_s(a, s, pos)
#define ds_insert_char(a, c, pos)       ds_insert_c(a, c, pos)
#define ds_insert_int(a, i, pos)        ds_insert_i(a, i, pos)
#define ds_insert_float(a, f, pos)      ds_insert_f(a, f, pos)

#define ds_erase(a, pos)                { a = _ds_erase(a, pos); }

#define ds_free(a)                      { if(a) { free(ds_raw(a)); a = NULL; } }

inline char *_ds_grow(char *dstr, uint32_t required_chars) {
    if(ds_cap(dstr) < required_chars) {
        uint32_t new_cap = ds_cap(dstr) > _DSTRING_START_CAP ? ds_cap(dstr) : _DSTRING_START_CAP;
        while(required_chars >= new_cap) {
            new_cap = 3 * (new_cap / 2);
        }
        dstr = (char *)((uint32_t *)realloc(ds_raw(dstr), new_cap * sizeof(char) + 2 * sizeof(uint32_t)) + 2);
        *(ds_raw(dstr) + 1) = new_cap;
    }
    return dstr;
}

inline char *ds_new(const char *str, ...) {
    char *dstr = NULL;
    va_list args;
    va_start(args, str);
    size_t required_len = vsnprintf(NULL, 0, str, args) + 1;
    va_end(args);

    dstr = _ds_grow(dstr, required_len);

    va_start(args, str);
    vsprintf(dstr, str, args);
    va_end(args);

    *(ds_raw(dstr)) = required_len;
    return dstr;
}

inline char *_ds_insert_s(char *str, const char *add, uint32_t pos) {
    if(str) {
        size_t add_str_len = strlen(add);

        uint32_t new_len = ds_size(str) + add_str_len;
        if(ds_cap(str) < new_len) {
            str = _ds_grow(str, new_len);
        }
        memmove(str + pos + add_str_len, str + pos, ds_size(str) - pos);
        strncpy(str + pos, add, add_str_len);
        *(ds_raw(str)) = new_len;
    }
    else {
        str = ds_new("%s", add);
    }

    return str;
}

inline char *_ds_erase(char *str, uint32_t i) {
    memmove(str + i, str + i + 1, ds_size(str) - i - 1);
    (ds_raw(str))[0]--;

    if((ds_raw(str))[0] < 1) {
        ds_free(str);
    }

    return str;
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
