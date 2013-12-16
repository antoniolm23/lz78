#include "everything.h"

/*******************************************************************************
 * 						COMPRESSOR DATA STRUCTURES
 * ****************************************************************************/

//flags to initialize the table
#define EMPTY_ENTRY -2
#define ROOT -3
#define EOFC 256

//Flags
#define ADD_OK 1
#define FULL_DICT 0

/*#define EMPTY_POS 2
#define OCCUPIED_POS 3
#define YOUR_POS 4
*/

#define FOUND 3
#define NOT_FOUND 4

//hash functions variables
#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}

#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}

typedef struct collision {
    
    unsigned int elem_pos;
    unsigned int elem_father;
    struct collision* next;
    
}collision_elem;


typedef struct ht_entry {
    
    unsigned int ht_father;			//current entry
    unsigned int ht_symbol;			//symbol of the current entry
    unsigned int label;
    collision_elem* next;		//collision list
    
}ht_entry;

typedef struct ht_table{
	
	int next_free_entry;		//next free entry in case of collision
	int total_size;				//total dimension of the table
	unsigned int next_label;	//next child label
	ht_entry* ht_array;
	//bool full;	
	
}ht_table;

/*******************************************************************************
 * 					DECOMPRESSOR DATA STRUCTURES
 * ****************************************************************************/

typedef struct entry{
	int father;				//index of the father in the tree
	unsigned int symbol;	//symbol associated to the arch
}entry;

typedef struct table{
	int next_pos;			//next position for insertion
	int size;				//total size
	entry* array;			//array of entities
}table;

/*******************************************************************************
 * 						COMPRESSOR TABLE FUNCTIONS
 * ****************************************************************************/

//insert int the top of the list
int list_ins(int, int, unsigned int, collision_elem**);

//search in the list at index the node that has father father and child symbol
int list_search(int index, int father, unsigned int symbol, ht_table* ht_array, int* flag);
void list_del(collision_elem*);

//initialization of the hash table, ht_table is passed by reference
void hash_init(int size, int symbols, ht_table* table);

/* Search a child of a father, in case the child is present returns 0 and father contains the child_label 
 *that represent the pointer to the child node in the tree, else returns the position in which do the insertion in the hash table 
 and father remains unchanged*/
int hash_search(int* father, unsigned int child, ht_table* table);

//computing hash function
int hash(int, int);

//return the new position or -1 if the table is full
int hash_add(int index, int father, unsigned int symbol, ht_table* table);

//int collision_add(int index, int father, unsigned int symbol, dictionary* dict);
//suppress the hash table 
void hash_suppress(ht_table* table);

/*******************************************************************************
 * 						DECOMPRESSOR TABLE FUNCTIONS
 * ****************************************************************************/

//init the table
void tab_init(table* t, int size, int symbols);

//returns if a new table is needed
int tab_insertion(int father, unsigned int symbol, table* t);

//retrieve a word in the dictionary
int tab_retrieve_word(int index, unsigned char* vector, int* size, table* t);

//suppress the table
void tab_suppression(table* t);