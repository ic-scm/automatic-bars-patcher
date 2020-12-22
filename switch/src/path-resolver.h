//Filesystem path resolver
//Copyright (C) 2020 I.C.

#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <string.h>

void path_resolve(char* (&path)) {
    //Allocate temporary buffer
    uint32_t input_len = strlen(path);
    uint32_t buf_size = input_len * 4;
    while(buf_size % 256 != 0) buf_size++;
    char* res = (char*)malloc(buf_size);
    
    //Return unmodified on malloc error
    if(res == NULL) return;
    
    //Fill buffer with null
    memset(res, 0, buf_size);
    
    //Search for *
    uint32_t input_star_pos = 0;
    
    for(uint32_t i=0; i<input_len; i++) {
        if(path[i] == '*') {
            input_star_pos = i;
            break;
        }
    }
    
    if(input_star_pos == 0) {
        //Not found, return unmodified
        free(res);
        return;
    }
    
    //Read back from * to / into temporary buffer
    uint32_t i_pos;
    uint32_t tmpbuf_key_pos = buf_size - 1;
    for(i_pos = input_star_pos - 1; i_pos > 0 && path[i_pos] != '/'; i_pos--) {
        res[--tmpbuf_key_pos] = path[i_pos];
    }
    
    
    //Read back the path before into beginning of temporary buffer
    i_pos++;
    for(uint32_t i=0; i < i_pos; i++) {
        res[i] = path[i];
    }
    
    
    //Open directory in beginning of path and take first entry matching search key
    DIR* dir;
    dirent* dir_entry;
    dir = opendir(res);
    
    if(dir == NULL) {
        //Give up and return unmodified
        free(res);
        return;
    }
    
    bool found = 0;
    
    while((dir_entry = readdir(dir)) != NULL) {
        if(strstr(dir_entry->d_name, res + tmpbuf_key_pos) == dir_entry->d_name) {
            found = 1;
            strcat(res, dir_entry->d_name);
            break;
        }
    }
    
    closedir(dir);
    
    if(!found) {
        //Return unmodified
        free(res);
        return;
    }
    
    
    //Add the rest of the path into result
    strcat(res, path + input_star_pos + 1);
    
    //Reallocate
    char* res_tmp = (char*)realloc(res, strlen(res) + 1);
    if(res_tmp == NULL) {
        //Give up and return unmodified
        free(res);
        return;
    }
    
    res = res_tmp;
    
    //Replace input data with result
    free(path);
    path = res;
}

