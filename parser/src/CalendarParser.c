#include "CalendarParser.h"
#include "QOL.h"
#include "LinkedListAPI.h"

ICalErrorCode createCalendar(char* fileName, Calendar** obj)
{
    FILE* fp;
    char *lineBuffer; //A buffer that holds an entire line (Unfolded)
    fp = fopen(fileName,"r");
    ICalErrorCode error = OK;
    int glError = 1;

    if(fp == NULL || fileName == NULL)
    {
        return INV_FILE;
    }

    if(!isValidICSfilename(fileName))
    {
        fclose(fp);
        return INV_FILE;
    }

    Calendar* toReturn;
    toReturn = malloc(sizeof(Calendar));
    if(toReturn == NULL)
    {
        fclose(fp);
        return OTHER_ERROR;
    }

    List* events = initializeList(printEvent,deleteEvent,compareEvents);
    if(events == NULL)
    {
        fclose(fp);
        deleteCalendar(toReturn);
        *obj = NULL;
        return OTHER_ERROR;
    }

    List* properties = initializeList(printProperty,deleteProperty,compareProperties);
    if(properties == NULL)
    {
        fclose(fp);
        freeList(events);
        deleteCalendar(toReturn);
        *obj = NULL;
        return OTHER_ERROR;
    }
    toReturn->events = events;
    toReturn->properties = properties;

    /* ---------- START CALENDAR ---------- */

    while((glError = getLine(&fp,&lineBuffer)) == 1)
    {
        if(strlen(lineBuffer) > 0 && lineBuffer[0] != ';')
        {
            //printf("%s ; %d\n",lineBuffer,strlen(lineBuffer));
            if(nCaseCmp(lineBuffer,"BEGIN:VCALENDAR") == 0)
            {
                break;
            }
            else
            {
                error = INV_CAL;
                free(lineBuffer);
                goto errorHandle;
            }

        }
        free(lineBuffer);
    }
    if(glError != 1)
    {
        if(glError == -1)
        {
                error = OTHER_ERROR;
                goto errorHandle;
        }
        else if(glError == -2 || glError == -3)
        {
                error = INV_FILE;
                free(lineBuffer);
                goto errorHandle;
        }
        if(glError == 0)
        {
            error = INV_FILE;
            free(lineBuffer);
            goto errorHandle;
        }
    }
    if(lineBuffer != NULL)
    {
         free(lineBuffer);
    }

    /* ---------- PARSING CALENDAR ---------- */
    bool inEvent = false;
    Event* theEvent = NULL;
    bool inAlarm = false;
    Alarm* theAlarm = NULL;
    Property* theProp = NULL;
    while(true)
    {
        glError = getLine(&fp,&lineBuffer);
        //printf("glerror = %d\n",glError);
        if(glError != 1 && glError != 0)
        {
            if(inAlarm && theAlarm != NULL)
            {
                deleteAlarm(theAlarm);
            }
            if(inEvent && theEvent != NULL)
            {
                deleteEvent(theEvent);
            }
            if(glError == -1)
            {
                error = OTHER_ERROR;
                goto errorHandle;
            }
            else if(glError == -2 || glError == -3)
            {
                error = INV_CAL;
                free(lineBuffer);
                goto errorHandle;
            }
        }

        if(strlen(lineBuffer) > 0 && lineBuffer[0] != ';')
        {
            //printf("%s ; %d\n",lineBuffer,strlen(lineBuffer));
            if(nCaseCmp(lineBuffer,"BEGIN:VEVENT") == 0)
            {
                if(inEvent || inAlarm)
                {
                    error = INV_CAL;
                    if(inEvent && theEvent != NULL)
                    {
                        error = INV_EVENT;
                        deleteEvent(theEvent);
                    }
                    if(inAlarm && theAlarm != NULL)
                    {
                        error = INV_ALARM;
                        deleteAlarm(theAlarm);
                    }
                        free(lineBuffer);
                        goto errorHandle;
                }

                error = makeEvent(&theEvent);
                if(error != OK)
                {
                    free(lineBuffer);
                    goto errorHandle;
                }
                inEvent = true;
            }
            else if(nCaseCmp(lineBuffer,"BEGIN:VALARM") == 0)
            {
                if(!inEvent || inAlarm)
                {
                    error = INV_ALARM;
                    if(inAlarm && theAlarm != NULL)
                    {
                        deleteAlarm(theAlarm);
                    }
                    if(inEvent && theEvent != NULL)
                    {
                        deleteEvent(theEvent);
                    }
                        free(lineBuffer);
                        goto errorHandle;
                }

                error = makeAlarm(&theAlarm);
                if(error != OK)
                {
                    deleteEvent(theEvent);
                    free(lineBuffer);
                    goto errorHandle;
                }
                inAlarm = true;
            }
            else if(nCaseCmp(lineBuffer,"END:VEVENT") == 0)
            {
                if(!inEvent || inAlarm)
                {
                    if(inAlarm && theAlarm != NULL)
                    {
                        deleteAlarm(theAlarm);
                    }
                    if(inEvent && theEvent != NULL)
                    {
                        deleteEvent(theEvent);
                    }
                        error = INV_CAL;
                        free(lineBuffer);
                        goto errorHandle;
                }
                inEvent = false;
                insertBack(toReturn->events,(void*)theEvent);
                theEvent = NULL;

            }
            else if(nCaseCmp(lineBuffer,"END:VALARM") == 0)
            {
                if(!inEvent || !inAlarm)
                {
                    if(inAlarm && theAlarm != NULL)
                    {
                        deleteAlarm(theAlarm);
                    }
                    if(inEvent && theEvent != NULL)
                    {
                        deleteEvent(theEvent);
                    }
                        error = INV_CAL;
                        free(lineBuffer);
                        goto errorHandle;
                }
                inAlarm = false;
                insertBack(theEvent->alarms,(void*)theAlarm);
                theAlarm = NULL;
            }
            else if(nCaseCmp(lineBuffer,"END:VCALENDAR") == 0)
            {
                if(inEvent || inAlarm)
                {
                    error = INV_CAL;
                    if(inAlarm && theAlarm != NULL)
                    {
                        error = INV_ALARM;
                        deleteAlarm(theAlarm);
                    }
                    if(inEvent && theEvent != NULL)
                    {
                        error = INV_EVENT;
                        deleteEvent(theEvent);
                    }
                        free(lineBuffer);
                        goto errorHandle;
                }
                break;
            }
            else if(!inEvent && !inAlarm)
            {
                error = makeProp(&theProp, lineBuffer);
                if(error != OK)
                {
                    free(lineBuffer);
                    goto errorHandle;
                }
                insertBack(toReturn->properties,(void*)theProp);
                theProp = NULL;
            }
            else if(inEvent && inAlarm)
            {
                error = makeProp(&theProp, lineBuffer);
                if(error != OK)
                {
                    free(lineBuffer);
                    deleteEvent(theEvent);
                    deleteAlarm(theAlarm);
                    goto errorHandle;
                }
                if(theAlarm == NULL)
                {
                    error = OTHER_ERROR;
                    free(lineBuffer);
                    goto errorHandle;
                }
                insertBack(theAlarm->properties,(void*)theProp);
                theProp = NULL;
            }
            else if(inEvent && !inAlarm)
            {
                error = makeProp(&theProp, lineBuffer);
                if(error != OK)
                {
                    deleteEvent(theEvent);
                    free(lineBuffer);
                    goto errorHandle;
                }
                if(theEvent == NULL)
                {
                    error = OTHER_ERROR;
                    free(lineBuffer);
                    goto errorHandle;
                }
                insertBack(theEvent->properties,(void*)theProp);
                theProp = NULL;
            }
            else if(!inEvent && inAlarm)
            {
                if(theAlarm != NULL)
                {
                    deleteAlarm(theAlarm);
                }
                error = INV_CAL;
                free(lineBuffer);
                goto errorHandle;
            }

        }
        if(glError == 0)
        {

            error = INV_CAL;
            free(lineBuffer);
            goto errorHandle;

        }
        free(lineBuffer);
    }
    if(lineBuffer != NULL)
    {
         free(lineBuffer);
    }
    /* ---------- POST INPUT VALIDATION ---------- */
    //CALENDAR PROPERTIES
    if(getLength(toReturn->events) < 1)
    {
        error = INV_CAL;
        goto errorHandle;
    }

    ListIterator calItr = createIterator(toReturn->properties);
    void* calVoid;
    Property* calRemove[2];
    int removecount = 0;
    bool hasVersion = false;
    bool hasProdId = false;
    while((calVoid = nextElement(&calItr)) != NULL)
    {
        Property* tmpProp = (Property*)calVoid;
        if(nCaseCmp(tmpProp->propName,"VERSION") == 0)
        {
            if(hasVersion == true)
            {
                error = DUP_VER;
                goto errorHandle;
            }
            else if(isValidVersion(tmpProp->propDescr))
            {
                toReturn->version = atof(tmpProp->propDescr);
                calRemove[removecount] = tmpProp;
                removecount++;
                hasVersion = true;
            }
            else
            {
                error = INV_VER;
                goto errorHandle;
            }

        }
        else if(nCaseCmp(tmpProp->propName,"PRODID") == 0)
        {
            if(hasProdId == true)
            {
                error = DUP_PRODID;
                goto errorHandle;
            }
            if(strlen(tmpProp->propDescr) > 0)
            {
                strcpy(toReturn->prodID,tmpProp->propDescr);
                calRemove[removecount] = tmpProp;
                removecount++;
                hasProdId = true;
            }
            else
            {
                error = INV_PRODID;
                goto errorHandle;
            }

        }
        else if(strlen(tmpProp->propName) == 0 || strlen(tmpProp->propDescr) == 0)
        {
            error = INV_CAL;
            goto errorHandle;
        }

    }
    if(hasVersion == false || hasProdId == false)
    {
        error = INV_CAL;
        goto errorHandle;
    }
    for(int i = 0; i < 2; i++)
    {
        void* data = deleteDataFromList(toReturn->properties,calRemove[i]);
        if(data != NULL)
        {
            deleteProperty(data);
        }
    }

    /* ---------- EVENT PROPERTIES ---------- */
    ListIterator eventItr = createIterator(toReturn->events);
    void* evtVoid;
    while((evtVoid = nextElement(&eventItr)) != NULL)
    {
        Event* tmpEvent = (Event*)evtVoid;
        bool hasCDT = false;
        bool hasSDT = false;
        bool hasUID = false;
        Property* evtRemove[3];
        int evtCount = 0;

        ListIterator propItr = createIterator(tmpEvent->properties);
        void* evtVoid;

        while((evtVoid = nextElement(&propItr)) != NULL)
        {
            Property* tmpProp = (Property*)evtVoid;
            if(strlen(tmpProp->propDescr) <= 0 || strlen(tmpProp->propName) <= 0)
            {
                error = INV_EVENT;
                goto errorHandle;
            }
            else if(nCaseCmp(tmpProp->propName,"DTSTAMP") == 0)
            {
                if(hasCDT)
                {
                    error = INV_EVENT;
                    goto errorHandle;
                }
                if(strlen(tmpProp->propDescr) <= 0)
                {
                    error = INV_EVENT;
                    goto errorHandle;
                }
                DateTime* DT;
                error = makeDT(&DT,tmpProp->propDescr);
                if(error != OK)
                {
                    if(error == OTHER_ERROR) //TODO
                    {
                        goto errorHandle;
                    }
                    else
                    {
                        goto errorHandle;
                    }
                }
                tmpEvent->creationDateTime = *DT; //Copies DT into the event
                deleteDate(DT);
                evtRemove[evtCount] = tmpProp;
                evtCount++;
                hasCDT = true;
            }
            else if(nCaseCmp(tmpProp->propName,"UID") == 0)
            {
                if(hasUID)
                {
                    error = INV_EVENT;
                    goto errorHandle;
                }
                if(strlen(tmpProp->propDescr) > 0)
                {
                    strcpy(tmpEvent->UID,tmpProp->propDescr);

                    evtRemove[evtCount] = tmpProp;
                    evtCount++;

                    hasUID = true;
                }
                else
                {
                    error = INV_EVENT;
                    goto errorHandle;
                }
            }
            else if(nCaseCmp(tmpProp->propName,"DTSTART") == 0)
            {
                if(strlen(tmpProp->propDescr) <= 0)
                {
                    error = INV_EVENT;
                    goto errorHandle;
                }
                DateTime* DT;
                error = makeDT(&DT,tmpProp->propDescr);
                if(error != OK)
                {
                    if(error == OTHER_ERROR)
                    {
                        goto errorHandle;
                    }
                    else
                    {
                        goto errorHandle;
                    }
                }
                tmpEvent->startDateTime = *DT;
                deleteDate(DT);
                evtRemove[evtCount] = tmpProp;
                evtCount++;
                hasSDT = true;
            }
        }
        if(!hasCDT|| !hasUID || !hasSDT)
        {
            error = INV_EVENT;
            goto errorHandle;
        }
        for(int i = 0; i < 3; i++)
        {
            void* data = deleteDataFromList(tmpEvent->properties,evtRemove[i]);
            if(data != NULL)
            {
                deleteProperty(data);
            }
        }

        ListIterator alarmItr = createIterator(tmpEvent->alarms);
        void* alrmVoid;
        while((alrmVoid = nextElement(&alarmItr)) != NULL)
        {
            Alarm* tmpAlrm = (Alarm*)alrmVoid;
            void* propVoid2;
            ListIterator propItr2 = createIterator(tmpAlrm->properties);
            bool hasAction = false;
            bool isTriggered = false;
            int alrmCount = 0;
            Property* alrmRemove[2];
            while((propVoid2 = nextElement(&propItr2)) != NULL)
            {
                Property* tmpProp2 = (Property*)propVoid2;
                if(strlen(tmpProp2->propDescr) <= 0 || strlen(tmpProp2->propName) <= 0)
                {
                    error = INV_ALARM;
                    goto errorHandle;
                }
                else if(nCaseCmp(tmpProp2->propName,"TRIGGER") == 0)
                {
                    if(isTriggered)
                    {
                        error = INV_ALARM;
                        goto errorHandle;
                    }
                    if(strlen(tmpProp2->propDescr) <= 0)
                    {
                        error = INV_ALARM;
                        goto errorHandle;
                    }
                    char* trig = malloc(strlen(tmpProp2->propDescr) + 1);
                    if(trig == NULL)
                    {
                        error = OTHER_ERROR;
                        goto errorHandle;
                    }
                    strcpy(trig,tmpProp2->propDescr);
                    tmpAlrm->trigger = trig;
                    alrmRemove[alrmCount] = tmpProp2;
                    alrmCount++;
                    isTriggered = true;


                }
                else if(nCaseCmp(tmpProp2->propName,"ACTION") == 0)
                {
                    if(hasAction)
                    {
                        error = INV_ALARM;
                        goto errorHandle;
                    }
                    if(strlen(tmpProp2->propDescr) <= 0 || strlen(tmpProp2->propDescr) >= 200)
                    {
                        error = INV_ALARM;
                        goto errorHandle;
                    }
                    strcpy(tmpAlrm->action,tmpProp2->propDescr);
                    alrmRemove[alrmCount] = tmpProp2;
                    alrmCount++;
                    hasAction = true;

                }
            }
            if(!hasAction || !isTriggered)
            {
                error = INV_ALARM;
                goto errorHandle;
            }
            for(int i = 0; i < 2; i++)
            {
                void* data = deleteDataFromList(tmpAlrm->properties,alrmRemove[i]);
                if(data != NULL)
                {
                    deleteProperty(data);
                }
            }

        }

    }


    /* ---------- SUCCESS  ---------- */
    fclose(fp);
    *obj = toReturn;

    return error; //Should Be OK.

    /* ---------- ERROR HANDLING  ---------- */
    errorHandle:
    fclose(fp);
    deleteCalendar(toReturn);
    *(obj) = NULL;

    return error;
}

void deleteCalendar(Calendar* obj)
{
    if(obj == NULL)
    {
        return;
    }
    else
    {
        if(obj->events != NULL)
        {
            freeList(obj->events);
        }
        if(obj->properties != NULL)
        {
            freeList(obj->properties);
        }
        free(obj);
    }
    return;
}

char* printCalendar(const Calendar* obj)
{
    char* toReturn = malloc(40);
    strcpy(toReturn,"please use printfcalendar instead");
    return toReturn;
}

char* printError(ICalErrorCode err)
{
    char* rtn = malloc(sizeof(char) * 100);
    switch(err)
    {
        case OK:
        strcpy(rtn,"<OK>\n");
        break;

        case INV_FILE:
        strcpy(rtn,"<ERROR> INVALID FILE <ERROR>\n");
        break;

        case INV_CAL:
        strcpy(rtn,"<ERROR> INVALID CAL <ERROR>\n");
        break;

        case INV_VER:
        strcpy(rtn,"<ERROR> INVALID VER <ERROR>\n");
        break;

        case DUP_VER:
        strcpy(rtn,"<ERROR> DUPE VER <ERROR>\n");
        break;

        case INV_PRODID:
        strcpy(rtn,"<ERROR> INVALID PRODID <ERROR>\n");
        break;

        case INV_EVENT:
        strcpy(rtn,"<ERROR> INVALID EVENT <ERROR>\n");
        break;

        case INV_DT:
        strcpy(rtn,"<ERROR> INVALID DATE-TIME <ERROR>\n");
        break;

        case INV_ALARM:
        strcpy(rtn,"<ERROR> INVALID ALARM <ERROR>\n");
        break;

        case WRITE_ERROR:
        strcpy(rtn,"<ERROR> WRITE ERROR <ERROR>\n");
        break;

        case OTHER_ERROR:
        strcpy(rtn,"<ERROR> OTHER ERROR <ERROR>\n");
        break;

        default:
        strcpy(rtn,"<ERROR> OTHER ERROR? <ERROR>\n");

    }
    return rtn;

}

ICalErrorCode writeCalendar(char* fileName, const Calendar* obj)
{
    if(obj == NULL)
    {
        return WRITE_ERROR;
    }
    // if(!validateCalendar(obj))
    // {
    //     return WRITE_ERROR;
    // }
    if(!isValidICSfilename(fileName))
    {
        return WRITE_ERROR;
    }

    FILE* fp = fopen(fileName,"w");

    return writeCalToFile(fp, obj, 0);
}

ICalErrorCode validateCalendar(const Calendar* obj)
{
    if(obj == NULL)
    {
        return INV_CAL;
    }

    ICalErrorCode err = OK;
    adderr(&err, baseCheck(obj));
    adderr(&err, propCheck(obj));
    return err;
}

char* dtToJSON(DateTime prop)
{
    int size = snprintf(NULL,0,"{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":%s}",prop.date,prop.time,(prop.UTC) ? "true":"false") + 1;
    char* str1 = malloc(sizeof(char) * (size));
    if(str1 == NULL)
    {
        return nullJSON();
    }
    if(sprintf(str1,"{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":%s}",prop.date,prop.time,(prop.UTC) ? "true":"false") <= 0)
    {
        return nullJSON();
    }
    return str1;
}

char* eventToJSON(const Event* event)
{
    if(event == NULL || event->properties == NULL|| event->alarms == NULL)
    {
        return nullJSON();
    }

    char* dt = dtToJSON(event->startDateTime);
    int size = snprintf(NULL,0,"{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"%s\",\"location\":\"%s\",\"organizer\":\"%s\"}",dt,getLength(event->properties) + 3,getLength(event->alarms),(getPropFromEvent(event,"SUMMARY") == NULL)?"":getPropFromEvent(event,"SUMMARY")->propDescr,(getPropFromEvent(event,"LOCATION") == NULL)?"":getPropFromEvent(event,"LOCATION")->propDescr,(getPropFromEvent(event,"ORGANIZER") == NULL)?"":getPropFromEvent(event,"ORGANIZER")->propDescr) + 1;
    char* str1 = malloc(sizeof(char) * (size));
    if(str1 == NULL)
    {
        free(dt);
        return nullJSON();
    }
    if(sprintf(str1,"{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"%s\",\"location\":\"%s\",\"organizer\":\"%s\"}",dt,getLength(event->properties) + 3,getLength(event->alarms),(getPropFromEvent(event,"SUMMARY") == NULL)?"":getPropFromEvent(event,"SUMMARY")->propDescr,(getPropFromEvent(event,"LOCATION") == NULL)?"":getPropFromEvent(event,"LOCATION")->propDescr,(getPropFromEvent(event,"ORGANIZER") == NULL)?"":getPropFromEvent(event,"ORGANIZER")->propDescr) <= 0)
    {
        free(dt);
        return nullJSON();
    }
    free(dt);
    return str1;

}

char* alarmToJSON(const Alarm* alarm)
{
    if(alarm == NULL || alarm->properties == NULL)
    {
        return nullJSON();
    }

    int size = snprintf(NULL,0,"{\"action\":\"%s\",\"trigger\":\"%s\",\"numProps\":%d}",alarm->action,alarm->trigger,getLength(alarm->properties) + 2) + 1;
    char* str1 = malloc(sizeof(char) * (size));
    if(str1 == NULL)
    {
        return nullJSON();
    }
    if(sprintf(str1,"{\"action\":\"%s\",\"trigger\":\"%s\",\"numProps\":%d}",alarm->action,alarm->trigger,getLength(alarm->properties) + 2) <= 0)
    {
        return nullJSON();
    }
    return str1;

}

char* propToJSON(const Property* prop)
{
  if(prop == NULL)
  {
      return nullJSON();
  }

  int size = snprintf(NULL,0,"{\"propName\":\"%s\",\"propDescr\":\"%s\"}",prop->propName,prop->propDescr) + 1;
  char* str1 = malloc(sizeof(char) * (size));
  if(str1 == NULL)
  {
      return nullJSON();
  }
  if(sprintf(str1,"{\"propName\":\"%s\",\"propDescr\":\"%s\"}",prop->propName,prop->propDescr) <= 0)
  {
      return nullJSON();
  }
  return str1;
}

char* propertyListToJSON(const List* propList)
{
  if(propList == NULL)
  {
      return nullJSONList();
  }

  ListIterator itr1 = createIterator((List*)propList);
  char* str = malloc(sizeof(char) * 2);
  str[0] = '[';
  str[1] = '\0';

  void* temp;
  int numLoops = 0;
  while((temp = nextElement(&itr1)) != NULL)
  {
      Property* tmpName = (Property*)temp;
      if(tmpName == NULL)
      {
          free(str);
          return nullJSON();
      }
      char* propStr = propToJSON(tmpName);
      int sizeAdd = strlen(propStr) + 1;
      if(numLoops == 0)
      {
          char* buff = malloc(sizeof(char) * (strlen(str) + sizeAdd));
          sprintf(buff,"%s%s",str,propStr);
          free(str);
          str = buff;
      }
      else
      {
          char* buff = malloc(sizeof(char) * (strlen(str) + sizeAdd + 1));
          sprintf(buff,"%s,%s",str,propStr);
          free(str);
          str = buff;
      }
      numLoops++;
      free(propStr);
  }
  char* buff = malloc(sizeof(char) * (strlen(str) + 2));
  sprintf(buff,"%s]",str);
  free(str);
  str = buff;
  return str;
}
char* alarmListToJSON(const List* alarmList)
{
    if(alarmList == NULL)
    {
        return nullJSONList();
    }

    ListIterator itr1 = createIterator((List*)alarmList);
    char* str = malloc(sizeof(char) * 2);
    str[0] = '[';
    str[1] = '\0';

    void* temp;
    int numLoops = 0;
    while((temp = nextElement(&itr1)) != NULL)
    {
        Alarm* tmpName = (Alarm*)temp;
        if(tmpName == NULL || tmpName->properties == NULL)
        {
            free(str);
            return nullJSON();
        }
        char* almStr = alarmToJSON(tmpName);
        int sizeAdd = strlen(almStr) + 1;
        if(numLoops == 0)
        {
            char* buff = malloc(sizeof(char) * (strlen(str) + sizeAdd));
            sprintf(buff,"%s%s",str,almStr);
            free(str);
            str = buff;
        }
        else
        {
            char* buff = malloc(sizeof(char) * (strlen(str) + sizeAdd + 1));
            sprintf(buff,"%s,%s",str,almStr);
            free(str);
            str = buff;
        }
        numLoops++;
        free(almStr);
    }
    char* buff = malloc(sizeof(char) * (strlen(str) + 2));
    sprintf(buff,"%s]",str);
    free(str);
    str = buff;
    return str;
}

char* eventListToJSON(const List* eventList)
{
    if(eventList == NULL)
    {
        return nullJSONList();
    }

    ListIterator itr1 = createIterator((List*)eventList);
    char* str = malloc(sizeof(char) * 2);
    str[0] = '[';
    str[1] = '\0';

    void* temp;
    int numLoops = 0;
    while((temp = nextElement(&itr1)) != NULL)
    {
        Event* tmpName = (Event*)temp;
        if(tmpName == NULL || tmpName->properties == NULL|| tmpName->alarms == NULL)
        {
            free(str);
            return nullJSON();
        }
        char* evtStr = eventToJSON(tmpName);
        int sizeAdd = strlen(evtStr) + 1;
        if(numLoops == 0)
        {
            char* buff = malloc(sizeof(char) * (strlen(str) + sizeAdd));
            sprintf(buff,"%s%s",str,evtStr);
            free(str);
            str = buff;
        }
        else
        {
            char* buff = malloc(sizeof(char) * (strlen(str) + sizeAdd + 1));
            sprintf(buff,"%s,%s",str,evtStr);
            free(str);
            str = buff;
        }
        numLoops++;
        free(evtStr);
    }
    char* buff = malloc(sizeof(char) * (strlen(str) + 2));
    sprintf(buff,"%s]",str);
    free(str);
    str = buff;
    return str;
}

char* calendarToJSON(const Calendar* cal)
{
    if(cal == NULL)
    {
        return nullJSON();
    }

    int size = snprintf(NULL,0,"{\"version\":%g,\"prodID\":\"%s\",\"numProps\":%d,\"numEvents\":%d}",cal->version,cal->prodID,getLength(cal->properties) + 2,getLength(cal->events)) + 1;
    char* str1 = malloc(sizeof(char) * (size));
    if(str1 == NULL)
    {
        return "This Is Bad.\n";
    }
    if(sprintf(str1,"{\"version\":%g,\"prodID\":\"%s\",\"numProps\":%d,\"numEvents\":%d}",cal->version,cal->prodID,getLength(cal->properties) + 2,getLength(cal->events)) <= 0)
    {
        return "This Is Bad.\n";
    }
    return str1;
}

Calendar* JSONtoCalendar(const char* str)
{
    if(str == NULL)
    {
        return NULL;
    }

    int size = strlen(str) + 1;
    char* tmpstr = malloc(sizeof(char) * (size));
    float ver = 0.0;
    if(sscanf(str,"{\"version\":\"%g\",\"prodID\":\"%[^\"]",&ver,tmpstr) != 2)
    {
        free(tmpstr);
        return NULL;
    }
    char* st = malloc(sizeof(char) * (strlen(tmpstr) + 1));
    strcpy(st,tmpstr);
    free(tmpstr);
    if(strlen(st) > 999)
    {
        free(st);
        return NULL;
    }
    Calendar* toReturn;
    toReturn = malloc(sizeof(Calendar));
    if(toReturn == NULL)
    {
        free(st);
        return NULL;
    }

    List* events = initializeList(printEvent,deleteEvent,compareEvents);
    if(events == NULL)
    {
        free(st);
        deleteCalendar(toReturn);
        return NULL;
    }

    List* properties = initializeList(printProperty,deleteProperty,compareProperties);
    if(properties == NULL)
    {
        free(st);
        freeList(events);
        deleteCalendar(toReturn);
        return NULL;
    }
    toReturn->events = events;
    toReturn->properties = properties;

    toReturn->version = ver;
    strcpy(toReturn->prodID,st);
    free(st);
    return toReturn;

}

Event* JSONtoEvent(const char* str)
{
    if(str == NULL)
    {
        return NULL;
    }

    int size = strlen(str) + 1;
    char* tmpstr = malloc(sizeof(char) * (size));
    char* tmpstr2 = malloc(sizeof(char) * (size));
    char* tmpstr3 = malloc(sizeof(char) * (size));
    char* tmpstr4 = malloc(sizeof(char) * (size));
    if(sscanf(str,"{\"UID\":\"%[^\"]\",\"DAT1\":\"%[^\"]\",\"DAT2\":\"%[^\"]\",\"SUM\":\"%[^\"]",tmpstr,tmpstr2,tmpstr3,tmpstr4) != 4)
    {
        free(tmpstr);
        free(tmpstr2);
        free(tmpstr3);
        free(tmpstr4);
        return NULL;
    }
    char* st = malloc(sizeof(char) * (strlen(tmpstr) + 1));
    char* st2 = malloc(sizeof(char) * (strlen(tmpstr2) + 1));
    char* st3 = malloc(sizeof(char) * (strlen(tmpstr3) + 1));
    char* st4 = malloc(sizeof(char) * (strlen(tmpstr4) + 1));

    strcpy(st,tmpstr);
    strcpy(st2,tmpstr2);
    strcpy(st3,tmpstr3);
    strcpy(st4,tmpstr4);
    free(tmpstr);
    free(tmpstr2);
    free(tmpstr3);
    free(tmpstr4);

    if(strlen(st) > 999)
    {
        free(st);
        free(st2);
        free(st3);
        free(st4);
        return NULL;
    }
    Event* toReturn;
    if(makeEvent(&toReturn) != OK)
    {
        free(st);
        free(st2);
        free(st3);
        free(st4);
        return NULL;
    }

    DateTime* toReturn2 = NULL;
    DateTime* toReturn3 = NULL;
    if(makeDT(&toReturn2,st2) != OK)
    {
        free(st);
        free(st2);
        free(st3);
        free(st4);
        free(toReturn);
        return NULL;
    }
    if(makeDT(&toReturn3,st3) != OK)
    {
        free(st);
        free(st2);
        free(st3);
        free(st4);
        free(toReturn);
        free(toReturn2);
        return NULL;
    }

    strcpy(toReturn->UID,st);
    toReturn->startDateTime = *toReturn2;
    toReturn->creationDateTime = *toReturn3;

    if(strlen(st4) > 0)
    {
        Property* retProp = malloc(sizeof(char) * (200 + strlen(st4) + 1));
        strcpy(retProp->propName,"SUMMARY");
        strcpy(retProp->propDescr,st4);
        insertBack(toReturn->properties,(void*)retProp);
    }
    free(st);
    free(st2);
    free(st3);
    free(st4);
    return toReturn;

}

void addEvent(Calendar* cal, Event* toBeAdded)
{
    if(cal != NULL && toBeAdded != NULL)
    {
        insertBack(cal->events,(void*)toBeAdded);
    }
}


void deleteEvent(void* toBeDeleted)
{
    if(toBeDeleted != NULL)
    {
        Event* Evnt = (Event*)(toBeDeleted);
        if(Evnt->properties != NULL)
        {
            freeList(Evnt->properties);
        }
        if(Evnt->alarms != NULL)
        {
            freeList(Evnt->alarms);
        }
        free(toBeDeleted);
    }
}
int compareEvents(const void* first, const void* second)
{
    Property* tmp1 = (Property*)first;
    Property* tmp2 = (Property*)second;

    return strcmp(tmp1->propName,tmp2->propName);
}

char* printEvent(void* toBePrinted)
{
    char* toReturn = malloc(40);
    strcpy(toReturn,"please use printfevent instead");
    return toReturn;
}

void deleteAlarm(void* toBeDeleted)
{

    if(toBeDeleted != NULL)
    {
        Alarm* Alrm = (Alarm*)(toBeDeleted);
        if(Alrm->properties != NULL)
        {
            freeList(Alrm->properties);
        }
        if(Alrm->trigger != NULL)
        {
            free(Alrm->trigger);
        }
        free(toBeDeleted);
    }
}

int compareAlarms(const void* first, const void* second)
{
    return 0;
}

char* printAlarm(void* toBePrinted)
{
    char* toReturn = malloc(40);
    strcpy(toReturn,"please use printfalarm instead");
    return toReturn;
}

void deleteProperty(void* toBeDeleted)
{
    if(toBeDeleted != NULL)
    {
        free((Property*)toBeDeleted);
    }
}

int compareProperties(const void* first, const void* second)
{
    Property* temp1 = (Property*)first;
    Property* temp2 = (Property*)second;
    int cmp1 = strcmp(temp1->propName,temp2->propName);
    int cmp2 = strcmp(temp1->propDescr,temp2->propDescr);
    int ret = 999;

    if(cmp1 < 0)
    {
        cmp1 = cmp1 * -1;
    }
    if(cmp2 < 0)
    {
        cmp2 = cmp2 * -1;
    }
    ret = cmp1 + cmp2;

    return ret;
}

char* printProperty(void* toBePrinted)
{
    char* toReturn = malloc(40);
    strcpy(toReturn,"please use printfproperty instead");
    return toReturn;
}

void deleteDate(void* toBeDeleted)
{
    if(toBeDeleted != NULL)
    {
        free(toBeDeleted);
    }
}

int compareDates(const void* first, const void* second)
{
    return 0;
}

char* printDate(void* toBePrinted)
{
    char* toReturn = malloc(40);
    strcpy(toReturn,"please use printfdatetimeinstead");
    return toReturn;
}

char* getCalInfo(char* fileName)
{
    Calendar* newCal = NULL;
    ICalErrorCode err = OK;
    err = createCalendar(fileName,&newCal);

    if(err != OK || newCal == NULL)
    {
        deleteCalendar(newCal);
        return nullJSON();
    }

    char* toReturn = calendarToJSON(newCal);
    deleteCalendar(newCal);

    if(toReturn == NULL)
    {
        return nullJSON();
    }

    return toReturn;
}

char* getEventInfo(char* fileName)
{
    Calendar* newCal = NULL;
    ICalErrorCode err = OK;
    err = createCalendar(fileName,&newCal);

    if(err != OK || newCal == NULL)
    {
        deleteCalendar(newCal);
        return nullJSON();
    }

    char* toReturn = eventListToJSON(newCal->events);
    deleteCalendar(newCal);

    if(toReturn == NULL)
    {
        return nullJSONList();
    }

    return toReturn;
}

char* getAlarmList(char* fileName, int num)
{
  Calendar* newCal = NULL;
  ICalErrorCode err = OK;
  err = createCalendar(fileName,&newCal);
  if(err != OK)
  {
    deleteCalendar(newCal);
    return nullJSONList();
  }
  ListIterator itr1 = createIterator(newCal->events);

  void* temp;
  int count = 0;
  while((temp = nextElement(&itr1)) != NULL)
  {
    Event* tmpName = (Event*)temp;
    if(count == num)
    {
      char* toReturn;
      toReturn = alarmListToJSON(tmpName->alarms);
      deleteCalendar(newCal);
      return toReturn;
    }
    count++;
  }
  deleteCalendar(newCal);
  return nullJSONList();
}

char* getPropList(char* fileName, int num)
{
  Calendar* newCal = NULL;
  ICalErrorCode err = OK;
  err = createCalendar(fileName,&newCal);
  if(err != OK)
  {
    deleteCalendar(newCal);
    return nullJSONList();
  }
  ListIterator itr1 = createIterator(newCal->events);

  void* temp;
  int count = 0;
  while((temp = nextElement(&itr1)) != NULL)
  {
    Event* tmpName = (Event*)temp;
    if(count == num)
    {
      char* toReturn;
      toReturn = propertyListToJSON(tmpName->properties);
      deleteCalendar(newCal);
      return toReturn;
    }
    count++;
  }
  deleteCalendar(newCal);
  return nullJSONList();
}

int isValidCalendar(char* fileName)
{
  Calendar* newCal = NULL;
  ICalErrorCode err = OK;
  err = createCalendar(fileName,&newCal);
  if(err != OK)
  {
    return 0;
  }
  err = validateCalendar(newCal);
  if(err != OK)
  {
    return 0;
  }
  return 1;
}

int addEventToCal(char* fileName, char* JSONInput)
{
  Calendar* newCal = NULL;
  ICalErrorCode err = OK;
  err = createCalendar(fileName,&newCal);
  if(err != OK)
  {
    deleteCalendar(newCal);
    return 0;
  }

  Event* toAdd = JSONtoEvent(JSONInput);

  if(toAdd == NULL)
  {
    deleteCalendar(newCal);
    return 0;
  }

  insertBack(newCal->events,(void*)toAdd);

  writeCalendar(fileName,newCal);
  return 1;

}

int createCalEvent(char* fileName, char* JSONInputE, char* JSONInputC)
{
  Calendar* newCal = NULL;
  newCal = JSONtoCalendar(JSONInputC);
  if(newCal == NULL)
  {
    return 0;
  }
  Event* toAdd = JSONtoEvent(JSONInputE);
  if(toAdd == NULL)
  {
    deleteCalendar(newCal);
    return 0;
  }
  insertBack(newCal->events,(void*)toAdd);
  writeCalendar(fileName,newCal);
  return 1;
}
