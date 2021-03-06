#include "debug.h"

typedef struct bitio{
  	
  	int fd;	//checks if the opening of the file was successful
  	bool writing;	//file opened in writing mode?
  	int next;
  	int end;
  	int size;	//size of the buffer in bits
  	uint64_t buf[MAX_SIZE];
  	
}bitio;

bitio* bit_open(char* name, char* mode) {
  
	int i;
  
	//fprintf(stderr, "bitopen\n\n");
  
	bitio* f=calloc(1, sizeof(bitio));//allocate the bitio struct
	//fprintf(stderr, "bitopen1\n\n");
  	if(f==NULL) {
    	fprintf(stderr, "error in allocating\n");
    	errno=ENOMEM;	//no memory available
    	return NULL;
  	}
  	
  	//deal with exceptions
  	if(name==NULL) {
	  fprintf(stderr, "namenull\n");
	  goto error;	
	}
  	if(mode==NULL) {
	  fprintf(stderr, "modenull\n");
	  goto error;	//way to deal with exceptions
	}
  
  	//check if the mode chosen is right
  	if(strcmp(mode, "r")!=0&&strcmp(mode,"w")!=0) 
		goto error;
  
  	//open the file in read or write and put the result in fd
  	//f->fd=open(name, (mode[0]=='r')?O_RDONLY:(O_WRONLY));//|O_CREAT|O_TRUNC));
	if(mode[0]=='r')
	    f->fd=open(name, O_RDWR);
	else 
		f->fd=open(name, O_CREAT|O_RDWR, S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH);
  	
	if(f->fd<0) { 
	  fprintf(stderr, "fd\n");
	  goto error;	//checks if the open succeded
	}
  	
  	//save in f->mode the way in which we choose to open the file
  	f->writing=(mode[0]=='r')?O_RDONLY:O_WRONLY;
  	//fprintf(stderr, "writing? %x\n\n\n", f->writing);
  	//f->size=sizeof(f->buf);	//size in bytes of the buffer
  	f->size=MAX_SIZE*64;		//size in bits
  	
  	//set the structure for reading mode
  	//if(f->writing==O_RDONLY) {
    	//fprintf(stderr, "apro il file in reading\n");
    	f->end=f->size;
    	f->next=0;
  	//}
  
  	//set the structure for writing mode
  	//else {
    //	f->end=f->size;
   // 	f->next=0;
  	//}
  
  	//set the buffer to zero in order to perform the writing operations
  	for(i=0; i<MAX_SIZE; i++) {
    	f->buf[i]=0;
  	}
    
  	return f;

error: 	free(f);
	fprintf(stderr, "error detected\n");
	errno=EINVAL;
	return NULL;

};

//in the write function len is expressed in bits
int bitio_write(bitio* f, uint64_t d, int len) { //len is expressed in bits
  	
  	int space, ofs, n, i, result, original;
  	uint64_t* p;
  	uint64_t tmp;
  	original=(int) d;
  	//check all the parameters
  	if((f==NULL)||(len<1)||(len>8*sizeof(d))) {
    	errno=EINVAL;
    	fprintf(stderr, "Error1\n");
    	return -1;
  	}
  
  	//check if we do the right operation
  	if(!f->writing) {
    	errno=EINVAL;
    	fprintf(stderr, "Error2\n");
    	return -1;
  	}
  
again:  space=f->end-f->next;
  //check if there's enough space on the buffer
  	if(space<0) {
  		errno=EINVAL;
    	fprintf(stderr, "Error3\n");
    	return -1;
  	}
  	
  	//check if the buffer is full 
  	if(space==0) {
   		
   		//write the buffer in the file	
    	result=(int)write(f->fd, f->buf, MAX_SIZE*8);
    	if(result<0) {
			fprintf(stderr, "Error in write\n");
			return -1;
		}
    	
    	//reset the buffer
    	for(i=0; i<MAX_SIZE; i++) {
      		f->buf[i]=0;
    	}
    	
    	//reset the structure
    	f->size=MAX_SIZE*64;
    	f->end=f->size;
    	f->next=0;
    
  	}

  	//go to the right address in the buffer
  	p=f->buf+(f->next/64);
  	//go to the right offset
  	ofs=f->next%64;
  	n=64-ofs;
  
  	if(len<n) n=len;
  	
  	//convert *p from little endian
  	tmp=le64toh(*p);
  
  	//shift the datum of ofs positions
  	tmp|=(d<<ofs);
  
  	if((ofs+n)>64) {
   		tmp&=((1<<(ofs+n))-1);
  	}
  	*p=htole64(tmp);
  	
  	//update the structure
  	f->next+=n;
  	len-=n;
  	d>>=n;
  	
  	//check if there are bits left to put in the buffer
  	if(len>0) 
		goto again;
    
    if(original==EOFC) {
    	
    	//fprintf(stderr, "\n\t\tEOF written\n");
    	//f->buf[(MAX_SIZE-1)]=d;
    	result=(int)write(f->fd, f->buf, (f->next/8)+1);
    	if(result<0) fprintf(stderr, "Error in write\n");
    	return 1; 
    
  	}
    
  	//if the operation succedes return 1
  	return 1;
}

//Read len bits and save on datum
int bitio_read(bitio* f, uint64_t* datum, int len) {
    
    int offset, readable, noffset;
    uint64_t* pointer;
    uint64_t tmp/*, tmp2*/;
    int res;
    
    //check all the parameters
    if((f == NULL) || (len < 1) || (len > sizeof(datum)*8)) {
        errno = EINVAL;
        fprintf(stderr, "Error1 %i\n", len);
        if(f == NULL)
            fprintf(stderr, "NULL\n");
        return -1;
    }
    //check if we do the right operation
    if(f->writing) {
        errno = EINVAL;
        fprintf(stderr, "Error2\n");
        return -1;
    }
	
again: 
	readable=f->end-f->next;
    if(readable<0) {
		errno=EINVAL;
		fprintf(stderr, "r: Error3 %i %i\n", f->next, f->end);
		return -1;
    }
    
    //all bits read, so read another block
    if(readable==f->size||readable==0) {
		res=read(f->fd, f->buf, MAX_SIZE*8);
		if(res<0) {
			fprintf(stderr, "error in read\n");
			return -1;
		}
		
		//reset the structure
		f->next=0;
		f->end=MAX_SIZE*64;
		f->size=MAX_SIZE*64;
	}
	
	//go to the right address
	pointer=f->buf+(f->next/64);
	//go to the right offset
	offset=f->next%64;
	noffset=64-offset;
	
	if(len<noffset) 
		noffset=len;
	
	//retrieve the datum on 64 bits
	tmp=le64toh(*pointer);
	tmp>>=offset;
	tmp &= (((uint64_t)1 << (len)) - 1);
	f->next+=noffset;
	len-=noffset;
	
	//check if I have to continue
	if(len>0) {
		tmp>>=len;
		goto again;
	}
	
	*datum=tmp;
    return 1;
}

//close the bitio structure
void bitio_close(bitio* b) {
	
	close(b->fd);		//close the file
	free(b);		//free the structure		
	
}

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

void hash_print(ht_table* table) {
	int i, size=table->total_size;
	for(i=0; i< size; i++) {
		fprintf(stderr, "%i: %i %i %i\n", i, table->ht_array[i].ht_father,
			table->ht_array[i].ht_symbol, table->ht_array[i].label
		);
	}
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
	for(i; i<size; i++) {
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
int tab_retrieve_word(int index, unsigned char* vector, int* size, table* t) {
	
	int tmp_index;
	int i=0;
	unsigned char c;
	
	if(t->array[index].father==EMPTY_ENTRY) return -1;
	
	//scan the array until reach the root
	while(t->array[index].father!=ROOT) {
		
		tmp_index=index;
		c=vector[i]=(unsigned char)t->array[index].symbol;
		i++;
		index=t->array[tmp_index].father;
		
	}
	i++;
	vector[i]=(unsigned char)t->array[index].symbol;
	*size=i++;
	return 0;
}

//delete the table
void tab_suppression(table* t) {
	free(t->array);
	t->size=0;
	t->next_pos=0;
}

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
	
	int res,i;
	
	//call the add string in the hash
	res=hash_add(index, father, symbol, &d->table);
	
	//if the hash is full suppress it and rebuid it
	if(res==-1) {
		//fprintf(stderr, "full dict, %i %i\n", father, symbol);
		//hash_print(&d->table);
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
int decomp_dict_reb_word(int index, unsigned char* vector, int* size, dec_dictionary* d) {
	
	int tmp=*size;
	int res,i;
	
	res=tab_retrieve_word(index, vector, &tmp, &d->tab);
	i=tmp--;
	for(i; i<*size; i++) {
		fprintf(stderr, "%i ", vector[i]);
	}
	fprintf(stderr, "\n");
	*size=tmp;
	
	return res;
}

//suppress the dictionary
void decomp_dict_suppress(dec_dictionary* d){
	
	tab_suppression(&d->tab);
	free(d);
}
//#include "dict.h"
//#include "bitio.h"

//extract the extension from the filename
bool extract_extension(char* filename, char* ext) {
	int len, i;
	len=(int)strlen(filename); //length of the filename
	for(i=len-1; i>=0; i--) {
		if(filename[i]=='.') {
			strcpy(ext, &filename[i+1]);
			return true;
		}
	}
	return false;
}

/* compressor algorithm for lz78:
 * from is the path of the file to compress, size is the size of the dictionary
 * to is the compressed file name, it can be chose by the user, 
 * 		otherwise a standard name is put in its place*/
void compress(char* from, char* to, int size){
	
	//fprintf(stderr, "compressor\n");
    //int, i, flen;	//length of filename
    //int i;
    unsigned int tmp=0; //temporary variable in which we put the byte read    
    unsigned int itmp=0, size_tmp=0; //itmp is the index to write;
    int position=0, father=0; //postion in the dictionary
    //int fd=open(filename,  'r'); //file descriptor
    dictionary* dict=malloc(sizeof(dictionary)); //dictionary
    int blen=1;     //bit lenght of my dictionary's indexes
    size_tmp=size;
    struct bitio* bit;
    header hdr;		//header of the file
    int result=0;
	int longest_match=1, tmp_longest_match=1;
	FILE* file_read, *file_write;
    
    //compute how many bit to write in the compressed file 
    while(size_tmp>>=1) blen++;
    
    fprintf(stderr, "blen: %i\n", blen);
    
    //initialize the dictionary
    comp_dict_init(dict, size, 256);
    
    father=0;
    
    //initialize the bitio structure to write bits instead of bytes
    bit=bit_open(to, "w");
    if(bit==NULL) fprintf(stderr, "null bitio\n");
    
    
    //creation of the header of the file
    hdr.dictionary_size=size;
	hdr.longest_match=0;	//prepare the space for the overwriting
	
    //write the header in the top of the file
    //flen=strlen(hdr.extension);
    bitio_write(bit, (uint64_t)hdr.dictionary_size, sizeof(int)*8);
	bitio_write(bit, (uint64_t)hdr.longest_match, sizeof(int)*8);
    //for(i=0; i<6; i++)
		//bitio_write(bit, (uint64_t)hdr.extension[i], sizeof(char)*8);
    
    //read a byte from the file
    //fprintf(stderr, "%i\n", fd);
    if(from==NULL) 
		file_read=stdin;
	else 
		file_read=fopen(from, "r");
    if(file_read==NULL) fprintf(stderr, "ERROR\n");
	
	//first read
	result=fread((char*)&tmp, 1, 1, file_read);
	//fprintf(stderr, "%i %i \n",tmp, result);
	
    father=tmp;
	
	//read the file until reach the EOF or an error occurs
    do {
			
		//read one byte from the file
		result=fread((char*)&tmp, 1, 1, file_read);
		//fprintf(stderr, "%i %i \n",tmp, result);        
		
		itmp=father;	
		tmp_longest_match++;
		//search if we already have the actual string in the dictionary
		position=comp_dict_search(&father, tmp, dict);
		//fprintf(stderr, "new_father: %i %i\n", father, position);
		
		if(position!=0) {
			if(longest_match < tmp_longest_match) 
				longest_match = tmp_longest_match;
			tmp_longest_match=1;
			//add the string in the dictionary
			comp_dict_add_word(position, itmp, tmp, dict);
			//fprintf(stderr, "%i: %i %i\n", position, father, tmp);
			
			bitio_write(bit, (uint64_t)itmp, blen);
			fprintf(stderr,  "%i \n", itmp);
			father=tmp;
		}
	} while(result!=0 || !feof(file_read));
    //write the last position reached
    bitio_write(bit, father, blen);
    
    //close all the structures writing EOFC on the compressed file
    bitio_write(bit, EOFC, blen);
    bitio_close(bit);
	//hash_print(&dict->table);
	//fprintf(stderr, "%i\n", longest_match);
	
	//write the longest match in the file
	file_write=fopen(to, "r+b");
	fseek(file_write, sizeof(int), SEEK_SET); 
	hdr.longest_match=longest_match;
	fwrite(&hdr.longest_match, sizeof(int), 1, file_write);
	fclose(file_write);
	
    //free all other structures
    comp_dict_suppress(dict);
    free(dict);
	fclose(file_read);
    
    
    fprintf(stderr, "compression executed\n");
}

void decompress(char* from, char* to){
	
	int index_bits, aux, child_root=0;
    bool first_read = true;
	int res_retrieve=0, size;
    uint64_t read_index, prev_current=0;
    struct bitio* comp_file;
    dec_dictionary* dict;
    unsigned char* vector;
	unsigned char* tmp_vec;
	int max_length, actual_length;
    header hdr;
	FILE* decomp;
    int i;
    //eofc_pos = dict->symbols;
    comp_file = bit_open(from, "r");       //Preparing my data structures
    
    //retrieve header
    bitio_read(comp_file, (uint64_t*)&hdr.dictionary_size, sizeof(int)*8);
	bitio_read(comp_file, (uint64_t*)&hdr.longest_match, sizeof(int)*8);
	
	//management of the vector in which put the symbols
	max_length=hdr.longest_match;
	vector=malloc(max_length+1);
	vector[max_length]='\0';
	tmp_vec=malloc(max_length+1);
	tmp_vec[max_length]='\0';
	
	fprintf(stderr, "end read\n");
    
    dict = malloc(sizeof(dictionary));
    decomp_dict_init(dict, hdr.dictionary_size, 256);
   
    aux = hdr.dictionary_size;         //Compute the number of bits representing the indexes
    fprintf(stderr, "%i\n", aux);
    index_bits = 1;
    while(aux >>= 1)
        index_bits++;
    
    if(to==NULL) 
		decomp=stdout;
	else
		decomp=fopen(to, "w");
	
    bitio_read(comp_file, &read_index, index_bits); //Read the first index
	
    while(read_index != EOFC) {   //Until the end of compressed file
		
		fprintf(stderr, "%i: ", (int)read_index);
		actual_length=max_length;
		//retrieve word from index
        res_retrieve=decomp_dict_reb_word(read_index, vector, &actual_length, dict);  
		
        //critical situation
        if(res_retrieve==-1) {
        	
			decomp_dict_insertion((int)prev_current, child_root, dict);
			decomp_dict_reb_word(read_index, vector, &actual_length, dict);
        	first_read=true;
        }
        
        child_root=vector[0];
        for(i=0; i<=actual_length; i++)
			tmp_vec[actual_length-i]=vector[i];
        size=actual_length;
        fwrite(&tmp_vec[0], size, 1, decomp);
		for(i=0; i<max_length; i++)
			tmp_vec[i]=vector[i]=0;
		//fprintf(stderr, "%s\n", &vector[actual_length]);
        //child_root=vector[(max_length--)];
		
        if(first_read==false) {
			decomp_dict_insertion(read_index, prev_current, dict);
        }
        prev_current=read_index;
        first_read = false;
        bitio_read(comp_file, &read_index, index_bits);                 //Read the index
    }
    
    //Closure of all data structures
    bitio_close(comp_file);                                 
    fclose(decomp);
    //print_dict(dict);
    decomp_dict_suppress(dict);
    
    fprintf(stderr, "decompression executed\n");
}


/***********************************************
 *Copyright La Marra Antonio Rotili Giacomo
 * (c) 2013
 **********************************************/

int main(int argc, char* argv[]) {
    //array to keep int account the options passed:
    /*0: c 1: v 2: i 3: o 4: d 5: s */
    bool option[7];
    char* input_filename=NULL;
    char* output_filename=NULL;
    int size=0;
    //int input_fd;
	//int output_fd;
	/* RAW Performance evaluation */
	//clock_t begin, end;
	//double time_spent;
    //used to address the parameters passed to the call
    //int optindex[6]={0, 0, 0, 0, 0, 0};
    //inizialization of option array
    int i;
    //char*  namefile;
    for(i=0; i<7; i++) option[i]=false;
    //checking of number of arguments passed
    if(argc<2)  {	//this means that the program was called without arguments
        fprintf(stderr, "Missing arguments\n");//print message on stderror
        exit(-1);//exit from the program
    }
    //checking the type of arguments passed
    int actopt;//takes into account the character corresponding to actual option
    while((actopt=getopt(argc, argv, "cdvi:o:s:k:"))!=-1) {
        switch(actopt) {
            //compression command
			case 'c':
                option[0]=true;
                break;
			
			//verbose output
            case 'd':
                option[1]=true;
                //optindex[1]=optind;
                break;
			
			//input filename
            case 'i':
                option[2]=true;
                input_filename=optarg;
                if(optarg==NULL) {
                    fprintf(stderr, "error, no filename passed\n");
                    return -1;
                }
                fprintf(stderr, "%s\n", input_filename);
                break;
				
			//output filename
            case 'o':
                option[3]=true;
                output_filename=optarg;
                if(optarg==NULL) {
                    fprintf(stderr, "error, no filename passed\n");
                    return -1;
                }
                fprintf(stderr, "%s\n", output_filename);
                //optindex[3]=optind;
                break;
				
			//decompression command
            case 'v':
                option[4]=true;
                //optindex[4]=optind;
                break;
			
			//dictionary size
            case 's':
                option[5]=true;
                size=atoi(optarg);
                //optindex[5]=optind;
                break;
			
			//input string
			case 'k':
				option[6]=true;
				
				//here we have to write the passed string to the stdin manually?
				
				break;
            default:
                break;
        }
    }
	
	//managing options
    if(size<900) size=1000;
	
	//compression
    if(option[0]==true) {
		if(option[6]==true) {
			input_filename=NULL;
		}
		else {
			if(input_filename==NULL) input_filename="file.txt";
			//input_fd=open(input_filename, 'r');
		}
		if(output_filename==NULL) output_filename="compressed.lz78";
		//begin = clock();
		compress(input_filename, output_filename, size);
		//end = clock();
		//time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		
		
		//if verbose print the time spent
		//if(option[4]==true) fprintf(stderr, "time spent: %f\n", time_spent);
		
		//if verbose print the compression ratio
		//if(option[4]==true) {
		//off_t i_fsize, o_fsize;
		//input_fd=open(input_filename, 'r');
		//i_fsize = lseek(input_fd, 0, SEEK_END);
		//output_fd=open(output_filename, 'r');
		//o_fsize = lseek(output_fd, 0, SEEK_END);
		//fprintf(stderr, "compression ratio: %f\n", (double)o_fsize/i_fsize);
		//}
	}
	
	//decompression
	if(option[1]==true) {
		if(input_filename==NULL) input_filename="compressed.lz78";
		if(output_filename==NULL) output_filename="decomp.txt";
		//better managing file extensions automatically or having the possibility of printing the 
		//out of the decompressor to the standard output?
		//begin = clock();
		decompress(input_filename, output_filename);
		//end = clock();
		//time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		//if(option[4]==true) fprintf(stderr, "time spent: %f\n", time_spent);
	}
	
    return 0;
}
