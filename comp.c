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
 * filename is the path of the file to compress, size is the size of the dictionary
 * compressed name is the compressed file name, it can be chose by the user, 
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
    int result=0, found=0;
	FILE* file_read;
    
    //compute how many bit to write in the compressed file 
    while(size_tmp>>=1) blen++;
    
    fprintf(stderr, "blen: %i\n", blen);
    
    //initialize the dictionary
    dict_init(dict, size, 256);
    
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
    if(from==NULL) file_read=stdin;
	else file_read=fopen(from, "r");
    //fprintf(stderr, "%c\n", (char)tmp);
	
	//first read
	result=fread(&tmp, 1, 1, file_read);
	
    father=tmp;
	
	//read the file until reach the EOF or an error occurs
    do {
			
		//read one byte from the file
		result=fread(&tmp, 1, 1, file_read);
		//fprintf(stderr, "%i",tmp );        
		
		itmp=father;	
		
		//search if we already have the actual string in the dictionary
		found=dict_search(&father, tmp, dict);
		//fprintf(stderr, "flag_ret: %i\n", flag);
		
		if(found!=0) {
			//add the string in the dictionary
			dict_add_word(position, father, tmp, dict);
			//fprintf(stderr, "%i: %i %i\t", position, father, tmp);
			
			bitio_write(bit, (uint64_t)itmp, blen);
			//fprintf(stdout,  "%i\n", itmp);
			father=tmp;
		}
	} while(result!=0 || feof(file_read));
    
    //write the last position reached
    bitio_write(bit, father, blen);
    
    //close all the structures writing EOFC on the compressed file
    bitio_write(bit, EOFC, blen);
    
    //free all other structures
    suppress_dictionary(dict);
    free(dict);
	fclose(file_read);
    bitio_close(bit);
    
    fprintf(stderr, "compression executed\n");
}


