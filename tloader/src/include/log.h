#pragma once 

typedef enum {
   TLOADER_NOLOG = 0,
   TLOADER_ERROR,
   TLOADER_DEBUG,
   TLOADER_LOGMAX
} TLOADER_VERBOSITY;

void SetLogVerbosity(TLOADER_VERBOSITY v);

void Log(TLOADER_VERBOSITY v, char * fmt, ...);


#define LOGERROR(fmt, ...) Log(TLOADER_ERROR, "[%s:%u][%s]: " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__)
#define LOGDEBUG(fmt, ...) Log(TLOADER_DEBUG, "[%s:%u][%s]: " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__)


