/*
 * tab.h
 * Antonio La Marra - Giacomo Rotili
 * February 2014
 *
 * Definitions  of type, data strucutre and functions prototypes by which the
 * dictionaries of compressor and decompressor are implemented.
 *
 * NOTE: The data structures used by the compressor and decompressor are
 *		 different, but have been defined on the same files for analogies
 *       functionality and not to create too many source files
 *
 *       Collisions are handled with a hybrid solution between the method of
 *       chaining and open addressing. The elements are added in the first found
 *       free entry, and in the collision list is added the position of this
 *       entry. All of this to obtain a performance trade-off between the two
 *       classical methods for resolving collisions and the two different
 *       phases in which a collision takes place: the search and the addition
 *       in the table.
 */

#include "everything.h"

/* Default values to initialize table entries of the compressor  */
#define EMPTY_ENTRY -2
#define ROOT -3
#define EOFC 256

/* Flags for search */
#define FOUND 3
#define NOT_FOUND 4

/* Computations used by the function lookup() to compute the hash (See tab.c) */

#define rot(x, k) (((x) << (k)) | ((x) >> (32 - (k))))

#define mix(a, b, c) \
{ \
	a -= c;  a ^= rot(c, 4);  c += b; \
	b -= a;  b ^= rot(a, 6);  a += c; \
	c -= b;  c ^= rot(b, 8);  b += a; \
	a -= c;  a ^= rot(c, 16);  c += b; \
	b -= a;  b ^= rot(a, 19);  a += c; \
	c -= b;  c ^= rot(b, 4);  b += a; \
}

#define final(a, b, c) \
{ \
	c ^= b; c -= rot(b, 14); \
	a ^= c; a -= rot(c, 11); \
	b ^= a; b -= rot(a, 25); \
	c ^= b; c -= rot(b, 16); \
	a ^= c; a -= rot(c, 4);  \
	b ^= a; b -= rot(a, 14); \
	c ^= b; c -= rot(b, 24); \
}

/*		*		*		*		*		*		*		*		*		*
 *							COMPRESSOR DATA STRUCTURES
 *		*		*		*		*		*		*		*		*		*/


/* Node element of the collision list */
typedef struct collision {
	unsigned int elem_pos;	/* position of the entry in the table */
	unsigned int elem_father;	/* father of the element in elem_pos */
	struct collision* next;
} collision_elem;

/* Entry of the hash table */
typedef struct ht_entry {
	unsigned int ht_father;	/* label of the father entry */
	unsigned int ht_symbol;	/* symbol of the current entry */
	unsigned int label;	/* label f the current entry*/
	collision_elem* next;	/* collision list */
} ht_entry;

/* Hash table object */
typedef struct ht_table {
	int next_free_entry;	/* Next free entry to use (in collision case) */
	int total_size;	/* dimension of the table */
	unsigned int next_label;	/* label to the next node added */
	ht_entry* ht_array;	/* "effective" table */
} ht_table;


/*		*		*		*		*		*		*		*		*		*
 *							DECOMPRESSOR DATA STRUCTURES
 *		*		*		*		*		*		*		*		*		*/


/* Entry of the table */
typedef struct d_entry {
	int father;	/* index of the father in the tree */
	unsigned int symbol;	/* symbol associated to the arch */
} d_entry;

/* Decompressor table */
typedef struct table {
	int next_pos;	/* next position for insertion */
	int size;	/* dimension of the table */
	d_entry* array;	/* "effective" table */
} table;


/*		*		*		*		*		*		*		*		*		*
 *							COMPRESSOR TABLE FUNCTIONS
 *		*		*		*		*		*		*		*		*		*/

/* LIST_INS
 *
 * PURPOSE: Insert an element at the top of collision list
 * PARAMETERS:	pos - position of the element in the table
 *				father - label of the father node of the element
 *				list - pointer to the collision list
 *
 * This list is passed by reference to be modified
 */
void list_ins(int pos, int father, collision_elem** list);


/* LIST_SEARCH
 *
 * PURPOSE: Search an element in the collision list
 * PARAMETERS:	index - Index of the entry where occurs the collision
 *				father - label of the father that caused the collision
 *				symbol - symbol that caused the collision
 *				ht_array - Hash table
 *				flag - to notify if the element has been found
 *RETURNS:	The position of the element otherwise 0 if it hasn't been found
 *
 * Flag is overwritten with the constants FOUND or NOT_FOUND
 */
int list_search(int index, int father, unsigned int symbol, ht_table* ht_array, int* flag);


/* LIST_DEL
 *
 * PURPOSE:	Destroy a collision list
 * PARAMETERS:	list - Collision list to destroy
 *
 * The list is totally deallocated and list is set NULL
 */
void list_del(collision_elem* list);


/* HASH_INIT
 *
 * PURPOSE: Initialize the hash table structure
 * PARAMETERS:	size - size of the table
 *				symbols - number of symbols of the sequence to compress
 *				table - hash table to initialize
 */
void hash_init(int size, int symbols, ht_table* table);


/* HASH_SEARCH
 *
 * PURPOSE: Search an element(node) in the hash table(tree)
 * PARAMETERS:	father - father of the node to search
 *				child - symbol of the node to search
 *				table - hash table in which search
 * RETURNS:	If the element is present returns -1 otherwise the position
 *			computed by the hash function
 *
 * If the element is present father is overwritten with the pointer(label) to
 * the child node just searched.
 */
int hash_search(int* father, unsigned int child, ht_table* table);


/* HASH
 * PURPOSE: Compute the hash values
 * PARAMETERS:	father - number obtained from a specific computing from father
 *							and child (See definition of hash_search() )
 *				size - Total size of the table
 *RETURNS:	The hash value, that is the position(maybe) of the node in the table
 */
int hash(int father, int size);


/* HASH_ADD
 * PURPOSE:	Insert an element(child node) to the table(tree)
 * PARAMETERS:	index - Position in which insert the node
 *				father - father of the node to insert
 *				symbol - symbol of the node to insert
 *				table - hash table in which insert
 *
 * RETURNS:	Returns 1 if the insertion successfully,otherwise -1 i.e. if the
 *			table is full
 */
int hash_add(int index, int father, unsigned int symbol, ht_table* table);


/* HASH_SUPPRESS
 * PURPOSE: Delete the hash table
 * PARAMETERS: table - Hash table to delete
 *
 * ht_array is deallocated; next_label and total_size are set to 0
 */
void hash_suppress(ht_table* table);


/* HASH_PRINT
 * PURPOSE: Print to video the contents of the hash table
 * PARAMETERS: table - Hash table to print
 */
void hash_print(ht_table* table);


/*		*		*		*		*		*		*		*		*		*
 *							DECOMPRESSOR TABLE FUNCTIONS
 *		*		*		*		*		*		*		*		*		*/


/* TAB_INIT
 *
 * PURPOSE:	Initialize the table structure
 * PARAMETERS:	size - size of the table
 *				symbols - number of symbols of the compressed sequence
 *				table - table to initialize
 */
void tab_init(table* t, int size, int symbols);

/* TAB_INSERTION
 * PURPOSE:	Insert an element(child node) to the table(tree)
 * PARAMETERS:	father - father of the node to insert
 *				symbol - symbol of the node to insert
 *				table - table in which insert
 *
 * RETURNS:	Returns the position in which insert next node, otherwise -1 i.e.
 *			the table is full
 */
int tab_insertion(int father, unsigned int symbol, table* t);

/* TAB_RETRIEVE_WORD
 * PURPOSE:	Retrieves the symbols of the nodes between the node with label index
 *			and the root, i.e. a word in the dictionary
 *PARAMETERS:	index - position of the node (last character in the word)
 *				vector - buffer in which store the word retrieved
 *				size - Size of the word retrieved
 *				t - table from which retrieve
 *RETURNS:	Returns 0 if the retrieval successfully, otherwise -1.
 *
 * In vector is stored the word retrieved and in size its length.
 */
int tab_retrieve_word(int index, int* vector, int* size, table* t);

/* TAB_SUPPRESSION
 * PURPOSE: Delete the table
 * PARAMETERS: t - Table to delete
 *
 * array is deallocated; next_pos and size are set to 0
 */
void tab_suppression(table* t);

/* PRINT_TAB
 * PURPOSE: Print to video the contents of the table
 * PARAMETERS: t - Table to print
 */
void print_tab(table* t);
