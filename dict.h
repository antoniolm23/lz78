#include "tab.h"

/*******************************************************************************
 * 					COMPRESSOR DICTIONARY & FUNCTIONS
 * ****************************************************************************/

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
void comp_dict_init(dictionary*, int, int);

//add a word in the dictionary
int comp_dict_add_word(int index, int father, unsigned int symbol, dictionary* d);

//search a couple father child in the dictionary
int comp_dict_search(int* father, unsigned int child, dictionary* D);

//destroy the dictionary
void comp_dict_suppress(dictionary* dict);

/*******************************************************************************
 * 					DECOMPRESSOR DICTIONARY & FUNCTIONS
 * ****************************************************************************/

typedef struct dec_dict{
	int size;				//max dimension of dictionary
	int symbols;			//symbols for initialization
	table tab;
}dec_dictionary;


//function to initialize the dictionary used by the decompressor
void decomp_dict_init(dec_dictionary* d, int size, int symbols);

//insertion in the decompressor dictionary
void decomp_dict_insertion(int father, unsigned int symbol, dec_dictionary* d);

//retrieve a word in the dictionary
//write in size how many positions there are in the rebuilt word
int decomp_dict_reb_word(int index, int* vector, int* size, dec_dictionary* d);

//suppress the decompressor dictionary
void decomp_dict_suppress(dec_dictionary* d);