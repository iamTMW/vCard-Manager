/* 
* Name: Taha Mohyuddin
* Student ID: 1275575 
* Date: 9 - Mar - 2025
* Course: CIS2750
*/

#define _POSIX_C_SOURCE 200809L 
#include <stdbool.h> 
#include <strings.h> 
#include "VCParser.h" 
#include "VCHelpers.h"
#include "LinkedListAPI.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 


/**
 * The below function parses a vCard file and creates a Card object with its properties and values.
 */
VCardErrorCode createCard(char* fileName, Card** obj) { 
    char l[256];  // This is the buffer to store lines from the file
    char acc[512] = ""; // This is the accumulator to store unfolded lines  
    // Below are the boolean flags to track essential vCard elements
    bool hB = false;
    bool hE = false;
    bool hasV = false;
    VCardErrorCode stat = OK;  // This will initialize return code as OK

    if (!fileName || !obj || strlen(fileName) == 0) {
        if (obj) *obj = NULL;  // This will ensure obj is set to NULL if invalid
        return INV_FILE;  // This will be returned if invalid file error
    }

    // Below i have Validated the file extension for .vcf or .vcard
    if (!fileVal(fileName)) {
        *obj = NULL;
        return INV_FILE;
    }

    // Below it will open file in binary mode to enforce CRLF endings 
    FILE *fp = fopen(fileName, "rb");
    if (!fp) {  // Here if file cannot be opened, it will return an error
        *obj = NULL;
        return INV_FILE;
    }

    // Below it will allocate memory for the Card structure
    *obj = malloc(sizeof(Card));
    if (!*obj) {  // Here if memory allocation fails, it will close file and return error
        fclose(fp);
        return OTHER_ERROR;
    }

    // Below i have initialized Card fields
    (*obj)->fn = NULL;  // Here the full name property has been set to NULL
    (*obj)->optionalProperties = initializeList(propertyToString, deleteProperty, compareProperties);
    (*obj)->birthday = NULL;  // Here the birthday field has been set to NULL
    (*obj)->anniversary = NULL;  // Here the anniversary field has been set to NULL

    // Below it will check if optional properties list was created successfully
    if (!(*obj)->optionalProperties) {
        free(*obj);  // This will free allocated memory
        fclose(fp);  // This will close file
        return OTHER_ERROR;
    }


    // Below it will read the file line by line
    while (fgets(l, sizeof(l), fp)) {
        // Below it will enforce CRLF line endings
        int len = strlen(l);
        if (len < 2 || !(l[len - 2] == '\r' && l[len - 1] == '\n')) {
            stat = INV_CARD; // This will return invalid card if the line doesn't end with CRLF
            break;
        }

        // Below it will remove CRLF from the end of the line
        if (len > 0 && (l[len - 1] == '\n' || l[len - 1] == '\r')) {
            l[len - 1] = '\0';
            len--; // This will update the length after removal
        }
        if (len > 0 && (l[len - 1] == '\n' || l[len - 1] == '\r')) {
            l[len - 1] = '\0';
        }


        // Below it will handle line folding lines starting with space or tab continue the previous line
        if (l[0] == ' ' || l[0] == '\t') {
            if (strlen(acc) + strlen(l + 1) < sizeof(acc))  
                strcat(acc, l + 1);  // This will append folded line to accumulator
            else {
                stat = OTHER_ERROR;  // Here if accumulated length exceeds buffer, then it will return error
                break;
            }
        } else {  
            // Below it will rocess the accumulated line before starting a new one
            if (strlen(acc) > 0) {
                stat = processL(acc, *obj, &hB, &hE, &hasV);
                if (stat != OK)
                    break;
                acc[0] = '\0';  // This will reset accumulator
            }
            strncpy(acc, l, sizeof(acc) - 1);  // This will copy current line into accumulator
            acc[sizeof(acc) - 1] = '\0';  // This will ensure null termination
        }
    }

    // Below it will process any remaining data in the accumulator
    if (stat == OK && strlen(acc) > 0)
        stat = processL(acc, *obj, &hB, &hE, &hasV);

    fclose(fp);  // This will close the file

    // Below it will validate if the card has required fields BEGIN, END, VERSION, and FN
    if (stat == OK) {
        if (!hB || !hE || !hasV || ((*obj)->fn == NULL))
            stat = INV_CARD;
    }

    // Below if an error occurred, it will clean up allocated memory
    if (stat != OK) {
        deleteCard(*obj);
        *obj = NULL;
    }

    // we cannot merge duplicate properties as it is required for Test 11 just heads up
    return stat;
}


/**
 * The below function frees all memory associated with a Card object, including its properties and values.
 */
void deleteCard(Card* obj) {
    if (obj == NULL) return;  // If the object is NULL, then there is nothing to delete
    if (obj->fn)  
        deleteProperty(obj->fn);  // Here it will free the full name  property if it exists
    freeList(obj->optionalProperties);  // Here it will free the list of optional properties
    if (obj->birthday)  
        deleteDate(obj->birthday);  // Here it will free the birthday field if it exists
    if (obj->anniversary)  
        deleteDate(obj->anniversary);  // Here it will free the anniversary field if it exists
    free(obj);  // Here it will free the Card structure itself
}

/* The below function converts a card object into a formatted string representation. */
char *cardToString(const Card *obj) {
    char *st = malloc(1000);  // This will allocate memory for the output string
    if (!st) return NULL;  // This will return NULL if memory allocation fails
    if (obj == NULL) return NULL;  // This will return NULL if the Card object is NULL
    // Below it will format the full name property, defaulting to N/A if missing
    sprintf(st, "vCard:\nFN: %s\n", obj->fn ? (char *)getFromFront(obj->fn->values) : "N/A");

    // Below it will append birthday if it exists
    if (obj->birthday) {
        char *bdayS = dateToString(obj->birthday);  // This will convert birthday to string
        strcat(st, "BDAY: ");
        strcat(st, bdayS);
        strcat(st, "\n");
        free(bdayS);  // This will free the allocated string for the birthday
    }

    // Below it will append anniversary if it exists
    if (obj->anniversary) {
        char *annivS = dateToString(obj->anniversary);  // Below it will convert anniversary to string
        strcat(st, "ANNIVERSARY: ");
        strcat(st, annivS);
        strcat(st, "\n");
        free(annivS);  // This will free the allocated string for the anniversary
    }

    // This will iterate through the optional properties and append them to the output string
    ListIterator lisit = createIterator(obj->optionalProperties);
    void *elem;
    while ((elem = nextElement(&lisit)) != NULL) {
        char *propStr = propertyToString(elem);  // This will convert property to string
        strcat(st, propStr);
        strcat(st, "\n");
        free(propStr);  // This will free the allocated string for the property
    }

    return st;  // This will return the formatted string representation of the Card object
}


// The below fuction returns a human readable string corresponding to a VCardErrorCode.
char *errorToString(VCardErrorCode err) {
    switch (err) {
        case OK: return "OK";  // Here the no errors, operation was successful
        case INV_CARD: return "Oops, Invalid Card Try Again";  // Here the vCard structure is incorrect or missing required fields 
        case INV_FILE: return "Oops, Invalid File Try Again";  // Here the file does not exist or is improperly formatted 
        case INV_DT: return "Oops, Invalid Date/Time Try Again";  // Here the DateTime field contains invalid data
        case INV_PROP: return "Oops, Invalid Property Try Again";  // Here the A property is incorrectly formatted or missing
        case WRITE_ERROR: return "Oops, Write Error Try Again";  // Here the error occurred while writing data to a file
        case OTHER_ERROR: return "Oops, Other Error Try Again";  // here catch all for unspecified errors
        default: return "Oops, Unknown Error Try Again";  // this handles unknown error codes
    }
}

/* The below function frees all memory associated with a Property object, including its fields and lists. */
void deleteProperty(void* toBeDeleted) {
    if (toBeDeleted == NULL)  
        return;  // So here if the property is NULL, there's nothing to delete
    Property* prop = (Property*) toBeDeleted;  // This will cast void pointer to Property type 
     if (prop->group != NULL)  
        free(prop->group);  // This will free the group field if allocated
    if (prop->name != NULL)  
        free(prop->name);  // This will free the name field if allocated
    if (prop->values != NULL)  
        freeList(prop->values);  // This will free the list of values
    if (prop->parameters != NULL)  
        freeList(prop->parameters);  // This will free the list of parameters

    free(prop);  // This will free the Property structure itself
}


// For the below function it compares two Properties by name in alphabetical order.
int compareProperties(const void* first, const void* second) {
    if (second == NULL || first == NULL)
        return 0; // This will return 0 if either input is NULL 
    Property* parameA = (Property*) first; // This will cast the first parameter to Property pointer
    Property* parameB = (Property*) second; // This will cast second parameter to Property pointer 
    char *str1 = parameA->name;
    char *str2 = parameB->name;
    // Below it will compare characters one by one
    while (*str1 != '\0' && *str2 != '\0') {
        if (*str1 < *str2) return -1; // Here str1 is smaller alphabetically
        if (*str1 > *str2) return 1;  // Here str1 is greater alphabetically
        str1++;
        str2++;
    }

    // We will come here If both strings are identical up to this point, if so then check their lengths
    if (*str1 == '\0' && *str2 == '\0') return 0; // Strings are equal
    if (*str1 == '\0') return -1; // str1 is shorter, so it comes first
    return 1; // str2 is shorter, so it comes first
}


// For the below function it converts a property object into a formatted string representation.
char* propertyToString(void* prop) {
    if (prop == NULL)
        return dupli_st("We have a NULL Property");  // This will return NULL property if input is NULL

    Property* p = (Property*) prop;  // This will cast input to property pointer
    char* re = malloc(1000);  // This will allocate memory for the resulting string
    if (!re)
        return NULL;  // This will return NULL if memory allocation fails
    // This will start with the property name followed by a colon
    sprintf(re, "%s:", p->name);

    // The below will create an iterator for the values list
    ListIterator lisit = createIterator(p->values);
    void* elem;
    // The following below will append each value to the string, separated by semicolons
    while ((elem = nextElement(&lisit)) != NULL) {
        strcat(re, (char*) elem);
        if (nextElement(&lisit) != NULL) {  // For this if there are more values, then it will add a semicolon separator
            strcat(re, ";");
        }
    }
    return re;  // This will return the formatted string
}

/* Below it will compares two Parameter objects by name in alphabetical order. */
int compareParameters(const void* first, const void* second) {
    if (second == NULL || first == NULL)
        return 0; // This will return 0 if either input is NULL (invalid comparison)
    Parameter* parameA = (Parameter*) first; // This will cast first parameter to parameter pointer
    Parameter* parameB = (Parameter*) second; // This will cast second parameter to parameter pointer
    return strcmp(parameA ->name, parameB->name);  // This will compare parameter names alphabetically
}

// This will return a formatted string representation of a parameter object.
char* parameterToString(void* param) {
    if (param == NULL)
        return dupli_st("We have a NULL Parameter"); // This will return NULL Parameter if input is NULL
    Parameter* parame = (Parameter*) param; // This will cast input to parameter pointer
    char* re = malloc(500); // This will allocate memory for the formatted string
    if (!re)
        return NULL; // This will return NULL if memory allocation fails
    sprintf(re, "%s=%s", parame->name, parame->value); // This will format the string as name=value
    return re;  // This will return the formatted string
}

// Frees a value stored as a dynamically allocated char
void deleteValue(void* toBeDeleted) {
    if (toBeDeleted == NULL)
        return; // If the value is NULL, there's nothing to free
    free((char*)toBeDeleted);  // This will free the allocated memory for the value
}

/*Compares two values (strings) in alphabetical order. */
int compareValues(const void* first, const void* second) {
    if (first == NULL || second == NULL)
        return 0; // This will return 0 if either input is NULL
    return strcmp((char*)first, (char*)second);
}

/* Returns a dynamically allocated string representation of a value. */
char* valueToString(void* val) {
    if (val == NULL)
        return dupli_st("We have a NULL Value"); // This will return NULL Value if input is NULL
    return dupli_st((char*)val); // This will duplicate and return the string value
}

/* This will return a formatted string representation of a DateTime object. */
char* dateToString(void* date) {
    if (date == NULL)  
        return dupli_st("We have NULL DateTime");  // This will return NULL DateTime if input is NULL

    DateTime* datet = (DateTime*) date;  // This will cast input to DateTime pointer

    char* re = malloc(500);  // This will allocate memory for the formatted string
    if (!re)  
        return NULL;  // This will return NULL if memory allocation fails

    // Below it will format the dateTime fields into a string
    sprintf(re, "Date: %s, Time: %s, UTC: %d, isText: %d, Text: %s", datet->date ? datet->date : "", datet->time ? datet->time : "", datet->UTC, datet->isText,  datet->text ? datet->text : "");  

    return re;  // Return the formatted string
}

/*Frees all memory associated with a DateTime object, including its fields. */
void deleteDate(void* toBeDeleted) {
    if (toBeDeleted == NULL)  
        return;  // This check if the DateTime object is NULL, there's nothing to free

    DateTime* datet = (DateTime*) toBeDeleted;  // This will cast void pointer to DateTime type
    if (datet->date != NULL)
        free(datet->date);  // This will free the allocated date string

    if (datet->time != NULL)  
        free(datet->time);  // This will free the allocated time string

    if (datet->text != NULL)  
        free(datet->text);  // This will free the allocated text string

    free(datet);  // This will free the DateTime structure itself
}

// This will compare two DateTime objects.
// For now, this is a stub that always returns 0.
int compareDates(const void* first, const void* second) {
    return 0; // This is an placeholder implementation, does not perform actual comparison
}

// The below function, frees all memory associated with a Parameter object, including its name and value.
void deleteParameter(void* toBeDeleted) {
    if (toBeDeleted == NULL)  
        return;  // This will check if the parameter is NULL, there's nothing to delete

    Parameter* param = (Parameter*) toBeDeleted;  // This will cast void pointer to Parameter type

    if (param->name != NULL)  
        free(param->name);  // This will free the name field if it was allocated

    if (param->value != NULL)  
        free(param->value);  // This will free the value field if it was allocated

    free(param);  // This will free the Parameter structure itself
} 

/* The below function will validate a vCard object to ensure it conforms to vCard 4.0 specifications */
VCardErrorCode validateCard(const Card* card) {
     if (!card || !card->fn || !card->optionalProperties)  // This will ensure essential components exist
        return INV_CARD;
    
    // Here Below it will validate each property in optionalProperties and count occurrences of N and KIND 
    int nCou = 0;
    int kCou = 0; 
    void* propI;
    for (ListIterator it = createIterator(card->optionalProperties); (propI = nextElement(&it)) != NULL;) {
        Property* prop = (Property*) propI;
        VCardErrorCode code = valP(prop);  // This will validate each property
        if (code != OK) {
            return code;
        }
        if (strcasecmp(prop->name, "N") == 0) {  // This will count occurrences of N property
            nCou++;
        }
        if (strcasecmp(prop->name, "KIND") == 0) { // This will count occurrences of KIND property
            kCou++;
        }
    }
    if (nCou > 1 || kCou > 1)  // This will ensure N and KIND properties do not appear more than once
    return INV_PROP;
    
    /* Below it will validate birthday DateTime if present */
    if (card->birthday) {
        if (!isValDT(card->birthday))  // This will Ensure BDAY follows DateTime rules
            return INV_DT;
    }
    
    /* Below it will validate anniversary DateTime if present */
    if (card->anniversary) {
        if (!isValDT(card->anniversary))  // This will ensure ANNIVERSARY follows DateTime rules
            return INV_DT;
    } 

    /* Below it will validate the FN property */
    if (!card->fn->name || strlen(card->fn->name) == 0)  // Here FN must have a non empty name
        return INV_PROP;
    if (!card->fn->values || getLength(card->fn->values) == 0)  // Here FN must have at least one value
        return INV_PROP;
    
    return OK;  // If all checks pass, then it will return OK
}

/* The Below function serializes a Card object into a .vcf file following the vCard 4.0 specification. */
VCardErrorCode writeCard(const char* fileName, const Card* obj) {
    // Below it will check if the fileName or if the Card object is NULL
    if (fileName == NULL || obj == NULL) {
        return WRITE_ERROR;
    }

    // Below it will open file for writing in binary mode
    FILE* out = fopen(fileName, "wb");
    if (!out) {
        return WRITE_ERROR;
    }

    // Below it will write BEGIN and VERSION at the start
    if (fprintf(out, "BEGIN:VCARD\r\nVERSION:4.0\r\n") < 0) {
        fclose(out);
        return WRITE_ERROR;
    }

    // Below it will validate FN before writing any properties
    if (!obj->fn || !obj->fn->name || strlen(obj->fn->name) == 0 || !obj->fn->values || getLength(obj->fn->values) == 0) {
        fclose(out);
        return WRITE_ERROR;
    }

    // Below it will write the FN property
    if (obj->fn->group && strlen(obj->fn->group) > 0) {
        if (fprintf(out, "%s.", obj->fn->group) < 0) {
            fclose(out);
            return WRITE_ERROR;
        }
    }
    if (fprintf(out, "%s:", obj->fn->name) < 0) {
        fclose(out);
        return WRITE_ERROR;
    }

    // Below it will iterate over FN values using index based approach
    ListIterator fnI = createIterator(obj->fn->values);
    void* v;
    int count = 0;
    while ((v = nextElement(&fnI)) != NULL) {
        if (count > 0 && fprintf(out, ";") < 0) {
            fclose(out);
            return WRITE_ERROR;
        }
        if (fprintf(out, "%s", (char*)v) < 0) {
            fclose(out);
            return WRITE_ERROR;
        }
        count++;
    }
    if (fprintf(out, "\r\n") < 0) {
        fclose(out);
        return WRITE_ERROR;
    }

    // Below it will write BDAY and ANNIVERSARY using helper function
    if (obj->birthday && !forDT(out, "BDAY", obj->birthday)) {
        fclose(out);
        return WRITE_ERROR;
    }
    if (obj->anniversary && !forDT(out, "ANNIVERSARY", obj->anniversary)) {
        fclose(out);
        return WRITE_ERROR;
    }

    // Below it will write all optional properties
    if (obj->optionalProperties) {
        ListIterator itProp = createIterator(obj->optionalProperties);
        void* propI;
        while ((propI = nextElement(&itProp)) != NULL) {
            Property* prop = (Property*)propI;
            
            // Below it will write group if present
            if (prop->group && strlen(prop->group) > 0) {
                if (fprintf(out, "%s.", prop->group) < 0) {
                    fclose(out);
                    return WRITE_ERROR;
                }
            }
            if (fprintf(out, "%s:", prop->name) < 0) {
                fclose(out);
                return WRITE_ERROR;
            }

            // Below it will write property values
            ListIterator itVal = createIterator(prop->values);
            void* v; // dedcalring a variable pointer void 
            count = 0; // assigning a varaible count a value of 0
            // Below it will iterate over property values and write them to the file
            while ((v = nextElement(&itVal)) != NULL) {
                // Below it will add a semicolon before subsequent values if more than one exists
                if (count > 0 && fprintf(out, ";") < 0) {
                    fclose(out);
                    return WRITE_ERROR;
                }
                // Below it will write the current value of the property
                if (fprintf(out, "%s", (char*)v) < 0) {
                    fclose(out);
                    return WRITE_ERROR;
                }
                // Below it will increment the count to track multiple values
                count++;
            }
            if (fprintf(out, "\r\n") < 0) {
                fclose(out);
                return WRITE_ERROR;
            }
        }
    }

    // Below it will write END:VCARD before closing the file
    if (fprintf(out, "END:VCARD\r\n") < 0) {
        fclose(out);
        return WRITE_ERROR;
    }

    // Below it will ensure all data is written before closing the file
    fflush(out);
    fclose(out);
    return OK;
} 
/* The below function will returns all optional properties from a valid vCard file, excluding FN, BDAY, and ANNIVERSARY */
char* getOptP(const char* filename) {
    static char buffer[10000] = "";  // This is static so the pointer is safe to return
    buffer[0] = '\0';  // This will clear the buffer before use

    // Below it will try creating the card from file
    Card* card;
    VCardErrorCode err = createCard((char*)filename, &card);
    if (err != OK) return NULL; // This will return NULL on failure

    // Below it will iterate over optional properties
    ListIterator it = createIterator(card->optionalProperties);
    void* elem;
    while ((elem = nextElement(&it)) != NULL) {
        Property* prop = (Property*)elem;

        // Below it will skip FN, BDAY, ANNIVERSARY since they are required and not optional
        if (strcasecmp(prop->name, "FN") == 0 ||
            strcasecmp(prop->name, "BDAY") == 0 ||
            strcasecmp(prop->name, "ANNIVERSARY") == 0)
            continue;
        // Below it will convert property to string and append to buffer
        char* pStr = propertyToString(prop);
        strcat(buffer, prop->name);
        strcat(buffer, ":");
        strcat(buffer, pStr);
        strcat(buffer, "\n");
        free(pStr); // This will clean up temporary string
    }

    deleteCard(card); // This will clean up card object
    return buffer; // This will return the final string of optional props
} 
/* The below function will create a new valid vCard file with only the FN /Full Name property*/
int createNewC(const char* fileName, const char* fullName) {
    // Below it will check for invalid or empty inputs
    if (!fileName || !fullName || strlen(fileName) == 0 || strlen(fullName) == 0) {
        return 1;
    }

    // Below it will ensure file extension is valid .vcf
    if (!fileVal(fileName)) return 1;
    
    // Below it will allocate memory for new Card
    Card* newCard = malloc(sizeof(Card));
    if (!newCard) return 1;

    // Below it will allocate and setup the FN property
    Property* fn = malloc(sizeof(Property));
    if (!fn) {
        free(newCard);
        return 1;
    }

    // Below it will set name to FN
    fn->name = malloc(strlen("FN") + 1);
    strcpy(fn->name, "FN");

    fn->group = NULL;
    fn->parameters = initializeList(parameterToString, deleteParameter, compareParameters);

    // Below it will add fullName to values list
    fn->values = initializeList(valueToString, deleteValue, compareValues);
    insertBack(fn->values, strdup(fullName));

    // Below it will assign fields to the new card
    newCard->fn = fn;
    newCard->optionalProperties = initializeList(propertyToString, deleteProperty, compareProperties);
    newCard->birthday = NULL;
    newCard->anniversary = NULL;

    // Below it wil validate the card before writing
    if (validateCard(newCard) != OK) {
        deleteCard(newCard);
        return 1;
    }

    // Below it will write the card to disk
    if (writeCard(fileName, newCard) != OK) {
        deleteCard(newCard);
        return 1;
    }

    deleteCard(newCard); // This will clean up memory
    return 0; // It will return this on success
}
