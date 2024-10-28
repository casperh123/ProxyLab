#ifndef CACHE_H
#define CACHE_H

typedef struct cache {
    //TODO implement an actual data store
    int size;
    int current_size;
    int max_size;
} cache;

cache create_cache(int size);
int get(int key);
int put(int key, int data);

#endif // CACHE_H
