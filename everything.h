/*
 * everything.h
 * Antonio La Marra - Giacomo Rotili
 * December 2013
 * 
 * Set of header files most used in the project
 *
 */

/* Header file  */
#pragma once        /* Avoid multiple include of this file during compilation */
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
//#include "endian.h"

#define EOFC 256    /* End of File for compressed files (symbols <= 255) */

/* Definition of the Boolean type */
#ifndef bool
typedef enum{true, false} bool;
#endif


