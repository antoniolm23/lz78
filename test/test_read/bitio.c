#include "bitio.h"

typedef struct bitio{
  	
  	int fd;	//checks if the opening of the file was successful
  	bool writing;	//file opened in writing mode?
  	int next;
  	int end;
  	int size;	//size of the buffer in bits
  	uint64_t buf[MAX_SIZE];
  	
}bitio;

bitio* bit_open(char* name, char* mode) {
  
	int i;
  
	//fprintf(stderr, "bitopen\n\n");
  
	bitio* f=calloc(1, sizeof(bitio));//allocate the bitio struct
	//fprintf(stderr, "bitopen1\n\n");
  	if(f==NULL) {
    	fprintf(stderr, "error in allocating\n");
    	errno=ENOMEM;	//no memory available
    	return NULL;
  	}
  	
  	//deal with exceptions
  	if(name==NULL) {
	  fprintf(stderr, "namenull\n");
	  goto error;	
	}
  	if(mode==NULL) {
	  fprintf(stderr, "modenull\n");
	  goto error;	//way to deal with exceptions
	}
  
  	//check if the mode chosen is right
  	if(strcmp(mode, "r")!=0&&strcmp(mode,"w")!=0) 
		goto error;
  
  	//open the file in read or write and put the result in fd
  	//f->fd=open(name, (mode[0]=='r')?O_RDONLY:(O_WRONLY));//|O_CREAT|O_TRUNC));
	if(mode[0]=='r')
	    f->fd=open(name, O_RDWR);
	else 
		f->fd=open(name, O_CREAT|O_RDWR, S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH);
  	
	if(f->fd<0) { 
	  fprintf(stderr, "fd\n");
	  goto error;	//checks if the open succeded
	}
  	
  	//save in f->mode the way in which we choose to open the file
  	f->writing=(mode[0]=='r')?O_RDONLY:O_WRONLY;
  	//fprintf(stderr, "writing? %x\n\n\n", f->writing);
  	//f->size=sizeof(f->buf);	//size in bytes of the buffer
  	f->size=MAX_SIZE*64;		//size in bits
  	
  	//set the structure for reading mode
  	if(f->writing==O_RDONLY) {
    	//fprintf(stderr, "apro il file in reading\n");
    	f->end=f->size;
    	f->next=0;
  	}
  
  	//set the structure for writing mode
  	else {
    	f->end=f->size;
    	f->next=0;
  	}
  
  	//set the buffer to zero in order to perform the writing operations
  	for(i=0; i<MAX_SIZE; i++) {
    	f->buf[i]=0;
  	}
    
  	return f;

error: 	free(f);
	fprintf(stderr, "error detected\n");
	errno=EINVAL;
	return NULL;

};

//in the write function len is expressed in bits
int bitio_write(bitio* f, uint64_t d, int len) { //len is expressed in bits
  	
  	int space, ofs, n, i, result, original;
  	uint64_t* p;
  	uint64_t tmp;
  	original=(int) d;
  	//check all the parameters
  	if((f==NULL)||(len<1)||(len>8*sizeof(d))) {
    	errno=EINVAL;
    	fprintf(stderr, "Error1\n");
    	return -1;
  	}
  
  	//check if we do the right operation
  	if(!f->writing) {
    	errno=EINVAL;
    	fprintf(stderr, "Error2\n");
    	return -1;
  	}
  
again:  space=f->end-f->next;
  //check if there's enough space on the buffer
  	if(space<0) {
  		errno=EINVAL;
    	fprintf(stderr, "Error3\n");
    	return -1;
  	}
  	
  	//check if the buffer is full 
  	if(space==0) {
   		
   		//write the buffer in the file	
    	result=(int)write(f->fd, f->buf, MAX_SIZE*8);
    	if(result<0) {
			fprintf(stderr, "Error in write\n");
			return -1;
		}
    	
    	//reset the buffer
    	for(i=0; i<MAX_SIZE; i++) {
      		f->buf[i]=0;
    	}
    	
    	//reset the structure
    	f->size=MAX_SIZE*64;
    	f->end=f->size;
    	f->next=0;
    
  	}

  	//go to the right address in the buffer
  	p=f->buf+(f->next/64);
  	//go to the right offset
  	ofs=f->next%64;
  	n=64-ofs;
  
  	if(len<n) n=len;
  	
  	//convert *p from little endian
  	tmp=le64toh(*p);
  
  	//shift the datum of ofs positions
  	tmp|=(d<<ofs);
  
  	if((ofs+n)>64) {
   		tmp&=((1<<(ofs+n))-1);
  	}
  	*p=htole64(tmp);
  	
  	//update the structure
  	f->next+=n;
  	len-=n;
  	d>>=n;
  	
  	//check if there are bits left to put in the buffer
  	if(len>0) 
		goto again;
    
    if(original==EOFC) {
    	
    	//fprintf(stderr, "\n\t\tEOF written\n");
    	//f->buf[(MAX_SIZE-1)]=d;
    	result=(int)write(f->fd, f->buf, (f->next/8)+1);
    	if(result<0) fprintf(stderr, "Error in write\n");
    	return 1; 
    
  	}
    
  	//if the operation succedes return 1
  	return 1;
}

//Read len bits and save on datum
int bitio_read(bitio* f, uint64_t* datum, int len) {
    
    int offset, readable, noffset;
    uint64_t* pointer;
    uint64_t tmp/*, tmp2*/;
    int res;
    
    //check all the parameters
    if((f == NULL) || (len < 1) || (len > sizeof(datum)*8)) {
        errno = EINVAL;
        fprintf(stderr, "Error1 %i\n", len);
        if(f == NULL)
            fprintf(stderr, "NULL\n");
        return -1;
    }
    //check if we do the right operation
    if(f->writing) {
        errno = EINVAL;
        fprintf(stderr, "Error2\n");
        return -1;
    }
	
again: 
	readable=f->end-f->next;
    if(readable<0) {
		errno=EINVAL;
		fprintf(stderr, "Error3\n");
		return -1;
    }
    
    //all bits read, so read another block
    if(readable==f->size) {
		res=read(f->fd, f->buf, MAX_SIZE*8);
		if(res<0) {
			fprintf(stderr, "error in read\n");
			return -1;
		}
		
		//reset the structure
		f->next=0;
		f->end=MAX_SIZE*8;
		f->size=MAX_SIZE*8;
	}
	
	//go to the right address
	pointer=f->buf+(f->next/64);
	//go to the right offset
	offset=f->next%64;
	noffset=64-offset;
	
	if(len<noffset) 
		noffset=len;
	
	//retrieve the datum on 64 bits
	tmp=le64toh(*pointer);
	tmp>>=offset;
	tmp &= (((uint64_t)1 << (len)) - 1);
	f->next+=noffset;
	len-=noffset;
	
	//check if I have to continue
	if(len>0) {
		tmp>>=len;
		goto again;
	}
	
    return 1;
}

//close the bitio structure
void bitio_close(bitio* b) {
	
	close(b->fd);		//close the file
	free(b);		//free the structure		
	
}
