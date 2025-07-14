#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <windows.h>
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

BOOL findFirstFileInDirByFileExtension(const char* directoryPath, const char* fileExtension, char* foundFilePath, size_t foundFilePathSize);

BOOL fileExists(const char* path);

int deleteFile(const char *filepath);

void strip_ext(char *fname);

char* formatMenuItemName(const char* filename_no_ext);

char* addEscapeBackslashes(const char* original_path);

#endif
