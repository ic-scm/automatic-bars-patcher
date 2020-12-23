//Game configuration manager for Nintendo Switch automatic BARS patcher
//Copyright (C) 2020 I.C.

#pragma once
#include <stdio.h>
#include <string.h>

//Default error message for memory allocation errors.
const char* malloc_errstr = "\x1b[31mMemory allocation error. Please make sure you have enough free memory, and try launching this program from a title instead of the album.\x1b[0m\n";

// ------------------------------


//BARS patching configs.

const char* config_path = "/switch/auto-bars-patcher/game-config.txt";

struct bars_game_config_t {
    //No newline characters allowed in any of these strings.
    //Short string for identification. Use only a-z, 0-9
    char* id;
    //Full game title.
    char* full_name;
    
    //Path to original unmodified BARS file.
    char* bars_path;
    //Path to original unmodified stream directory.
    char* stream_dir;
    //Path to modded stream directory.
    char* mod_stream_dir;
    //Output path for the patched BARS file.
    char* output_bars_path;
};

//Limit of game entries in the configuration file.
#define MAX_CONFIG_SIZE 128

struct bars_config_storage_t {
    bars_game_config_t* entries [MAX_CONFIG_SIZE];
    uint16_t entries_loaded;
};


#include "default_config.h"

//Initializes config storage.
void config_init(bars_config_storage_t* config) {
    config->entries_loaded = 0;
    config_free(config);
}

//Frees a config entry.
void config_free_entry(bars_game_config_t* (&entry)) {
    free(entry->id);
    free(entry->full_name);
    free(entry->bars_path);
    free(entry->stream_dir);
    free(entry->mod_stream_dir);
    free(entry->output_bars_path);
    
    free(entry);
    entry = NULL;
}

//Frees all allocated config entries.
void config_free(bars_config_storage_t* config) {
    uint16_t i;
    
    for(i = 0; i < config->entries_loaded && i < MAX_CONFIG_SIZE; i++) {
        config_free_entry(config->entries[i]);
    }
    
    //Wipe everything else with nullptr
    for(; i < MAX_CONFIG_SIZE; i++) {
        config->entries[i] = NULL;
    }
    
    config->entries_loaded = 0;
}

//Allocates an entry and its strings with data.
//Returns 0 on success, and 1 on memory error.
bool config_alloc_load_entry(bars_game_config_t* (&entry), const char* id, const char* full_name, const char* bars_path, const char* stream_dir, const char* mod_stream_dir, const char* output_bars_path) {
    entry = (bars_game_config_t*)malloc(sizeof(bars_game_config_t));
    if(entry == NULL) return 1;
    
    entry->id = NULL;
    entry->full_name = NULL;
    entry->bars_path = NULL;
    entry->stream_dir = NULL;
    entry->mod_stream_dir = NULL;
    entry->output_bars_path = NULL;
    
    entry->id = (char*)malloc(strlen(id) + 1);
    entry->full_name = (char*)malloc(strlen(full_name) + 1);
    entry->bars_path = (char*)malloc(strlen(bars_path) + 1);
    entry->stream_dir = (char*)malloc(strlen(stream_dir) + 1);
    entry->mod_stream_dir = (char*)malloc(strlen(mod_stream_dir) + 1);
    entry->output_bars_path = (char*)malloc(strlen(output_bars_path) + 1);
    
    if(entry->id == NULL || entry->full_name == NULL || entry->bars_path == NULL || entry->stream_dir == NULL || entry->mod_stream_dir == NULL || entry->output_bars_path == NULL) {
        config_free_entry(entry);
        return 1;
    }
    
    strcpy(entry->id, id);
    strcpy(entry->full_name, full_name);
    strcpy(entry->bars_path, bars_path);
    strcpy(entry->stream_dir, stream_dir);
    strcpy(entry->mod_stream_dir, mod_stream_dir);
    strcpy(entry->output_bars_path, output_bars_path);
    
    return 0;
}


//Writes config to config file (config_path).
//Returns 0 on success, and 1 on file errors.
bool config_write(bars_config_storage_t* config) {
    //Try opening file
    FILE* cfile;
    cfile = fopen(config_path, "wb");
    if(cfile == NULL) return 1;
    
    
    //Write config data to file
    for(uint16_t entry = 0; entry < config->entries_loaded; entry++) {
        //Header for this game config
        fprintf(cfile, "^%s\n", config->entries[entry]->id);
        //Full game name
        fprintf(cfile, "full_name=%s\n", config->entries[entry]->full_name);
        //BARS path
        fprintf(cfile, "bars_path=%s\n", config->entries[entry]->bars_path);
        //Stream directory
        fprintf(cfile, "stream_dir=%s\n", config->entries[entry]->stream_dir);
        //Modded stream directory
        fprintf(cfile, "mod_stream_dir=%s\n", config->entries[entry]->mod_stream_dir);
        //Output BARS path
        fprintf(cfile, "output_bars_path=%s\n\n", config->entries[entry]->output_bars_path);
    }
    
    
    //Check for file errors and close file
    if(ferror(cfile) != 0) {
        fclose(cfile);
        return 1;
    }
    
    fclose(cfile);
    
    return 0;
}

//Loads config from config file (config_path).
//Returns 0 on success, and other codes on errors.
/*
 * Error codes:
 * 1 - Memory error
 * 2 - File I/O error
 * 3 - File too big
 * 4 - Config has too many entries
 * 
 */
unsigned char config_read(bars_config_storage_t* config) {
    config_free(config);
    
    FILE* cfile;
    size_t fsize;
    char* fbuf;
    cfile = fopen(config_path, "rb");
    
    if(cfile == NULL) {
        //Load and write default config if the file doesn't exist
        if(errno == ENOENT) {
            bool cres = config_load_default(config);
            if(cres) return 1;
            
            //This function can fail, the config_load function will most likely load the default config again on the next run.
            config_write(config);
            
            return 0;
        }
        
        //Return with error otherwise
        return 2;
    }
    else {
        //Read full file into memory
        fseek(cfile, 0, SEEK_END);
        
        fsize = ftell(cfile);
        
        fseek(cfile, 0, SEEK_SET);
        
        //Check if file size isn't too big (4MB)
        if(fsize > 4000000) {
            fclose(cfile);
            return 3;
        }
        
        fbuf = (char*)malloc(fsize);
        if(fbuf == NULL) {
            fclose(cfile);
            return 1;
        }
        
        size_t bytes_read = fread(fbuf, 1, fsize, cfile);
        if(bytes_read != fsize || ferror(cfile) != 0) {
            fclose(cfile);
            free(fbuf);
            return 2;
        }
        
        fclose(cfile);
    }
    
    //Read line by line
    uint32_t pos = 0;
    
    //Size of line buffer
    const uint16_t lsize = 4096;
    //Character space for null terminated string
    uint16_t lsizen = lsize - 1;
    //Length of string in buffer
    uint16_t llen;
    char line[lsize];
    
    //Parsing state
    int32_t entry_pos = -1;
    //String key storage, one array for key in config, and second for pointers to the write location.
    const uint16_t key_array_size = 5;
    const char* key_array[key_array_size] = {"full_name=", "bars_path=", "stream_dir=", "mod_stream_dir=", "output_bars_path="};
    char** key_array_output[key_array_size] = {NULL, NULL, NULL, NULL, NULL};
    
    
    while(pos < fsize) {
        //Read until newline
        llen = 0;
        while(pos < fsize && fbuf[pos] != '\n' && fbuf[pos] != '\r') {
            line[llen++] = fbuf[pos++];
            //Don't overflow line buffer while also reading until newline in fbuf.
            //This part does not need to be efficient as this will rarely happen,
            //in normal config files you would never have a line of config that is more than 4096 characters.
            if(llen >= lsizen) llen--;
        }
        
        //Terminate line
        line[llen] = '\0';
        
        //Make sure llen is correct
        llen = strlen(line);
        
        //Increment position in fbuf until we are no longer on a LF or CR character.
        while(pos < fsize && (fbuf[pos] == '\n' || fbuf[pos] == '\r')) pos++;
        
        //Continue if the line is a comment, empty, etc.
        if(llen == 0) continue;
        if(line[0] == '#' || line[0] == ' ') continue;
        
        //Continue if the line is not a game header and we haven't gotten any yet.
        if(entry_pos == -1 && line[0] != '^') continue;
        
        //Parse current line
        
        //New game header
        if(line[0] == '^') {
            entry_pos++;
            //Give up if we reached the limit of game entries
            if(entry_pos >= MAX_CONFIG_SIZE) {
                free(fbuf);
                config_free(config);
                return 4;
            }
            
            //Allocate the game entry
            config->entries[entry_pos] = (bars_game_config_t*)malloc(sizeof(bars_game_config_t));
            if(config->entries[entry_pos] == NULL) {
                free(fbuf);
                config_free(config);
                return 1;
            }
            
            //Write GameID
            config->entries[entry_pos]->id = (char*)malloc(llen);
            if(config->entries[entry_pos]->id == NULL) {
                free(config->entries[entry_pos]);
                free(fbuf);
                config_free(config);
                return 1;
            }
            
            strcpy(config->entries[entry_pos]->id, line + 1);
            
            //Set pointer addresses in key array
            key_array_output[0] = &config->entries[entry_pos]->full_name;
            key_array_output[1] = &config->entries[entry_pos]->bars_path;
            key_array_output[2] = &config->entries[entry_pos]->stream_dir;
            key_array_output[3] = &config->entries[entry_pos]->mod_stream_dir;
            key_array_output[4] = &config->entries[entry_pos]->output_bars_path;
            
            //Null and allocate all pointers with empty strings
            for(uint16_t k=0; k < key_array_size; k++)
                *key_array_output[k] = NULL;
            
            for(uint16_t k=0; k < key_array_size; k++)
                *key_array_output[k] = (char*)malloc(1);
            
            bool malloc_error = 0;
            for(uint16_t k=0; k < key_array_size; k++)
                if(*key_array_output[k] == NULL) { malloc_error = 1; break; }
            
            if(malloc_error) {
                //Free previously allocated things in this entry, free everything else allocated in this function, return with error
                for(uint16_t k=0; k < key_array_size; k++)
                    free(*key_array_output[k]);
                
                free(config->entries[entry_pos]->id);
                free(config->entries[entry_pos]);
                free(fbuf);
                config_free(config);
                return 1;
            }
            
            //Null the allocated strings
            for(uint16_t k=0; k < key_array_size; k++)
                *key_array_output[k][0] = '\0';
            
            config->entries_loaded++;
        }
        
        //Anything else, after game header
        else {
            //Check if current line matches any keys
            bool matched = 0;
            uint16_t key;
            
            for(uint16_t k=0; k < key_array_size; k++) {
                if(strstr(line, key_array[k]) == line) {
                    matched = 1;
                    key = k;
                    break;
                }
            }
            
            //Write the rest of the current line to matching key output
            if(matched) {
                //Starting position in line of string to write
                uint32_t start_pos = strlen(key_array[key]);
                
                char* tmpstr = (char*)malloc((llen + 1) - start_pos);
                if(tmpstr == NULL) {
                    free(fbuf);
                    config_free(config);
                    return 1;
                }
                
                strcpy(tmpstr, line + start_pos);
                
                //Replace current string in config with newly loaded string
                free(*key_array_output[key]);
                *key_array_output[key] = tmpstr;
            }
        }
        
    }
    
    free(fbuf);
    
    return 0;
}

