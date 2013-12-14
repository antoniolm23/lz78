#include "dict.h"
//#include "everything.h"

/*******************************************************************************
 * 					COMPRESSOR DICTIONARY
 * ****************************************************************************/

//initialize the dictionary
void comp_dict_init(dictionary* dict, int dict_tot, int symbols) {
	
	dict->dict_size=dict_tot;
	dict->symbols=symbols;
	hash_init(dict_tot, symbols, &dict->table);
	
}

//add a new string in the dictionary
int comp_dict_add_word(int index, int father, unsigned int symbol, dictionary* d) {
	
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
int comp_dict_search(int* father, unsigned int child, dictionary* D){
	int tmp_father=*father;
	int tmp = hash_search(&tmp_father, child, &D->table);
	//fprintf(stderr, "dict_index: %i\n", tmp);
	*father=tmp_father;
	return tmp;
}

//suppress the dictionary
void comp_dict_suppress(dictionary* d) {
	hash_suppress(&d->table);
	d->dict_size=0;
	d->symbols=0;
} 

/*******************************************************************************
 * 					DECOMPRESSOR DICTIONARY & FUNCTIONS
 * ****************************************************************************/

//function to initialize the dictionary used by the decompressor
void decomp_dict_init(dec_dictionary* d, int size, int symbols) {
	
	d->size=size;
	d->symbols=symbols;
	
	tab_init(&d->tab, size, symbols);
	
}

//insertion in the decompressor dictionary
void decomp_dict_insertion(int father, unsigned int symbol, dec_dictionary* d){
	
	int res;
	res=tab_insertion(father, symbol, &d->tab);
	 
	//check if we need a new table
	if(res==-1) {
		
		tab_suppression(&d->tab);
		tab_init(&d->tab, d->size, d->symbols);
		
	}
	
}

//returns how many positions there are in the rebuilt word
int decomp_dict_reb_word(int index, int* vector, int* size, dec_dictionary* d) {
	
	int tmp=*size;
	int res;
	res=tab_retrieve_word(index, &(*vector), &tmp, &d->tab);
	*size=tmp;
	
	return res;
}