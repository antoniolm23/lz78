/***********************************************
 *Copyright La Marra Antonio Rotili Giacomo
 * (c) 2013
 **********************************************/
//#include "everything.h"
#include "comp.h"
//#include "dict.h"
//#include "hash.h"
#include "everything.h"

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
	
	/*//decompression
	if(option[1]==true) {
		if(input_filename==NULL) input_filename="compressed.lz78";
		if(output_filename==NULL) output_filename="decomp.txt";
		//better managing file extensions automatically or having the possibility of printing the 
		//out of the decompressor to the standard output?
		begin = clock();
		decompress(input_filename, output_filename);
		end = clock();
		time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
		if(option[4]==true) fprintf(stderr, "time spent: %f\n", time_spent);
	}*/
	
    return 0;
}
