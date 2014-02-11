/*
 * dict.c
 * Antonio La Marra - Giacomo Rotili
 * February 2014
 *
 * This file contains the definitions of the functions for use dictionary and
 * dec_dictionary structure. (See dict.h)
 *
 * NOTE: Almost all the functions simply invoke the methods of the underlying
 *			structures. (See dict.h)
 */

#include "dict.h"

/*		*		*		*		*		*		*		*		*		*
 *								COMPRESSOR DICTIONARY
 *		*		*		*		*		*		*		*		*		*/


/* COMP_DICT_INIT
 *
 * PURPOSE: Initialize the dictionary
 */
void comp_dict_init(dictionary* dict, int dict_tot, int symbols) {
	
	/* Setting other parameter of the structure */
	dict->dict_size = dict_tot;
	dict->symbols = symbols;
	hash_init(dict_tot, symbols, &dict->table);
}


/* COMP_DICT_ADD_WORD
 *
 * PURPOSE: Add a word to the dictionary
 */
int comp_dict_add_word(int index, int father, unsigned int symbol, dictionary* d) {
	
	int res;	/* variable to store hash_add() return */
	
	res = hash_add(index, father, symbol, &d->table);
	
	if(res == -1) {	/* Hash table full? */
		hash_suppress(&d->table);	/* Destroy actual dictionary */
		hash_init(d->dict_size, d->symbols, &d->table);	/* Built a new one*/
	}
	return res;
}


/* COMP_DICT_SEARCH
 *
 * PURPOSE: Search a word in the dictionary
 */
int comp_dict_search(int* father, unsigned int child, dictionary* D){
	
	int tmp, tmp_father;	/* Temporary variables */
	
	tmp_father = *father;
	tmp = hash_search(&tmp_father, child, &D->table);
	*father = tmp_father;
	
	return tmp;
}


/* COMP_DICT_SUPPRESS
 *
 * PURPOSE: Destroy the dictionary
 */
void comp_dict_suppress(dictionary* d) {
	
	hash_suppress(&d->table);	/* Destroy the hash table */
	/* reset parameters of the dictionary */
	d->dict_size = 0;
	d->symbols = 0;
	free(d);
}


/*		*		*		*		*		*		*		*		*		*
 *								DECOMPRESSOR DICTIONARY
 *		*		*		*		*		*		*		*		*		*/



/* DECOMP_DICT_INIT
 *
 * PURPOSE: Initialize the dictionary
 */
void decomp_dict_init(dec_dictionary* d, int size, int symbols) {
	
	d->size = size;
	d->symbols = symbols;
	tab_init(&d->tab, size, symbols);
}


/* DECOMP_DICT_INSERTION
 *
 * PURPOSE: Insert a word to the dictionary
 */
void decomp_dict_insertion(int father, unsigned int symbol, dec_dictionary* d){
	
	int res;
	
	res = tab_insertion(father, symbol, &d->tab);
	if(res == -1) {	/* Table full? */
		tab_suppression(&d->tab);	/* Destroy the table */
		tab_init(&d->tab, d->size, d->symbols);	/* Built a new one*/
	}
}


/* DECOMP_DICT_REB_WORD
 *
 * PURPOSE: Retrieve a word in the dictionary
 */
int decomp_dict_reb_word(int index, int* vector, int* size, dec_dictionary* d) {
	
	int tmp, res;	/* Support variables */
	
	tmp = *size;
	res = tab_retrieve_word(index, vector, &tmp, &d->tab);
	*size = tmp;
	
	return res;
}


/* DECOMP_DICT_SUPPRESS
 *
 * PURPOSE: Destroy the dictionary
 */
void decomp_dict_suppress(dec_dictionary* d){
	
	tab_suppression(&d->tab);	/* Destroy the table */
	free(d);	/* Deallocate the dictionary */
}
