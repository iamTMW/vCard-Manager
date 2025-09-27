/* 
* Name: Taha Mohyuddin
* Student ID: 1275575 
* Date: 9 - Feb - 2025
* Course: CIS2750
*/

#include "VCParser.h" 
#include "VCHelpers.h"
#include "LinkedListAPI.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Below it will check if the file extension is valid in the form of .vcf or .vcard */
int fileVal(const char* fN) {
    if (!fN) return 0;  // This will return 0 if the filename is NULL.
    const char* exten = NULL;
    for (int i = 0; fN[i] != '\0'; i++) {
        if (fN[i] == '.') {
            exten = &fN[i];  // This will update pointer whenever . is found.
        }
    }
    return (exten && (strcmp(exten, ".vcard") == 0 || strcmp(exten, ".vcf") == 0));  // This will check if the extension matches.
}

// Below are the helper functions that i made 
char* dupli_st(const char* soup) {
    if (!soup) return NULL;  // This will check if the input string is NULL; if so, return NULL.
    size_t l = strlen(soup);  // This will Calculate the length of the input string.
    char* c = malloc(l + 1);  // This allocates memory for the new string, including space for the null terminator.
    if (c)  // This will will check if memory allocation was successful.
        strcpy(c, soup);  // This will copy the original string into the allocated memory.
    return c;  // This will return the duplicated string.
} 

/* Below the following parses a list of date time components and returns a dynamically allocated DateTime object. */
DateTime* parseDT(List* values) {
    int cou = 0; // Here I am declaring a variable count 
    char* fVal = NULL;
    char* sVal = NULL; 

    DateTime* datet = malloc(sizeof(DateTime));
    if (!datet) return NULL;
    datet->UTC = false;
    datet->isText = false;
    // Below I have Initialized date, time, and text fields with empty strings.
    datet->time = malloc(1); if (datet->time) datet->time[0] = '\0';
    datet->text = malloc(1); if (datet->text) datet->text[0] = '\0';
    datet->date = malloc(1); if (datet->date) datet->date[0] = '\0';
    // Below it will check if any memory allocation failed.
    if (!datet->date || !datet->time || !datet->text) {
        free(datet->date); free(datet->time); free(datet->text);
        free(datet);
        return NULL;
    }
    
    // Below it will iterate through the list to extract date time components.
    ListIterator lisit = createIterator(values);
    void* el;
    while ((el = nextElement(&lisit)) != NULL) {
        if (cou == 0)
            fVal = (char*)el;
        else if (cou == 1)
            sVal = (char*)el;
        cou++;
    }
    // For the following if the list is empty, then it will return the initialized DateTime object.
    if (cou == 0) {
        return datet;
    }  // Below it will check If only one value exists, then it will determine if it's a date, time, or text.
    else if (cou == 1) {
        if (fVal[0] == 'T') { 
            free(datet->time);
            datet->time = dupli_st(fVal + 1);
        } 
        else if (strchr(fVal, 'T') != NULL) { 
            //Below it will Handle the date time values 
            char* tP = strchr(fVal, 'T');
            size_t dLen = tP - fVal;
            char* dPart = dupli_st_2(fVal, dLen);
            char* tPart = dupli_st(tP + 1);
            if (!dPart || !tPart) {
                free(dPart); free(tPart);
                deleteDate(datet);
                return NULL;
            }
            // So below it will check for UTC indicator
            size_t tLen = strlen(tPart);
            if (tLen > 0 && tPart[tLen - 1] == 'Z') {
                tPart[tLen - 1] = '\0';
                datet->UTC = true;
            }
            free(datet->date); datet->date = dPart;
            free(datet->time); datet->time = tPart;
        }
        else if (strlen(fVal) == 6 && strspn(fVal, "0123456789") == 6) {
            // So below it will handle time-only values 
            free(datet->time);
            datet->time = dupli_st(fVal);
            free(datet->date); 
            datet->date = dupli_st(""); // This will ensure that the date is empty
            free(datet->text);
            datet->text = dupli_st("");
            datet->isText = false;
        }   
        else if (strncmp(fVal, "--", 2) == 0 && strlen(fVal) == 6) {
            // Below it will Handle truncated date format 
            free(datet->date); 
            datet->date = dupli_st(fVal);
            free(datet->time); 
            datet->time = dupli_st("");
            free(datet->text);
            datet->text = dupli_st("");
            datet->isText = false;
        }
        else if (strspn(fVal, "0123456789") == strlen(fVal) && strlen(fVal) == 8) { 
            // Below it will handles full numeric date format 
            free(datet->date);
            datet->date = dupli_st(fVal);
        }
        else { 
            // Below it will treat it as text
            datet->isText = true;
            free(datet->text);
            datet->text = dupli_st(fVal);
        } 
    }
    // So if two values exist, then it will parse both separately.
    else {
        if (fVal[0] == 'T') {
            free(datet->time);
            datet->time = dupli_st(fVal + 1);
        } else if (strchr(fVal, 'T') != NULL) {
            char* tP = strchr(fVal, 'T');
            size_t dLen = tP - fVal;
            char* dPart = dupli_st_2(fVal, dLen);
            char* tPart = dupli_st(tP + 1);
            if (!dPart || !tPart) {
                free(dPart); free(tPart);
                deleteDate(datet);
                return NULL;
            }
            // It will check if the time ends with Z utc.
            size_t tLen = strlen(tPart);
            if (tLen > 0 && tPart[tLen - 1] == 'Z') {
                tPart[tLen - 1] = '\0';
                datet->UTC = true;
            }
            free(datet->date); datet->date = dPart;
            free(datet->time); datet->time = tPart;
        } else {
            // Here it will handle cases where the first value is a valid date or text.
            if (strncmp(fVal, "--", 2) == 0 && strlen(fVal) == 6) {
                free(datet->date);
                datet->date = dupli_st(fVal);
            } else if (strspn(fVal, "0123456789") == strlen(fVal) && strlen(fVal) == 8) {
                free(datet->date);
                datet->date = dupli_st(fVal);
            } else {
                datet->isText = true;
                free(datet->text);
                datet->text = dupli_st(fVal);
            }
        }
        // It will process the second value as a time component here.
        if (sVal[0] == 'T') {
            free(datet->time);
            datet->time = dupli_st(sVal + 1);
        } else if (strchr(sVal, 'T') != NULL) {
            char* tP = strchr(sVal, 'T');
            char* tPart = dupli_st(tP + 1);
            if (!tPart) {
                deleteDate(datet);
                return NULL;
            }
            free(datet->time); datet->time = tPart;
        } else {
            free(datet->time);
            datet->time = dupli_st(sVal);
        }
    }
    return datet;
}



/* The following below will processes a single line of a vCard and updates the Card structure accordingly. */
VCardErrorCode processL(char* l, Card* c, bool* hB, bool* hE, bool* hasV) {
    // This will check for the BEGIN:VCARD line, and if found mark it as found
    if (strcmp(l, "BEGIN:VCARD") == 0) {
        *hB = true;
        return OK;
    }
    // This will check for the END:VCARD line, and if found it will mark it as found
    if (strcmp(l, "END:VCARD") == 0) {
        *hE = true;
        return OK;
    }
    // This will check for the VERSION field, while ensuring it is 4.0
    if (strncmp(l, "VERSION:", 8) == 0) {
        if (strcmp(l + 8, "4.0") != 0)  // If it is not version 4.0, then it will return invalid card error
            return INV_CARD;
        *hasV = true;
        return OK;
    }
    // Below it will find the position of the first colon : which separates the header from the value
    char* col = strchr(l, ':');
    if (!col || col == l)  // It will be invalid property if no colon or colon is at the start
        return INV_PROP;

    // This will extract the header portion and everything before the colon
    int preL = col - l;
    char* preS = malloc(preL + 1);
    if (!preS) return OTHER_ERROR;
    for (size_t i = 0; i < preL; i++) {
        preS[i] = l[i];
    }
    preS[preL] = '\0';  // The following will null terminate the header string

    // This will Extract the value portion and everything after the colon below 
    char* valP = malloc(strlen(col + 1) + 1);
    if (!valP) {
        free(preS);
        return OTHER_ERROR;
    }
    strcpy(valP, col + 1);

    /* --- Parse header --- */
    char* sPtr = NULL;
    char* tok = strtok_r(preS, ";", &sPtr);
    if (!tok) {  // If there's no valid header, return an invalid property error
        free(preS);
        free(valP);
        return INV_PROP;
    }

    // Below it will check if there is a group prefix separated by .
    char* dP = strchr(tok, '.');
    char* gStr = NULL;
    char* pN = NULL;
    if (dP) {
        *dP = '\0';
        gStr = dupli_st(tok);     // Here it will cxtract group name
        pN = dupli_st(dP + 1); // Here it will extract property name
    } else {
        gStr = dupli_st("");   // This will be a default empty group name
        pN = dupli_st(tok); // This is the property name
    }

    // Below it will ensure memory allocation was successful
    if (!gStr || !pN) {
        free(preS);
        free(valP);
        free(gStr);
        free(pN);
        return OTHER_ERROR;
    }

    // Below it will allocate memory for the property structure
    Property* newpr = malloc(sizeof(Property));
    if (!newpr) {
        free(preS);
        free(valP);
        free(gStr);
        free(pN);
        return OTHER_ERROR;
    }

    // Below it will initialize the property fields
    newpr->group = gStr;
    newpr->name = pN;
    newpr->parameters = initializeList(parameterToString, deleteParameter, compareParameters);
    newpr->values = initializeList(valueToString, deleteValue, compareValues);

    // Below it will ensure list initialization succeeded
    if (!newpr->parameters || !newpr->values) {
        free(preS);
        free(valP);
        deleteProperty(newpr);
        return OTHER_ERROR;
    }

    // Below it will parse any additional parameters in the header
    while ((tok = strtok_r(NULL, ";", &sPtr)) != NULL) {
        char* eqPos = strchr(tok, '=');
        char* pNam = NULL;
        char* pVal = NULL;

        if (eqPos) {
            int pNamL = eqPos - tok;
            pNam = malloc(pNamL + 1);
            strncpy(pNam, tok, pNamL);
            pNam[pNamL] = '\0';
            pVal = dupli_st(eqPos + 1);
        } else {
            // No = means it's a parameter like "pref" or "home"
            pNam = dupli_st("TYPE");  // Default name
            pVal = dupli_st(tok);
        }

        if (!pNam || !pVal || strlen(pVal) == 0) {
            if (pNam) free(pNam);
            if (pVal) free(pVal);
            free(preS);
            free(valP);
            deleteProperty(newpr);
            return INV_PROP;
        }


        // Below it will create a new Parameter structure
        Parameter* param = malloc(sizeof(Parameter));
        if (!param) {
            free(preS);
            free(valP);
            free(pNam);
            free(pVal);
            deleteProperty(newpr);
            return OTHER_ERROR;
        }

        // Here below i am Assigning parameter fields and insert into the property's parameter list
        param->name = pNam;
        param->value = pVal;
        insertBack(newpr->parameters, param);
    }
    free(preS);

    // Below the following is the Parseing value part 
    char* sPtrV = valP;
    char* valTok;
    while ((valTok = strsep(&sPtrV, ";")) != NULL) {
        char* valStr = dupli_st(valTok);
        if (!valStr) {
            free(valP);
            deleteProperty(newpr);
            return OTHER_ERROR;
        }
        insertBack(newpr->values, valStr);
    }
    free(valP);

    // Here below I am Inserting the property into the Card structure 
    if (strcmp(newpr->name, "FN") == 0) {  // Here the FN is a required field
        if (c->fn == NULL)
            c->fn = newpr;  // This will store FN property in the card structure
        else
            insertBack(c->optionalProperties, newpr);  // This will store duplicate FN properties in optionalProperties
    } else if (strcmp(newpr->name, "BDAY") == 0) {  
    //printf("DEBUG: Found BDAY property\n");
    DateTime* datet = parseDT(newpr->values);
    deleteProperty(newpr);
    
    if (!datet) {
        //printf("DEBUG: parseDT returned NULL for BDAY\n");
        return OTHER_ERROR;
    }
    
    //printf("DEBUG: Stored BDAY: Date: '%s', Time: '%s', UTC: %d, isText: %d, Text: '%s'\n", datet->date, datet->time, datet->UTC, datet->isText, datet->text);
    
    c->birthday = datet;
}

     else if (strcmp(newpr->name, "ANNIVERSARY") == 0) {  
        // Here ANNIVERSARY should also be parsed as a DateTime object
        DateTime* datet = parseDT(newpr->values);
        deleteProperty(newpr);
        if (!datet)
            return OTHER_ERROR;
        c->anniversary = datet;
    } else {
        // Below it will store all other properties in the optionalProperties list
        insertBack(c->optionalProperties, newpr);
    }

    return OK;  // This will eeturn success
}

/* The following function duplicates a string up to a specified length. */
char* dupli_st_2(const char* sour, size_t maxL) {
    if (!sour) return NULL;  // This will return NULL if the input string is NULL.
    size_t numC = 0;  // Initialize a counter to track characters to copy.
    while (numC < maxL && sour[numC] != '\0')  // Iterate until maxL or null terminator.
        numC++;
    char* c = malloc(numC + 1);  // Allocate memory for the new string.
    if (!c) return NULL;  // Check if memory allocation was successful.
    for (size_t i = 0; i < numC; i++) {
        c[i] = sour[i];  // Copy each character.
    }
    c[numC] = '\0';  // Ensure null termination.
    return c;  // Return the duplicated string.
} 

/* The following below function will check if a property name is valid */
bool chkPN(const char* n) {
    if (!n) return false;

    /* Below are the list of valid property names */
    const char* validPs[] = {
        "FN", "N", "BDAY", "ANNIVERSARY", "ADR", "TEL", "EMAIL", "IMPP", "LANG",
        "TZ", "GEO", "TITLE", "ORG", "PHOTO", "LOGO", "SOUND", "URL", "KEY",
        "UID", "NOTE", "FBURL", "CALADRURI", "CALURI", "XML", "KIND", "GENDER", "CATEGORIES"
    };

    int validPC = sizeof(validPs) / sizeof(validPs[0]); // This will calculate the number of valid property names in the validPs array

    // Below it will iterate through the list of valid property names
    for (int i = 0; i < validPC; i++) {
        if (strcasecmp(n, validPs[i]) == 0) {
            return true;
        }
    }

     // Allow Apple proprietary extensions like X-ABADR, X-ABLabel
    if (strncasecmp(n, "X-AB", 4) == 0) return true;

    // Allow iOS-style grouped fields like item1.ADR, item3.URL, etc.
    if (strncasecmp(n, "item", 4) == 0) return true; 

    return false;
} 
/* The following below function will validate a DateTime struct */
bool isValDT(const DateTime* dt) {
    // Below it will check if the DateTime object is NULL
    if (!dt) return false;
    // Below it will check if the DateTime is stored as text
    if (dt->isText) {
        // Below it will ensure that text based DateTime has only a text value
        if ((dt->date && strlen(dt->date) > 0) || (dt->time && strlen(dt->time) > 0) || dt->UTC || !dt->text || strlen(dt->text) == 0) // Text should not be empty
            return false;
    } else {  // Below it will validate a non-text DateTime
        // Below it will check that text is empty for non text DateTime
        if (strlen(dt->text) != 0)
            return false;

        // Below it will check if only a date is present, it must be exactly 8 digits
        if (strlen(dt->date) > 0 && strlen(dt->time) == 0) {
            // Allow full 8-digit date OR truncated date like --MMDD
            if (strlen(dt->date) == 8 || (strlen(dt->date) == 6 && strncmp(dt->date, "--", 2) == 0));  // valid
            else
                return false;
        }

        // Below it will check if only a time is present, it must be exactly 6 digits
        else if (strlen(dt->time) > 0 && strlen(dt->date) == 0) {
            if (strlen(dt->time) != 6)
                return false;
        } 
        // Below it will check if both date and time are present, they must be 8 and 6 digits 
        else if (strlen(dt->date) > 0 && strlen(dt->time) > 0) {
            if (strlen(dt->date) != 8 || strlen(dt->time) != 6)
                return false;
        } 
        // Below it will return false if none of the valid conditions are met
        else {
            return false;
        }
    }
    
    return true;  // This will be true if all conditions pass, therefore the DateTime is valid
}


/* The below function will validate a single property */
VCardErrorCode valP(const Property* prop) {  
    if (prop && prop->name)
        //printf("Validating property: %s\n", prop->name);

    // Below it will check if the property itself is NULL
    if (!prop)
        return INV_PROP; 
    // Below it will check if the property name is NULL or empty
    if (!prop->name || strlen(prop->name) == 0)
        return INV_PROP; 
    // Below it will check if the property group is NULL
    if (!prop->group) 
        return INV_PROP; 
    // Below it will check if the property values list is NULL or empty
    if (!prop->values || getLength(prop->values) == 0)
        return INV_PROP; 
    // Below it will check if the parameters list is NULL
    if (!prop->parameters)
        return INV_PROP;
    
    // Below it will iterate over the list of parameters and validate each
    ListIterator pI = createIterator(prop->parameters);
    void* itemP;
    while ((itemP = nextElement(&pI)) != NULL) {
        Parameter* par = (Parameter*) itemP;
        if (!par->name || strlen(par->name) == 0 ||
            !par->value || strlen(par->value) == 0)
            return INV_PROP;
    }
    
    // Below the VERSION property should not appear in optionalProperties 
    if (strcasecmp(prop->name, "VERSION") == 0)
        return INV_CARD;
    
    // Below the BDAY and ANNIVERSARY properties must not appear in optionalProperties
    if (strcasecmp(prop->name, "BDAY") == 0 || strcasecmp(prop->name, "ANNIVERSARY") == 0) {
        //printf("BDAY or ANNIVERSARY found in optionalProperties: %s\n", prop->name);
        return INV_DT;
    }
    
    if (!chkPN(prop->name)) {
        //printf("Rejected unknown property name: %s\n", prop->name);
        return INV_PROP;
    }

    
    // Below the for the property N, it will ensure exactly 5 components
    if (strcasecmp(prop->name, "N") == 0) {
        if (getLength(prop->values) != 5)
            return INV_PROP;
    }
    
    return OK;
}
// The folloiwng below function will format a DateTime property into vCard format and writes it to the file.
bool forDT(FILE* out, const char* label, const DateTime* dt) {
    // Below it will check if any parameter is NULL
    if (!dt || !label || !out) return false;
    // Below it will handle text based DateTime formatting
    if (dt->isText) {
        return fprintf(out, "%s;VALUE=text:%s\r\n", label, dt->text) >= 0;
    }
    // Below it will handle full DateTime format 
    if (strlen(dt->date) > 0 && strlen(dt->time) > 0) {
        return fprintf(out, "%s:%sT%s\r\n", label, dt->date, dt->time) >= 0;
    }
    // Below it will handle Date only format 
    if (strlen(dt->date) > 0) {
        return fprintf(out, "%s:%s\r\n", label, dt->date) >= 0;
    }
    // Below it will handle Time only format 
    if (strlen(dt->time) > 0) {
        return fprintf(out, "%s:%s\r\n", label, dt->time) >= 0;
    }
    // Below it will return false if DateTime format is invalid
    return false;
}