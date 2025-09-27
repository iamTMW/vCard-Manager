/* 
* Name: Taha Mohyuddin
* Student ID: 1275575 
* Date: 27 - Mar - 2025
* Course: CIS2750
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "VCParser.h"

/* Below is the helper function to simplify string return to Python */
char* copyStr(const char* src) {
    if (!src) return NULL;
    char* str = malloc(strlen(src) + 1);
    if (str) strcpy(str, src);
    return str;
}

/* Below is the function to validate a vCard file and returns the validation result as an error code */
int valiF(const char* filename) {
    Card* card = NULL;
    VCardErrorCode err = createCard((char*)filename, &card);
    if (err == OK) {
        err = validateCard(card); // This will run full validation only if parsing succeeded
    }
    if (card) deleteCard(card);
    return err; // This will eeturns OK if valid, or appropriate error code
}

/* Below function will extract and return the Full Name (FN) from a vCard file, or NULL on error */
char* getFNFromFile(const char* filename) {
    Card* card = NULL;
    VCardErrorCode err = createCard((char*)filename, &card);
    if (err != OK || !card || !card->fn || !card->fn->values) return NULL;
    char* fn = getFromFront(card->fn->values);
    char* result = copyStr(fn); // This will safely return a heap copy of FN
    deleteCard(card);
    return result;
}

/* Below function will generates a short summary string for a vCard */
char* CardSumm(const char* filename) {
    Card* card = NULL;
    VCardErrorCode err = createCard((char*)filename, &card);
    if (err != OK || !card) return copyStr("Invalid vCard");

    char* summary = malloc(512); // This will be enough space for summary
    char* fn = card->fn && card->fn->values ? getFromFront(card->fn->values) : NULL;
    int otherProps = getLength(card->optionalProperties);

    sprintf(summary, "FN: %s, Props: %d", fn ? fn : "N/A", otherProps);
    deleteCard(card);
    return summary;
}

/* Below it will update the FN in the vCard file and write it back to disk */
int upFN(const char* filename, const char* newFN) {
    Card* card = NULL;
    VCardErrorCode err = createCard((char*)filename, &card);
    if (err != OK || !card) return err;

    // Below it will remove old FN property if it exists
    if (card->fn) deleteProperty(card->fn);

    // Below it will create a new FN property
    Property* fnProp = malloc(sizeof(Property));
    fnProp->group = copyStr(""); // This is for empty group
    fnProp->name = copyStr("FN");
    fnProp->parameters = initializeList(parameterToString, deleteParameter, compareParameters);
    fnProp->values = initializeList(valueToString, deleteValue, compareValues);
    insertBack(fnProp->values, copyStr(newFN));

    card->fn = fnProp;

    // Below it will validate and write back to file
    err = validateCard(card);
    if (err == OK) {
        err = writeCard(filename, card);
    }

    deleteCard(card);
    return err;
}
/* Below the function will return a string version of the BDAY property if present, else NULL */
char* BDAY_File(char* filename) {
    Card* c;
    if (createCard(filename, &c) != OK || !c || !c->birthday) return NULL;
    char* s = dateToString(c->birthday);  // This will convert date struct to string
    deleteCard(c);
    return s;
}
/* Below the function will return a string version of the ANNIVERSARY property if present, else NULL */
char* ANNIV_File(char* filename) {
    Card* c;
    if (createCard(filename, &c) != OK || !c || !c->anniversary) return NULL;
    char* s = dateToString(c->anniversary);  // This will convert date struct to string
    deleteCard(c);
    return s;
} 
/* Below the function will return a newline separated list of optional properties excluding FN, BDAY, ANNIV */
char* OptPropsW(const char* filename) {
    return getOptP(filename); // This will just call the existing helper
}
