#include <stdio.h>      // For file I/O (fopen, fprintf, fclose) and printf
#include <windows.h>    // For Windows API functions like GetModuleFileNameA, MessageBoxA, FindFirstFileA, FindNextFileA, FindClose, GetFileAttributesA
#include <shlwapi.h>    // For PathRemoveFileSpecA, PathFindExtensionA, PathFindFileNameA (requires linking with shlwapi.lib)
#include <string.h>     // For string manipulation functions like strcpy, strcat, snprintf, strlen, _stricmp
#include <stdlib.h>     // For malloc, free, system
#include <shellapi.h>   // For ShellExecuteA
#include <ctype.h>      // For toupper

// Function to remove file extension
void strip_ext(char *fname)
{
    char *end = fname + strlen(fname);

    while (end > fname && *end != '.') {
        --end;
    }

    if (end > fname) {
        *end = '\0';
    }

}
// Function to delete the specified file
// Returns 0 on success, -1 on failure.
int deleteFile(const char *filepath) {
    if (filepath == NULL) {
        fprintf(stderr, "Error: Filepath cannot be NULL.\n");
        return -1; // Indicate an error
    }
    if (remove(filepath) == 0) {
        printf("File '%s' deleted successfully.\n", filepath);
        return 0; // Success
    } else {
        perror("Error deleting file");
        return -1; // Indicate an error
    }
}

// Function to escape backslashes in a string for .reg file format.
// It allocates new memory for the escaped string, which the caller must free.
char* escapeBackslashes(const char* original_path) {
    if (original_path == NULL) {
        return NULL;
    }
    
    size_t original_len = strlen(original_path);
    // Allocate enough memory for worst-case scenario (all backslashes doubled) + null terminator
    char* escaped_path = (char*)malloc(original_len * 2 + 1);

    if (escaped_path == NULL) {
        perror("Failed to allocate memory for escaped path");
        return NULL;
    }

    size_t i = 0; // Index for original_path
    size_t j = 0; // Index for escaped_path

    while (original_path[i] != '\0') {
        if (original_path[i] == '\\') {
            escaped_path[j++] = '\\'; // Add first backslash
            escaped_path[j++] = '\\'; // Add second backslash
        } else {
            escaped_path[j++] = original_path[i];
        }
        i++;
    }
    escaped_path[j] = '\0'; // Null-terminate the escaped string

    return escaped_path;
}

// Function to check if a file exists using Windows API.
// Returns TRUE if the file exists and is not a directory, FALSE otherwise.
BOOL fileExists(const char* path) {
    DWORD attributes = GetFileAttributesA(path);
    return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}

// Function to format a string for a context menu item.
// E.g., "my_script_name" becomes "My Script Name".
// Allocates new memory, which the caller must free.
char* formatMenuItemName(const char* filename_no_ext) {
    if (filename_no_ext == NULL) {
        return NULL;
    }

    size_t len = strlen(filename_no_ext);
    char* formatted_name = (char*)malloc(len + 1); // Max same length + null terminator
    if (formatted_name == NULL) {
        perror("Failed to allocate memory for formatted name");
        return NULL;
    }

    BOOL capitalize_next = TRUE; // Flag to capitalize the next character
    for (size_t i = 0; i <= len; ++i) { // Iterate including the null terminator
        if (filename_no_ext[i] == '_') {
            formatted_name[i] = ' '; // Replace underscore with space
            capitalize_next = TRUE;  // Capitalize the character after the space
        } else if (capitalize_next && filename_no_ext[i] != '\0') { // Corrected variable name
            formatted_name[i] = (char)toupper((unsigned char)filename_no_ext[i]); // Capitalize character
            capitalize_next = FALSE; // Reset flag
        } else {
            formatted_name[i] = filename_no_ext[i]; // Copy character as is
            capitalize_next = FALSE; // Reset flag
        }
    }
    formatted_name[len] = '\0'; // Ensure null termination

    return formatted_name;
}

// Function to create a .reg file for a given .bat file and attempt to import it.
// This function now takes the full path to the .bat file AND an optional icon path.
// Returns 0 on success, 1 on failure.
int createRegFileForBat(const char* bat_full_path, const char* custom_icon_full_path_optional) {
    char bat_dir[MAX_PATH];                // Directory containing the .bat file
    char bat_filename_only[MAX_PATH];      // Just the filename of the .bat file
    char reg_file_full_path[MAX_PATH];     // Full path to the generated .reg file
    char shell_execute_params[MAX_PATH + 50]; // Parameters for ShellExecute

    char* reg_bat_path = NULL;             // Escaped .bat path for .reg file
    char* reg_icon_path = NULL;            // Escaped icon path for .reg file (dynamically allocated)
    char* menu_item_name = NULL;           // Formatted name for the context menu item

    FILE* fp; // File pointer for the .reg file

    // Extract directory and filename from the full .bat path
    strcpy(bat_dir, bat_full_path);
    PathRemoveFileSpecA(bat_dir); // bat_dir now holds the directory

    strcpy(bat_filename_only, PathFindFileNameA(bat_full_path)); // bat_filename_only holds the filename
    strip_ext(bat_filename_only);
    
    // Format the menu item name (e.g., "Text Replace" -> "Text Replace")
    menu_item_name = formatMenuItemName(bat_filename_only);
    if (menu_item_name == NULL) {
        fprintf(stderr, "Error: Failed to format menu item name for %s.\n", bat_filename_only);
        return 1;
    }

    const char* final_icon_path_source; // This will hold the path to be escaped
    // Use the provided custom_icon_full_path_optional if it's valid and exists
    if (custom_icon_full_path_optional != NULL && strlen(custom_icon_full_path_optional) > 0 && fileExists(custom_icon_full_path_optional)) {
        final_icon_path_source = custom_icon_full_path_optional;
        printf("Using custom icon: %s\n", final_icon_path_source);
    } else {
        final_icon_path_source = "shell32.dll,4"; // Default folder icon from shell32.dll
        printf("Using default icon for '%s' (no custom .ico found or path invalid).\n", bat_filename_only);
    }

    // Construct the full path for the .reg file (e.g., "Add_Text_Replace_ContextMenu.reg")
    // The .reg file will be placed in the same directory as the .bat file.
    snprintf(reg_file_full_path, MAX_PATH, "%s\\Add_%s_ContextMenu.reg", bat_dir, bat_filename_only);
    reg_file_full_path[MAX_PATH - 1] = '\0'; // Ensure null termination

    // Escape backslashes in paths for .reg file content
    reg_bat_path = escapeBackslashes(bat_full_path);
    reg_icon_path = escapeBackslashes(final_icon_path_source);

    if (reg_bat_path == NULL || reg_icon_path == NULL) {
        MessageBoxA(NULL, "Error: Failed to allocate memory for escaped paths. Exiting.", "Error", MB_OK | MB_ICONERROR);
        free(reg_bat_path);
        free(reg_icon_path); // Free reg_icon_path here too
        free(menu_item_name);
        return 1;
    }

    // Open the .reg file for writing
    fp = fopen(reg_file_full_path, "w");
    if (fp == NULL) {
        char msg_buf[MAX_PATH + 100]; // Buffer for error message
        snprintf(msg_buf, sizeof(msg_buf), "Error: Could not open file '%s' for writing.\nMake sure you have write permissions in '%s'.", PathFindFileNameA(reg_file_full_path), bat_dir);
        msg_buf[sizeof(msg_buf) - 1] = '\0'; // Ensure null termination
        MessageBoxA(NULL, msg_buf, "Error", MB_OK | MB_ICONERROR);
        free(reg_bat_path);
        free(reg_icon_path); // Free reg_icon_path here too
        free(menu_item_name);
        return 1;
    }

    // Write the .reg file content
    fprintf(fp, "Windows Registry Editor Version 5.00\n\n");
    fprintf(fp, "; This file was dynamically generated for '%s' by the C program.\n\n", bat_filename_only);

    // --- Create the "Scripts" submenu for the BACKGROUND context menu ---
    fprintf(fp, "[HKEY_CURRENT_USER\\Software\\Classes\\Directory\\Background\\shell\\Scripts]\n");
    fprintf(fp, "\"Icon\"=\"shell32.dll,4\"\n");
    fprintf(fp, "\"SubCommands\"=\"\"\n\n");

    // --- Add the specific script entry within the "Scripts" submenu (BACKGROUND) ---
    fprintf(fp, "[HKEY_CURRENT_USER\\Software\\Classes\\Directory\\Background\\shell\\Scripts\\shell\\%s]\n", bat_filename_only); // Use subdir name for key
    fprintf(fp, "\"Icon\"=\"%s\"\n", reg_icon_path); // This will now be escaped
    fprintf(fp, "@=\"%s\"\n\n", menu_item_name);

    fprintf(fp, "[HKEY_CURRENT_USER\\Software\\Classes\\Directory\\Background\\shell\\Scripts\\shell\\%s\\command]\n", bat_filename_only); // Use subdir name for key
    fprintf(fp, "@=\"\\\"%s\\\" \\\"%%V\\\"\"\n\n", reg_bat_path); 

    // --- Create the "Scripts" submenu for the FOLDER context menu ---
    fprintf(fp, "[HKEY_CURRENT_USER\\Software\\Classes\\Directory\\shell\\Scripts]\n");
    fprintf(fp, "\"Icon\"=\"shell32.dll,4\"\n");
    fprintf(fp, "\"SubCommands\"=\"\"\n\n");

    // --- Add the specific script entry within the "Scripts" submenu (FOLDER) ---
    fprintf(fp, "[HKEY_CURRENT_USER\\Software\\Classes\\Directory\\shell\\Scripts\\shell\\%s]\n", bat_filename_only); // Use subdir name for key
    fprintf(fp, "\"Icon\"=\"%s\"\n", reg_icon_path); // This will now be escaped
    fprintf(fp, "@=\"%s\"\n\n", menu_item_name);

    fprintf(fp, "[HKEY_CURRENT_USER\\Software\\Classes\\Directory\\shell\\Scripts\\shell\\%s\\command]\n", bat_filename_only); // Use subdir name for key
    fprintf(fp, "@=\"\\\"%s\\\" \\\"%%1\\\"\"\n", reg_bat_path);

    fclose(fp); // Close the .reg file

    printf("'%s' has been generated in '%s'.\n", PathFindFileNameA(reg_file_full_path), bat_dir);

    // Prepare command line arguments for regedit.exe to silently import the .reg file
    snprintf(shell_execute_params, sizeof(shell_execute_params), "/s \"%s\"", reg_file_full_path);
    shell_execute_params[sizeof(shell_execute_params) - 1] = '\0'; // Ensure null termination
    printf("Attempting to run: regedit.exe %s\n", shell_execute_params);

    // Automatically import the .reg file using ShellExecuteA
    HINSTANCE result = ShellExecuteA(NULL, "open", "regedit.exe",
                                     shell_execute_params,
                                     NULL, SW_HIDE);

    // Check if ShellExecute failed (returns a value <= 32 on failure)
    if ((ULONG_PTR)result <= 32) {
        char msg_buf[MAX_PATH + 200];
        snprintf(msg_buf, sizeof(msg_buf), "Failed to automatically import the .reg file for '%s' (Error Code: %lu).\nThis often requires Administrator privileges.\nPlease double-click '%s' manually.", bat_filename_only, (ULONG_PTR)result, PathFindFileNameA(reg_file_full_path));
        msg_buf[sizeof(msg_buf) - 1] = '\0'; // Ensure null termination
        MessageBoxA(NULL, msg_buf, "Error", MB_OK | MB_ICONERROR);
        printf("Error: Failed to automatically import the .reg file for '%s' (Error Code: %lu). Please double-click '%s' manually.\n", bat_filename_only, (ULONG_PTR)result, PathFindFileNameA(reg_file_full_path));
    } else {
        printf("Context menu entry for '%s' added successfully!\n", bat_filename_only);
        // Add a small delay to ensure regedit has time to process before deleting
        Sleep(500); 
        deleteFile(reg_file_full_path);
    }

    // Free dynamically allocated memory
    free(reg_bat_path);
    free(reg_icon_path); // Free the dynamically allocated escaped icon path
    free(menu_item_name);

    return 0;
}

int main() {
    char current_exe_path[MAX_PATH]; // Buffer for the full path of this executable
    char base_dir[MAX_PATH];         // Buffer for the directory of this executable (e.g., C:\MyTool)
    char scripts_dir[MAX_PATH];      // Buffer for the 'scripts' subdirectory (e.g., C:\MyTool\scripts)
    WIN32_FIND_DATAA findFileData;   // Structure to hold file information for directories
    WIN32_FIND_DATAA batFindFileData; // Structure to hold file information for .bat files
    WIN32_FIND_DATAA icoFindFileData; // Structure to hold file information for .ico files
    HANDLE hFind = INVALID_HANDLE_VALUE; // Handle for directory search operations
    HANDLE hBatFind = INVALID_HANDLE_VALUE; // Handle for .bat file search operations
    HANDLE hIcoFind = INVALID_HANDLE_VALUE; // Handle for .ico file search operations
    char search_path_dirs[MAX_PATH]; // Path for FindFirstFile (e.g., "C:\MyTool\scripts\*")
    char search_path_bats[MAX_PATH]; // Path for FindFirstFile (e.g., "C:\MyTool\scripts\Subdir\*.bat")
    char search_path_icos[MAX_PATH]; // Path for FindFirstFile (e.g., "C:\MyTool\scripts\Subdir\*.ico")


    // 1. Get the full path of the current executable (this C program)
    if (GetModuleFileNameA(NULL, current_exe_path, MAX_PATH) == 0) {
        MessageBoxA(NULL, "Error: Could not get current executable path.", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // 2. Extract the base directory from the executable's path (e.g., C:\MyTool)
    strcpy(base_dir, current_exe_path);
    PathRemoveFileSpecA(base_dir);

    // 3. Construct the path to the 'scripts' subdirectory (e.g., C:\MyTool\scripts)
    snprintf(scripts_dir, MAX_PATH, "%s\\scripts", base_dir);
    scripts_dir[MAX_PATH - 1] = '\0';

    // Check if the 'scripts' directory exists
    DWORD scripts_dir_attributes = GetFileAttributesA(scripts_dir);
    if (scripts_dir_attributes == INVALID_FILE_ATTRIBUTES || !(scripts_dir_attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        MessageBoxA(NULL, "Error: 'scripts' directory not found or not accessible in the executable's directory.", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    printf("Scanning directory: %s for subdirectories...\n", scripts_dir);

    // Construct search path for all entries in the 'scripts' directory (e.g., "C:\MyTool\scripts\*")
    snprintf(search_path_dirs, MAX_PATH, "%s\\*", scripts_dir);
    search_path_dirs[MAX_PATH - 1] = '\0'; // Ensure null termination

    // Find the first entry in the 'scripts' directory
    hFind = FindFirstFileA(search_path_dirs, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        MessageBoxA(NULL, "Error: No entries found in 'scripts' directory or not accessible.", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Loop through all entries in the 'scripts' directory
    do {
        // Check if the entry is a directory and not "." or ".."
        if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            strcmp(findFileData.cFileName, ".") != 0 &&
            strcmp(findFileData.cFileName, "..") != 0) {
            
            char sub_dir_full_path[MAX_PATH]; // Full path to the subdirectory (e.g., "C:\MyTool\scripts\Text Replace")
            snprintf(sub_dir_full_path, MAX_PATH, "%s\\%s", scripts_dir, findFileData.cFileName);
            sub_dir_full_path[MAX_PATH - 1] = '\0';

            printf("Found subdirectory: %s\n", sub_dir_full_path);

            char bat_full_path[MAX_PATH] = {0}; // Initialize to empty string
            char ico_full_path[MAX_PATH] = {0}; // Initialize to empty string

            // --- Search for the first .bat file within this subdirectory ---
            snprintf(search_path_bats, MAX_PATH, "%s\\*.bat", sub_dir_full_path);
            search_path_bats[MAX_PATH - 1] = '\0';

            hBatFind = FindFirstFileA(search_path_bats, &batFindFileData);

            if (hBatFind != INVALID_HANDLE_VALUE) {
                // Found at least one .bat file in the subdirectory
                snprintf(bat_full_path, MAX_PATH, "%s\\%s", sub_dir_full_path, batFindFileData.cFileName);
                bat_full_path[MAX_PATH - 1] = '\0';
                printf("  Found .bat file: %s\n", bat_full_path);
                FindClose(hBatFind); // Close the inner search handle for .bat
            } else {
                printf("  Skipping subdirectory '%s': No .bat files found.\n", findFileData.cFileName);
                continue; // Skip to next directory if no .bat file is found
            }

            // --- Search for the first .ico file within this subdirectory (optional) ---
            snprintf(search_path_icos, MAX_PATH, "%s\\*.ico", sub_dir_full_path);
            search_path_icos[MAX_PATH - 1] = '\0';

            hIcoFind = FindFirstFileA(search_path_icos, &icoFindFileData);
            if (hIcoFind != INVALID_HANDLE_VALUE) {
                snprintf(ico_full_path, MAX_PATH, "%s\\%s", sub_dir_full_path, icoFindFileData.cFileName);
                ico_full_path[MAX_PATH - 1] = '\0';
                printf("  Found .ico file: %s\n", ico_full_path);
                FindClose(hIcoFind); // Close the inner search handle for .ico
            } else {
                printf("  No .ico file found in '%s'. Using default icon.\n", findFileData.cFileName);
            }
            
            // Call the function to create and import the .reg file for this .bat file
            // Pass both the .bat file path and the found .ico file path (or empty if none)
            createRegFileForBat(bat_full_path, ico_full_path);
        }
    } while (FindNextFileA(hFind, &findFileData) != 0); // Continue until no more entries are found in scripts_dir

    // Check for errors that might have occurred during enumeration (other than ERROR_NO_MORE_FILES)
    DWORD dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES) {
        char msg_buf[100];
        snprintf(msg_buf, sizeof(msg_buf), "Error enumerating directories: %lu", dwError);
        MessageBoxA(NULL, msg_buf, "Error", MB_OK | MB_ICONERROR);
        printf("Error enumerating directories: %lu\n", dwError);
    }

    FindClose(hFind); // Close the outer search handle

    printf("All relevant .bat files in subdirectories processed.\n");
    printf("You may need to restart File Explorer (or your PC) for changes to appear.\n");

    return 0;
}
