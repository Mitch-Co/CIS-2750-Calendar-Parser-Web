#include "QOL.h"
#include "LinkedListAPI.h"
#include "CalendarParser.h"

void emptyInputBuffer(char *lineBuffer, char inputBuff[3], int* pos)
{
    for(int i = 0; i < 3; i++)
    {
        if(inputBuff[i] != '\r' &&   inputBuff[i] != '\n')
        {
            lineBuffer[(*pos)] = inputBuff[i];
            (*pos)++;
            inputBuff[i] = '\n';
        }   
    }
}

bool initializeInputBuffer(FILE** fpp, char *lineBuffer, char inputBuff[3], int* pos)
{

    inputBuff[0] = '\n';
    inputBuff[1] = '\n';
    inputBuff[2] = '\n';

    char c = '0';
    FILE* fp = *fpp;
    for(int i = 0; i < 3; i++)
    {
        c = fgetc(fp);
        if(!feof(fp))
        {
            inputBuff[i] = c;
        }
        else
        {
            if(i > 0)
            {
                emptyInputBuffer(lineBuffer, inputBuff, pos);
            }
            return false;

        }
    }
    return true;
}


int getLine(FILE** fpp, char **LB)
{
    char inputBuff[3];
    int linePos = 0;
    char c = '0';
    bool hitsEOF = false;

    int lineBufferSize = LINEBUFFERINTERVAL;
    (*LB) = (char*)malloc(sizeof(char) * LINEBUFFERINTERVAL);
    if((*LB) == NULL)
    {
        return -1;
    }
    
    if(fpp == NULL)
    {
        return -2;
    }
    char* lineBuffer = *LB;
    
    FILE* fp = *fpp;
    fpos_t savedPos;

    if(fp == NULL)
    {
        return -2;
    }

    initializeInputBuffer(&fp, lineBuffer, inputBuff, &linePos);
    fgetpos(fp, &savedPos);

    while(true)
    {
        if(lineBufferSize <= linePos + 4)
        {
            (*LB) = realloc((*LB),(sizeof(char) * LINEBUFFERINTERVAL) + lineBufferSize);
            lineBufferSize += LINEBUFFERINTERVAL;
            if((*LB) == NULL)
            {
                return -1;
            }
            //printf("LineBufferSize = %d, linePos = %d\n",lineBufferSize,linePos);
        }
        if(inputBuff[0] == '\r' && inputBuff[1] == '\n')
        {
            if((inputBuff[2] == '\t' || inputBuff[2] == ' '))
            {
                if(!initializeInputBuffer(&fp, (*LB), inputBuff, &linePos))
                {
                    break;
                }
            }
            else
            {
                fsetpos(fp,&savedPos);
                break;
            }

        }
        else if (inputBuff[1] == '\n')
        {
            return -3;
        }
        fgetpos(fp, &savedPos);
        c = fgetc(fp);

        if(!feof(fp))
        {
            if(inputBuff[0] != '\r' && inputBuff[0] != '\n')
            {
                (*LB)[linePos] = inputBuff[0];
                linePos++;
            }
            inputBuff[0] = inputBuff[1];
            inputBuff[1] = inputBuff[2];
            inputBuff[2] = c;

        }
        else
        {
            hitsEOF = true;
            emptyInputBuffer((*LB), inputBuff, &linePos);
            break;
        }

    }
    (*LB)[linePos] = '\0';
    if(hitsEOF == true)
    {
        return 0;
    }
    
    return 1;

}

void strUp(char* A)
{
    if(A == NULL)
    {
        return;
    }

    for(int i = 0; i < strlen(A); i++)
    {
        A[i] = toupper(A[i]);
    }
}

int nCaseCmp(char* A, char* B)
{
    if(A == NULL || B == NULL)
    {
        return 0;
    }
    char* AA = malloc(sizeof(char) * (strlen(A) + 1));
    if(AA == NULL)
    {
        return 0;
    }
    strcpy(AA,A);

    char* BB = malloc(sizeof(char) * (strlen(B) + 1));
    if(BB == NULL)
    {
        free(AA);
        return 0;
    }
    strcpy(BB,B);

    int rtn = 0;
    
    strUp(AA);
    strUp(BB);

    rtn = strcmp(AA,BB);

    free(AA);
    free(BB);
    return rtn;
}

int findColon(char* line)
{
    if(line != NULL)
    {
        for(int i = 0; i < strlen(line); i++)
        {
            if(line[i] == ';' || line[i] == ':')
            {
                return i;
            }
        }
        return -1;
    }
    return -2;
}

bool strcrop(char* copyTo, char* copyFrom, int start, int end)
{
    if(copyTo == NULL || copyFrom == NULL)
    {
        return false;
    }
    
    int copyToIndex = 0;
    for(int i = start; i <= end; i++)
    {
        copyTo[copyToIndex] = copyFrom[i];
        copyToIndex++;
    }
    copyTo[copyToIndex] = '\0';

    return true;
    
}

bool isValidICSfilename(char* fileName)
{
    if(fileName == NULL)
    {
        return false;
    }
    
    int L = strlen(fileName)-1;
    if((L <= 3))
    {
        return false;
    }
    L = L - 3;

    if(!(fileName[L] == '.' && toupper(fileName[L + 1]) == 'I' && toupper(fileName[L + 2]) == 'C' && toupper(fileName[L + 3]) == 'S'))
    {
        return false;
    }
    return true;
}

ICalErrorCode nullProp(Property **toMake)
{
    Property* toReturn = (Property*)malloc(sizeof(Property) + 200);
    if(toReturn == NULL)
    {
        return OTHER_ERROR;
    }
    (toReturn->propDescr)[0] = '\0';
    (toReturn->propName)[0] = '\0';
    *toMake = toReturn;
    return OK;
}

ICalErrorCode makeProp(Property **toMake, char* line)
{
    if(line == NULL)
    {
        return nullProp(toMake);
    }
    int posOfSplit = findColon(line);

    int len = strlen(line);

    int size1 = posOfSplit + 1; //Includes terminating character
    int size2 = len - posOfSplit; //Includes terminating character

    //printf("SIZE 1 = %d, SIZE 2 = %d\n",size1,size2);

    if(size2 <= 0)
    {
        return nullProp(toMake);
    }
    if(size2 <= 0 || size1 >= 200 || size1 <= 1|| posOfSplit <= 0)
    {
        return nullProp(toMake);
    }

    Property* toReturn = (Property*)malloc(sizeof(Property) + (sizeof(char) * size2));
    
    if(toReturn == NULL)
    {
        return OTHER_ERROR;
    }

    strcrop(toReturn->propName,line,0,posOfSplit - 1);

    strcrop(toReturn->propDescr,line,posOfSplit + 1, strlen(line) - 1);

    (*toMake) = toReturn;

    return OK;

}

ICalErrorCode addPropToEvent(Event* addTo, Property* toAdd)
{
    if(addTo != NULL && toAdd != NULL)
    {
        insertBack((addTo->properties), (void*) toAdd);
        return OK;
    }
    else
    {
        return OTHER_ERROR;
    }
}
ICalErrorCode addAlarmToEvent(Event* addTo, Alarm* toAdd)
{
    if(addTo != NULL && toAdd != NULL)
    {
        insertBack((addTo->alarms), (void*) toAdd);
        return OK;
    }
    else
    {
        return OTHER_ERROR;
    }
}
ICalErrorCode makeEvent(Event** toMake)
{
    (*toMake) = (Event*)malloc(sizeof(Event));
    if(*toMake != NULL)
    {
        List* prop = initializeList(printProperty,deleteProperty,compareProperties);
        if(prop == NULL)
        {
            free(*toMake);
            return OTHER_ERROR;
        }
        (*toMake)->properties = prop;

        List*  alrm = initializeList(printAlarm,deleteAlarm,compareAlarms);
        if(alrm == NULL)
        {
            freeList(prop);
            free(*toMake);
            return OTHER_ERROR;
        }
        (*toMake)->alarms = alrm;
        
        return OK;
    }
    else
    {
        return OTHER_ERROR;
    }
}
ICalErrorCode addPropToAlarm(Alarm* addTo, Property* toAdd)
{
    if(addTo != NULL && toAdd != NULL)
    {
        insertBack((addTo->properties), (void*) toAdd);
        return OK;
    }
    else
    {
        return OTHER_ERROR;
    }
}

ICalErrorCode makeAlarm(Alarm** toMake)
{
    (*toMake) = (Alarm*)malloc(sizeof(Alarm));
    if(*toMake != NULL)
    {
        List* prop = initializeList(printProperty,deleteProperty,compareProperties);
        if(prop == NULL)
        {
            free(*toMake);
            return OTHER_ERROR;
            
        }
        (*toMake)->properties = prop;
        (*toMake)->trigger = NULL;
        return OK;
    }
    else
    {
        return OTHER_ERROR;
    }
}
ICalErrorCode makeDT(DateTime** toMake, char* line)
{
    int length = strlen(line);
    if(length > 16 || length < 15)
    {
        return INV_DT;
    }
    
    if(line[8] != 'T')
    {
        return INV_DT;
    };
    for(int i = 0; i < strlen(line); i++)
    {
        if(!isdigit(line[i]) && line[i] != 'T' && line[i] != 'Z')
        {
            return INV_DT;
        }
    }

    DateTime* toReturn = malloc(sizeof(DateTime));
    if(toReturn == NULL)
    {
        return OTHER_ERROR;
    }

    if(length == 16)
    {
        if(line[15] == 'Z')
        {
            toReturn->UTC = true;
        }
        else
        {
            deleteDate(toReturn);
            return INV_DT;
        }
    }
    else
    {
        toReturn->UTC = false;
    }

    strcrop(toReturn->date,line,0,7);
    strcrop(toReturn->time,line,9,14);

    *toMake = toReturn;

    return OK;

}
ICalErrorCode writeStringToFile(FILE* fp, char* str, int linefold)
{
    // fprintf(fp,"%s\r\n",str);
    int count = 0;
    for(int i = 0; i < strlen(str); i++)
    {
        if(!(linefold <= 0) && count >= linefold)
        {
            fputs("\r\n ",fp);
            count = 0;
        }
        fputc(str[i],fp);
        count++;
    }
    fputs("\r\n",fp);
    return OK;
}
ICalErrorCode writeDTToFile(FILE* fp, char* name, DateTime *obj, int linefold)
{
    char* str = malloc(sizeof(char)*(strlen(name) + 8 + 2 + 6 + 2)); //8 for Date, 6 for Time, 2 for 'T' and ':', 2 for UTC and \0
    if(str == NULL)
    {
        return OTHER_ERROR;
    }
    int err = 0;
    if(obj->UTC)
    {
        err = sprintf(str,"%s:%sT%sZ",name,obj->date,obj->time);
        if(err < 0)
        {
            free(str);
            return OTHER_ERROR;
        }
    }
    else
    {
        err = sprintf(str,"%s:%sT%s",name,obj->date,obj->time);
        if(err < 0)
        {
            free(str);
            return OTHER_ERROR;
        }
    }

    ICalErrorCode errr = OK;
    errr = writeStringToFile(fp, str, linefold);
    free(str);
    return errr;
}
ICalErrorCode writePropToFile(FILE* fp, Property* obj, int linefold)
{
    char* str = malloc(sizeof(char) * (strlen(obj->propName) + strlen(obj->propDescr) + 2));
    int err = 0;
    if(findColon(obj->propDescr) < 0)
    {
        err = sprintf(str,"%s:%s",obj->propName,obj->propDescr);
        if(err < 0)
        {
            free(str);
            return OTHER_ERROR;
        }
    }
    else
    {
        err = sprintf(str,"%s;%s",obj->propName,obj->propDescr);
        if(err < 0)
        {
            free(str);
            return OTHER_ERROR;
        }
    }
    ICalErrorCode errr = OK;
    errr = writeStringToFile(fp, str, linefold);
    free(str);
    return errr;
}
ICalErrorCode writeAlarmToFile(FILE* fp, Alarm* obj, int linefold)
{
    fprintf(fp,"BEGIN:VALARM\r\n");

    char* str1 = malloc(sizeof(char) * (strlen(obj->action) + 10));
    if(str1 == NULL)
    {
        return OTHER_ERROR;
    }
    if(sprintf(str1,"ACTION%c%s",(findColon(obj->action) < 0) ? ':':';',obj->action) <= 0)
    {
        free(str1);
        return WRITE_ERROR;
    }
    writeStringToFile(fp,str1,linefold);
    free(str1);
    
    char* str2 = malloc(sizeof(char) * (strlen(obj->trigger) + 15));
    if(str2 == NULL)
    {
        return OTHER_ERROR;
    }
    if(sprintf(str2,"TRIGGER%c%s",(findColon(obj->trigger) < 0) ? ':':';',obj->trigger) <= 0)
    {
        free(str2);
        return WRITE_ERROR;
    }
    writeStringToFile(fp,str2,linefold);
    free(str2);

    ListIterator itr1 = createIterator(obj->properties);
    void* temp;
    ICalErrorCode err = OK;
    while((temp = nextElement(&itr1)) != NULL)
    {
        Property* tmpName = (Property*)temp;
        err = writePropToFile(fp, tmpName, linefold);
        if(err != OK)
        {
            return err;
        }
    }
    fprintf(fp,"END:VALARM\r\n");
    return OK;
}
ICalErrorCode writeEventToFile(FILE* fp, Event* obj, int linefold)
{
    fprintf(fp,"BEGIN:VEVENT\r\n");

    char* str = malloc(sizeof(char) * (4 + sizeof(obj->UID)));
    if(str == NULL)
    {
        return OTHER_ERROR;
    }
    if(sprintf(str,"UID:%s",obj->UID) <= 0)
    {
        free(str);
        return WRITE_ERROR;
    }
    writeStringToFile(fp,str,linefold);
    free(str);
    
    writeDTToFile(fp,"DTSTAMP",&(obj->creationDateTime),linefold);
    writeDTToFile(fp,"DTSTART",&(obj->startDateTime),linefold);

    ListIterator itr1 = createIterator(obj->properties);
    ListIterator itr2 = createIterator(obj->alarms);

    void* temp;
    ICalErrorCode err = OK;

    while((temp = nextElement(&itr1)) != NULL)
    {
        Property* tmpName = (Property*)temp;
        err = writePropToFile(fp, tmpName, linefold);
        if(err != OK)
        {
            return err;
        }
    }
    while((temp = nextElement(&itr2)) != NULL)
    {
        Alarm* tmpName = (Alarm*)temp;
        err = writeAlarmToFile(fp, tmpName, linefold);
        if(err != OK)
        {
            return err;
        }
    }

    fprintf(fp,"END:VEVENT\r\n");
    return OK;
}
ICalErrorCode writeCalToFile(FILE* fp, const Calendar* obj, int linefold)
{
    /* CHECK AND CORRECT ERRORS */
    if(obj == NULL)
    {
        return WRITE_ERROR;
    }

    fprintf(fp,"BEGIN:VCALENDAR\r\n");
    
    if(linefold < 0)
    {
        linefold = 0;
    }

    /* WRITE CAL SPECIFIC STUFF TO FILE */
    char* str1 = malloc(sizeof(char) * (sizeof(float) + 100)); //100 Extra added for "VERSION", and because im unsure that sizeof(float) will work
    if(str1 == NULL)
    {
        return OTHER_ERROR;
    }
    if(sprintf(str1,"VERSION:%g",obj->version) <= 0)
    {
        free(str1);
        return WRITE_ERROR;
    }
    writeStringToFile(fp,str1,linefold);
    free(str1);
    
    char* str2 = malloc(sizeof(char) * (sizeof(obj->prodID)) + 1);
    if(str2 == NULL)
    {
        return OTHER_ERROR;
    }
    if(sprintf(str2,"PRODID:%s",obj->prodID) <= 0)
    {
        free(str2);
        return WRITE_ERROR;
    }
    writeStringToFile(fp,str2,linefold);
    free(str2);

    /* ITERATE/WRITE THROUGH EVENTS AND PROPERTIES */
    ListIterator itr1 = createIterator(obj->properties);
    ListIterator itr2 = createIterator(obj->events);

    void* temp;
    ICalErrorCode err = OK;
    while((temp = nextElement(&itr1)) != NULL)
    {
        Property* tmpName = (Property*)temp;
        err = writePropToFile(fp, tmpName, linefold);
        if(err != OK)
        {
            fclose(fp);
            return err;
        }
    }
    while((temp = nextElement(&itr2)) != NULL)
    {
        Event* tmpName = (Event*)temp;
        err = writeEventToFile(fp, tmpName, linefold);
        if(err != OK)
        {
            fclose(fp);
            return err;
        }
    }
    fprintf(fp,"END:VCALENDAR");
    fclose(fp);
    return OK;
}

void printfdatetime(DateTime* dt)
{
    printf("<DT> date = %s | time = %s | UTC %s <DT>\n",dt->date,dt->time,(dt->UTC) ? "true":"false");
}
void printfproperty(Property* P)
{
    printf("<Prop> NAME = %s (size = %lu)| DESC = %s (size = %lu) <Prop>\n",P->propName,strlen(P->propName),P->propDescr,strlen(P->propDescr));
}
void printfalarm(Alarm* A)
{
    printf("\nALARM:\n");
    printf("\nALARM:\nAction = %s\nTrigger = %s\n",A->action,A->trigger);
    ListIterator itr1 = createIterator(A->properties);
    void* temp;

    while((temp = nextElement(&itr1)) != NULL)
    {
        Property* tmpName = (Property*)temp;
        printfproperty(tmpName);
    }
    printf("END OF ALARM\n");
}
void printfevent(Event* E)
{
    printf("\nEVENT:\n\n");
    printf("UID = %s\n",E->UID);
    printf("Creation Date Time = ");
    printfdatetime(&E->creationDateTime);
    printf("\n");
    printf("Start Date Time = ");
    printfdatetime(&E->startDateTime);
    printf("\n");
    ListIterator itr1 = createIterator(E->properties);
    ListIterator itr2 = createIterator(E->alarms);

    void* temp;

    while((temp = nextElement(&itr1)) != NULL)
    {
        Property* tmpName = (Property*)temp;
        printfproperty(tmpName);
    }
    while((temp = nextElement(&itr2)) != NULL)
    {
        Alarm* tmpName = (Alarm*)temp;
        printfalarm(tmpName);
    }

    printf("END OF EVENT\n\n");

}
void printfcalendar(Calendar* Cal)
{
    if(Cal == NULL)
    {
        return;
    }
    ListIterator itr1 = createIterator(Cal->properties);
    ListIterator itr2 = createIterator(Cal->events);
    printf("<CALENDAR> VERSION = %f | PRODID = %s <CALENDAR>\n\n",Cal->version,Cal->prodID);

    void* temp;

    while((temp = nextElement(&itr1)) != NULL)
    {
        Property* tmpName = (Property*)temp;
        printfproperty(tmpName);
    }
    while((temp = nextElement(&itr2)) != NULL)
    {
        Event* tmpName = (Event*)temp;
        printfevent(tmpName);
    }

}

bool isValidVersion(char* line)
{
    if(strlen(line) == 1 && isdigit(line[0]))
    {
        return true;
    }
    if(strlen(line) < 3)
    {
        return false;
    }
    if(line[strlen(line) - 1] == '.' || line[0] == '.')
    {
        return false;
    }
    for(int i = 0; i < strlen(line); i++)
    {
        if(!isdigit(line[i]) && line[i] != '.')
        {
            return false;
        }
    }
    return true;
}

void adderr(ICalErrorCode* err, ICalErrorCode toAdd)
{
    ICalErrorCode tempErr = *err;
    if(toAdd == OK)
    {
        return;
    }
    if(toAdd == OTHER_ERROR)
    {
        *err = OTHER_ERROR;
        return;
    }
    if(toAdd == INV_CAL)
    {
        if(tempErr != OTHER_ERROR)
        {
            *err = INV_CAL;
            return;
        }
        return;
    }
    if(toAdd == INV_EVENT)
    {
        if(tempErr != INV_CAL && tempErr != OTHER_ERROR)
        {
            *err = INV_EVENT;
            return;
        }
        return;
    }
    if(toAdd == INV_ALARM)
    {
        if(tempErr != INV_CAL && tempErr != OTHER_ERROR && tempErr != INV_EVENT)
        {
            *err = INV_ALARM;
            return;
        }
        return;
    }
    *err = toAdd;
    return;
}

ICalErrorCode canBeAnalyzedAlarm(Alarm* obj)
{
    if(obj == NULL)
    {
        return INV_ALARM;
    }
    if(obj->properties == NULL)
    {
        return INV_ALARM;
    }
    
    return OK;
}

ICalErrorCode canBeAnalyzedEvent(Event* obj)
{
    if(obj == NULL)
    {
        return INV_EVENT;
    }
    if(obj->properties == NULL || obj->alarms == NULL)
    {
        return INV_EVENT;
    }

    ListIterator itr2 = createIterator(obj->alarms);

    void* temp;
    ICalErrorCode err = OK;

    while((temp = nextElement(&itr2)) != NULL)
    {
        Alarm* tmpName = (Alarm*)temp;
        err = canBeAnalyzedAlarm(tmpName);
        if(err != OK)
        {
            return err;
        }
    }
    return OK;
}

ICalErrorCode canBeAnalyzed(const Calendar* obj)
{
    if(obj == NULL)
    {
        return INV_CAL;
    }
    if(obj->prodID == NULL || obj->events == NULL || obj->properties == NULL)
    {
        return INV_CAL;
    }
    void* temp;
    ICalErrorCode err = OK;
    ListIterator itr2 = createIterator(obj->events);

    while((temp = nextElement(&itr2)) != NULL)
    {
        Event* tmpName = (Event*)temp;
        err = canBeAnalyzedEvent(tmpName);
        if(err != OK)
        {
            return err;
        }
    }
    return OK;
}

bool baseCheckDT(DateTime obj)
{
    for(int i = 0; i < 9; i++)
    {
        if((obj.date)[i] == '\0')
        {
            if(i == 0)
            {
                return false;
            }
            break;
        }
        if(i == 8)
        {
            return false;
        }
    }
    for(int i = 0; i < 7; i++)
    {
        if((obj.time)[i] == '\0')
        {
            if(i == 0)
            {
                return false;
            }
            break;
        }
        if(i == 6)
        {
            return false;
        }
    }
    if(strlen(obj.date) < 8)
    {
        return false;
    }
    if(strlen(obj.time) < 6)
    {
        return false;
    }
    return true;
}
bool baseCheckProp(Property* obj)
{
    if(obj == NULL)
    {
        return false;
    }
    for(int i = 0; i < 200; i++)
    {
        if((obj->propName)[i] == '\0')
        {
            if(i == 0)
            {
                return false;
            }
            break;
        }
        if(i == 199)
        {
            return false;
        }
    }
    if((obj->propDescr)[0] == '\0')
    {
        return false;
    }
    return true;
}
ICalErrorCode baseCheckAlarm(Alarm* obj)
{
    if(obj == NULL)
    {
        return INV_ALARM;
    }
    for(int i = 0; i < 200; i++)
    {
        if((obj->action)[i] == '\0')
        {
            if(i == 0)
            {
                return INV_ALARM;
            }
            break;
        }
        if(i == 199)
        {
            return INV_ALARM;
        }
    }
    if(obj->properties == NULL || obj->trigger == NULL)
    {
        return INV_ALARM;
    }
    if(strlen(obj->trigger) <= 0)
    {
        return INV_ALARM;
    }
    
    ListIterator itr1 = createIterator(obj->properties);
    void* temp;
    while((temp = nextElement(&itr1)) != NULL)
    {
        Property* tmpName = (Property*)temp;
        bool correct = baseCheckProp(tmpName);
        if(correct == false)
        {
            return INV_ALARM;
        }
    }
    return OK;
}
ICalErrorCode baseCheckEvent(Event* obj)
{
    if(obj == NULL)
    {
        return INV_EVENT;
    }
    if(obj->properties == NULL || obj->alarms == NULL)
    {
        return INV_EVENT;
    }
    for(int i = 0; i < 1000; i++)
    {
        if((obj->UID)[i] == '\0')
        {
            if(i == 0)
            {
                return INV_EVENT;
            }
            break;
        }
        if(i == 999)
        {
            return INV_EVENT;
        }
    }
    if(!baseCheckDT(obj->creationDateTime))
    {
        return INV_EVENT;
    }
    if(!baseCheckDT(obj->startDateTime))
    {
        return INV_EVENT;
    }
    ListIterator itr1 = createIterator(obj->properties);
    ListIterator itr2 = createIterator(obj->alarms);

    void* temp;
    ICalErrorCode err = OK;

    while((temp = nextElement(&itr1)) != NULL)
    {
        Property* tmpName = (Property*)temp;
        bool correct = baseCheckProp(tmpName);
        if(correct == false)
        {
            return INV_EVENT;
        }
    }
    while((temp = nextElement(&itr2)) != NULL)
    {
        Alarm* tmpName = (Alarm*)temp;
        err = baseCheckAlarm(tmpName);
        if(err != OK)
        {
            return err;
        }
    }
    return OK;
}
ICalErrorCode baseCheck(const Calendar* obj)
{
    if(obj == NULL)
    {
        return INV_CAL;
    }
    if(obj->prodID == NULL || obj->events == NULL || obj->properties == NULL)
    {
        return INV_CAL;
    }
    for(int i = 0; i < 1000; i++)
    {
        if((obj->prodID)[i] == '\0')
        {
            if(i == 0)
            {
                return INV_CAL;
            }
            break;
        }
        if(i == 999)
        {
            return INV_CAL;
        }
    }
    if(getLength(obj->events) <= 0 || getLength(obj->properties) < 0)
    {
        return INV_CAL;
    }

    void* temp;
    ICalErrorCode err = OK;
    ListIterator itr1 = createIterator(obj->properties);
    ListIterator itr2 = createIterator(obj->events);
    while((temp = nextElement(&itr1)) != NULL)
    {
        Property* tmpName = (Property*)temp;
        bool correct = baseCheckProp(tmpName);
        if(correct == false)
        {
            return INV_CAL;
        }
    }
    while((temp = nextElement(&itr2)) != NULL)
    {
        Event* tmpName = (Event*)temp;
        adderr(&err, baseCheckEvent(tmpName));
    }

    return err;
}

bool isValidPropCheck(char* propNme, char* validProps[], int propsValue[], int size)
{
    for(int i = 0; i < size; i++)
    {
        if(nCaseCmp(validProps[i],propNme) == 0)
        {
            if(propsValue[i] != 0)
            {
                if(propsValue[i] > 0)
                {
                    propsValue[i]--;
                }
                return true;
            }
            else
            {
                return false;
            }
            
        }
        if(i == (size - 1))
        {
            return false;
        }
    }
    return false;
}

ICalErrorCode propCheckAlarm(Alarm* obj)
{
    if(obj == NULL)
    {
        return INV_ALARM;
    }
    if(obj->properties == NULL)
    {
        return INV_ALARM;
    }
    if(nCaseCmp(obj->action,"AUDIO") != 0)
    {
        return INV_ALARM;
    }

    char* propNames[3] = {"DURATION","REPEAT","ATTACH"};
    int propSizes[3] = {1,1,1};
    ListIterator itr1 = createIterator(obj->properties);

    void* temp;
    bool hasDur = false;
    bool hasRep = false;
    while((temp = nextElement(&itr1)) != NULL)
    {
        Property* tmpName = (Property*)temp;
        for(int i = 0; i < 3; i++)
        {
            if(nCaseCmp(propNames[i],tmpName->propName) == 0)
            {
                if(propSizes[i] != 0)
                {
                    if(propSizes[i] > 0)
                    {
                        propSizes[i]--;
                        if(i == 0)
                        {
                            hasDur = true;
                        }
                        else if(i == 1)
                        {
                            hasRep = true;
                        }
                    }
                    break;
                }
                else
                {
                    return INV_ALARM;
                }
            
            }
            if(i == 2)
            {
                return INV_ALARM;
            }
        }
    }
    if((hasDur && hasRep) || !(hasDur || hasRep))
    {
        return OK;
    }
    else
    {
        return INV_ALARM;
    }
    
}

ICalErrorCode propCheckEvent(Event* obj)
{
    if(obj == NULL)
    {
        return INV_EVENT;
    }
    if(obj->properties == NULL || obj->alarms == NULL)
    {
        return INV_EVENT;
    }
    //dtend is index 15 and duration is 16
    char* propNames[27] = 
    {"CLASS",
    "CREATED",
    "DESCRIPTION",
    "GEO",
    "LAST-MODIFIED",
    "LOCATION",
    "ORGANIZER",
    "PRIORITY",
    "SEQUENCE",
    "STATUS",
    "SUMMARY",
    "TRANSP",
    "URL",
    "RECURRENCE-ID",
    "RRULE",
    "DTEND",
    "DURATION",
    "ATTACH",
    "ATTENDEE",
    "CATEGORIES",
    "COMMENT",
    "CONTACT",
    "EXDATE",
    "REQUEST-STATUS",
    "RELATED-TO",
    "RESOURCES",
    "RDATE"};
    int propSizes[27] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
    ListIterator itr1 = createIterator(obj->properties);
    ListIterator itr2 = createIterator(obj->alarms);

    void* temp;
    ICalErrorCode err = OK;

    while((temp = nextElement(&itr1)) != NULL)
    {
        Property* tmpName = (Property*)temp;
        for(int i = 0; i < 27; i++)
        {
            if(nCaseCmp(propNames[i],tmpName->propName) == 0)
            {
                if(propSizes[i] != 0)
                {
                    if(propSizes[i] > 0)
                    {
                        propSizes[i]--;
                        if(i == 15)
                        {
                            propSizes[16] = 0;
                        }
                        else if(i == 16)
                        {
                            propSizes[15] = 0;
                        }
                    }
                    break;
                }
                else
                {
                    return INV_EVENT;
                }
            
            }
            if(i == 26)
            {
                return INV_EVENT;
            }
        }
    }
    while((temp = nextElement(&itr2)) != NULL)
    {
        Alarm* tmpName = (Alarm*)temp;
        err = propCheckAlarm(tmpName);
        if(err != OK)
        {
            return err;
        }
    }
    return OK;
}

ICalErrorCode propCheck(const Calendar* obj)
{
    if(obj == NULL)
    {
        return INV_CAL;
    }
    if(obj->prodID == NULL || obj->events == NULL || obj->properties == NULL)
    {
        return INV_CAL;
    }

    char* propNames[2] = {"CALSCALE","METHOD"};
    int propSizes[2] = {1, 1};
    void* temp;
    ICalErrorCode err = OK;
    ListIterator itr1 = createIterator(obj->properties);
    ListIterator itr2 = createIterator(obj->events);
    while((temp = nextElement(&itr1)) != NULL)
    {
        Property* tmpName = (Property*)temp;
        if(!isValidPropCheck(tmpName->propName, propNames, propSizes, 2))
        {
            return INV_CAL;
        }
        
    }
    while((temp = nextElement(&itr2)) != NULL)
    {
        Event* tmpName = (Event*)temp;
        adderr(&err, propCheckEvent(tmpName));

    }
    return err;

}

Property* getPropFromEvent(const Event* obj, char* toFind)
{
    ListIterator itr1 = createIterator(obj->properties);

    void* temp;
    while((temp = nextElement(&itr1)) != NULL)
    {
        Property* tmpName = (Property*)temp;
        if(nCaseCmp(tmpName->propName,toFind) == 0)
        {
            return tmpName;
        }
    }
    return NULL;

}

int findColonAfter(char* line, int pos)
{
    if(line != NULL && pos >= (strlen(line) - 1))
    {
        for(int i = pos + 1; i < strlen(line); i++)
        {
            if(line[i] == ';' || line[i] == ':')
            {
                return i;
            }
        }
        return -1;
    }
    return -2;
}

int findCharAfter(char* line,char toFind, int pos)
{
    if(line != NULL && pos >= (strlen(line) - 1))
    {
        for(int i = pos + 1; i < strlen(line); i++)
        {
            if(line[i] == toFind)
            {
                return i;
            }
        }
        return -1;
    }
    return -2;
}

char* nullJSON()
{
    char* toReturn = malloc(sizeof(char) * 3);
    strcpy(toReturn,"{}");
    return toReturn;
}

char* nullJSONList()
{
    char* toReturn = malloc(sizeof(char) * 3);
    strcpy(toReturn,"[]");
    return toReturn;
}

// char** extractJsonProperty(char* json, int* pos)
// {
//     int startPoint = pos + 1;
//     int middle = findColonAfter(json, startPoint);
//     int endPoint = findCharAfter(json,',', startPoint);
//     int setend = endPoint + 1;

//     char* ret[2];

//     int size1 = (middle - 2) - startpoint;
//     int size2 = endPoint - (middle + 2);

//     ret[0] = malloc(sizeof(char) * (size1 + 1));
//     ret[1] = malloc(sizeof(char) * (size1 + 2));

//     strcrop(ret[0],json,startPoint,)
//     strcrop(ret[1],json,)


// }
