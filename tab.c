/*
 * tab.c
 * Antonio La Marra - Giacomo Rotili
 * December 2013
 *
 * This file contains the definitions of the functions for use ht_table and table
 * structure. (See tab.h)
 */

#include "tab.h"

/*      *       *       *       *       *       *       *       *       *
 *                      COMPRESSOR FUNCTIONS
 *      *       *       *       *       *       *       *       *       */


/* LOOKUP
 *
 * PURPOSE: "Effective" hash function
 */
uint32_t lookup(const void *key, size_t length, uint32_t initval) {
  
  	uint32_t  a, b, c;
  	const uint8_t  *k;
  	const uint32_t *data32Bit;

  	data32Bit = key;
  	a = b = c = 0xdeadbeef + (((uint32_t)length) << 2) + initval;

  	while (length > 12) {
    	a +=  *(data32Bit++);
    	b +=  *(data32Bit++);
    	c +=  *(data32Bit++);
    	mix(a, b, c);
    	length -=  12;
  	}

  	k = (const uint8_t *)data32Bit;
    
  	switch (length) {
    	case 12:
            c +=  ((uint32_t)k[11]) << 24;
    	case 11:
            c +=  ((uint32_t)k[10]) << 16;
    	case 10:
            c +=  ((uint32_t)k[9]) << 8;
    	case 9 :
            c +=  k[8];
    	case 8 :
            b +=  ((uint32_t)k[7]) << 24;
    	case 7 :
            b +=  ((uint32_t)k[6]) << 16;
    	case 6 :
            b +=  ((uint32_t)k[5]) << 8;
    	case 5 :
            b +=  k[4];
    	case 4 :
            a +=  ((uint32_t)k[3]) << 24;
    	case 3 :
            a +=  ((uint32_t)k[2]) << 16;
    	case 2 :
            a +=  ((uint32_t)k[1]) << 8;
    	case 1 :
            a +=  k[0];
        	break;
    	case 0 : return c;
  	}
  	final(a, b, c);
  	return c;
}


/* LIST_INS
 *
 * PURPOSE: Insert an element at the top of collision list
 */
int list_ins(int pos, int father, collision_elem** list) {
    
    collision_elem* p, *q, *t;
    
    /* create a new element */
    p = (collision_elem*)malloc(sizeof(collision_elem));
	if(p == NULL) exit(-1);
	
    p->elem_pos = pos;
    p->elem_father = father;
    p->next = NULL;
    
    /* insert in the top of the list */
    q = t = *list;
    *list = p;
    p->next = q;
}

/* LIST_DEL
 *
 * PURPOSE: Destroy a collision list
 */
void list_del(collision_elem* list) {
	
	if(list == NULL)    /* empty list */
        return;
	
    collision_elem* p, *q;
    p = list;
    q = p;
    
    while(q->next != NULL) {
		p = q;
		q = q->next;
		free(p);
    }
    free(q);
    list = NULL;
}

/* LIST_SEARCH
 *
 * PURPOSE: Search an element in the collision list
 */
int list_search(int index, int father, unsigned int symbol, ht_table* table, int* flag) {
    
    int pos;
	collision_elem* e = table->ht_array[index].next;    /* Retrieve list */
	
	if(e == NULL) { /* empty list */
		*flag = NOT_FOUND;
		return 0;
	}
	
	while(e != NULL) {  /* Scan the whole list */
        /* if the father is equal, also checks in the table if the child is equal */
		if(e->elem_father == father) {
			pos = e->elem_pos;
			if(table->ht_array[pos].ht_symbol == symbol) {  /* element found */
				*flag = FOUND;
				return pos;
			}	
		}
		e = e->next;
	}
	*flag = NOT_FOUND;  /* element not found */
	return 0;
}

/* HASH_INIT
 *
 * PURPOSE: Initialize the hash table structure
 */
void hash_init(int size, int symbols, ht_table* table) {
	
	int i, y;	/* for(;;) iterator */

	table->ht_array = malloc((sizeof(ht_entry)) * size);    /* table allocation */
	if(table->ht_array == NULL) exit(-1);
    /*Initialize the first entry with the symbols of the sequence to compress */
	for(i = 0; i < symbols; i++) {
		y = i;
		table->ht_array[y].ht_father = ROOT;
		table->ht_array[y].ht_symbol = i;
		table->ht_array[y].label = i;
		table->ht_array[y].next = NULL;
    }
    
    /* Insert EOFC symbol */
    table->ht_array[symbols].ht_father = ROOT;
    table->ht_array[symbols].ht_symbol = EOFC;
	table->ht_array[symbols].label = EOFC;
    table->ht_array[symbols].next = NULL;
    
    /* Remaining part of the table */
    symbols++;
    for(i = symbols; i < size; i++) {
		table->ht_array[i].ht_symbol = EMPTY_ENTRY;
		table->ht_array[i].ht_father = EMPTY_ENTRY;
		table->ht_array[i].label = EMPTY_ENTRY;
		table->ht_array[i].next = NULL;
    }
    
    /* Setting other parameter of the structure */
    table->next_free_entry = symbols;
    table->total_size = size;
    table->next_label = symbols;
}


/* HASH
 *
 * PURPOSE: Compute the hash values
 */
int hash(int father, int size) {
  
    unsigned int position, initval = 12345; /* Initialization parameter */
    
    position = lookup(&father, sizeof(int), initval);   /* hash computation */
    
    return (position % size);   /* Delimit the result to the size of the table*/
}


/* HASH_SEARCH
 *
 * PURPOSE: Search an element(node) in the hash table(tree)
 */
int hash_search(int* father, unsigned int child, ht_table* table) {
	
	int index = 0, pos = 0; /* indexes in the hash table */
	int tmp = *father;	/* to store the input of the hash function */
	int new_flag;	/* to store list_search() flag result */
	
	tmp = tmp << 8; /* Merge the two input for the one of the hash function */
	tmp += child;
	
	index = hash(tmp, table->total_size); /* Compute hash values */
	
	if(table->ht_array[index].ht_father == EMPTY_ENTRY &&   /* Entry empty? */
       table->ht_array[index].ht_symbol == EMPTY_ENTRY) {
		return index;
	}
	
	if(table->ht_array[index].ht_father == (*father) && /* Found the element? */
       table->ht_array[index].ht_symbol == child) {
		/* Update father with the pointer of the found node, this allow to continue
           the research in the case of match (at compressor algorithm level) */
        *father = table->ht_array[index].label;
		return -1;
	}
	
	/* search in the collision list */
	pos = list_search(index, *father, child, table, &new_flag);
	if(new_flag == FOUND) { /* Found the element? */
		*father = table->ht_array[pos].label; /* See above */
		return -1;
	}
	
	/* Collision and element not present in the table */
	return index;
}


/* HASH_ADD
 *
 * PURPOSE: Insert an element(child node) to the table(tree)
 */
int hash_add(int index, int father, unsigned int symbol, ht_table* table) {
	
	int i = 0, pos = 0;	/* iterator */
	
	if(table->ht_array[index].ht_father !=  EMPTY_ENTRY) {  /* Collision? */
        /* Update the collision list */
		list_ins(table->next_free_entry, father, &table->ht_array[index].next);
        pos = table->next_free_entry;   /* position of the first free entry */
        
        /* Search and update position of the first free entry */
        table->next_free_entry++;
		index = table->next_free_entry;
		for(i = table->next_free_entry; i < table->total_size; i++) {
			if(table->ht_array[i].ht_father  ==  EMPTY_ENTRY &&
				table->ht_array[i].ht_symbol  ==  EMPTY_ENTRY){
				table->next_free_entry = i;
				break;
				}
		}
	}
	
    if(pos == 0)    /* No collision? */
        pos = index;    /* "normal" insertion */
	
	/* Insertion */
	table->ht_array[pos].ht_father = father;
	table->ht_array[pos].ht_symbol = symbol;
	table->ht_array[pos].label = table->next_label;
	table->ht_array[pos].next = NULL;
	
	table->next_label++;    /* Update the label for the next insertion */
	if(table->next_label == table->total_size)  /* Hash table full? */
		return -1;
	
	return 1;   /* insert successfully */
}


/* HASH_SUPPRESS
 *
 * PURPOSE: Delete the hash table
 */
void hash_suppress(ht_table* table) {
    
	int i = 0;
	
	for(i = 0; i < table->total_size; i++)  /* Deallocated collision lists */
		list_del(table->ht_array[i].next);

	free(table->ht_array);  /* Deallocated the table */
	table->next_label = 0;  /* reset parameters of the table */
	table->total_size = 0;
}


/* HASH_PRINT
 *
 * PURPOSE: Print to video the contents of the hash table
 */
void hash_print(ht_table* table) {
	int i;  /* iterator */
	for(i = 0; i < table->total_size; i++)
		fprintf(stderr, "%i: %i %c %i\n", i, table->ht_array[i].ht_father,
			table->ht_array[i].ht_symbol, table->ht_array[i].label);
}


/*******************************************************************************
 * DECOMPRESSOR FUNCTIONS
 * ****************************************************************************/

/* TAB_INIT
 *
 * PURPOSE: Initialize the table structure
 */
int tab_init(table* t, int size, int symbols) {
	
	int i;
    
	t->array = malloc(size*sizeof(d_entry));    /* table allocation */
	if(t->array == NULL) return -1;
	
	/*Initialize the first entry with the symbols of the compressed sequence */
	for(i = 0; i < symbols; i++) {
		t->array[i].father = ROOT;
		t->array[i].symbol = i;
	}
	
	/* Insert EOFC symbol */
	t->array[symbols].father = ROOT;
	t->array[symbols].symbol = EOFC;
	t->size = size;
	t->next_pos = symbols + 1;
	
	/* Remaining part of the table */
	for(i = symbols+1; i<size; i++) {
		t->array[i].father = EMPTY_ENTRY;
		t->array[i].symbol = EMPTY_ENTRY;
	}
	
	return 1;
}


/* TAB_INSERTION
 *
 * PURPOSE: Insert an element(child node) to the table(tree)
 */
int tab_insertion(int father, unsigned int symbol, table* t) {
	
	int pos = t->next_pos;
	
	/* Insertion */
	t->array[pos].father = father;
	t->array[pos].symbol = symbol;
	t->next_pos++;
    
	if(t->next_pos == t->size)  /* Full table? */
		return -1;
	else
        return pos;
}


/* TAB_RETRIEVE_WORD
 *
 * PURPOSE: Retrieves the symbols of the nodes between the node index and the
 *          root, i.e. a word in the dictionary.
 */
int tab_retrieve_word(int index, int* vector, int* size, table* t) {
	
	int pos = 0;
	int tmp_index = index;
	
	if(t->array[index].father == EMPTY_ENTRY)   /* Empty entry? */
		return -1;
	
	/* Scan the table(tree) until reach the root */
	while(t->array[index].father != ROOT) {
		vector[pos] = (char)t->array[index].symbol; /* Store pos-th symbol */
		pos++;
		index = t->array[tmp_index].father; /* Retrieve father node */
		tmp_index = index;
	}
    
	vector[pos] = (char)t->array[index].symbol; /*Store root symbol */
	*size = pos;
	return 0;
}


/* TAB_SUPPRESSION
 *
 * PURPOSE: Delete the table
 */
void tab_suppression(table* t) {
    
	free(t->array); /* Deallocated the table */
	t->size = 0;    /* reset parameters of the table */
	t->next_pos = 0;
}


/* PRINT_TAB
 *
 * PURPOSE: Print to video the contents of the table
 */
void print_tab(table* t) {
    
	int i; /* iterator */
    
	for(i = 0; i < t->size; i++) {
        fprintf(stderr, "%i: %i %i\n", i, t->array[i].father, t->array[i].symbol);
	}
}
