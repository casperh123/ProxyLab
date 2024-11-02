//
// Created by casper on 10/28/24.
//

#include "cache.h"
#include <string.h>

cache_node* cache_get(cache *cache, int key) {

  cache_node *current = cache->head;
  cache_node *previous = NULL;

  if (current == NULL) {
    return NULL;
  }

  while (current != NULL) {

    if (current->key == key) {

        //Already at the head. No Need to rearrange.
        if (current == cache->head) {
            return current;
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

        return current;
    }

    current = current->next;
  }

  return NULL;
}

int cache_put(cache *cache, int key, char *data) {

    cache_node new_entry = {
        .next = NULL,
        .previous = NULL,
        .key = key,
        .data = data,
        .size = cache->buffer_size
    };

    cache_node* node = &new_entry;

    cache->tail->next = NULL;
    cache->tail = NULL;
    cache->head->previous = node;
    node->next = cache->head;
    cache->head = node;

    return node;
}

int append_data(cache_node, char* data) {

}

//TODO implement cases for head, tail and middle
int cache_remove(cache* cache, int key)  {

    cache_node* current = cache->head;
    cache_node* previous = NULL;

    while(current != NULL) {
        if(current->key == key) {
            previous->next = current->next;
            current->next->previous = previous;

            return 0;
        }

        previous = current;
        current = current->next;
    }

    return -1;
}
