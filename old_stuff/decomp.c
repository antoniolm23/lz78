//decompressor algorithm for lz78
//filename is the path of the file to decompress, size is the size of the dictionary
void decompress(char* input, char* filename) {
  
    int /*i,*/index_bits, flag, aux, child_root=0, pos;
    bool first_read = true, res_retrieve=false;
    uint64_t read_index, prev_current=0;
    bitio* comp_file;
    char* tmp;
    dictionary* dict;
    word* sequence = NULL;      //Variable in which store the word in each step
    word* print_seq;
    header hdr;
    int decomp_file;
    
    //eofc_pos = dict->symbols;
    comp_file = bit_open(input, "r");       //Preparing my data structures
    
    //retrieve header
    bitio_read(comp_file, (uint64_t*)&hdr.dictionary_size, sizeof(int)*8);
    //for(i=0; i<6; i++)
		//bitio_read(comp_file, (uint64_t*)&hdr.extension[i], sizeof(char)*8);
    
    dict = malloc(sizeof(dictionary));
    dict_init(dict, hdr.dictionary_size, 256);
   
    
    aux = hdr.dictionary_size;         //Compute the number of bits representing the indexes
    fprintf(stderr, "%i\n", aux);
    index_bits = 1;
    while(aux >>= 1)
        index_bits++;
    
    tmp=malloc(strlen(filename));
    strcpy(tmp, filename);
    //strcat(tmp, ".\0");
    //strcat(tmp, hdr.extension);
	
    
    decomp_file=open(tmp, O_CREAT|O_RDWR, S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH);
    
    bitio_read(comp_file, &read_index, index_bits); //Read the first index
    //decomp_file = open(decompname, O_CREAT|O_RDWR, S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH);
    //fprintf(stderr, "\n%i\n", fdim);
    
	fprintf(stderr, "%s %i\n", tmp, index_bits);
	
    while(read_index != EOFC) {   //Until the end of compressed file
		
		//fprintf(stdout, "%i\n", (int)read_index);
		
		//retrieve word from index
        res_retrieve=dict_retrieve_word(dict, read_index, &sequence);  
        //fprintf(stderr, "\n%x\n", (unsigned int)read_index);
	
        //critical situation
        if(res_retrieve==false) {
        	pos=dict_search((int)prev_current, child_root, dict, &flag);
        	dict_add_word(pos, (int)prev_current, child_root, dict, &flag);
        	dict_retrieve_word(dict, read_index, &sequence);
        	first_read=true;
        }
        //fprintf(stderr, "decompressor %i\n", decomp_file);
        child_root = sequence->symbol;
	//fprintf(stderr, "decompressor %i\n", decomp_file);
        print_seq = sequence;	//Write word on file
        
        while(print_seq != NULL) {
			//fprintf(stdout, "%i\n",print_seq->symbol);
	    
	    write(decomp_file, (void*)&(print_seq->symbol), 1);
            print_seq = print_seq->next;
            free(sequence);                             //Deallocate word structure
            sequence = print_seq;
        }
        
        if(first_read==false) {
			//Add the first symbol of my sequence, as child of the previous current node
            pos=dict_search((int)prev_current, child_root, dict, &flag);
            dict_add_word(pos, (int)prev_current, child_root, dict, &flag); 
        }
        first_read = false;
        prev_current = read_index;
        bitio_read(comp_file, &read_index, index_bits);                 //Read the index
    }
    
    //Closure of all data structures
    close(comp_file->fd);                                 
    free(comp_file);
    close(decomp_file);
    //print_dict(dict);
    suppress_dictionary(dict);
    
    fprintf(stderr, "decompression executed\n");
}
