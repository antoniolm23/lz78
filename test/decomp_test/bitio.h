//library to manage bit

#include "everything.h"

//max size of the buffer
#define MAX_SIZE 3

struct bitio;

//in the write function len is expressed in bits
struct bitio* bit_open(char* name, char* mode);

//first function that opens a file in read (r) mode or write (w) mode
int bitio_write(struct bitio* f, uint64_t d, int len);

//Read len bits and save on datum
int bitio_read(struct bitio* f, uint64_t* datum, int len);

//close a bitio structure
void bitio_close(struct bitio* );
