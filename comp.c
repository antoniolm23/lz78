/*
 * comp.c
 * Antonio La Marra - Giacomo Rotili
 * December 2013
 *
 * Implementation of the compression and decompression algorithms.
 *
 *
 */


#include "comp.h"
#include "sys/stat.h"


/* COMPRESS
 *
 * PURPOSE: Implement the LZ78 compress algorithm
 */
void compress(char* from, char* to, int size) {
	
    int position = -1; /* position of a word in the dictionary */
    int father = 0; /* last match found */
    int blen = 1;     /* length of dictionary's indexes (in bit) */
    int result = 0; /* variable to store fread() return */
	int longest_match = 1;  /* Length of longest match */
    int tmp_longest_match = 1;
    unsigned int size_tmp = 0;  /* store variable for size */
    unsigned int tmp = 0; /* Symbol read from file to compress (1 byte)*/
    unsigned int itmp = 0; /* index to write */
    bool first_cycle = true;    /* flag to discriminate first symbol read, which
                                   that will always produce a match 
                                   (See HASH_INIT in tab.c) */
    
    FILE* file_read, *file_write;   /* To handle input and output file */
    
    struct bitio* bit;  /* Bitio interface to write indexes on file compressed */
    dictionary* dict; /* pointer to dictionary object */
    header hdr;		/* header of the file */
	
    dict = malloc(sizeof(dictionary)); /* Create the dictionary */
    
    /* Compute lenght of dictionary's indexes */
    size_tmp = size;
    while(size_tmp >>= 1)
        blen++;
    
    /* Initialize the dictionary; reading a byte at time, the alphabet is composed
     by the bit strings in [0, 255] */
    comp_dict_init(dict, size, 256);
    
    /* Create the output file trough bitio interface */
    bit = bit_open(to, "w");
    if(bit == NULL)
        fprintf(stderr, "null bitio\n");
    
    /* creation of the file's header */
    hdr.dictionary_size = size;
	hdr.longest_match = 0;
	
    /* Write the header at the beginning of the file */
    bitio_write(bit, (uint64_t)hdr.dictionary_size, (sizeof(int) * 8));
	bitio_write(bit, (uint64_t)hdr.longest_match, (sizeof(int) * 8));

    if(from == NULL) /* Sequence to compress from keyboard? */
		file_read = stdin;
	else
		file_read = fopen(from, "r");
	
	result = fread(&tmp, 1, 1, file_read);  /* Read the first symbol */
    father = tmp;
	
    do {
		itmp = father;	
		tmp_longest_match++;    /* Update length of the actual "matching" string */
        
		if(first_cycle != true) /* Not first symbol read? */
            /* search the actual string in the dictionary */
			position = comp_dict_search(&father, tmp, dict);

		if(position != -1) {    /* No Match */
			if(longest_match < tmp_longest_match) /* New longest match? */
				longest_match = tmp_longest_match;    /* Update longest length */
			tmp_longest_match = 1;  /* Reset */
            
            /* add the string in the dictionary */
			comp_dict_add_word(position, father, tmp, dict);
			
            /* Write on compressed file, label of the last match */
			bitio_write(bit, (uint64_t)itmp, blen);
            
			father = tmp;   /* Next match search will start from this symbol */
		}
		result = fread(&tmp, 1, 1, file_read);  /* read next symbol */
        first_cycle = false;    /* First iteration end */
        
	} while(result != 0 || !feof(file_read));   /* Until reach the EOF or an 
                                                   error occurs */
    
    bitio_write(bit, father, blen); /* Write the last position reached */
    bitio_write(bit, EOFC, blen);   /* Write EOF on the compressed file */
    bitio_close(bit);   /* Close compressed file */
	
    /* Write the longest match in compressed file's header */
	file_write = fopen(to, "r+b");  /* Open compressed file */
	fseek(file_write, sizeof(int), SEEK_SET); /* Beginning of file*/
	hdr.longest_match = longest_match;
	fwrite(&hdr.longest_match, sizeof(int), 1, file_write);
	fclose(file_write); /* Close compressed file */

    /* Deallocate all data structures used */
    comp_dict_suppress(dict);
    free(dict);
	fclose(file_read);
    
    fprintf(stderr, "Compression executed\n");
}


/* DECOMPRESS
 *
 * PURPOSE: Implement the LZ78 decompress algorithm
 */
void decompress(char* from, char* to) {
	
    int max_length; /* Length of the longest word in dictionary */
    int actual_length = 0;  /* Length of the just retrieved word */
    int tmp_length;
    int i;  /* iterator */
    int res_retrieve = 0;   /* result of retrieve operation */
    int* vector;    /* To store the retrieved words from the dictionary */
	unsigned int index_bits = 1;    /* length of dictionary's indexes (in bit) */
    unsigned int aux;
    unsigned int child_root = 0;
    bool first_read = true;
	struct bitio* comp_file;    /* Bitio interface to read indexes on file compressed */
    uint64_t read_index, prev_current = 0;
    dec_dictionary* dict;   /* dictionary*/
    header hdr; /* To store file's header */
	FILE* decomp;   /* To handle output file */
    
    /* Open compressed file trough bitio interface */
    comp_file = bit_open(from, "r");
	
    /* retrieve header's information */
    bitio_read(comp_file, (uint64_t*)&hdr.dictionary_size, sizeof(int)*8);
	bitio_read(comp_file, (uint64_t*)&hdr.longest_match, sizeof(int)*8);
	
	/* construction of vector  */
	max_length = (hdr.longest_match * sizeof(int));
	vector = malloc(max_length + 1);
	vector[max_length] = '\0';
    
    /* Create and initialize the dictionary */
    dict = malloc(sizeof(dec_dictionary));
    decomp_dict_init(dict, hdr.dictionary_size, 256);
   
    /* Compute lenght of dictionary's indexes */
    aux = hdr.dictionary_size;
    while(aux >>=  1)
        index_bits++;
    
    if(to == NULL)  /* Print decompressed sequence on video? (TODO) */
		decomp = stdout;
	else
		decomp = fopen(to, "w");    /* File decompressed */
	
    bitio_read(comp_file, &read_index, index_bits); /* Read the first index */
	
    while(read_index !=  EOFC) {   /* Until end of compressed file */
		
		/* retrieve word */
        res_retrieve = decomp_dict_reb_word(read_index, vector, &actual_length, dict);  
		
        /* Management of bug in LZ78 decompression algorithm */
        if(res_retrieve == -1) {
			decomp_dict_insertion(prev_current, child_root, dict);
			actual_length = 0;
			decomp_dict_reb_word(read_index, vector, &actual_length, dict);
        	first_read = true;
        }
        
        tmp_length = actual_length;
        child_root = vector[actual_length]; /* First symbol of the sequence to 
                                               write */
		
        /* Write the retrieved sequence on file */
		for(i = tmp_length; i >= 0; i--)
			fwrite((char*)&vector[i], 1, 1, decomp);
		
        if(first_read == false)
            /* Update dictionary with new word */
			decomp_dict_insertion(prev_current, child_root, dict);
		
        prev_current = read_index;
        first_read = false; /* First iteration end */
        
        bitio_read(comp_file, &read_index, index_bits); /* Read next index */
    }

    /* Deallocate all data structures used */
    bitio_close(comp_file);                                 
    fclose(decomp);
    decomp_dict_suppress(dict);
    free(vector);
    
    fprintf(stderr, "Decompression executed\n");
}


/* VERBOSE_MODE
 *
 * PURPOSE: Compute and print verbose mode information
 */
void verbose_mode(bool comp, struct timeval s_time, struct timeval f_time,
                  char* i_name, char* o_name) {
    
    double comp_ratio;  /* Compression ratio*/
    char print[4];
    struct stat input_stat, output_stat;    /* Information of files */
    struct timeval elapsed_time;    /* Time between start and finish processing */
    
    if (comp == true)
        strcpy(print, "C\0");
    else
        strcpy(print, "Dec\0");
    
    /* Compute elapsed time */
    elapsed_time.tv_sec = f_time.tv_sec - s_time.tv_sec;
    elapsed_time.tv_usec = f_time.tv_usec - s_time.tv_usec;
    
    /* Retrieve information of files */
    stat(i_name, &input_stat);
    stat(o_name, &output_stat);
    
    /* Compute compression ratio */
    comp_ratio = (((double)output_stat.st_size / input_stat.st_size) * 100);
    
    /* Print all information on video */
    fprintf(stderr, "%sompression of %s: \n", print, i_name);
    fprintf(stderr, "User ID: %i\n", input_stat.st_uid);
    fprintf(stderr, "Size: %llu bytes\n", (long long int)input_stat.st_size);
    fprintf(stderr, "%sompression ratio: %g%%\n", print, comp_ratio);
    fprintf(stderr, "Time: %lu.%hu s\n", elapsed_time.tv_sec, (short)elapsed_time.tv_usec);
}
