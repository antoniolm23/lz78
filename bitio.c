/*
 * bitio.c
 * Antonio La Marra - Giacomo Rotili
 * February 2014
 *
 * This file contains the definition of the BITIO interface and its functions.
 *
 * NOTE: The GOTO statement is used as jump statement and to deal with
 *		 exceptions. Its usage is minimal.
 */

#include "bitio.h"

/* Definition of the BITIO object interface */
typedef struct bitio {
	
	FILE* fd;	/* File pointer to handle the file related to the interface */
	bool writing;	/* true = write mode, false = read mode */
	int next;	/* Number of the next bit in the buffer, to be written/read */
	int end;	/* NUmber of the last bit in the buffer, to be written/read */
	int size;	/* Size of the buffer (in bits) */
	uint64_t buf[MAX_SIZE];	/* Buffer in which we store the bits to be written-
							 read*/
} bitio;


/* BIT_OPEN
 *
 * PURPOSE: Open a file through the BITIO interface
 */
bitio* bit_open(char* name, char* mode) {
	
	int i;
	bitio* f = calloc(1, sizeof(bitio));
	
	/* Checking parameters */
	if(f == NULL) {
		fprintf(stderr, "error in allocating\n");
		errno = ENOMEM;	/* No memory available */
		return NULL;
	}
	
	if(name == NULL) {
		fprintf(stderr, "name null\n");
		goto error;
	}
	
	if(mode == NULL) {
		fprintf(stderr, "mode null\n");
		goto error;
	}
	
	/* Checking syntax of the mode parameter */
	if(strcmp(mode, "r")!= 0 && strcmp(mode,"w")!= 0)
		goto error;
	
	/* "Effective" opening of the file */
	if(mode[0] ==  'r')
		f->fd = fopen(name, "r+");
	else
		f->fd = fopen(name, "w+");
	
	if(f->fd == NULL) {
		fprintf(stderr, "fd\n");
		goto error;
	}
	
	/* Setting parameters of the interface */
	f->writing = (mode[0] == 'r')?O_RDONLY:O_WRONLY;
	f->size = MAX_SIZE*64;	/* Each element of bug is 64-bit unsigned integer */
	
	if(f->writing == O_RDONLY) {	/* reading mode */
		f->end = f->size;
		f->next = 0;
	} else {	/* writing mode */
		f->end = f->size;
		f->next = 0;
	}
	
	for(i = 0; i<MAX_SIZE; i++)	/* Initializing the buffer */
		f->buf[i] = 0;
	
	return f;
	
error:	/* Error Handling */
	free(f);
	fprintf(stderr, "error detected\n");
	errno = EINVAL;
	return NULL;
}

/* BITIO_WRITE
 *
 * PURPOSE: Write the first len bits of d in the file currently related to f
 */
int bitio_write(bitio* f, uint64_t d, int len) {
	
	int space;	/* Free space in the buffer (in bits) */
	int ofs;	/* internal offset to each element of the buffer (<64) */
	int n;	/* maximum number of bits that can be written in a iteration (<64) */
	int i;	/* for(;;) iterator */
	int result;	/* variable to store fwrite() return */
	int original;	/* temporary variable to store the original value of d */
	uint64_t* p;	/* current position in the bitio buffer */
	uint64_t tmp;	/* temporary variable to store one element of buffer */
	
	original = (int)d;
	
	/* Checking parameters */
	if((f == NULL) || (len < 1) || (len > (8 * sizeof(d)))) {
		errno = EINVAL;
		fprintf(stderr, "Error1\n");
		return -1;
	}
	
	if(!f->writing) {	/* Operation allowed? */
		errno = EINVAL;
		fprintf(stderr, "Error2\n");
		return -1;
	}
	
again:
	space = (f->end - f->next);
	if(space < 0) {
		errno = EINVAL;
		fprintf(stderr, "Error3\n");
		return -1;
	}
	
	if(space == 0) {	/* If the buffer is full, it is emptied by writing all
							of its content to the file */
		result = (int)fwrite(f->buf, 1, (MAX_SIZE * 8), f->fd);
		if(result<0) {
			fprintf(stderr, "Error in write\n");
			return -1;
		}
		
		/* Re-Initializing the structure */
		for(i = 0; i < MAX_SIZE; i++)
			f->buf[i] = 0;
		f->size = (MAX_SIZE * 64);
		f->end = f->size;
		f->next = 0;
	}
	
	p = (f->buf + (f->next / 64));
	ofs = (f->next % 64);
	n = (64 - ofs);	/* number of writable bits in the current element of buf */
	if(len < n)	/* Enough space in the current element of buf */
		n = len;
	
	tmp = le64toh(*p);	/* Read and convert current item of buf to host order */
	tmp |= (d << ofs);	/* Write the data into the correct position, without
							overwriting the bits already present */
	
	*p = htole64(tmp);	/* Store in buffer */
	
	/* Structure update */
	f->next += n;
	len -= n;
	d >>= n;
	
	if(len > 0)	/* There are still bits in d to store in buffer? */
		goto again;
	
	if(original == EOFC) {	/* End of compressed sequence? */
		/* Writing the last remaining bits in the buffer */
		result = (int)fwrite(f->buf, 1, ((f->next / 8) + 1), f->fd);
		if(result<0)
			fprintf(stderr, "Error in write\n");
		return 1;
	}
	return 1;
}

/* BITIO_READ
 *
 * PURPOSE: Read the first len bits of datum from the file currently related to f
 */
int bitio_read(bitio* f, uint64_t* datum, int len) {
	
	int offset;	/* internal offset to each element of the buffer (<64) */
	int readable;	/* Readable bits in the buffer */
	int noffset;	/* max n° of bits that can be written in a iteration (<64) */
	int res;	/* variable to store fread() return  */
	int old_n;	/* variable to store old value of noffset */
	uint64_t tmp = 0, tmp2 = 0;	/* temporary variables to store element of buffer */
	uint64_t* pointer;	/* current position in the bitio buffer */
	bool rep = false;	/* flag to discriminate readings in one or more iteration */
	
	/* Checking parameters */
	if((f == NULL) || (len < 1) || (len > (sizeof(datum) * 8))) {
		errno = EINVAL;
		fprintf(stderr, "Error1 %i\n", len);
		if(f == NULL)
			fprintf(stderr, "NULL\n");
		return -1;
	}
	
	if(f->writing) {	/* Operation allowed? */
		errno = EINVAL;
		fprintf(stderr, "Error2\n");
		return -1;
	}
	
again:
	readable = (f->end - f->next);
	if(readable < 0) {
		errno = EINVAL;
		fprintf(stderr, "Error3\n");
		return -1;
	}
	
	if(readable == f->size) {	/* There aren't bits to be read */
		res = fread(f->buf, 1, (MAX_SIZE * 8), f->fd);	/* Refill the buffer */
		if(res < 0) {
			fprintf(stderr, "error in read\n");
			return -1;
		}
		/* Re-Initializing the structure */
		f->next = 0;
		f->end = (MAX_SIZE * 64);
	}
	
	pointer = (f->buf + (f->next / 64));
	offset = (f->next % 64);
	noffset = (64 - offset);	/* readable bits in the current element of buf */
	if(len < noffset)	/* Enough number of bits in the current element of buf */
		noffset = len;
	
	tmp = le64toh(*pointer);
	tmp >>= offset;
	
	if(len < 64)
		tmp &= (((uint64_t)1 << len) - 1);	/* tmp cleanup of not required bits */
	
	/* Structure update */
	f->next = (f->next + noffset) % (f->end);
	len -= noffset;
	
	if(len > 0) {	/* There are still bits to read? */
		tmp2 = tmp;	/* Save bits already read*/
		rep = true;
		old_n = noffset;
		goto again;
	}
	if(rep == true) {	/* Had already read other bits*/
		tmp <<= old_n;
		tmp = (tmp | tmp2);	/* Merging of the two temporary readings */
	}
	
	htole64(tmp);
	*datum = tmp;	/* read result */
	
	return 1;
}

/* BITIO_CLOSE
 *
 * PURPOSE: Close a file through the BITIO interface
 */
void bitio_close(bitio* b) {
	
	fclose(b->fd);
	free(b);
}
