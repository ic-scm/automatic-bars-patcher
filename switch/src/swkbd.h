//libnx SwKbd code for automatic BARS patcher
//Copyright (C) 2020 I.C.

#include <stdlib.h>
#include <switch.h>

void swkbd_edit_text(char* (&str), const char* hint) {
    SwkbdConfig swkbd;
    
    uint32_t res;
    
    //Allocate output buffer 4 times size input, aligned to 0x100
    uint32_t res_str_bufsize = strlen(str) * 4;
    while(res_str_bufsize % 256 != 0) res_str_bufsize++;
    char* res_str = (char*)malloc(res_str_bufsize);
    if(res_str == NULL) return;
    
    //Allocate swkbd
    res = swkbdCreate(&swkbd, 0);
    if(res != 0) {
        free(res_str);
        return;
    }
    
    //Set initial and hint text
    swkbdConfigSetInitialText(&swkbd, str);
    swkbdConfigSetGuideText(&swkbd, hint);
    
    //Set cursor position to the end of current text
    swkbdConfigSetInitialCursorPos(&swkbd, 1);
    
    //Set normal type and blurred background
    swkbdConfigSetType(&swkbd, SwkbdType_Normal);
    swkbdConfigSetBlurBackground(&swkbd, 1);
    
    //Show keyboard to user and wait for it to finish
    res = swkbdShow(&swkbd, res_str, res_str_bufsize);
    
    //Free swkbd
    swkbdClose(&swkbd);
    
    if(res != 0) {
        free(res_str);
        return;
    }
    
    //Realloc output buffer to size of string
    char* res_str_tmp = (char*)realloc(res_str, strlen(res_str) + 1);
    if(res_str_tmp == NULL) {
        free(res_str);
        return;
    }
    
    res_str = res_str_tmp;
    
    //Replace input data with result
    free(str);
    str = res_str;
}
