#include "dict.h"
//#include "everything.h"

//initialize the dictionary
void dict_init(dictionary* dict, int dict_tot, int symbols) {
	
	dict->dict_size=dict_tot;
	dict->symbols=symbols;
	hash_init(dict_tot, symbols, &dict->table);
	
}

//add a new string in the dictionary
int dict_add_word(int index, int father, unsigned int symbol, dictionary* d) {
	
	int res;
	
	//call the add string in the hash
	res=hash_add(index, father, symbol, &d->table);
	
	//if the hash is full suppress it and rebuid it
	if(res==-1) {
		//fprintf(stderr, "full dict, %i %i\n", father, symbol);
		hash_suppress(&d->table);
		hash_init(d->dict_size, d->symbols, &d->table);
		//pos=hash_add(index, father, symbol, &d->table, flag);
	}
	
	return res;
}

//search a couple father son in the table
int dict_search(int* father, unsigned int child, dictionary* D){	
	int tmp = hash_search(father, child, &D->table);
	//fprintf(stderr, "dict_index: %i\n", tmp);
	return tmp;
}

//suppress the dictionary
void suppress_dictionary(dictionary* d) {
	hash_suppress(&d->table);
	d->dict_size=0;
	d->symbols=0;
} 

