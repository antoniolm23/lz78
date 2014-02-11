/*
 * dict.h
 * Antonio La Marra - Giacomo Rotili
 * December 2013
 *
 * Definitions data structure and functions prototypes by which the dictionaries
 * of compressor and decompressor are implemented.
 *
 * NOTE:	The dictionaries used by the compressor and decompressor are
 *			different, but have been defined on the same files for analogies
 *			functionality and not to create too many source files
 *
 *			We have tried to abstract as much as possible the structure of the
 *			dictionary in order to modify or change its implementation unless the
 *			compressor and the decompressor will be affected.
 */

#include "tab.h"

/*		*		*		*		*		*		*		*		*		*
 *								COMPRESSOR DICTIONARY & FUNCTIONS
 *		*		*		*		*		*		*		*		*		*/


/* Dictionary object */
typedef struct dict {
	
	int dict_size;	/* Dimension of the dictionary */
	int symbols;	/* Cardinality of the alphabet with which words are written */
	ht_table table;	/* Hash table, core of the dictionary */

} dictionary;


/* COMP_DICT_INIT
 *
 * PURPOSE: Initialize the dictionary
 * PARAMETERS:	dict - Dimension of the dictionary
 *				dict_tot - Dimension of the dictionary
 *				symbols - Cardinality of the alphabet
 */
void comp_dict_init(dictionary* dict, int dict_tot, int symbols);


/* COMP_DICT_ADD_WORD
 *
 * PURPOSE: Add a word to the dictionary
 * PARAMETERS:	index - Position in which insert the word
 *				father - word(node) that has produced the last match
 *				symbol - symbol to append to the last match(father node)
 *				d - dictionary in which insert
 * RETURNS: Returns 1 if the insertion successfully, otherwise -1 i.e. the
 *			dictionary is full
 *
 * When the dictionary is full, it is discarded and a new one is created, 
 * empty of course
 */
int comp_dict_add_word(int index, int father, unsigned int symbol, dictionary* d);


/* COMP_DICT_SEARCH
 *
 * PURPOSE: Search a word in the dictionary
 * PARAMETERS:	father - word(node) that has produced the last match
 *				child - last symbol read(child node)
 *				D - dictionary in which search
 * RETURNS: If the word is present returns -1 otherwise the the location which 
 *			to insert the word
 *
 * If the word is present father is overwritten with the pointer(label) to the 
 * word just searched(father node), from which will start the search at the 
 * next reading of a character
 */
int comp_dict_search(int* father, unsigned int child, dictionary* D);


/* COMP_DICT_SUPPRESS
 *
 * PURPOSE: Destroy the dictionary
 * PARAMETERS: dict - Dictionary to delete
 *
 * table is deallocated; symbols and dict_size are set to 0
 */
void comp_dict_suppress(dictionary* dict);


/*		*		*		*		*		*		*		*		*		*
 *							DECOMPRESSOR DICTIONARY & FUNCTIONS
 *		*		*		*		*		*		*		*		*		*/

/* Dictionary object */
typedef struct dec_dict{
	int size;	/* Dimension of the dictionary */
	int symbols;	/* Cardinality of the alphabet with which words are written */
	table tab;	/* Table, core of the dictionary */
}dec_dictionary;


/* DECOMP_DICT_INIT
 *
 * PURPOSE: Initialize the dictionary
 * PARAMETERS:	d - Dimension of the dictionary
 *				size - Dimension of the dictionary
 *				symbols - Cardinality of the alphabet
 */
void decomp_dict_init(dec_dictionary* d, int size, int symbols);


/* DECOMP_DICT_INSERTION
 *
 * PURPOSE: Insert a word to the dictionary
 * PARAMETERS:	father - last word(node) added to the dictionary
 *				symbol - symbol to append to the last added word(father node)
 *				d - dictionary in which insert
 */
void decomp_dict_insertion(int father, unsigned int symbol, dec_dictionary* d);


/* DECOMP_DICT_REB_WORD
 *
 * PURPOSE: Retrieve a word in the dictionary
 * PARAMETERS:	index - label of the word to retrieve
 *				vector - buffer in which store the word retrieved
 *				size - size of the word retrieved
 *				d - dictionary from which retrieve
 * RETURNS:	Returns 0 if the retrieval successfully, otherwise -1.
 *
 * In vector is stored the word retrieved and in size its length.
 */
int decomp_dict_reb_word(int index, int* vector, int* size, dec_dictionary* d);


/* DECOMP_DICT_SUPPRESS
 *
 * PURPOSE: Destroy the dictionary
 * PARAMETERS: d - Dictionary to destroy
 *
 * tab is deallocated
 */
void decomp_dict_suppress(dec_dictionary* d);
