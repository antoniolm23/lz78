/*
 * bitio.h
 * Antonio La Marra - Giacomo Rotili
 * December 2013
 * 
 * This file contains the function prototypes of the BITIO interface; it allows
 * to perform I/O file operations of variable-length in bits.
 *
 * NOTE: bitio is an opaque data type, the internal structure can be found in 
 * bitio.c
 */

#include "everything.h"

#define MAX_SIZE 3  /* Maximum length of the internal buffer  of BITIO */

struct bitio; /* Opaque data type */

/* BIT_OPEN
 *
 * PURPOSE: Open a file through the BITIO interface
 * PARAMETERS: name - file path to open
 *             mode - mode in which to open the file, "r" to read mode,
 *                    "w" to write mode
 * RETURNS: If the file is successfully opened, returns a pointer to a bitio 
 *          object, otherwise a null pointer
 * 
 * If the file doesn't exist, it is created
 */
struct bitio* bit_open(char* name, char* mode);


/* BITIO_WRITE
 *
 * PURPOSE: Write the first len bits of d in the file currently related to f
 * PARAMETERS: f - pointer to a bitio object that specifies an output file
 *             d - Block of memory (64 bit - integer) where are stored the bits 
 *                 to be written
 *             len - number of bits to write
 * RETURNS: If the write operation has been performed successfully returns 1
 *          otherwise -1
 */
int bitio_write(struct bitio* f, uint64_t d, int len);


/* BITIO_READ
 *
 * PURPOSE: Read the first len bits of datum from the file currently related to f
 * PARAMETERS: f - pointer to a bitio object that specifies an input file
 *             datum - block of memory (64 bit - integer) in which store the
 *                     bits to be read
 *             len - number of bits to read
 * RETURNS: If the read operation has been performed successfully returns 1
 *          otherwise -1
 *
 * Datum is overwritten with a integer composed by the bits read and (64 - len) 
 * zeros
 */
int bitio_read(struct bitio* f, uint64_t* datum, int len);


/* BITIO_CLOSE
 *
 * PURPOSE: Close a file through the BITIO interface
 * PARAMETERS: b - pointer to a bitio object that specifies the file to close
 *
 * The bitio object pointed by b is deallocated
 */
void bitio_close(struct bitio* b);
