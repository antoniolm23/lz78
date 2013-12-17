#include "comp.h"
#include "sys/stat.h"

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
    bool first_cycle = true;
    
    //compute how many bit to write in the compressed file 
    while(size_tmp>>=1)
        blen++;
    
    //fprintf(stderr, "blen: %i\n", blen);
    
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
		//fprintf(stderr, "%i ",tmp );        
		
		itmp=father;	
		tmp_longest_match++;
		//search if we already have the actual string in the dictionary
		if(first_cycle!=true)
        position=comp_dict_search(&father, tmp, dict);
		//fprintf(stderr, "new_father: %i %i\n", father, position);
		
		if(position!=0) {
			if(longest_match < tmp_longest_match) 
				longest_match = tmp_longest_match;
			tmp_longest_match=1;
			//add the string in the dictionary
			comp_dict_add_word(position, father, tmp, dict);
			//fprintf(stderr, "%i: %i %i\n", position, father, tmp);
			
			bitio_write(bit, (uint64_t)itmp, blen);
			//fprintf(stderr,  "%i\n", itmp);
			father=tmp;
		}
        //read one byte from the file
		result=fread(&tmp, 1, 1, file_read);
        first_cycle=false;
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
	
	//fprintf(stderr, "\t\tlongest match: %i\n", longest_match);
	
	fwrite(&hdr.longest_match, sizeof(int), 1, file_write);
	fclose(file_write);
	//hash_print(&dict->table);
    //free all other structures
    comp_dict_suppress(dict);
    free(dict);
	fclose(file_read);
    
    
    fprintf(stderr, "compression executed\n");
}

void decompress(char* from, char* to){
	
	unsigned int index_bits, aux, child_root=0;
    bool first_read = true;
	int res_retrieve=0, tmp_length;
    uint64_t read_index, prev_current=0;
    struct bitio* comp_file;
    dec_dictionary* dict;
    int* vector;
	int max_length, actual_length, i;
    header hdr;
	FILE* decomp;
    
    //eofc_pos = dict->symbols;
    comp_file = bit_open(from, "r");       //Preparing my data structures
	
    //retrieve header
    bitio_read(comp_file, (uint64_t*)&hdr.dictionary_size, sizeof(int)*8);
	bitio_read(comp_file, (uint64_t*)&hdr.longest_match, sizeof(int)*8);
	
	//fprintf(stderr, "aaaaa %i %i\n", hdr.dictionary_size, hdr.longest_match);
	
	//management of the vector in which put the symbols
	max_length=hdr.longest_match*sizeof(int);
	//fprintf(stderr, "ml: %i\n", max_length);
	vector=malloc(max_length+1);
	vector[max_length]='\0';
    
    dict = malloc(sizeof(dec_dictionary));
    decomp_dict_init(dict, hdr.dictionary_size, 256);
   
    aux = hdr.dictionary_size;         //Compute the number of bits representing the indexes
    //fprintf(stderr, "%i\n", aux);
    index_bits = 1;
    while(aux >>= 1)
        index_bits++;
    
    if(to==NULL) 
		decomp=stdout;
	else
		decomp=fopen(to, "w");
	
    bitio_read(comp_file, &read_index, index_bits); //Read the first index
	
    while(read_index != EOFC) {   //Until the end of compressed file
		
		//fprintf(stderr, "%i\n", (int)read_index);
		actual_length=0;
		//retrieve word from index
        res_retrieve=decomp_dict_reb_word(read_index, vector, &actual_length, dict);  
		
        //critical situation
        if(res_retrieve==-1) {
			//child_root=vector[0];
        	//fprintf(stderr, "$ret$: ");
			decomp_dict_insertion(prev_current, child_root, dict);
			actual_length=0;
			decomp_dict_reb_word(read_index, vector, &actual_length, dict);
        	first_read=true;
        }
        tmp_length=actual_length;
		//actual_length--;
        child_root=vector[actual_length];
		
        //size=max_length-actual_length;
		for(i=tmp_length; i>=0; i--) {
			//fprintf(stderr, "%c", (char)vector[i]);
			fwrite((char*)&vector[i], 1, 1, decomp);
		}
		//fprintf(stderr, "\n");
        //child_root=vector[(max_length--)];
		
        if(first_read==false) 
			decomp_dict_insertion(prev_current, child_root, dict);
		
        prev_current=read_index;
        first_read = false;
        bitio_read(comp_file, &read_index, index_bits);                 //Read the index
    }
    
    fprintf(stderr, "decompression executed\n");
    
    print_tab(&dict->tab);
	free(vector);
    //Closure of all data structures
    bitio_close(comp_file);                                 
    fclose(decomp);
    //print_dict(dict);
    decomp_dict_suppress(dict);
    
    
}


//Compute and print verbose mode information
void verbose_mode(bool comp, struct timeval s_time, struct timeval f_time,
                                                char* i_name, char* o_name) {
    double comp_ratio;
    struct stat input_stat, output_stat;
    struct timeval elapsed_time;
    
    elapsed_time.tv_sec = f_time.tv_sec - s_time.tv_sec;
    elapsed_time.tv_usec = f_time.tv_usec - s_time.tv_usec;
    stat(i_name, &input_stat);  //Files information
    stat(o_name, &output_stat);
    comp_ratio = (((double)output_stat.st_size / input_stat.st_size) * 100);
    if (comp == true)
        fprintf(stderr, "Compression of %s: \n", i_name);
    else
        fprintf(stderr, "Decompression of %s: \n", i_name);
    fprintf(stderr, "User ID: %i\n", input_stat.st_uid);
    fprintf(stderr, "Size: %llu bytes\n", input_stat.st_size);
    if (comp == true)
        fprintf(stderr, "Compression ratio: %g%%\n", comp_ratio);
    else
        fprintf(stderr, "Decompression ratio: %g%%\n", comp_ratio);
    fprintf(stderr, "Time: %lu.%hu s\n", elapsed_time.tv_sec, (short)elapsed_time.tv_usec);
}
