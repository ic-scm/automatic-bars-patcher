//Game configuration manager for Nintendo Switch automatic BARS patcher
//Copyright (C) 2020 I.C.

#pragma once

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

//Loads config from config file.
//Returns 0 on success, and other codes on errors.
unsigned char config_load(bars_config_storage_t* config) {
    config_free(config);
    
    
    return 0;
}

