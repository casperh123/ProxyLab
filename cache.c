//
// Created by casper on 10/28/24.
//

#include "cache.h"
#include <string.h>

cache_node cache_get(cache *cache, int key) {

  cache_node *current = cache->head;
  cache_node *previous = NULL;

  if (current == NULL) {
    return NULL;
  }

  while (current != NULL) {

    if (current->key == key) {

        //Already at the head. No Need to rearrange.
        if (current == cache->head) {
            return current->value;
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


        return current->value;
    }

    current = current->next;
  }

  return NULL;
}

int cache_put(cache *cache, int key, int data);
