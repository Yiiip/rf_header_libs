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

#define ds_free(a)                      { if(a) free(ds_raw(a)); }

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

inline char *ds_erase(char *str, uint32_t i) {
    memmove(str + i, str + i + 1, ds_size(str) - i - 1);
    (ds_raw(str))[0]--;

    if((ds_raw(str))[0] < 1) {
        ds_free(str);
    }

    return str;
}

#endif

