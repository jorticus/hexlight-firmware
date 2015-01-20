/* 
 * File:   common.h
 * Author: Jared
 *
 * Created on 10 June 2014, 8:57 PM
 */

#ifndef COMMON_H
#define	COMMON_H

#include <stdint.h>

typedef unsigned char byte;
typedef unsigned int uint;

#ifndef	__cplusplus
typedef unsigned char bool;
#endif

#define true 1
#define false 0

#define _XKEYCHECK // TODO: Don't know why I had to define this, but it makes the program compile.
#include <GenericTypeDefs.h>

#endif	/* COMMON_H */

