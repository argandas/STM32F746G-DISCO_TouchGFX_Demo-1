#ifndef __SD_LOG_H_
#define __SD_LOG_H_

#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "fatfs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DBG_SDLOG_ENABLED          1

#define SD_LOG_HEADER_SIZE         (sizeof(sd_log_header_t))
#define SD_LOG_ENTRY_SIZE          (sizeof(sd_log_entry_t))
#define SD_LOG_PENDING             ((uint8_t) pdFALSE)
#define SD_LOG_SENT                ((uint8_t) pdTRUE)

typedef struct {
  uint32_t ulCount;
  uint32_t ulPending;
  uint32_t ulIndex;
} sd_log_header_t;

typedef struct {
  uint32_t ulID;
  uint32_t ulData;
  uint8_t  ucStatus;
} sd_log_entry_t;

typedef struct {
    char* api_key;
    char* server;
    char *url;
} sd_cfg_t;

FRESULT sd_log(uint32_t data);
FRESULT sd_log_dump(void);
FRESULT sd_log_clear(void);
FRESULT sd_log_entry_get_pending(sd_log_entry_t* pxEntry);
FRESULT sd_log_entry_set_sent(uint32_t ulID);
void sd_log_entry_print(sd_log_entry_t* xLogEntry);
void sd_log_header_print(sd_log_header_t* xLogHeader);

FRESULT sd_cfg_set(sd_cfg_t* xConfig);
FRESULT sd_cfg_get(sd_cfg_t* xConfig);

#ifdef __cplusplus
}
#endif

#endif /* __SD_LOG_H_ */
