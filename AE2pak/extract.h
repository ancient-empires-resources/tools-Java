#ifndef EXTRACT_H
#define EXTRACT_H

#include "pak_limits.h"

#define FILE_LIST_LOG "_file_list.log"

// Extract .pak file to extract directory.
// The extract directory must exist beforehand, or the program will NOT work.
void extract(const char* pakFile, const char* extractDir, const char* fileListLOG);

#endif
