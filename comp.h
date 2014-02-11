/*
 * comp.h
 * Antonio La Marra - Giacomo Rotili
 * December 2013
 *
 * Definition of data and function prototypes that implemented the compression
 * and decompression algorithms.
 *
 * NOTE: To implement the verbose option of the program has been defined as a
 *		specific function, in order to maintain the main more simple and linear
 *
 */

#include "dict.h"

/* COMPRESS
 *
 * PURPOSE: Implement the LZ78 compress algorithm
 * PARAMETERS:	from - path of the file to compress
 *				to - name of compressed file
 *				size - Size of the dictionary to use for compression
 *
 * The compressed file name can be chose by the user, otherwise a standard name
 * is chosen.
 */
void compress(char* from, char* to, int size);


/* DECOMPRESS
 *
 * PURPOSE: Implement the LZ78 decompress algorithm
 * PARAMETERS:	from - path of the file to decompress
 *				to - name of decompressed file
 */
void decompress(char* from, char* to);


/* VERBOSE_MODE
 *
 * PURPOSE: Compute and print verbose mode information
 * PARAMETERS:	comp - Flag to distinguish compression decompression
 *				s_time - Start time processing
 *				f_time - Finish time processing
 *				i_name - path of input file
 *				o_name - path of output file
 */
void verbose_mode(bool comp, struct timeval s_time, struct timeval f_time, char*
					i_name, char* o_name);
