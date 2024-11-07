//
// Created by casper on 02-11-24.
//

unsigned int hash(const char* url) {
    unsigned int hash = 0;
    while (*url) {
        hash = (hash * 33) + *url;
        url++;
    }
    return hash;
}
