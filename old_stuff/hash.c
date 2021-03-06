#include "tab.h"

/*******************************************************************************
 * COMPRESSOR FUNCTIONS
 * ****************************************************************************/

//effective hash function
uint32_t lookup(const void *key, size_t length, uint32_t initval) {
  
  //fprintf(stderr, "lookup function\n");
  
  	uint32_t  a,b,c;
  	const uint8_t  *k;
  	const uint32_t *data32Bit;

  	data32Bit = key;
  	a = b = c = 0xdeadbeef + (((uint32_t)length)<<2) + initval;

  	while (length > 12) {
    	a += *(data32Bit++);
    	b += *(data32Bit++);
    	c += *(data32Bit++);
    	mix(a,b,c);
    	length -= 12;
  	}

  	k = (const uint8_t *)data32Bit;
  	switch (length) {
    	case 12: c += ((uint32_t)k[11])<<24;
    	case 11: c += ((uint32_t)k[10])<<16;
    	case 10: c += ((uint32_t)k[9])<<8;
    	case 9 : c += k[8];
    	case 8 : b += ((uint32_t)k[7])<<24;
    	case 7 : b += ((uint32_t)k[6])<<16;
    	case 6 : b += ((uint32_t)k[5])<<8;
    	case 5 : b += k[4];
    	case 4 : a += ((uint32_t)k[3])<<24;
    	case 3 : a += ((uint32_t)k[2])<<16;
    	case 2 : a += ((uint32_t)k[1])<<8;
    	case 1 : a += k[0];
        	break;
    	case 0 : return c;
  	}
  	final(a,b,c);
  	
  	return c;
}


//list passed by reference, the insertion is always on the top of the list
int list_ins(int pos, int father, unsigned int c, collision_elem** list) {
    
    //fprintf(stderr, "list_insertion %i %i\n", pos, father);
    
    collision_elem* p, *q, *t;
    
    //create a new element
    p=(collision_elem*)malloc(sizeof(collision_elem));
    p->elem_pos=pos;
    p->elem_father=father;
    p->next=NULL; 
    
    //insert in the top of the list     
    q=t=*list;
    *list=p;
    p->next=q;
		
    return true;
}

//deleting of the list
void list_del(collision_elem* list) {
  
    //fprintf(stderr, "list deletion\n");
	
	if(list==NULL) return;
	
    collision_elem* p, *q;
    
    p=list;
    q=p;
    
    while(q->next!=NULL) {
	
		p=q;
		q=q->next;
		free(p);	
    }
    list=NULL;
}

//search in the list
int list_search(int index, int father, unsigned int symbol, ht_table* table, int* flag){

	collision_elem* e=table->ht_array[index].next;
	int pos;
	
	//if the list is empty set the flag and return 0
	if(e==NULL) {
		*flag=NOT_FOUND;
		return 0;
	}
	
	//scroll the list to see if there is the element
	while(e!=NULL) {
		
		//if the father is equal access in the position of the dictionary to see if the sons are equal
		if(e->elem_father==father) {
			pos=e->elem_pos;
			if(table->ht_array[pos].ht_symbol==symbol) {
				*flag=FOUND;
				return pos;
			}	
		}
		e=e->next;
	}
	
	*flag=NOT_FOUND;
	return 0;
}

//initialization of the dictioanry
void hash_init(int size, int symbols, ht_table* table) {
	
	int i;	//iterator
	int y;
	
	table->ht_array=malloc((sizeof(ht_entry))*size);
	
	for(i = 0; i < symbols; i++) {
		y=i;
		table->ht_array[y].ht_father = ROOT;
		table->ht_array[y].ht_symbol = i;
		table->ht_array[y].label = i;
		table->ht_array[y].next = NULL;
    }
    
    //Add EOFC to the table
    table->ht_array[symbols].ht_father = ROOT;
    table->ht_array[symbols].ht_symbol = EOFC;
	table->ht_array[symbols].label = EOFC;
    table->ht_array[symbols].next = NULL;
    
    //fprintf(stderr, "end initialize first part\n");
    symbols++;
    //initialize the remaining part of the table 
    for(i = symbols; i < size; i++) {
		table->ht_array[i].ht_symbol = EMPTY_ENTRY;
		table->ht_array[i].ht_father = EMPTY_ENTRY;
		table->ht_array[i].label=EMPTY_ENTRY;
		table->ht_array[i].next= NULL;
    }
    
    //update all the other fields of the ht_table structure
    table->next_free_entry=symbols;
    table->total_size=size;
    table->next_label=symbols;
    //fprintf(stderr, "init: %i, %i, %i\n", table->next_free_entry,table->total_size, table->actual_size);
    	
}


//hash function, returns the position 
int hash(int father, int size) {
    
    //fprintf(stderr, "hash calling\n");
  
    unsigned int position;
    unsigned int initval=12345;
    
    //hash computation using the lookup function
    position=lookup(&father, sizeof(int), initval);
    
    //fprintf(stderr, "end hash calling %i\n", (position%SIZE));
    
    return (position%size);
}

int hash_search(int* father,unsigned int child, ht_table* table) {
	
	int index=0, pos=0; //used as indexes in the hash table
	int tmp=*father;	//temporary variable to be used as input of the hash function
	int new_flag;	//new flag to be used in case we have to search in the collision list
	
	//construct the input of the hash function
	tmp=tmp<<8;
	tmp+=child;
	
	index=hash(tmp, table->total_size);
	//fprintf(stderr, "index: %i\n", index);
	//check if the entry is empty
	if(table->ht_array[index].ht_father==EMPTY_ENTRY && table->ht_array[index].ht_symbol==EMPTY_ENTRY)
		//fprintf(stderr, "emptypos %i\n", index);
		return index;
	
	//check if the entry is occupied by the same symbol
	if(table->ht_array[index].ht_father==(*father) && table->ht_array[index].ht_symbol==child) {
		//fprintf(stderr, "yourpos\n");
		*father=table->ht_array[index].label; //the new father is the pointer to the child node
		return 0;
	}
	
	//search in the collision list
	pos=list_search(index, *father, child, table, &new_flag);
	if(new_flag==FOUND) {
		*father=table->ht_array[pos].label; //the new father is the pointer to the child node
		return 0;
	}
	
	//the entry is occupied by another symbol thus we have a collision, return the index, it's up 
	//to the insertion to manage collisions
	return index;
	
	
}

//add a new child in the hash table
int hash_add(int index, int father, unsigned int symbol, ht_table* table) {
	
	int i;	//simple iterator
	//int index; used to reach a position in the hash table
	//index=hash_search(father, symbol, table, flag);
	//fprintf(stderr, "flag: %i", *flag);
	//if the position is occupied by another string then we have to manage collison
	if(table->ht_array[index].ht_father != EMPTY_ENTRY) {
		list_ins(table->next_free_entry, father, symbol, &table->ht_array[index].next);
		index=table->next_free_entry;
		for(i=table->next_free_entry; i<table->total_size; i++) {
			if(table->ht_array[i].ht_father==EMPTY_ENTRY &&
				table->ht_array[i].ht_symbol==EMPTY_ENTRY){
				//fprintf(stderr, "\n\t\t\tnew free:%i\n\n\n", i);
				table->next_free_entry=i;
				break;
				}
		}
	}
	
	//now we can do the insertion
	table->ht_array[index].ht_father=father;
	table->ht_array[index].ht_symbol=symbol;
	table->ht_array[index].label=table->next_label;
	table->ht_array[index].next=NULL;
	
	//increase the label for the next insertion
	table->next_label++;
	//ratio=(double)(table->actual_size)/(double)(table->total_size);
	//fprintf(stderr, "%f %f\n", ratio, max_r);
	if(table->next_label>table->total_size) {
		//fprintf(stderr, "full_dict\n");
		return -1;
		}
	
	//fprintf(stderr, "hash_add: %i\n", table->ht_array[index].ht_father);
	//all went ok, there is space we can continue using this dictionary
	return 1;	
		
}

//suppress the hash table
void hash_suppress(ht_table* table) {
	int i;
	
	//free the memory allocated to the collision lists
	for(i=0; i<table->total_size; i++) 
		list_del(table->ht_array[i].next);
	
	//free the memory allocated to the table
	free(table->ht_array);
	
	//reset the parameters of the table
	table->next_label=0;
	table->total_size=0;
	
}

/*******************************************************************************
 * DECOMPRESSOR FUNCTIONS
 * ****************************************************************************/

//initialize the decompressor table
void tab_init(table* t, int size, int symbols) {
	
	int i;
	
	t->array=malloc(size*sizeof(entry));
	
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

