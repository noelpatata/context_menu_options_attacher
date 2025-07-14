#include <stdio.h>
#include <windows.h>
#include <shlwapi.h>
#include <string.h>
#include <stdlib.h>
#include <shellapi.h>
#include <ctype.h>

#include "FileUtil.h"
#include "RegistryEditUtil.h"

int createRegFileForBat(const char* bat_full_path, const char* custom_icon_full_path_optional) {
    RegContext context = {0};

    if (!initRegContext(&context, bat_full_path, custom_icon_full_path_optional)) {
        return 1;
    }

    if (!writeRegFile(&context)) {
        freeRegContext(&context);
        return 1;
    }

    int result = applyRegFile(&context);

    freeRegContext(&context);

    return result;
}

int main() {
    char current_exe_path[MAX_PATH];
    char current_path[MAX_PATH];
    char scripts_dir[MAX_PATH];
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char search_script_folders_wildcard[MAX_PATH];

    //get current file path
    GetModuleFileNameA(NULL, current_exe_path, MAX_PATH);

    
    strcpy(current_path, current_exe_path);
    //remove fileName
    PathRemoveFileSpecA(current_path);

    PathCombineA(scripts_dir, current_path, "scripts");
    scripts_dir[MAX_PATH - 1] = '\0';

    //Check if the 'scripts' directory exists
    DWORD scripts_dir_attributes = GetFileAttributesA(scripts_dir);
    if (scripts_dir_attributes == INVALID_FILE_ATTRIBUTES || !(scripts_dir_attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        
        MessageBoxA(NULL, "Info: El proyecto no incluye ninguna carpeta scripts\\. Cree dentro la carpeta e incluya las carpetas de los scripts que se deseen agregar al menú contextual.", "Información", MB_OK | MB_ICONINFORMATION);
        return 1;
    }

    printf("Buscando scripts...\n");

    PathCombineA(search_script_folders_wildcard, scripts_dir, "*");
    search_script_folders_wildcard[MAX_PATH - 1] = '\0';

    //finds all files in scripts\ folder
    hFind = FindFirstFileA(search_script_folders_wildcard, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        MessageBoxA(NULL, "Info: El proyecto no incluye ningún Script. Cree dentro de scripts\\Nombre_Script e incluya el .bat que se añadirá en el menú contextual.", "Información", MB_OK | MB_ICONINFORMATION);
        return 1;
    }

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
        strcmp(findFileData.cFileName, ".") == 0 ||
        strcmp(findFileData.cFileName, "..") == 0)
        {
            continue;
        }

        //concat script
        char script_dir_full_path[MAX_PATH];
        PathCombineA(script_dir_full_path, scripts_dir, findFileData.cFileName);
        script_dir_full_path[MAX_PATH - 1] = '\0';
        
        printf("--------------------------------------------------------------------\n");
        printf(" Encontrado! : %s\n", script_dir_full_path);
        
        char bat_full_path[MAX_PATH] = {0};
        char ico_full_path[MAX_PATH] = {0};
        
        if (findFirstFileInDirByFileExtension(script_dir_full_path, ".bat", bat_full_path, sizeof(bat_full_path))) {
            printf("   Se ha encontrado .bat en %s\n", bat_full_path);
        }
        else {
            printf("   No se ha encontrado .bat en '%s'.\n", findFileData.cFileName);
            continue;
        }

        if (findFirstFileInDirByFileExtension(script_dir_full_path, ".ico", ico_full_path, sizeof(ico_full_path))) {
            printf("   Se ha encontrado .ico en %s\n", ico_full_path);
        }
        else {
            printf("   No  se ha encontrado .ico en '%s'.\n", findFileData.cFileName);
        }
                
        createRegFileForBat(bat_full_path, ico_full_path);
        printf("--------------------------------------------------------------------\n");
            
    } while (FindNextFileA(hFind, &findFileData) != 0);

    FindClose(hFind);

    return 0;
}
