# rf_header_libs
### Tiny single-header C/++ libs

## rf_darray
### Dependent on the CRT by default (can be changed)
rf_darray provides functionality for managing dynamically-sizing arrays. Arrays allocated by rf_darray resize as they need to and function just like normal arrays (subscript operator still works, random-access in O(1) time, etc.). Accessing the size/capacity of the array can be done in O(1) time as well (it is stored, not calculated).

## rf_dstring
### Dependent on the CRT
rf_dstring provides functionality that makes working with C-strings easier. It provides functionality for appending types to strings, erasing characters, etc. It is specifically built for heap-allocated C-strings. Keep in mind that using the stack for strings is almost always a better option!

## rf_hashtable
### Dependent on the CRT
rf_hashtable provides functionality for managing a hash table. The user directly controls how many spots are available in the hashtable. It works with any combination of types (though the default hashing function is specifically built for strings), and the user can provide their own hashing function too.

## rf_mtr
### Dependent on the CRT and pthread
rf_mtr provides functionality that makes multithreaded resource loading easier. The user provides a number of resources and an array of C-strings containing the relative (to the executable) filenames of the resources. The user can then request resources. When the resources have finished loading, a void * pointing to the data loaded from the file as well as an int64_t that holds how many bytes the void * contains. Interpreting/freeing this data is completely up to the user (unless the data was never grabbed after being loaded).

## rf_utils
### Dependent on the CRT
rf_utils is a file that just contains some macros/typedefs that I find useful when programming in almost every case. There are some nice macros for foreach loops, forrng ("for range") loops, memory allocation, and some general number/math operations. There's also typedefs for fixed-length types, like i8 for int8_t, i16 for int16_t, u32 for uint32_t, r32 for float, etc.
