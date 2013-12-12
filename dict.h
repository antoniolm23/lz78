//#include "everything.h"
#include "hash.h"

//structure that defines the dictionary
typedef struct dict{
	
	int dict_size;
	int symbols;
	ht_table table;	
	
}dictionary;

/*function to initialize the dictionary, the arguments are:
 * dictionary to initialize, dimension, symbols
 * in the initialization ht_array points to an array of
 * SIZE entries */
void dict_init(dictionary*, int, int);

//add a word in the dictionary
int dict_add_word(int index, int father, unsigned int symbol, dictionary* d);

//search a couple father child in the dictionary
int dict_search(int* father, unsigned int child, dictionary* D);

//destroy the dictionary
void suppress_dictionary(dictionary* dict);


