//#include "everything.h"
#include "dict.h"
#include "bitio.h"

typedef struct hdr {
    int dictionary_size;   //bit length
    int longest_match;   //original filename
}header;

/*compress function, parameters are:
 * filename of the file from which we're reading data 
 * filename in which put the compressed data
 * size of the dictionary*/
void compress(char* from, char* to, int size);

/*decompress function*/
void decompress(char* from, char* to);
