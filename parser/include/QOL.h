#ifndef QOL_H
#define QOL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include "CalendarParser.h"

// DO NOT DROP LINEBUFFERINTERVAL TO BELOW 4, EVER, UNDER ANY CIRCUMSTANCES
#define LINEBUFFERINTERVAL 85

void emptyInputBuffer(char *lineBuffer, char inputBuff[3], int* pos);

// Initializes the buffer with 3 new values from the file
bool initializeInputBuffer(FILE** fpp, char *lineBuffer, char inputBuff[3], int* pos);

// Shifts the input buffer by 1 character; toAdd is the character added to the input buffer, and inputBuff[0] is added to the linebuffer
bool shiftInputBuffer(FILE** fpp, char *lineBuffer, char inputBuff[3], int* pos, char toAdd);

// Returns false if fp is at EOF or is NULL, returns true if any input has been obtained, even if zero length
// *LB always has a new pointer assigned, of at least LINEBUFFERINTERVAL bytes; this is true even if the function fails
/*
    Error Codes:
    -1: malloc/realloc fails
    -2 file reading fails
    -3 newline is present without supporting /r
*/
int getLine(FILE** fpp, char **LB);

//Uppercases a string
void strUp(char* A);

//Compares two strings, case insensitively
int nCaseCmp(char* A, char* B);

//Finds the first semicolon or colon and returns its position in the array
int findColon(char* line);

//Copies a specific part of a string from one string to the beginning of another
bool strcrop(char* copyTo, char* copyFrom, int start, int end);

//Self Explanitory
bool isValidICSfilename(char* fileName);

//Generates a property containing nothing except for null terminators
ICalErrorCode nullProp(Property **toMake);

//Generates a Property from a string. Returns OK, OTHER_ERROR or a nullProp
ICalErrorCode makeProp(Property **toMake, char* line);

//Adds a property to an event (UNTESTED)
ICalErrorCode addPropToEvent(Event* addTo, Property* toAdd);

//Makes an Event. Returns OTHER_ERROR or OK
ICalErrorCode makeEvent(Event** toMake);

//Adds a property to an event (UNTESTED)
ICalErrorCode addPropToAlarm(Alarm* addTo, Property* toAdd);

//Makes an Alarm. Returns OTHER_ERROR or OK
ICalErrorCode makeAlarm(Alarm** toMake);

//Makes an Date_Time. Returns OTHER_ERROR, OK or INV_DT
ICalErrorCode makeDT(DateTime** toMake, char* line);

//Functions that write to an open file pointer fp.

ICalErrorCode writeStringToFile(FILE* fp, char* str, int linefold);

ICalErrorCode writeDTToFile(FILE* fp, char* name, DateTime *obj, int linefold);

ICalErrorCode writePropToFile(FILE* fp, Property* obj, int linefold);

ICalErrorCode writeAlarmToFile(FILE* fp, Alarm* obj, int linefold);

ICalErrorCode writeEventToFile(FILE* fp, Event* obj, int linefold);

ICalErrorCode writeCalToFile(FILE* fp, const Calendar* obj, int linefold);

//Print functions that print use printf and do not return anything
void printfdatetime(DateTime* dt);

void printfproperty(Property* P);

void printfalarm(Alarm* A);

void printfevent(Event* E);

void printfcalendar(Calendar* Cal);

//Checks to see if the version is valid
bool isValidVersion(char* line);

//Given an error (toAdd) either adds or does not add the error, depending on the priority of the error
void adderr(ICalErrorCode* err, ICalErrorCode toAdd);

ICalErrorCode canBeAnalyzedAlarm(Alarm* obj);

ICalErrorCode canBeAnalyzedEvent(Event* obj);

ICalErrorCode canBeAnalyzed(const Calendar* obj);

bool baseCheckDT(DateTime obj);

bool baseCheckProp(Property* obj);

ICalErrorCode baseCheckAlarm(Alarm* obj);

ICalErrorCode baseCheckEvent(Event* obj);

ICalErrorCode baseCheck(const Calendar* obj);

bool isValidPropCheck(char* propNme, char* validProps[], int propsValue[], int size);

ICalErrorCode propCheckAlarm(Alarm* obj);

ICalErrorCode propCheckEvent(Event* obj);

ICalErrorCode propCheck(const Calendar* obj);

Property* getPropFromEvent(const Event* obj, char* toFind);

int findColonAfter(char* line, int pos);

int findCharAfter(char* line, char toFind, int pos);

char* nullJSON();

char* nullJSONList();

//char** extractJsonProperty(char* json, int pos);
#endif