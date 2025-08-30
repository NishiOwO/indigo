#ifndef __INDIGO_H__
#define __INDIGO_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define VERSION "1.00"

/* main.c */
void IndigoMainRoutine(void);
void IndigoMainUIRoutine(void);

/* ui driver */
void IndigoShowSplash(void);
void IndigoShowMain(void);

#endif
