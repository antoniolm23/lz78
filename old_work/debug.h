/*
 This is an header file
 */
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>
#include "endian.h" //Mac OS requirement
#define EOFC 256
#define int_stdin 0
#define int_stdout 1
#define int_stderr 2

//#undef SIZE
//#define SIZE 2048

#ifndef bool
typedef enum{true, false} bool;	//definition of bool type
#endif

#define MAX_SIZE 3

struct bitio;

//in the write function len is expressed in bits
struct bitio* bit_open(char* name, char* mode);

//first function that opens a file in read (r) mode or write (w) mode
int bitio_write(struct bitio* f, uint64_t d, int len);

//Read len bits and save on datum
int bitio_read(struct bitio* f, uint64_t* datum, int len);

//close a bitio structure
void bitio_close(struct bitio* );


//#include "everything.h"

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
void tab_suppression(table* t);//#include "everything.h"
//#include "tab.h"

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

//suppress the decompressor dictionary
void decomp_dict_suppress(dec_dictionary* d);

//returns how many positions there are in the rebuilt word
int decomp_dict_reb_word(int index, unsigned char* vector, int* size, dec_dictionary* d);
//#include "everything.h"
//#include "dict.h"
//#include "bitio.h"

typedef struct hdr {
    int dictionary_size;   //bit length
    int longest_match;   //original filename
}header;

/*compress function, parameters are:
 * filename of the file from which we're reading data 
 * filename in which put the compressed data
 * size of the dictionary*/
void compress(char* from, char* to, int size);

/*decompress function*/
void decompress(char* from, char* to);
//library to manage bit

//#include "everything.h"

//max size of the buffer