/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#ifndef NDEBUG
#include <signal.h>
#endif

#include "cache.h"

//#define USE_REGULAR_MALLOC

#ifdef USE_REGULAR_MALLOC
#define excitevm_smalloc(X) malloc(X)
#define excitevm_srealloc(X,Y) realloc(X,Y)
#define excitevm_scalloc(X,Y) calloc(X,Y)
#define excitevm_sfree(X) free(X)
#define excitevm_init()
#define excitevm_enter()
#else
#include "excitevm/excitevm_c.h"
#endif

#ifndef NDEBUG
const uint64_t redzone_pattern = 0xdeadbeefcafebabe;
int cache_error = 0;
#endif

const int initial_pool_size = 64;

cache_t* cache_create(const char *name, size_t bufsize, size_t align,
                      cache_constructor_t* constructor,
                      cache_destructor_t* destructor) {
    cache_t* ret =  /*excitevm_s*/calloc(1, sizeof(cache_t));
    char* nm = strdup(name);
    void** ptr =  /*excitevm_s*/calloc(initial_pool_size, sizeof(void*));
    if (ret == NULL || nm == NULL || ptr == NULL ||
        pthread_mutex_init(&ret->mutex, NULL) == -1) {
        /*excitevm_s*/free(ret);
        /*excitevm_s*/free(nm);
        /*excitevm_s*/free(ptr);
        return NULL;
    }

    ret->name = nm;
    ret->ptr = ptr;
    ret->freetotal = initial_pool_size;
    ret->constructor = constructor;
    ret->destructor = destructor;

#ifndef NDEBUG
    ret->bufsize = bufsize + 2 * sizeof(redzone_pattern);
#else
    ret->bufsize = bufsize;
#endif

    return ret;
}

static inline void* get_object(void *ptr) {
#ifndef NDEBUG
    uint64_t *pre = ptr;
    return pre + 1;
#else
    return ptr;
#endif
}

void cache_destroy(cache_t *cache) {
    while (cache->freecurr > 0) {
        void *ptr = cache->ptr[--cache->freecurr];
        if (cache->destructor) {
            cache->destructor(get_object(ptr), NULL);
        }
//assert(0);
        /*excitevm_s*/free(ptr);
    }
//assert(0);
     /*excitevm_s*/free(cache->name);
     /*excitevm_s*/free(cache->ptr);
    pthread_mutex_destroy(&cache->mutex);
     /*excitevm_s*/free(cache);
}

void* cache_alloc(cache_t *cache) {
    void *ret;
    void *object;
    pthread_mutex_lock(&cache->mutex);
    if (cache->freecurr > 0) {
        ret = cache->ptr[--cache->freecurr];
        object = get_object(ret);
    } else {
        object = ret = /*excitevm_s*/malloc(cache->bufsize);
        if (ret != NULL) {
            object = get_object(ret);

            if (cache->constructor != NULL &&
                cache->constructor(object, NULL, 0) != 0) {
//assert(0);
                 /*excitevm_s*/free(ret);
                object = NULL;
            }
        }
    }
    pthread_mutex_unlock(&cache->mutex);

#ifndef NDEBUG
    if (object != NULL) {
        /* add a simple form of buffer-check */
        uint64_t *pre = ret;
        *pre = redzone_pattern;
        ret = pre+1;
        memcpy(((char*)ret) + cache->bufsize - (2 * sizeof(redzone_pattern)),
               &redzone_pattern, sizeof(redzone_pattern));
    }
#endif

    return object;
}

void cache_free(cache_t *cache, void *ptr) {
    pthread_mutex_lock(&cache->mutex);

#ifndef NDEBUG
    /* validate redzone... */
    if (memcmp(((char*)ptr) + cache->bufsize - (2 * sizeof(redzone_pattern)),
               &redzone_pattern, sizeof(redzone_pattern)) != 0) {
        raise(SIGABRT);
        cache_error = 1;
        pthread_mutex_unlock(&cache->mutex);
        return;
    }
    uint64_t *pre = ptr;
    --pre;
    if (*pre != redzone_pattern) {
        raise(SIGABRT);
        cache_error = -1;
        pthread_mutex_unlock(&cache->mutex);
        return;
    }
    ptr = pre;
#endif
    if (cache->freecurr < cache->freetotal) {
        cache->ptr[cache->freecurr++] = ptr;
    } else {
        /* try to enlarge free connections array */
        size_t newtotal = cache->freetotal * 2;
        void **new_free =  /*excitevm_s*/realloc(cache->ptr, sizeof(char *) * newtotal);
        if (new_free) {
            cache->freetotal = newtotal;
            cache->ptr = new_free;
            cache->ptr[cache->freecurr++] = ptr;
        } else {
            if (cache->destructor) {
                cache->destructor(ptr, NULL);
            }
//assert(0);
             /*excitevm_s*/free(ptr);

        }
    }
    pthread_mutex_unlock(&cache->mutex);
}

