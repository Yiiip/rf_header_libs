/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
          SINGLE-HEADER C/++ HASH TABLE LIBRARY
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    DESCRIPTION
        This library provides functionality that
        allows easy creation/usage of hash tables.
        It works with any type (but you should take
        care to ensure that you keep the types you
        use, or at least the size of the types you
        use, consistent for each hash table).
        
        NOTE: This library is new and incomplete!
              It has not yet gone through rigorous 
              testing so it may have several issues,
              including leaks/crashes/etc. It'll
              become more air-tight overtime (unless
              it's already working perfectly! a man can
              dream)
        
        Its only dependency is the CRT.

    USAGE
        There are a few notable functions for usual
        usage:
        
        * rf_hash_init       initializes an rf_HashTable object.
                             takes the number of array slots
                             desired and a function pointer to
                             a custom hash function. if NULL
                             is passed for the hash function,
                             a default string one will be used.
        
        * rf_hash_add        tries to add an element to the table.
                             takes the hash table, a key, and a
                             pointer to the element to be added.
                             returns 1 on success, 0 on failure.
                         
        * rf_hash_delete     tries to delete an element given
                             some key. returns 1 on success, 0
                             on failure. if successful, it will
                             delete the element that was last
                             added.
        
        * rf_hash_clean_up   cleans up all allocated memory in
                             a hash table. if any cleaning-up
                             needs to be done for any particular
                             element, it is up to you to clean
                             that up first.

    EXAMPLE
        // default hashing function --------v 
        rf_HashTable h = rf_hash_init(1000, 0);
        
        int a = 12345;
        rf_hash_add(h, "This is a key!", &a);
        int b = 67890;
        rf_hash_add(h, "This is another key!", &b);
        
        rf_hash_get(h, "This is a key!", &b);
        rf_hash_get(h, "This is another key!", &a);
        
        printf("a: %i\nb: %i\n", a, b);
        
        // ^ a will be 67890, b will be 12345.
        // note: this is a very expensive way to swap values-
        //       it is just a demonstration ;)
        
        rf_hash_clean_up(h);

    WARNING
        It is best practice to NOT mix and match types
        with a single hash table. Just don't do it!
        The only way it'll remain 100% safe is if all
        of the types you use are the same length in bytes.
        If not, you're hosed!
        
        Here's an example:
        
        int a = 5;
        const char *b = "Hello";
        rf_hash_add(h, "KEY", &a);
        rf_hash_add(h, "KEY", &b);
        
        // this is bad! the program will try to 
        // store b's information inside of a.
        // this might have unintended consequences.
        rf_hash_get(h, "KEY", &a);

    LICENSE INFORMATION IS AT THE END OF THE FILE
*/

#ifndef _RF_HASHTABLE_H
#define _RF_HASHTABLE_H

#include <stdlib.h> // calloc, malloc
#include <string.h> // memcpy
#include <stdint.h> // uint64_t, int8_t, etc.

#define rf_hash_clean_up(h)          (_rf__hash_clean_up(&h))
#define rf_hash_add(h, key, element) (_rf__hash_add(&h, key, element, sizeof(*(element))))
#define rf_hash_delete(h, key)          (_rf__hash_delete(&h, key))
#define rf_hash_get(h, key, element) (_rf__hash_get(&h, key, element, sizeof(*(element))))

typedef uint64_t (* rf_HashFunc)(const void *key);

typedef struct rf_HashNode {
    void *data;
    rf_HashNode *next;
} rf_HashNode;

typedef struct rf_HashTable {
    uint64_t array_size;
    rf_HashNode **array;
    rf_HashFunc hash_function;
} rf_HashTable;

uint64_t _rf__hash_str_default(const void *key) {
    char *str = (char *)key;
    
    uint64_t val = 37;
    while(*str++) {
        val = (val * 54059) ^ (str[0] * 76963);
    }
    return val;
}

inline rf_HashTable rf_hash_init(uint64_t array_size, rf_HashFunc hash_function) {
    rf_HashTable h;
    h.array_size = array_size;
    h.array = (rf_HashNode **)calloc(array_size, sizeof(rf_HashNode *));
    h.hash_function = hash_function ? hash_function : _rf__hash_str_default;
    return h;
}

inline void _rf__hash_clean_up(rf_HashTable *h) {
    for(uint64_t i = 0; i < h->array_size; ++i) {
        for(rf_HashNode *j = h->array[i]; j;) {
            rf_HashNode *next = j->next;
            free(j);
            j = next;
        }
    }
    free(h->array);
    h->array = 0;
    h->array_size = 0;
}

inline int8_t _rf__hash_add(rf_HashTable *h, const void *key, void *element, uint64_t element_size) {
    uint64_t arr_pos = h->hash_function(key) % h->array_size;
    
    rf_HashNode *new_node = (rf_HashNode *)calloc(1, sizeof(rf_HashNode));
    new_node->data = malloc(element_size);
    memcpy(new_node->data, element, element_size);
    new_node->next = 0;
    
    if(h->array[arr_pos]) {
        for(rf_HashNode *i = h->array[arr_pos];; i = i->next) {
            if(!i->next) {
                i->next = new_node;
                return 1;
            }
        }
    }
    else {
        h->array[arr_pos] = new_node;
        return 1;
    }
    
    free(new_node->data);
    free(new_node);
    return 0;
}

inline int8_t _rf__hash_delete(rf_HashTable *h, const void *key) {
    uint64_t arr_pos = h->hash_function(key) % h->array_size;
    
    if(h->array[arr_pos]) {
        rf_HashNode *last = NULL;
        for(rf_HashNode *i = h->array[arr_pos];; i = i->next) {
            if(!i->next) {
                if(last) {
                    last->next = NULL;
                }
                free(i->data);
                free(i);
                return 1;
            }
            else {
                last = i;
            }
        }
    }
    
    return 0;
}

inline int8_t _rf__hash_get(rf_HashTable *h, const void *key, void *element, uint64_t element_size) {
    uint64_t arr_pos = h->hash_function(key) % h->array_size;
    
    if(h->array[arr_pos]) {
        for(rf_HashNode *i = h->array[arr_pos];; i = i->next) {
            if(!i->next) {
                memcpy(element, i->data, element_size);
                return 1;
            }
        }
    }
    
    return 0;
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
