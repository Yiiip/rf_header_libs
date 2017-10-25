/*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   SINGLE-HEADER C/++ MULTI-THREADED RESOURCE LIBRARY
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    DESCRIPTION
        This library provides the bare-bones basics
        for multi-threaded resource management. Using
        indices and filenames corresponding with those
        indices, the functions in this file will
        launch threads as necessary in order to load
        raw data, if found, at said filenames.
        This raw data can then be retrieved and used
        however you want.
        Its only dependencies are the CRT/pthread.

    USAGE
        There are a few notable functions for usual
        usage:

        --------------------------------------------

        * mtr_new
              returns a newly allocated 'Resource
              Master'. Takes the number of resources
              you want and an array of C-strings
              (managed by you) that hold the
              filenames of these resources. Should
              be called before any other mtr_
              prefixed function.
        * mtr_clean_up
              cleans up an mtr_ResourceMaster. Frees
              all associated memory and resources.
        * mtr_update
              checks for new resource requests.
              This should happen frequently; if
              you're using this in a game, call this
              every frame.
        * mtr_request_resource
              requests the loading thread to start
              in order to load a given resource.
        * mtr_grab_resource_data
              takes any loaded data associated with
              a resource and returns it (via passed
              pointers). Returns 1 if successful,
              0 otherwise.

        --------------------------------------------

        In order to start using this, you should
        create/initialize (via mtr_new) an
        mtr_ResourceMaster. Be sure to call
        mtr_update frequently, and call
        mtr_request_resource as you please.
        When the resource is finished loading,
        mtr_grab_resource_data will be successful
        and you can use your loaded data.
        Be sure to call mtr_clean_up if/when you
        want any mtr-allocated memory freed.

    EXAMPLE
        enum {
            RS_FILE_1,
            RS_FILE_2,
            RS_FILE_3,
            RS_FILE_4,
            MAX_RS
        };
        const char *resource_filenames[MAX_RS] = {
            "file1.txt",
            "file2.gif",
            "file3.mp4",
            "file4.mp3",
        };
        mtr_ResourceMaster *rs_master = mtr_new(
            MAX_RS,
            resource_filenames
        );
        mtr_request_resource(rs_master, RS_FILE_1);
        while(true) {
            mtr_update(rs_master);
            printf("I'm updating while stuff is being loaded!\n");
            int64_t data_len = 0;
            void *data = NULL;
            if(mtr_grab_resource_data(rs_master, RS_FILE_1, &data, &data_len)) {
                printf("The file is finished loading! Here are the contents!\n\n");
                for(int64_t i = 0; i < data_len; i++) {
                    printf("%c", ((char *)data)[i]);
                }
                free(data);
                break;
            }
        }
        mtr_clean_up(rs_master);

    WARNING
        You're in charge of how the data loaded
        is interpreted. If you're not careful,
        you'll have some nasty crashes. This
        library is solely for the purpose of
        making the bare-bones basics of
        multi-threaded resource management
        convenient. You can add a pretty
        client-side interface if you want
        where you don't have to worry about
        this stuff, but that is not the point
        of this library; that would make it
        less general!

    LICENSE INFORMATION IS AT THE END OF THE FILE
*/

#ifndef _MT_RESOURCE_LOADING_H
#define _MT_RESOURCE_LOADING_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

typedef struct mtr_Resource {
    int8_t need_load;
    const char *filename;
    int64_t data_len;
    void *data;
} mtr_Resource;

typedef struct mtr_ResourceMaster {
    int8_t need_load,
           is_loading,
           need_finish;

    pthread_t         load_thread;
    pthread_mutex_t   mutex;

    uint16_t resource_count;
    mtr_Resource *resources;
} mtr_ResourceMaster;

inline void *_mtr_resource_load_thread(void *resources) {
    mtr_ResourceMaster *r = (mtr_ResourceMaster *)resources;

    for(uint16_t i = 0; i < r->resource_count; i++) {
        if(r->resources[i].need_load) {
            pthread_mutex_lock(&r->mutex);
            int8_t data_loaded = r->resources[i].data != 0;
            pthread_mutex_unlock(&r->mutex);

            if(!data_loaded) {
                FILE *file = fopen(r->resources[i].filename, "rb");
                if(file) {
                    int64_t file_size;
                    char *buffer;
                    fseek(file, 0, SEEK_END);
                    file_size = ftell(file);
                    rewind(file);
                    buffer = (char *)calloc(file_size + 1, sizeof(char));
                    fread(buffer, file_size, 1, file);
                    fclose(file);

                    pthread_mutex_lock(&r->mutex);
                    r->resources[i].data_len = file_size;
                    r->resources[i].data = (void *)buffer;
                    pthread_mutex_unlock(&r->mutex);
                }
            }

            pthread_mutex_lock(&r->mutex);
            r->resources[i].need_load = 0;
            pthread_mutex_unlock(&r->mutex);
        }
    }

    pthread_mutex_lock(&r->mutex);
    r->need_finish = 1;
    pthread_mutex_unlock(&r->mutex);

    return NULL;
}

inline mtr_ResourceMaster *mtr_new(uint16_t resource_count, const char **filenames) {
    mtr_ResourceMaster *r = (mtr_ResourceMaster *)malloc(sizeof(mtr_ResourceMaster));

    r->need_load = 0;
    r->is_loading = 0;
    r->need_finish = 0;

    r->mutex = PTHREAD_MUTEX_INITIALIZER;

    r->resource_count = resource_count;
    r->resources = (mtr_Resource *)calloc(resource_count, sizeof(mtr_Resource));
    for(uint16_t i = 0; i < resource_count; ++i) {
        r->resources[i].filename = filenames[i];
    }

    return r;
}

inline void mtr_clean_up(mtr_ResourceMaster *r) {
    for(uint16_t i = 0; i < r->resource_count; i++) {
        free(r->resources[i].data);
    }
    free(r->resources);
    free(r);
}

inline void mtr_update(mtr_ResourceMaster *r) {
    if(r->is_loading) {
        int8_t finished = 0;
        if(!pthread_mutex_trylock(&r->mutex)) {
            finished = r->need_finish;
            pthread_mutex_unlock(&r->mutex);
        }

        if(finished) {
            pthread_join(r->load_thread, NULL);
            r->need_load = 0;
            r->is_loading = 0;
            r->need_finish = 0;
        }
    }
    else {
        if(r->need_load) {
            pthread_join(r->load_thread, NULL);
            r->is_loading = 1;
            pthread_create(&r->load_thread, NULL, _mtr_resource_load_thread, (void *)r);
        }
    }
}

inline void mtr_request_resource(mtr_ResourceMaster *r, uint16_t index) {
    r->resources[index].need_load = 1;

    pthread_mutex_lock(&r->mutex);
    r->need_load = 1;
    pthread_mutex_unlock(&r->mutex);
}

inline int8_t mtr_resource_data_ready(mtr_ResourceMaster *r, uint16_t index) {
    pthread_mutex_lock(&r->mutex);
    if(r->resources[index].data) {
        pthread_mutex_unlock(&r->mutex);
        return 1;
    }
    pthread_mutex_unlock(&r->mutex);
    return 0;
}

inline int8_t mtr_grab_resource_data(mtr_ResourceMaster *r, uint16_t index, void **data, int64_t *data_len) {
    pthread_mutex_lock(&r->mutex);
    if(r->resources[index].data) {
        *data = r->resources[index].data;
        *data_len = r->resources[index].data_len;
        r->resources[index].data = NULL;
        r->resources[index].data_len = 0;
        pthread_mutex_unlock(&r->mutex);
        return 1;
    }
    pthread_mutex_unlock(&r->mutex);
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
