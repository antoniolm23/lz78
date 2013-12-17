/*
 This is an header file
 */
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "endian.h" //Mac OS requirement

#define EOFC 256
#define int_stdin 0
#define int_stdout 1
#define int_stderr 2

//#undef SIZE
//#define SIZE 2048

#ifndef bool
typedef enum{true, false} bool;	//definition of bool type
#endif


