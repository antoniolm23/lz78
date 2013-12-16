#include "tab.h"

//initialize the decompressor table
void tab_init(table* t, int size, int symbols) {
	
	int i;
	//initialize the first entries as did in the compressor_table
	for(i=0; i<symbols; i++) {
		t->array[i].father=ROOT;
		t->array[i].symbol=i;
	}
	
	//put the eof
	t->array[symbols].father=ROOT;
	t->array[symbols].symbol=EOFC;
	t->size=size;
	t->next_pos=symbols++;
	
	i=symbols+1;
	for(i=0; i<size; i++) {
		t->array[i].father=EMPTY_ENTRY;
		t->array[i].symbol=EMPTY_ENTRY;
	}
	
}

/*returns if a new table is needed:
 * 0 if no new table needed
 * -1 if a new table is needed*/
int tab_insertion(int father, unsigned int symbol, table* t) {
	
	int pos=t->next_pos;
	
	//add the entry
	t->array[pos].father=father;
	t->array[pos].symbol=symbol;
	t->next_pos++;
	
	//check if there is space
	if(t->next_pos>t->size) return -1;
	else return 0;
}

//retrieve a word in the dictionary
int tab_retrieve_word(int index, int* vector, int* size, table* t) {
	
	int pos=*size-1;
	int tmp_index=index;
	
	if(t->array[index].father==EMPTY_ENTRY) return -1;
	
	//scan the array until reach the root
	while(t->array[index].father!=ROOT) {
		
		vector[pos]=t->array[index].symbol;
		pos--;
		index=t->array[tmp_index].father;
		tmp_index=index;
	}
	
	*size=pos;
	return 0;
}

//delete the table
void tab_suppression(table* t) {
	free(t->array);
	t->size=0;
	t->next_pos=0;
}

