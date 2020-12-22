//Nintendo Switch homebrew app for automatic BARS patcher
//Copyright (C) 2020 I.C.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <switch.h>

#define BARSPATCHER_VERSION_NX
#include "../../bars-patcher-core/bars-patcher.h"

#include "config.h"
#include "swkbd.h"
#include "path-resolver.h"

int main(int argc, char** args) {
    //Initial libnx configuration
    consoleInit(NULL);
    //Controllers
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);
    uint64_t kDown;
    
    //Redirect all stderr output to stdout.
    dup2(1, 2);
    
    //Print initial welcome message
    printf("Automatic BARS Patcher \x1b[32m%s\x1b[0m\nCopyright (C) 2020 I.C.\nThis program is free software, see the license file for more information.\n", barspatcher_getVersionString());
    printf("Report issues to \x1b[36mhttps://github.com/ic-scm/automatic-bars-patcher/issues\x1b[0m.\n\n");
    
    //Initialize config storage directory for this program.
    mkdir("/switch", 0777);
    mkdir("/switch/auto-bars-patcher", 0777);
    
    //Initialize config storage.
    bars_config_storage_t* config_storage = (bars_config_storage_t*)malloc(sizeof(bars_config_storage_t));
    
    //Check for memory allocation errors
    if(config_storage == NULL) {
        printf("%s", malloc_errstr);
        consoleUpdate(NULL);
        usleep(5000000);
        abort();
    }
    
    config_init(config_storage);
    
    if( config_load_default(config_storage) ) {
        printf("%s", malloc_errstr);
        consoleUpdate(NULL);
        usleep(5000000);
        abort();
    }
    
    if(config_storage->entries_loaded == 0) {
        printf("\x1b[31mError:\x1b[0m There are no games in the config file. Please check the game-config.txt file, or delete it to load the default configuration.\n");
        consoleUpdate(NULL);
        usleep(5000000);
        exit(1);
    }
    
    
    {
        //Stage 1: Game selection by user.
        printf("Games in config file: \x1b[32m%d\x1b[0m. Select a game!\n\x1b[33m( < > Select   (A) Confirm )\x1b[0m\n\n", config_storage->entries_loaded);
        uint16_t selected_game = 0;
        bool text_refresh = 1;
        bool confirmed = 0;
        bool editing_paths = 0;
        
        while(1) {
            if(!appletMainLoop()) goto main_exit;
            //Refresh inputs
            padUpdate(&pad);
            kDown = padGetButtonsDown(&pad);
            //Exit on (+) press
            if(kDown & HidNpadButton_Plus) goto main_exit;
            //Confirm and break on (A) press
            if(kDown & HidNpadButton_A) {
                confirmed = 1;
                text_refresh = 1;
            }
            
            //Selection controls
            if(kDown & HidNpadButton_Left) {
                text_refresh = 1;
                if(selected_game != 0) selected_game--;
                else selected_game = config_storage->entries_loaded - 1;
            }
            if(kDown & HidNpadButton_Right) {
                text_refresh = 1;
                if(selected_game < config_storage->entries_loaded - 1) selected_game++;
                else selected_game = 0;
            }
            
            //Selection display
            if(text_refresh) {
                text_refresh = 0;
                
                //Fill line with 60 spaces
                printf("\r");
                for(uint8_t i=0; i<6; i++) printf("          ");
                
                printf("\r(%d/%d) %s[ %s ]\x1b[0m", selected_game+1, config_storage->entries_loaded, (confirmed ? "\x1b[32m" : ""), config_storage->entries[selected_game]->full_name);
                
                //Break on confirmed flag
                if(confirmed) {
                    printf("\n\n");
                    confirmed = 0;
                    break;
                }
            }
            
            consoleUpdate(NULL);
        }
        
        //Stage 2: Confirm and edit game settings.
        
        //Running in a loop when editing paths.
        editing_paths = 1;
        while(editing_paths) {
            editing_paths = 0;
            
            //Resolve for * in all paths
            path_resolve(config_storage->entries[selected_game]->bars_path);
            path_resolve(config_storage->entries[selected_game]->stream_dir);
            path_resolve(config_storage->entries[selected_game]->mod_stream_dir);
            path_resolve(config_storage->entries[selected_game]->output_bars_path);
            
            printf("\x1b[35mOriginal BARS file: \x1b[0m%s\n\n\x1b[35mOriginal BWAV folder: \x1b[0m%s\n\n\x1b[35mModded BWAV folder: \x1b[0m%s\n\n\x1b[35mPatched BARS output: \x1b[0m%s\n\n", 
                   config_storage->entries[selected_game]->bars_path,
                   config_storage->entries[selected_game]->stream_dir,
                   config_storage->entries[selected_game]->mod_stream_dir,
                   config_storage->entries[selected_game]->output_bars_path
            );
            
            printf("\x1b[36mAre these settings correct?\x1b[0m \x1b[33m( (A) Accept & Patch   (B) Edit )\x1b[0m\n");
            
            while(1) {
                if(!appletMainLoop()) goto main_exit;
                //Refresh inputs
                padUpdate(&pad);
                kDown = padGetButtonsDown(&pad);
                //Exit on (+) press
                if(kDown & HidNpadButton_Plus) goto main_exit;
                //Confirm and break on (A) press
                if(kDown & HidNpadButton_A) {
                    confirmed = 1;
                    text_refresh = 1;
                }
                //Break and go into edit mode on (B) press
                if(kDown & HidNpadButton_B) {
                    confirmed = 1;
                    text_refresh = 1;
                    editing_paths = 1;
                }
                
                //Selection display
                if(text_refresh) {
                    text_refresh = 0;
                    //Break on confirmed flag
                    if(confirmed) {
                        printf("\n");
                        confirmed = 0;
                        break;
                    }
                }
                
                consoleUpdate(NULL);
            }
            
            
            //Stage 2.5 optional: Editing paths
            if(editing_paths) {
                
                //Currently select path. (1-4: bars_path -> output_bars_path, 0: Done)
                uint32_t selected_path = 0;
                
                while(1) {
                    printf("\x1b[36mWhich setting would you like to edit?\x1b[0m\n\x1b[33m( ( <  ) Original BARS file   (  > ) Original BWAV folder )\x1b[0m\n\x1b[33m( ( /\\ ) Modded BWAV folder   ( \\/ ) Patched BARS output  )\x1b[0m\n");
                    printf("\x1b[33m( (B) Done )\x1b[0m\n\n");
                    
                    //Wait for input
                    while(1) {
                        if(!appletMainLoop()) goto main_exit;
                        //Refresh inputs
                        padUpdate(&pad);
                        kDown = padGetButtonsDown(&pad);
                        //Exit on (+) press
                        if(kDown & HidNpadButton_Plus) goto main_exit;
                        
                        //Take inputs
                        if(kDown & HidNpadButton_B) { selected_path = 0; break; }
                        if(kDown & HidNpadButton_Left) { selected_path = 1; break; }
                        if(kDown & HidNpadButton_Right) { selected_path = 2; break; }
                        if(kDown & HidNpadButton_Up) { selected_path = 3; break; }
                        if(kDown & HidNpadButton_Down) { selected_path = 4; break; }
                        
                        consoleUpdate(NULL);
                    }
                    
                    if(selected_path == 0) break;
                    
                    //Edit path with system keyboard
                    if(selected_path == 1) {
                        swkbd_edit_text(config_storage->entries[selected_game]->bars_path, "Original BARS file path");
                        printf("\x1b[35mNew original BARS file: \x1b[0m%s\n\n", config_storage->entries[selected_game]->bars_path);
                    }
                    else if(selected_path == 2) {
                        swkbd_edit_text(config_storage->entries[selected_game]->stream_dir, "Original BWAV folder path");
                        printf("\x1b[35mNew original BWAV folder: \x1b[0m%s\n\n", config_storage->entries[selected_game]->stream_dir);
                    }
                    else if(selected_path == 3) {
                        swkbd_edit_text(config_storage->entries[selected_game]->mod_stream_dir, "Modded BWAV folder path");
                        printf("\x1b[35mNew modded BWAV folder: \x1b[0m%s\n\n", config_storage->entries[selected_game]->mod_stream_dir);
                    }
                    else if(selected_path == 4) {
                        swkbd_edit_text(config_storage->entries[selected_game]->output_bars_path, "Patched BARS output path");
                        printf("\x1b[35mNew patched BARS output: \x1b[0m%s\n\n", config_storage->entries[selected_game]->output_bars_path);
                    }
                }
                
                //Ask about saving to config file
                printf("\x1b[36mWould you like to save the new settings to the config file?\x1b[0m\n\x1b[33m( (A) Yes   (B) No )\x1b[0m\n");
                //Wait for input, and write config file on A press
                while(1) {
                    if(!appletMainLoop()) goto main_exit;
                    //Refresh inputs
                    padUpdate(&pad);
                    kDown = padGetButtonsDown(&pad);
                    //Exit on (+) press
                    if(kDown & HidNpadButton_Plus) goto main_exit;
                    
                    //Take inputs
                    if(kDown & HidNpadButton_A) {
                        //TODO config writer
                        
                        break;
                    }
                    if(kDown & HidNpadButton_B) break;
                    
                    consoleUpdate(NULL);
                }
                
                printf("\n");
            }
            
        }
        
        //Stage 3: Main BARS patching.
        //TODO: Run barspatcher_run on another thread
        printf("\x1b[32mRunning BARS patcher, please wait...\x1b[0m\n");
        consoleUpdate(NULL);
        
        unsigned char bars_res;
        bars_res = barspatcher_run(0,
            config_storage->entries[selected_game]->stream_dir,
            config_storage->entries[selected_game]->mod_stream_dir,
            config_storage->entries[selected_game]->bars_path,
            config_storage->entries[selected_game]->output_bars_path
        );
        
        printf("\n");
        
        if(bars_res == 0) {
            printf("\x1b[32mSuccess!\x1b[0m");
        }
        else if(bars_res > 0 && bars_res < 100) {
            printf("\x1b[33mWarning: Some tracks were skipped. Patched tracks will still work in the game. \x1b[0m");
        }
        else if(bars_res > 100) {
            printf("\x1b[31mBARS patching error.\x1b[0m");
        }
        
        printf(" Press (+) to exit.\n");
        
        
        //Stage 4: Done, wait for exit command
        while(appletMainLoop()) {
            //Refresh inputs
            padUpdate(&pad);
            kDown = padGetButtonsDown(&pad);
            
            //Exit on (+) press
            if(kDown & HidNpadButton_Plus) break;
            
            consoleUpdate(NULL);
        }
        
    }
    
    main_exit:
    
    config_free(config_storage);
    free(config_storage);
    consoleExit(NULL);
    return 0;
}
