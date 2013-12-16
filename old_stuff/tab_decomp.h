//decompressor data structures are different from the compressor ones
#include "everything.h"

struct entry{
	int father;				//index of the father in the tree
	unsigned int symbol;	//symbol associated to the arch
}entry;

struct tab{
	int next_pos;			//next position for insertion
	int size;				//total size
	entry* array;			//array of entities
}table;

struct dict{
	int size;				//max dimension of dictionary
	int symbols;			//symbols for initialization
	int actual_size;		//actual size
	table* array;
}dictionary;

/***************dictionary functions***************************/
void dict_insertion(int father, unsigned int symbol, dictionary* d);
void dict_init(dictionary* d, int size, int symbols);
void dict_suppress(dictionary* d);
//returns how many positions there are in the rebuilt word
void dict_reb_word(int* vector, dictionary* d, int& size);

/***************table functions*******************************/
void tab_insertion(int father, unsigned int symbol, table* t);
void tab_init(table* t, int size);
//returns if a new table is needed
int tab_insertion(int father, unsigned int symbol, table* t);
void tab_suppression(table* t);
int tab_retrieve_word(int* vector, table* t, int& size);
