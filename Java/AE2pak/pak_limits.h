#ifndef PAK_LIMITS_H
#define PAK_LIMITS_H

#include <limits.h>

// Field sizes

#define FILE_DATA_START_POS_BYTES 2
#define TOTAL_NUM_FILES_BYTES 2

#define FILENAME_LEN_BYTES 2
#define FILE_DATA_START_OFFSET_BYTES 4
#define FILE_SIZE_BYTES 2

// Fixed-width field limits

#define FILE_DATA_START_POS_MAX UINT16_MAX
#define TOTAL_NUM_FILES_MAX UINT16_MAX

#define FILENAME_LEN_MAX UINT16_MAX
#define FILE_DATA_START_OFFSET_MAX UINT32_MAX
#define FILE_SIZE_MAX UINT16_MAX

#endif
