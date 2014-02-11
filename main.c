/*
* main.c
* Antonio La Marra - Giacomo Rotili
* January 2014
*
* Implementation of lz78 command.
*
* NOTE: main merely reads and interprets the command parameters, then calling 
		compress() or decompress().
*/


#include "everything.h"
#include "comp.h"

int main(int argc, char* argv[]) {

	int size = 0;	/* size of the dictionary used by compressor */
	int actopt;	/* takes into account the character corresponding to actual
					option */
	/* flags for the options */
	bool comp_opt = false;	/* compression option */
	bool decomp_opt = false;	/* decompression option */
	bool output_opt = false;	/* output option */
	bool keyboard_opt = false;	/* keyboard option */
	bool input_opt = false;	/* input option */
	bool size_opt = false;	/* size option */
	bool verbose_opt = false;	/* verbose mode option */
	char* input_filename = NULL;
	char* output_filename = NULL;
	char* title = "-- LZ78 compression implementation of A.LaMarra G.Rotili --\n";
	char* key_inst = "Type the string to compress at the end press Ctrl+D\n";
	struct timeval start_time;	/* Start time to clock processing */
	struct timeval finish_time;	/* Finish time to clock processing */

	/* Checking syntax of the command */
	if(argc < 2) {	/* Surely missing parameters */
		fprintf(stderr, "Missing options!\n");
		exit(-1);
	}

	/* Checking options entered by the user */

	while((actopt = getopt(argc, argv, "cdvi:o:s:k:")) != -1) {
		switch(actopt) {
			case 'c':	/* compression */
				comp_opt = true;
				break;

			case 'd':	/* decompression */
				decomp_opt = true;
				break;

			case 'i':	/* input filename */
				input_opt = true;
				input_filename = optarg;
				if(optarg == NULL) {
					fprintf(stderr, "error, no filename passed\n");
					return -1;
				}
				break;

			case 'o':	/* output filename */
				output_opt = true;
				output_filename = optarg;
				if(optarg == NULL) {
					fprintf(stderr, "error, no filename passed\n");
					return -1;
				}
				break;

			case 'v':	/* verbose mode */
				verbose_opt = true;
				break;

			case 's':   /* size of dictionary */
				size_opt = true;
				size = atoi(optarg);    /* optarg is a string */
				break;

			case 'k':   /* sequence to compress from keyboard */
				keyboard_opt = true;
				break;

			default:
			break;
		}
	}

	if(size < 900 || size_opt == false)	/* Default size of the dictionary */
		size = 900;

/*		*		*		*		*		*		*		*		*		*
*								COMPRESSOR
*		*		*		*		*		*		*		*		*		*/

	if(comp_opt == true) {
		if(keyboard_opt == true) {	/* No file to compress, input from keyboard */
			input_filename = NULL;
			fprintf(stderr, "%s", key_inst);	/* keyboard input instruction for use */
		}
	else {
		if(input_opt == false)
			input_filename = "file.txt";	/* default file to compress */
	}

	if(output_opt == false)
		output_filename = "compressed.lz78";    /* default name of compressed file */
	if(verbose_opt == true) {	/* verbose mode on? */
		fprintf(stderr, "%s", title);
		gettimeofday(&start_time, NULL); /* Start time to clock processing */
	}

	/* start compression */
	compress(input_filename, output_filename, size);
	}

/*		*		*		*		*		*		*		*		*		*
*								DECOMPRESSOR
*		*		*		*		*		*		*		*		*		*/

	if(decomp_opt == true) {
		if(input_opt == false)
			input_filename = "compressed.lz78";	/* default name of compressed file */
		if(output_opt == false)
			output_filename = NULL;	/* decompression to video */
		if(verbose_opt == true) {	/* verbose mode on? */
			fprintf(stderr, "%s", title);
			gettimeofday(&start_time, NULL);	/* Start to clock processing */
	}

	/* start decompression */
	decompress(input_filename, output_filename);
	}
	
	if(verbose_opt ==  true) {	/* verbose mode on? */
		gettimeofday(&finish_time, NULL); /* Finish to clock processing */
		verbose_mode(comp_opt, start_time, finish_time, input_filename, output_filename);
	}

	return 0;
}
