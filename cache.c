//
// Created by casper on 10/28/24.
//

#include "cache.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char* cache_get(cache *cache, int key) {

  cache_node *current = cache->head;
  cache_node *previous = NULL;

  if (current == NULL) {
    return NULL;
  }

  while (current != NULL) {

    if (current->key == key) {

        //Already at the head. No Need to rearrange.
        if (current == cache->head) {
            return current->data;
        }

        //Detach current from previous next, and set previous next to next element in list.
        current->previous->next = current->next;

        //If next is present, set next previous to previoues
        if(current->next != NULL) {
            current->next->previous = current->previous;
        } else {
            cache->tail = current->previous;
        }

        //Set the current entry as the head;
        current->next = cache->head;
        current->previous = NULL;
        cache->head->previous = current;
        cache->head = current;

        return current->data;
    }

    current = current->next;
  }

  return NULL;
}

int cache_put(cache *cache, int key, char *data, size_t data_size) {

    if(data_size > cache->max_object_size) {
        return 0;
    }

    //Allocate heap memory
    cache_node* new_node = malloc(sizeof(cache_node));

    new_node->data = malloc(data_size);

    //Copy the data
    memcpy(new_node->data, data, data_size);

    new_node->key = key;
    new_node->size = data_size;
    new_node->next = NULL;
    new_node->previous = NULL;

    cache->size += data_size;

    cache_prepend(cache, new_node);

    while(cache->size > cache->max_size) {
        cache_remove(cache, cache->tail->key);
    }

    return 1;
}

int cache_prepend(cache *cache, cache_node *node) {
    if(cache->head == NULL) {
        cache->head = node;
        cache->tail = node;

        return 1;
    }

    cache->head->previous = node;
    node->next = cache->head;
    cache->head = node;

    return 1;
}

//TODO implement cases for head, tail and middle
int cache_remove(cache* cache, int key)  {

    cache_node* current = cache->head;

    while(current != NULL) {

        if(current->key == key) {

            if(current->previous == NULL && current->next == NULL) {
                cache->head = NULL;
                cache->tail = NULL;
            } else if (current->next == NULL) {
                current->previous->next = NULL;
                cache->tail = current->previous;
            } else if (current->previous == NULL) {
                cache->head = current->next;
                current->next->previous = NULL;
            } else {
                current->next->previous = current->previous;
                current->previous->next = current->next;
            }

            cache->size -= current->size;

            free(current->data);
            free(current);

            return 0;
        }
        current = current->next;
    }
    return -1;
}
