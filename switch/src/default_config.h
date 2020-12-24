//Permanent default configuration.
//Copyright (C) 2020 I.C.

//If you are here to add a new game configuration, look to the bottom of this file.

void config_free(bars_config_storage_t*);
bool config_alloc_load_entry(bars_game_config_t*(&), const char*, const char*, const char*, const char*, const char*, const char*);

//Loads the permanent default configuration into config storage.
//Returns 0 on success, and 1 on memory error.
bool config_load_default(bars_config_storage_t* config) {
    config_free(config);
    
    bool res;
    
    //Animal Crossing: New Horizons
    const char* acnh_id = "acnh";
    const char* acnh_full_name = "Animal Crossing: New Horizons";
    const char* acnh_bars_path = "/switch/nxdumptool/RomFS/Animal Crossing_ New Horizons*/Sound/Resource/Bgm_Base.bars";
    const char* acnh_stream_dir = "/switch/nxdumptool/RomFS/Animal Crossing_ New Horizons*/Sound/Resource/Stream/";
    const char* acnh_mod_stream_dir = "/atmosphere/contents/01006F8002326000/romfs/Sound/Resource/Stream/";
    const char* acnh_output_bars_path = "/atmosphere/contents/01006F8002326000/romfs/Sound/Resource/Bgm_Base.bars";
    
    res = config_alloc_load_entry(config->entries[config->entries_loaded],
        acnh_id, acnh_full_name, acnh_bars_path, acnh_stream_dir, acnh_mod_stream_dir, acnh_output_bars_path
    );
    if(res) { config_free(config); return 1; }
    config->entries_loaded++;
    
    
    //Example configuration for new game
    //Please add new games below the last entry and above this example entry.
    /*
    const char* newgame_id = "newgame";
    const char* newgame_full_name = "Example Game";
    
    //One '*' character at the end of a directory name in the path can be used and will be automatically resolved to the full directory path.
    //Useful when NXDUMPTOOL dumps the game into a directory that includes the update version in its name.
    //This works for all 4 path strings.
    
    const char* newgame_bars_path = "/switch/nxdumptool/RomFS/Example Game/Sound/Resource/Bgm_Base.bars";
    const char* newgame_stream_dir = "/switch/nxdumptool/RomFS/Example Game/Sound/Resource/Stream";
    const char* newgame_mod_stream_dir = "/atmosphere/contents/---/romfs/Sound/Resource/Stream";
    const char* newgame_output_bars_path = "/atmosphere/contents/---/romfs/Sound/Resource/Bgm_Base.bars";
    
    res = config_alloc_load_entry(config->entries[config->entries_loaded],
        newgame_id, newgame_full_name, newgame_bars_path, newgame_stream_dir, newgame_mod_stream_dir, newgame_output_bars_path
    );
    if(res) { config_free(config); return 1; }
    config->entries_loaded++;
    */
    
    return 0;
}
