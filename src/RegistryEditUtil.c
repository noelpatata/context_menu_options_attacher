#include "RegistryEditUtil.h"
#include "FileUtil.h"

#include <shlwapi.h>
#include <shellapi.h>

BOOL initRegContext(RegContext* context, const char* bat_full_path, const char* custom_icon_full_path_optional) {
    if (context == NULL || bat_full_path == NULL) {
        return FALSE;
    }

    strcpy(context->bat_dir, bat_full_path);
    PathRemoveFileSpecA(context->bat_dir);

    strcpy(context->bat_filename_only, PathFindFileNameA(bat_full_path));
    strip_ext(context->bat_filename_only);
    
    context->menu_item_name = formatMenuItemName(context->bat_filename_only);
    if (context->menu_item_name == NULL) {
        fprintf(stderr, "      Error: Formateando nombre del archivo -> %s.\n", context->bat_filename_only);
        return FALSE;
    }

    const char* icon_path_source;
    if (custom_icon_full_path_optional != NULL && strlen(custom_icon_full_path_optional) > 0 && fileExists(custom_icon_full_path_optional)) {
        icon_path_source = custom_icon_full_path_optional;
        printf("      Usando .ico custom: %s\n", icon_path_source);
    } else {
        icon_path_source = "shell32.dll,4";
        printf("      Usando .ico por defecto ya que no se ha encontrado el icono para '%s'.\n", context->bat_filename_only);
    }

    char reg_filename[MAX_PATH + 20];
    snprintf(reg_filename, sizeof(reg_filename), "Add_%s_ContextMenu.reg", context->bat_filename_only);
    reg_filename[sizeof(reg_filename) - 1] = '\0';

    PathCombineA(context->reg_file_full_path, context->bat_dir, reg_filename);
    context->reg_file_full_path[sizeof(context->reg_file_full_path) - 1] = '\0';

    context->escaped_bat_path = addEscapeBackslashes(bat_full_path);
    context->escaped_icon_path = addEscapeBackslashes(icon_path_source);

    if (context->escaped_bat_path == NULL || context->escaped_icon_path == NULL) {
        MessageBoxA(NULL, "      Error: Error añadiendo '\\' de escapado para el .reg, no se puede continuar.", "Error", MB_OK | MB_ICONERROR);
        freeRegContext(context);
        return FALSE;
    }
    return TRUE;
}

void freeRegContext(RegContext* context) {
    if (context == NULL) return;
    free(context->menu_item_name);
    free(context->escaped_bat_path);
    free(context->escaped_icon_path);
    context->menu_item_name = NULL;
    context->escaped_bat_path = NULL;
    context->escaped_icon_path = NULL;
}

BOOL writeRegFile(const RegContext* context) {
    if (context == NULL) return FALSE;

    FILE* fp = fopen(context->reg_file_full_path, "w");
    if (fp == NULL) {
        char msg_buf[MAX_PATH + 100];
        snprintf(msg_buf, sizeof(msg_buf), "Error: Escribiendo en '%s'.\nRevisa los permisos que tienes sobre '%s'.", PathFindFileNameA(context->reg_file_full_path), context->bat_dir);
        msg_buf[sizeof(msg_buf) - 1] = '\0';
        MessageBoxA(NULL, msg_buf, "Error", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    fprintf(fp, "Windows Registry Editor Version 5.00\n\n");
    fprintf(fp, "; This file was dynamically generated for '%s' by the C program.\n\n", context->bat_filename_only);

    fprintf(fp, "[HKEY_CURRENT_USER\\Software\\Classes\\Directory\\Background\\shell\\Scripts]\n");
    fprintf(fp, "\"Icon\"=\"shell32.dll,4\"\n");
    fprintf(fp, "\"SubCommands\"=\"\"\n\n");

    fprintf(fp, "[HKEY_CURRENT_USER\\Software\\Classes\\Directory\\Background\\shell\\Scripts\\shell\\%s]\n", context->bat_filename_only);
    fprintf(fp, "\"Icon\"=\"%s\"\n", context->escaped_icon_path);
    fprintf(fp, "@=\"%s\"\n\n", context->menu_item_name);

    fprintf(fp, "[HKEY_CURRENT_USER\\Software\\Classes\\Directory\\Background\\shell\\Scripts\\shell\\%s\\command]\n", context->bat_filename_only);
    fprintf(fp, "@=\"\\\"%s\\\" \\\"%%V\\\"\"\n\n", context->escaped_bat_path); 

    fprintf(fp, "[HKEY_CURRENT_USER\\Software\\Classes\\Directory\\shell\\Scripts]\n");
    fprintf(fp, "\"Icon\"=\"shell32.dll,4\"\n");
    fprintf(fp, "\"SubCommands\"=\"\"\n\n");

    fprintf(fp, "[HKEY_CURRENT_USER\\Software\\Classes\\Directory\\shell\\Scripts\\shell\\%s]\n", context->bat_filename_only);
    fprintf(fp, "\"Icon\"=\"%s\"\n", context->escaped_icon_path);
    fprintf(fp, "@=\"%s\"\n\n", context->menu_item_name);

    fprintf(fp, "[HKEY_CURRENT_USER\\Software\\Classes\\Directory\\shell\\Scripts\\shell\\%s\\command]\n", context->bat_filename_only);
    fprintf(fp, "@=\"\\\"%s\\\" \\\"%%1\\\"\"\n", context->escaped_bat_path);

    fclose(fp);
    printf("'%s' generado con éxito en '%s'.\n", PathFindFileNameA(context->reg_file_full_path), context->bat_dir);
    return TRUE;
}

int applyRegFile(const RegContext* context) {
    if (context == NULL) return 1;

    char shell_execute_params[sizeof(context->reg_file_full_path)+5];
    snprintf(shell_execute_params, sizeof(shell_execute_params), "/s \"%s\"", context->reg_file_full_path);
    shell_execute_params[sizeof(shell_execute_params) - 1] = '\0';
    printf("Intentando ejecutar: regedit.exe %s\n", shell_execute_params);

    HINSTANCE result = ShellExecuteA(NULL, "open", "regedit.exe",
                                     shell_execute_params,
                                     NULL, SW_HIDE);

    if ((ULONG_PTR)result <= 32) {
        char msg_buf[MAX_PATH + 200];
        snprintf(msg_buf, sizeof(msg_buf), "Error importando el archivo .reg del '%s' (Error Code: %llu).\nEs posible que se requieran permisos elevados.\nPuedes probar a hacer doble click en '%s' para registrarlo manualmente.", context->bat_filename_only, (ULONG_PTR)result, PathFindFileNameA(context->reg_file_full_path));
        msg_buf[sizeof(msg_buf) - 1] = '\0';
        MessageBoxA(NULL, msg_buf, "Error", MB_OK | MB_ICONERROR);
        return 1;
    } else {
        printf("Se ha añadido '%s' al menú contextual!\n", context->bat_filename_only);
        Sleep(500); 
        deleteFile(context->reg_file_full_path);
        return 0;
    }
}