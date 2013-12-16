#include "comp.h"
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
	int longest_match=1, tmp_longest_match=0;
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
    //fprintf(stderr, "%c\n", (char)tmp);
	
	//first read
	result=fread(&tmp, 1, 1, file_read);
	
    father=tmp;
	
	//read the file until reach the EOF or an error occurs
    do {
			
		//read one byte from the file
		result=fread(&tmp, 1, 1, file_read);
		//fprintf(stderr, "%i ",tmp );        
		
		itmp=father;	
		tmp_longest_match++;
		//search if we already have the actual string in the dictionary
		position=comp_dict_search(&father, tmp, dict);
		//fprintf(stderr, "new_father: %i %i\n", father, position);
		
		if(position!=0) {
			if(longest_match < tmp_longest_match) 
				longest_match = tmp_longest_match;
			tmp_longest_match=0;
			//add the string in the dictionary
			comp_dict_add_word(position, father, tmp, dict);
			//fprintf(stderr, "%i: %i %i\n", position, father, tmp);
			
			bitio_write(bit, (uint64_t)itmp, blen);
			//fprintf(stderr,  "\n");
			father=tmp;
		}
	} while(result!=0 || !feof(file_read));
    //write the last position reached
    bitio_write(bit, father, blen);
    
    //close all the structures writing EOFC on the compressed file
    bitio_write(bit, EOFC, blen);
    bitio_close(bit);
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
	int max_length, actual_length;
    header hdr;
	FILE* decomp;
    
    //eofc_pos = dict->symbols;
    comp_file = bit_open(from, "r");       //Preparing my data structures
    
    //retrieve header
    bitio_read(comp_file, (uint64_t*)&hdr.dictionary_size, sizeof(int)*8);
	bitio_read(comp_file, (uint64_t*)&hdr.longest_match, sizeof(int)*8);
	
	//management of the vector in which put the symbols
	max_length=hdr.longest_match;
	vector=malloc(max_length+1);
	vector[max_length]='\0';
    
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
		
		//fprintf(stdout, "%i\n", (int)read_index);
		actual_length=max_length;
		//retrieve word from index
        res_retrieve=decomp_dict_reb_word(read_index, vector, &actual_length, dict);  
		
        //critical situation
        if(res_retrieve==-1) {
        	
			decomp_dict_insertion((int)prev_current, child_root, dict);
			decomp_dict_reb_word(read_index, vector, &actual_length, dict);
        	first_read=true;
        }
        
        child_root=vector[actual_length--];
        
        size=max_length-actual_length;
        fwrite(&vector[actual_length], size, 1, decomp);
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


