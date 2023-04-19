#include "sdlog.h"
#include "jsmn.h"
#include <stdlib.h>
#include <stdio.h>

#if (DBG_SDLOG_ENABLED == 1 )
  extern void cli_dbg(const char* label, const char* fn, const char* fmt, ...);
  #define DBG_SDLOG(...) cli_dbg((char*)"SDLog",  __FUNCTION__, __VA_ARGS__)
#else
  #define DBG_SDLOG(...) (void)0
#endif

static FRESULT sd_log_append(sd_log_entry_t* xLogEntry);
static FRESULT sd_log_header_read(FIL* fp, sd_log_header_t* xLogHeader);
static FRESULT sd_log_header_write(FIL* fp, sd_log_header_t* xHeader);
static FRESULT sd_log_header_verify(FIL* fp, sd_log_header_t* xHeader);
static FRESULT sd_log_entry_read(FIL* fp, sd_log_entry_t* xLogEntry, uint32_t ulID);
static FRESULT sd_log_entry_write(FIL* fp, sd_log_entry_t* xLogEntry, uint32_t ulID);

static const char* sd_log_filename = "sdlog.BIN";
static const char* sd_cfg_filename = "sdcfg.INI";

/* JSON PV */
const char* sd_cfg_fmt = "{\"api_key\": \"%s\", \"server\": \"%s\", \"url\": \"%s\"}";
char sd_cfg_buf[128];

char sd_cfg_key[32];
char sd_cfg_srv[32];
char sd_cfg_url[32];

FRESULT sd_cfg_set(sd_cfg_t* xConfig)
{
  FRESULT fr;                                    /* FatFs function common result code */
  FIL fil;                                       /* File object */
  uint32_t ulBytesWritten = 0;

  memset(sd_cfg_buf, 0x00, sizeof(sd_cfg_buf));
  sprintf((char *)sd_cfg_buf, sd_cfg_fmt, xConfig->api_key, xConfig->server, xConfig->url);

  DBG_SDLOG("Config = %s\r\n", sd_cfg_buf);

  /* Open log file */
  fr = f_open(&fil, (char*)sd_cfg_filename, FA_CREATE_ALWAYS | FA_WRITE);
  if(fr == FR_OK)
  {
    fr = f_write(&fil, (void*)sd_cfg_buf, strlen(sd_cfg_buf), (UINT *)&ulBytesWritten);
    if(fr == FR_OK)
    {
      if (ulBytesWritten != strlen(sd_cfg_buf))
      {
        DBG_SDLOG("fr = %d, size mismatch (%ld != %ld)\r\n", fr, ulBytesWritten, strlen(sd_cfg_buf));
      }
    }
    else
    {
      DBG_SDLOG("f_write (fr = %d)\r\n",  fr);
    }

    f_close(&fil);
  }
  else
  {
    DBG_SDLOG("f_open (fr = %d)\r\n",  fr);
  }

  return fr;
}

static int jsoneq(const char *json, const jsmntok_t * const pxTok, const char *s) 
{
  int xStatus = 0;

  uint32_t ulStringSize = (uint32_t)pxTok->end - (uint32_t)pxTok->start;

  if (pxTok->type == JSMN_STRING)
  {
    if (strlen(s) == ulStringSize)
    {
      if (0 == strncmp(&json[pxTok->start], s, ulStringSize)) 
      {
        xStatus = 1;
      }
    }
  }

  return xStatus;
}

FRESULT sd_cfg_get(sd_cfg_t* xConfig)
{
  FRESULT fr;                                    /* FatFs function common result code */
  FIL fil;                                       /* File object */
  uint32_t ulBytes = 0;
  jsmn_parser xJSMNParser;
  jsmntok_t pxTok[16]; /* We expect no more than 128 tokens */
  uint32_t ulTokenIndex;
  int32_t lNumTokens;
  char* TokenStart;
  uint32_t TokenLen;

  memset(sd_cfg_buf, 0x00, sizeof(sd_cfg_buf));
  
  xConfig->api_key = NULL;
  xConfig->server = NULL;
  xConfig->url = NULL;

  DBG_SDLOG("File name = %s\r\n",  (char*)sd_cfg_filename);

  /* Open log file */
  fr = f_open(&fil, (char*)sd_cfg_filename, FA_OPEN_EXISTING | FA_READ);
  if(fr == FR_OK)
  {
    DBG_SDLOG("File size = %d bytes\r\n", f_size(&fil));

    fr = f_read(&fil, (void*)sd_cfg_buf, f_size(&fil), (UINT *)&ulBytes);
    if(fr == FR_OK)
    {
      DBG_SDLOG("Config = %s\r\n", sd_cfg_buf);

      jsmn_init(&xJSMNParser);

      lNumTokens = jsmn_parse(&xJSMNParser, sd_cfg_buf, ulBytes, pxTok, 16);
      if (0 < lNumTokens)
      {
        /* Assume the top-level element is an object */
        if (pxTok[0].type == JSMN_OBJECT) 
        {
          for (ulTokenIndex = 1; ulTokenIndex < lNumTokens; ulTokenIndex++)
          {

            TokenStart = &sd_cfg_buf[ pxTok[ ulTokenIndex + (uint32_t)1].start ];
            TokenLen = pxTok[ulTokenIndex + (uint32_t)1].end - pxTok[ulTokenIndex + (uint32_t)1].start;

            if (jsoneq(sd_cfg_buf, &pxTok[ulTokenIndex], "api_key") == 1)
            {
              strncpy(sd_cfg_key, TokenStart, TokenLen);
              xConfig->api_key = &sd_cfg_key[0];
            }
            else if (jsoneq(sd_cfg_buf, &pxTok[ulTokenIndex], "server") == 1)
            {
              strncpy(sd_cfg_srv, TokenStart, TokenLen);
              xConfig->server = &sd_cfg_srv[0];
            }
            else if (jsoneq(sd_cfg_buf, &pxTok[ulTokenIndex], "url") == 1)
            {
              strncpy(sd_cfg_url, TokenStart, TokenLen);
              xConfig->url = &sd_cfg_url[0];
            }
          }
        }
        else
        {
          fr = FR_DENIED;
          DBG_SDLOG("Object expected (type = %d)", pxTok[0].type);
        }
      }
      else
      {
        fr = FR_DENIED;
        DBG_SDLOG("Failed to parse JSON (num = %d)", lNumTokens);
      }
    }
    else
    {
      DBG_SDLOG("f_write (fr = %d)\r\n",  fr);
    }

    f_close(&fil);
  }
  else
  {
    DBG_SDLOG("f_open (fr = %d)\r\n",  fr);
  }

  return fr;
}



static FRESULT sd_log_append(sd_log_entry_t* xLogEntry)
{
  FRESULT fr;                                    /* FatFs function common result code */
  FIL fil;                                       /* File object */
  sd_log_header_t xLogHeader;

  /* Open log file */
  fr = f_open(&fil, (char*)sd_log_filename, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
  if(fr == FR_OK)
  {
    fr = sd_log_header_verify(&fil, &xLogHeader);
    if (fr == FR_OK)
    {
      /* Add ID */
      xLogEntry->ulID = xLogHeader.ulCount;

      /*##-5- Write data to the text file ################################*/
      fr = sd_log_entry_write(&fil, xLogEntry, 0xFFFFFFFF);
      if (fr == FR_OK)
      {
        xLogHeader.ulCount++;
        xLogHeader.ulPending++;

        fr = sd_log_header_write(&fil, &xLogHeader);
      }
    }
    else
    {
      DBG_SDLOG("fr = %d\r\n", fr);
    }

    /*##-6- Close the open text file #################################*/
    f_close(&fil);
  }
  else
  {
    DBG_SDLOG("f_open (fr = %d)\r\n",  fr);
  }
  
  return fr;
}

static FRESULT sd_log_header_read(FIL* fp, sd_log_header_t* xLogHeader)
{
  FRESULT fr = FR_OK;  
  uint32_t ulBytesRead = 0;                   /* File write/read counts */

  fr = f_lseek(fp, 0);
  if(fr == FR_OK)
  {
    fr = f_read(fp, (void*)xLogHeader, SD_LOG_HEADER_SIZE, (UINT *)&ulBytesRead);
    if (fr == FR_OK)
    {
      if (ulBytesRead != SD_LOG_HEADER_SIZE)
      {
        fr = FR_DENIED;
        DBG_SDLOG("fr = %d, size mismatch (%ld != %ld)\r\n", fr, ulBytesRead, SD_LOG_HEADER_SIZE);
      }
    }
    else
    {
      DBG_SDLOG("f_read (fr = %d)\r\n",  fr);
    }
  }
  else 
  {
    DBG_SDLOG("f_lseek (fr = %d)\r\n",  fr);
  }

  return fr;
}

static FRESULT sd_log_header_write(FIL* fp, sd_log_header_t* xHeader)
{
  FRESULT fr;  
  uint32_t ulBytesWritten = 0;

  fr = f_lseek(fp, 0);
  if(fr == FR_OK)
  {
    fr = f_write(fp, (void*)xHeader, SD_LOG_HEADER_SIZE, (UINT *)&ulBytesWritten);
    if (fr == FR_OK)
    {
      if (ulBytesWritten != SD_LOG_HEADER_SIZE)
      {
        fr = FR_DENIED;
        DBG_SDLOG("fr = %d, size mismatch (%ld != %ld)\r\n", fr, ulBytesWritten, SD_LOG_HEADER_SIZE);
      }
      
      f_sync(fp);    
    }
    else
    {
      DBG_SDLOG("f_write (fr = %d)\r\n",  fr);
    }
  }
  else 
  {
    DBG_SDLOG("f_lseek (fr = %d)\r\n",  fr);
  }

  return fr;
}

static FRESULT sd_log_header_verify(FIL* fp, sd_log_header_t* xHeader)
{
  FRESULT fr;                                 /* FatFs function common result code */
  uint32_t ulFileSize = 0;
  uint32_t ulLogEntries = 0;

  ulFileSize = f_size(fp);

  if (0 == ulFileSize)
  {
    xHeader->ulCount = 0;
    xHeader->ulPending = 0;
    xHeader->ulIndex = 0;
    fr = sd_log_header_write(fp, xHeader);
  }
  else
  {
    if (((ulFileSize - SD_LOG_HEADER_SIZE) % SD_LOG_ENTRY_SIZE) == 0)
    {
      ulLogEntries = (uint32_t) ((ulFileSize - SD_LOG_HEADER_SIZE) / SD_LOG_ENTRY_SIZE);
      fr = sd_log_header_read(fp, xHeader);
      if (fr == FR_OK)
      {
        if (xHeader->ulCount == ulLogEntries)
        {
          fr = FR_OK;
        }
        else
        {
          fr = FR_DENIED;
          DBG_SDLOG("Header count mismatch (%d != %d)\r\n", xHeader->ulCount, ulLogEntries);
        }
      }
      else
      {
        DBG_SDLOG("fr = %d\r\n", fr);
      }
    }
    else
    {
      fr = FR_DENIED;
      DBG_SDLOG("File size = %d\r\n",  ulFileSize);
    }
  }

  return fr;
}

static FRESULT sd_log_entry_read(FIL* fp, sd_log_entry_t* xLogEntry, uint32_t ulID)
{
  FRESULT fr;  
  uint32_t ulBytesRead = 0;                   /* File write/read counts */

  fr = f_lseek(fp, SD_LOG_HEADER_SIZE + (SD_LOG_ENTRY_SIZE * ulID));
  if(fr == FR_OK)
  {
    fr = f_read(fp, (void*)xLogEntry, SD_LOG_ENTRY_SIZE, (UINT *)&ulBytesRead);
    if (fr == FR_OK)
    {
      if (ulBytesRead != SD_LOG_ENTRY_SIZE)
      {
        fr = FR_DENIED;
        DBG_SDLOG("fr = %d, size mismatch (%ld != %ld)\r\n", fr, ulBytesRead, SD_LOG_ENTRY_SIZE);
      }
    }
    else
    {
      DBG_SDLOG("f_read (fr = %d)\r\n",  fr);
    }
  }
  else 
  {
    DBG_SDLOG("f_lseek (fr = %d)\r\n",  fr);
  }

  return fr;
}

static FRESULT sd_log_entry_write(FIL* fp, sd_log_entry_t* xLogEntry, uint32_t ulID)
{
  FRESULT fr;  
  uint32_t ulBytesWritten = 0;                   /* File write/read counts */
  uint32_t ulOffset = 0;

  if (0xFFFFFFFF == ulID)
  {
    ulOffset = f_size(fp);
  }
  else
  {
    ulOffset = (uint32_t)(SD_LOG_HEADER_SIZE + (SD_LOG_ENTRY_SIZE * ulID));
  }

  fr = f_lseek(fp, ulOffset);
  if(fr == FR_OK)
  {
    fr = f_write(fp, (void*)xLogEntry, SD_LOG_ENTRY_SIZE, (UINT *)&ulBytesWritten);
    if (fr == FR_OK)
    {
      if (ulBytesWritten != SD_LOG_ENTRY_SIZE)
      {
        fr = FR_DENIED;
        DBG_SDLOG("fr = %d, size mismatch (%ld != %ld)\r\n", fr, ulBytesWritten, SD_LOG_ENTRY_SIZE);
      }
    }
    else
    {
      DBG_SDLOG("f_write (fr = %d)\r\n",  fr);
    }
    f_sync(fp);
  }
  else 
  {
    DBG_SDLOG("f_lseek (fr = %d)\r\n",  fr);
  }

  return fr;
}

FRESULT sd_log(uint32_t ulData)
{
  FRESULT fr;
  
  sd_log_entry_t xLogEntry = {
    .ulID = 0xFFFFFFFF,
    .ulData = ulData,
    .ucStatus = SD_LOG_PENDING
  };

  fr = sd_log_append(&xLogEntry);
  if (fr == FR_OK)
  {
    sd_log_entry_print(&xLogEntry);
  }
  else
  {
    DBG_SDLOG("sd_append (fr = %d)\r\n",  fr);
  }

  return fr;
}

FRESULT sd_log_dump(void)
{
  FRESULT fr;                                 /* FatFs function common result code */
  FIL fil;                                    /* File object */
  uint32_t ulFileSize = 0;
  uint32_t ulBytesRead = 0;
  uint32_t ulLogIndex = 0;
  sd_log_header_t xLogHeader;
  sd_log_entry_t xLogEntry;

  DBG_SDLOG("File name = %s\r\n",  (char*)sd_log_filename);

  /*##-7- Open the text file object with read access ###############*/
  fr = f_open(&fil, (char*)sd_log_filename, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
  if(fr == FR_OK)
  {
    DBG_SDLOG("File size = %d bytes\r\n", f_size(&fil));

    fr = sd_log_header_verify(&fil, &xLogHeader);
    if (FR_OK == fr)
    {
      sd_log_header_print(&xLogHeader);

      for (ulLogIndex = 0; ulLogIndex < xLogHeader.ulCount; ulLogIndex++)
      {
        /*##-8- Read data from the text file ###########################*/
        fr = sd_log_entry_read(&fil, &xLogEntry, ulLogIndex);
        if (fr == FR_OK)
        {
          sd_log_entry_print(&xLogEntry);
          osDelay(10);
        }
        else 
        {
          DBG_SDLOG("f_read (fr = %d)\r\n",  fr);
        }        
      }
    }
    else
    {
      DBG_SDLOG("fr = %d\r\n", fr);
    }

    f_close(&fil);
  }
  else
  {
    DBG_SDLOG("f_open (fr = %d)\r\n",  fr);
  }
  
  return fr;
}


FRESULT sd_log_clear(void)
{
  return f_unlink( (char*)sd_log_filename );
}

FRESULT sd_log_entry_get_pending(sd_log_entry_t* pxEntry)
{
  FRESULT fr;                                 /* FatFs function common result code */
  FIL fil;                                    /* File object */
  uint32_t ulLogIndex = 0;
  sd_log_header_t xLogHeader;

  /* Open log file */
  fr = f_open(&fil, (char*)sd_log_filename, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
  if(fr == FR_OK)
  {
    /* Check log file integrity */
    fr = sd_log_header_verify(&fil, &xLogHeader);
    if (FR_OK == fr)
    {
      /* If there are pending log entries */
      if (0 < xLogHeader.ulPending)
      {
        for (ulLogIndex = xLogHeader.ulIndex; ulLogIndex < xLogHeader.ulCount; ulLogIndex++)
        {
          /* Read entry */
          fr = sd_log_entry_read(&fil, pxEntry, ulLogIndex);
          if (fr == FR_OK)
          {
            /* If current entry is pending, end the loop */
            if (pxEntry->ucStatus == SD_LOG_PENDING)
            {
              /* Update header search index */
              xLogHeader.ulIndex = pxEntry->ulID;
              fr = sd_log_header_write(&fil, &xLogHeader);
              DBG_SDLOG("Log Entry Pending: ID = %d\r\n", pxEntry->ulID);
              break;
            }
            else
            {
              /* Error, entry already sent */
              fr = FR_DENIED;
            }
          }
        }
      }
      else
      {
        fr = FR_DENIED;
      }
    }
    else
    {
      DBG_SDLOG("fr = %d\r\n", fr);
    }

    f_close(&fil);
  }
  else
  {
    DBG_SDLOG("f_open (fr = %d)\r\n",  fr);
  }
  
  return fr;
}

FRESULT sd_log_entry_set_sent(uint32_t ulID)
{
  FRESULT fr;                                    /* FatFs function common result code */
  FIL fil;                                       /* File object */
  sd_log_header_t xLogHeader;
  sd_log_entry_t xLogEntry;

  /* Open log file */
  fr = f_open(&fil, (char*)sd_log_filename, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
  if(fr == FR_OK)
  {
    /* Check log file integrity */
    fr = sd_log_header_verify(&fil, &xLogHeader);
    if (fr == FR_OK)
    {      
      /* Check if ID exists */
      if (ulID <= xLogHeader.ulCount)
      {
        /* Read entry */
        fr = sd_log_entry_read(&fil, &xLogEntry, ulID);
        if(fr == FR_OK)
        {
          /* ID Matches and not already marked as sent */
          if ((xLogEntry.ulID == ulID) && (xLogEntry.ucStatus == 0))
          {
            /* Mark as sent */
            xLogEntry.ucStatus = SD_LOG_SENT;

            /* Save the changes */
            fr = sd_log_entry_write(&fil, &xLogEntry, xLogEntry.ulID);
            if(fr == FR_OK)
            {
              /* Save header changes */
              xLogHeader.ulPending--;
              fr = sd_log_header_write(&fil, &xLogHeader);
              DBG_SDLOG("Log Entry Sent: ID = %d\r\n", xLogEntry.ulID);
            }
          }
          else
          {
            /* Do nothing, was already marked as sent */
          }
        }
      }
      else
      {
        /* Invalid ID */
        fr = FR_DENIED;
      }
    }
    else
    {
      DBG_SDLOG("fr = %d\r\n", fr);
    }

    f_close(&fil);
  }
  return fr;
}

void sd_log_entry_print(sd_log_entry_t* xLogEntry)
{
  if (xLogEntry != NULL)
  {
    DBG_SDLOG("Log entry [0x%04X] Status = %d, Data = %d\r\n", 
      xLogEntry->ulID, 
      xLogEntry->ucStatus, 
      xLogEntry->ulData
    );
  }
}

void sd_log_header_print(sd_log_header_t* xLogHeader)
{
  if (xLogHeader != NULL)
  {
    DBG_SDLOG("Log header: Count = %d, Pending = %d, Index = %d\r\n", 
      xLogHeader->ulCount, 
      xLogHeader->ulPending, 
      xLogHeader->ulIndex
    );
  }
}
