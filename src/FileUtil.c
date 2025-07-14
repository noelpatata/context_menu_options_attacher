#include "FileUtil.h"

#include <shlwapi.h>

BOOL findFirstFileInDirByFileExtension(const char* directoryPath, const char* fileExtension, char* foundFilePath, size_t foundFilePathSize) {
    char search_path[MAX_PATH];
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    snprintf(search_path, MAX_PATH, "%s\\*%s", directoryPath, fileExtension);
    search_path[MAX_PATH - 1] = '\0';

    hFind = FindFirstFileA(search_path, &findFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        snprintf(foundFilePath, foundFilePathSize, "%s\\%s", directoryPath, findFileData.cFileName);
        foundFilePath[foundFilePathSize - 1] = '\0';
        FindClose(hFind);
        return TRUE;
    }

    return FALSE;
}

BOOL fileExists(const char* path) {
    DWORD attributes = GetFileAttributesA(path);
    return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}

int deleteFile(const char *filepath) {
    if (filepath == NULL) {
        return -1;
    }
    if (remove(filepath) == 0) {
        return 0;
    } else {
        return -1;
    }
}

void strip_ext(char *fname) {
    char *end = fname + strlen(fname);

    while (end > fname && *end != '.') {
        --end;
    }

    if (end > fname) {
        *end = '\0';
    }
}

char* formatMenuItemName(const char* filename_no_ext) {
    if (filename_no_ext == NULL) {
        return NULL;
    }

    size_t len = strlen(filename_no_ext);
    char* formatted_name = (char*)malloc(len + 1);
    if (formatted_name == NULL) {
        return NULL;
    }

    BOOL capitalize_next = TRUE;
    for (size_t i = 0; i <= len; ++i) {
        if (filename_no_ext[i] == '_') {
            formatted_name[i] = ' ';
            capitalize_next = TRUE;
        } else if (capitalize_next && filename_no_ext[i] != '\0') {
            formatted_name[i] = (char)toupper((unsigned char)filename_no_ext[i]);
            capitalize_next = FALSE;
        } else {
            formatted_name[i] = filename_no_ext[i];
            capitalize_next = FALSE;
        }
    }
    formatted_name[len] = '\0';

    return formatted_name;
}

char* addEscapeBackslashes(const char* original_path) {
    if (original_path == NULL) {
        return NULL;
    }
    
    size_t original_len = strlen(original_path);
    char* escaped_path = (char*)malloc(original_len * 2 + 1);

    if (escaped_path == NULL) {
        return NULL;
    }

    size_t i = 0;
    size_t j = 0;

    while (original_path[i] != '\0') {
        if (original_path[i] == '\\') {
            escaped_path[j++] = '\\';
            escaped_path[j++] = '\\';
        } else {
            escaped_path[j++] = original_path[i];
        }
        i++;
    }
    escaped_path[j] = '\0';

    return escaped_path;
}
