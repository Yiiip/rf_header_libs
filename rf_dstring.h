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

        * rf_ds_size     returns number of chars including
                         null terminator

        * rf_ds_cap      returns number of chars allowed
                         with current allocation. if this
                         is exceeded, reallocation will
                         be necessary.

        * rf_ds_length   returns number of chars without
                         null terminator

        * rf_ds_new      creates a new string- flags can be
                         passed and will be inserted into
                         the string. this uses sprintf so
                         flag functionality will match that.

        * rf_ds_add_s    appends a string to a given string
         (aka ds_add_string)

        * rf_ds_add_c    appends a char to a given string
         (aka ds_add_char)

        * rf_ds_add_i    appends an int to a given string
         (aka ds_add_int)

        * rf_ds_add_f    appends a floating point value to
                         a given string
         (aka ds_add_float)

        * drf_s_insert_s inserts a string to a given string
                         at a given position
         (aka ds_insert_string)

        * rf_ds_insert_c inserts a char to a given string
                         at a given position
         (aka ds_insert_char)

        * rf_ds_insert_i inserts an int to a given string
                         at a given position
         (aka ds_insert_int)

        * rf_ds_insert_f inserts a floating point value to
                         a given string at a given position
         (aka ds_insert_float)

        * rf_ds_free     frees all memory associated with
                         a string.

        To declare a string ready to use with these
        functions, simply create a char * and point it
        to NULL (You can also use the dstring typedef
        if you please). Remember to call ds_free before
        the pointer goes out of scope if you don't want
        the memory to be allocated for the lifetime of
        the program.

    EXAMPLE

        rf_dstring str = NULL;
        str = rf_ds_new("This is a number: %i", 123);
        rf_ds_add_string(str, "\nHere's another number: ");
        rf_ds_add_int(str, 321);
        rf_ds_insert_float(str, 123.45, 12);
        rf_ds_free(str);

    WARNING

        Memory might be reallocated. Therefore, don't
        expect a pointer to a character to remain
        constant.

        Also, do not try to insert something at a
        position greater than the length of the
        string. Consider:

        rf_dstring test = rf_ds_new("Hello");
        rf_ds_insert_s(test, "This will crash maybe", 1000);
        
        // the string we're trying to append states
        // information that is probably correct

        
        rf_dstring test = ds_new("Hello");
        rf_ds_insert_s(test, "This won't crash I bet", 5);
        
        // this is safe

    LICENSE INFORMATION IS AT THE END OF THE FILE
*/

#ifndef _RF_DSTRING_H
#define _RF_DSTRING_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#define _RF_DSTRING_START_CAP 32

typedef char *rf_dstring;

#define _rf__ds_raw(a)                     (a ? ((uint32_t *)a) - 2 : NULL)
#define rf_ds_size(a)                      (a ? _rf__ds_raw(a)[0] : 0)
#define rf_ds_cap(a)                       (a ? _rf__ds_raw(a)[1] : 0)

#define rf_ds_length(a)                    (rf_ds_size(a) > 0 ? rf_ds_size(a) - 1 : 0)
#define rf_ds_len(a)                       rf_ds_length(a)

#define rf_ds_add_s(a, s)                  { a = _rf__ds_insert_s(a, s, rf_ds_length(a)); }
#define rf_ds_add_c(a, c)                  { char _rf__ds_c_str[2] = { c, 0 }; rf_ds_add_s(a, _rf__ds_c_str); }
#define rf_ds_add_i(a, i)                  { char _rf__ds_i_str[16] = { 0 }; sprintf(_rf__ds_i_str, "%i", i); rf_ds_add_s(a, _rf__ds_i_str); }
#define rf_ds_add_f(a, f)                  { char _rf__ds_f_str[24] = { 0 }; sprintf(_rf__ds_f_str, "%f", f); rf_ds_add_s(a, _rf__ds_f_str); }

#define rf_ds_insert_s(a, s, pos)          { a = _rf__ds_insert_s(a, s, pos); }
#define rf_ds_insert_c(a, c, pos)          { char _rf__ds_c_str[2] = { c, 0 }; rf_ds_insert_s(a, _rf__ds_c_str, pos); }
#define rf_ds_insert_i(a, i, pos)          { char _rf__ds_i_str[16] = { 0 }; sprintf(_rf__ds_i_str, "%i", i); rf_ds_insert_s(a, _rf__ds_i_str, pos); }
#define rf_ds_insert_f(a, f, pos)          { char _rf__ds_f_str[24] = { 0 }; sprintf(_rf__ds_f_str, "%f", f); rf_ds_insert_s(a, _rf__ds_f_str, pos); }

#define rf_ds_add_string(a, s)             rf_ds_add_s(a, s)
#define rf_ds_add_char(a, c)               rf_ds_add_c(a, c)
#define rf_ds_add_int(a, i)                rf_ds_add_i(a, i)
#define rf_ds_add_float(a, f)              rf_ds_add_f(a, f)

#define rf_ds_insert_string(a, s, pos)     rf_ds_insert_s(a, s, pos)
#define rf_ds_insert_char(a, c, pos)       rf_ds_insert_c(a, c, pos)
#define rf_ds_insert_int(a, i, pos)        rf_ds_insert_i(a, i, pos)
#define rf_ds_insert_float(a, f, pos)      rf_ds_insert_f(a, f, pos)

#define rf_ds_erase(a, pos)                { a = _rf__ds_erase(a, pos); }

#define rf_ds_free(a)                      { if(a) { free(_rf__ds_raw(a)); a = NULL; } }

inline char *_rf__ds_grow(char *dstr, uint32_t required_chars) {
    if(rf_ds_cap(dstr) < required_chars) {
        uint32_t new_cap = rf_ds_cap(dstr) > _RF_DSTRING_START_CAP ? rf_ds_cap(dstr) : _RF_DSTRING_START_CAP;
        while(required_chars >= new_cap) {
            new_cap = 3 * (new_cap / 2);
        }
        dstr = (char *)((uint32_t *)realloc(_rf__ds_raw(dstr), new_cap * sizeof(char) + 2 * sizeof(uint32_t)) + 2);
        *(_rf__ds_raw(dstr) + 1) = new_cap;
    }
    return dstr;
}

inline char *rf_ds_new(const char *str, ...) {
    char *dstr = NULL;
    va_list args;
    va_start(args, str);
    size_t required_len = vsnprintf(NULL, 0, str, args) + 1;
    va_end(args);

    dstr = _rf__ds_grow(dstr, required_len);

    va_start(args, str);
    vsprintf(dstr, str, args);
    va_end(args);

    *(_rf__ds_raw(dstr)) = required_len;
    return dstr;
}

inline char *_rf__ds_insert_s(char *str, const char *add, uint32_t pos) {
    if(str) {
        size_t add_str_len = strlen(add);

        uint32_t new_len = rf_ds_size(str) + add_str_len;
        if(rf_ds_cap(str) < new_len) {
            str = _rf__ds_grow(str, new_len);
        }
        memmove(str + pos + add_str_len, str + pos, rf_ds_size(str) - pos);
        strncpy(str + pos, add, add_str_len);
        *(_rf__ds_raw(str)) = new_len;
    }
    else {
        str = rf_ds_new("%s", add);
    }

    return str;
}

inline char *_rf__ds_erase(char *str, uint32_t i) {
    memmove(str + i, str + i + 1, rf_ds_size(str) - i - 1);
    (_rf__ds_raw(str))[0]--;

    if((_rf__ds_raw(str))[0] < 2) {
        rf_ds_free(str);
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
