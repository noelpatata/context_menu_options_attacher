#ifndef REGISTRY_EDIT_UTIL_H
#define REGISTRY_EDIT_UTIL_H

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

typedef struct {
    char bat_dir[MAX_PATH];
    char bat_filename_only[MAX_PATH];
    char reg_file_full_path[MAX_PATH + 64];
    char* menu_item_name;
    char* escaped_bat_path;
    char* escaped_icon_path;
} RegContext;

BOOL initRegContext(RegContext* context, const char* bat_full_path, const char* custom_icon_full_path_optional);
void freeRegContext(RegContext* context);
BOOL writeRegFile(const RegContext* context);
int applyRegFile(const RegContext* context);

#endif
