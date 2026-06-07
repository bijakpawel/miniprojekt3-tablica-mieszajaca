#include "hash_tables.hpp"
#include "pomocnik.h"

#include <iostream>

#include <string>
#include <vector>

int main(){

    zmierzTabele<ChainingHashTable>("wyniki/pomiary_chaining.csv");
    zmierzTabele<LinearProbingHashTable>("wyniki/pomiary_linear.csv");
    zmierzTabele<AvlBucketHashTable>("wyniki/pomiary_avl.csv");
    return 0;
}