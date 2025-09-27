/* 
* Name: Taha Mohyuddin
* Student ID: 1275575 
* Date: 9 - Mar - 2025
* Course: CIS2750
*/

#ifndef VCHELPERS_H
#define VCHELPERS_H

#include <stddef.h>
#include "VCParser.h"      
#include "LinkedListAPI.h" 

/* Prototypes for internal helper functions */
int fileVal(const char* fN);
char* dupli_st(const char* soup);
char* dupli_st_2(const char* sour, size_t maxL);
VCardErrorCode processL(char* l, Card* c, bool* hB, bool* hE, bool* hasV) ;
DateTime* parseDT(List* values); 
bool chkPN(const char* n);
bool isValDT(const DateTime* dt);
VCardErrorCode valP(const Property* prop); 
bool forDT(FILE* out, const char* label, const DateTime* dt);




#endif
 