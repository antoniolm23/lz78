/***********************************************
 *Copyright La Marra Antonio Rotili Giacomo
 * (c) 2013
 **********************************************/
#include "everything.h"
#include "comp.h"

int main(int argc, char* argv[]) {
    
    bool comp_opt = false, decomp_opt = false, input_opt = false,
    output_opt = false, verbose_opt = false, size_opt = false,
    keyboard_opt = false;
    char* input_filename=NULL;
    char* output_filename=NULL;
    int size=0;
    struct timeval start_time, finish_time;
    
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
                comp_opt = true;
                break;
			
			//verbose output
            case 'd':
                decomp_opt=true;
                break;
			
			//input filename
            case 'i':
                input_opt=true;
                input_filename=optarg;
                if(optarg==NULL) {
                    fprintf(stderr, "error, no filename passed\n");
                    return -1;
                }
                break;
				
			//output filename
            case 'o':
                output_opt=true;
                output_filename=optarg;
                if(optarg==NULL) {
                    fprintf(stderr, "error, no filename passed\n");
                    return -1;
                }
                //optindex[3]=optind;
                break;
				
			//decompression command
            case 'v':
                verbose_opt=true;
                //optindex[4]=optind;
                break;
			
			//dictionary size
            case 's':
                size_opt=true;
                size=atoi(optarg);
                //optindex[5]=optind;
                break;
			
			//input from keyboard
			case 'k':
				keyboard_opt=true;
				//here we have to write the passed string to the stdin manually?
				break;
            default:
                break;
        }
    }
	
	//Default size of the dictionary
    if(size<900 || size_opt==false) size=900;
	
	//Compression
    if(comp_opt==true) {
		if(keyboard_opt==true)
			input_filename=NULL;
		else {
			if(input_opt==false)
                input_filename="file.txt";
			//input_fd=open(input_filename, 'r');
		}
		if(output_opt==false)
            output_filename="compressed.lz78";
        if(verbose_opt == true) {
            fprintf(stderr, "-- LZ78 compression implementation of A.LaMarra G.Rotili --\n");
            gettimeofday(&start_time,NULL); //Start time to clock
        }
		compress(input_filename, output_filename, size);
	}
	
	//Decompression
	if(decomp_opt==true) {
		if(input_filename==NULL)
            input_filename="compressed.lz78";
		if(output_filename==NULL)
            output_filename="decomp.txt";
		//better managing file extensions automatically or having the possibility of printing the 
		//out of the decompressor to the standard output?
        if(verbose_opt == true) {
            fprintf(stderr, "-- LZ78 compression implementation of A.LaMarra G.Rotili --\n");
            gettimeofday(&start_time,NULL); //Start time to clock
        }
		decompress(input_filename, output_filename);
	}
	
    //VERBOSE MODE informations
    if(verbose_opt == true) {
        gettimeofday(&finish_time, NULL); //Finish time to clock
        verbose_mode(comp_opt, start_time, finish_time, input_filename, output_filename);
    }
    
    return 0;
}
