#ifndef CACHE_H
#define CACHE_H

typedef struct node {
    struct node* previous;
    struct node* next;
    int key;
    int value;
    int size;
} node;

typedef struct cache {
    node* head;
    node* tail;
    int size;
    int max_size;
    int max_object_size;
} cache;

int cache_get(cache *cache, int key);
int cache_put(cache *cache, int key, int data);
int cache_generate_key();

#endif // CACHE_H
