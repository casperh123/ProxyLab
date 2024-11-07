#ifndef CACHE_H
#define CACHE_H

#include <stddef.h>
typedef struct cache_node {
    struct cache_node* previous;
    struct cache_node* next;
    int key;
    char* data;
    size_t size;
} cache_node;

typedef struct cache {
    cache_node* head;
    cache_node* tail;
    size_t size;
    size_t max_size;
    size_t max_object_size;
    size_t buffer_size;
} cache;

char* cache_get(cache *cache, int key);
int cache_put(cache *cache, int key, char *data, size_t data_size);
int cache_append(cache *cache, cache_node *node);
int cache_prepend(cache_node *node);
int cache_remove(cache *cache, int key);

#endif // CACHE_H
