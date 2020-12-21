//Automatic BARS patcher
//Copyright (C) 2020 I.C.
//https://github.com/ic-scm/automatic-bars-patcher

//This patcher is based on the original simple version of the BARS patcher.
//See here: https://github.com/ic-scm/bars-patcher

#pragma once
#include <stdio.h>
#include <errno.h>
#include <fstream>
#include <cstring>

//Slicing functions
#include "utils.h"

//Platform specific libraries
#ifdef BARSPATCHER_VERSION_PC
#include <dirent.h>
#else
#error "No supported BARSPATCHER_VERSION defined."
#endif

//Limit of directory entries when reading the mod stream directory.
#define BARSPATCHER_DIRLIST_LIMIT 8192

//Bytes allocated for full file path strings
#define BARSPATCHER_PATHSTR_BYTES 16384

//Bytes allocated for reading BWAV file headers
#define BARSPATCHER_OGBWAV_MEMBLOCK_SIZE 0x100
#define BARSPATCHER_MODBWAV_MEMBLOCK_SIZE 65536

const char* barspatcher_version = "v0.1.0";

//Returns the version string of this code.
const char* barspatcher_getVersionString() {return barspatcher_version;}

//Returns error string from code returned by barspatcher_run.
const char* barspatcher_getErrorString(unsigned char code) {
    switch(code) {
        case 0: return "Success";
        case 255: return "Could not open input BARS file";
        case 254: return "Could not read input BARS file";
        case 253: return "Input BARS file is too big";
        case 249: return "Could not open output BARS file for writing";
        case 248: return "Could not save output BARS file";
        case 239: return "Could not open original BWAV files";
        case 238: return "Could not open modded BWAV files";
        case 237: return "Could not read original BWAV files";
        case 236: return "Could not read modded BWAV files";
        case 229: return "Could not open modded BWAV directory";
        case 228: return "The modded BWAV directory has no files";
        case 200: return "All tracks were skipped; BARS file was not patched";
        case 101: return "Directory path too long";
        case 100: return "Memory allocation error";
    }
    
    if(code > 0 && code < 100) return "Some files were skipped";
    
    return "Unknown error";
}

/*
 * Main BARS patcher function
 * 
 * verbose - Verbose output
 * og_stream_dirname - Path to directory with original BWAV files
 * mod_stream_dirname - Path to directory with modded BWAV files
 * bars_input_filename - Path to original unmodified BARS file
 * bars_output_filename - Path for the output patched BARS file
 * 
 * Returns:
 * 0 - No error
 * 1 to 99 - Number of skipped files (99 could mean 99 or more)
 * 100 to 255 - Errors, barspatcher_getErrorString can be used to get a string from the error code
 * 
 */
unsigned char barspatcher_run(bool verbose, const char* og_stream_dirname, const char* mod_stream_dirname, const char* bars_input_filename, const char* bars_output_filename) {
    //Check if directory paths aren't too long
    if(strlen(og_stream_dirname) >= BARSPATCHER_PATHSTR_BYTES-300 || strlen(mod_stream_dirname) >= BARSPATCHER_PATHSTR_BYTES-300) {
        printf("%s.\n", barspatcher_getErrorString(101));
        return 101;
    }
    
    //Check if output file path can be opened for writing
    std::ofstream ofile;
    ofile.open(bars_output_filename, std::ios::out | std::ios::binary | std::ios::app);
    if(!ofile.is_open()) {
        perror(bars_output_filename);
        return 249;
    }
    ofile.close();
    
    //Open and read input BARS file
    unsigned char* bars_data;
    uint64_t bars_size;
    {
        std::ifstream ifile;
        ifile.open(bars_input_filename, std::ios::in | std::ios::binary | std::ios::ate);
        
        if(!ifile.is_open()) {
            perror(bars_input_filename);
            return 255;
        }
        
        bars_size = ifile.tellg();
        
        //64MB memory allocation limit for file data
        if(bars_size >= 64000000) {
            printf("BARS input files larger than 64MB are not currently supported. The input file is %.1fMB.\n", (float)bars_size/1000000);
            
            ifile.close();
            
            return 253;
        }
        
        bars_data = (unsigned char*)malloc(bars_size);
        if(bars_data == NULL) {
            printf("Could not allocate memory for BARS data.\n");
            
            ifile.close();
            
            return 100;
        }
        
        ifile.seekg(0);
        ifile.read((char*)bars_data, bars_size);
        
        if(!ifile.good()) {
            perror(bars_input_filename);
            free(bars_data);
            ifile.close();
            return 254;
        }
        
        ifile.close();
    }
    
    //Read mod stream directory listing
    char* mod_dir_list[BARSPATCHER_DIRLIST_LIMIT];
    mod_dir_list[0] = NULL;
    uint16_t mod_dir_list_count = 0;
    
    DIR* mod_dir;
    dirent* mod_dir_entry;
    mod_dir = opendir(mod_stream_dirname);
    if(mod_dir == NULL) {
        perror(mod_stream_dirname);
        
        //Free everything that was previously allocated in this function
        free(bars_data);
        
        return 229;
    }
    
    while((mod_dir_entry = readdir(mod_dir)) != NULL) {
        //Ignore entries that are not normal files
        if(mod_dir_entry->d_type != DT_REG) continue;
        
        mod_dir_list[mod_dir_list_count] = (char*)malloc(strlen(mod_dir_entry->d_name)+1);
        
        //Check for memory allocation errors
        if(mod_dir_list[mod_dir_list_count] == NULL) {
            printf("Could not allocate memory for the mod stream directory listing.\n");
            
            //Free everything that was previously allocated in this function
            free(bars_data);
            for(uint16_t i=0; mod_dir_list[i] != NULL; i++) free(mod_dir_list[i]);
            closedir(mod_dir);
            
            return 100;
        }
        
        strcpy(mod_dir_list[mod_dir_list_count], mod_dir_entry->d_name);
        
        mod_dir_list_count++;
        //Set next pointer to nullptr
        mod_dir_list[mod_dir_list_count] = NULL;
        
        //Check if we didn't run out of directory listing space
        if(mod_dir_list_count >= BARSPATCHER_DIRLIST_LIMIT-1) {
            printf("Directory listing is too big. This should not happen if you are correctly modding a game's audio tracks, please open a new issue in the repository of this program if the game you are modding has more than %d audio tracks.\n", BARSPATCHER_DIRLIST_LIMIT-1);
            
            //Free everything that was previously allocated in this function
            free(bars_data);
            for(uint16_t i=0; mod_dir_list[i] != NULL; i++) free(mod_dir_list[i]);
            closedir(mod_dir);
            
            return 100;
        }
    }
    
    closedir(mod_dir);
    
    if(mod_dir_list_count == 0) {
        printf("The mod directory has no files.\n");
        
        //Free everything that was previously allocated in this function
        free(bars_data);
        
        return 228;
    }
    
    //Read information from every original and modded BWAV file in the modded BWAV list, patch the BARS file
    //Success/skip counter
    uint16_t patched_files = 0, skipped_files = 0;
    
    //BWAV file handles
    std::ifstream og_bwav;
    std::ifstream mod_bwav;
    //Memory blocks for reading from BWAV files
    unsigned char og_bwav_data[BARSPATCHER_OGBWAV_MEMBLOCK_SIZE];
    unsigned char mod_bwav_data[BARSPATCHER_MODBWAV_MEMBLOCK_SIZE];
    
    //Memory block for slicing functions output
    unsigned char slice_output[0x100];
    
    //Full path strings
    char og_path[BARSPATCHER_PATHSTR_BYTES];
    char mod_path[BARSPATCHER_PATHSTR_BYTES];
    strcpy(og_path, og_stream_dirname);
    strcpy(mod_path, mod_stream_dirname);    
    strcat(og_path, "/");
    strcat(mod_path, "/");
    
    //Pointers to beginning of file name in previous full path strings.
    char* og_path_filename = og_path + strlen(og_path);
    char* mod_path_filename = mod_path + strlen(mod_path);
    
    for(uint16_t entry=0; mod_dir_list[entry] != NULL; entry++) {
        //Make full paths for both files
        strcpy(og_path_filename, mod_dir_list[entry]);
        strcpy(mod_path_filename, mod_dir_list[entry]);
        
        //Try opening original BWAV
        uint64_t og_bwav_size;
        
        og_bwav.open(og_path, std::ios::in | std::ios::binary | std::ios::ate);
        if(!og_bwav.is_open()) {
            //Skip if file doesn't exist
            if(errno == ENOENT) {
                printf("Warning: %s doesn't have a matching original file, skipping.\n", mod_dir_list[entry]);
                skipped_files++;
                continue;
            }
            
            //Error if the opening failed for any other reason
            perror(og_path);
            
            //Free everything that was previously allocated in this function
            free(bars_data);
            for(uint16_t i=0; mod_dir_list[i] != NULL; i++) free(mod_dir_list[i]);
            
            return 239;
        }
        
        //Try opening modded BWAV
        uint64_t mod_bwav_size;
        
        mod_bwav.open(mod_path, std::ios::in | std::ios::binary | std::ios::ate);
        if(!mod_bwav.is_open()) {
            perror(og_path);
            
            //Free everything that was previously allocated in this function
            og_bwav.close();
            free(bars_data);
            for(uint16_t i=0; mod_dir_list[i] != NULL; i++) free(mod_dir_list[i]);
            
            return 238;
        }
        
        //Read information from both BWAV files
        og_bwav_size = og_bwav.tellg();
        mod_bwav_size = mod_bwav.tellg();
        og_bwav.seekg(0);
        mod_bwav.seekg(0);
        og_bwav.read((char*)og_bwav_data, (BARSPATCHER_OGBWAV_MEMBLOCK_SIZE > og_bwav_size ? og_bwav_size : BARSPATCHER_OGBWAV_MEMBLOCK_SIZE));
        mod_bwav.read((char*)mod_bwav_data, (BARSPATCHER_MODBWAV_MEMBLOCK_SIZE > mod_bwav_size ? mod_bwav_size : BARSPATCHER_MODBWAV_MEMBLOCK_SIZE));
        
        //Check for read errors
        if(!og_bwav.good() || !mod_bwav.good()) {
            //Which file has the error
            bool which_error = !og_bwav.good();
            if(which_error) perror(og_path);
            else perror(mod_path);
            
            //Free everything that was previously allocated in this function
            og_bwav.close();
            mod_bwav.close();
            free(bars_data);
            for(uint16_t i=0; mod_dir_list[i] != NULL; i++) free(mod_dir_list[i]);
            
            if(which_error) return 237;
            else return 236;
        }
        
        og_bwav.close();
        mod_bwav.close();
        
        //Make sure that both files are BWAV files
        if(strcmp(barspatcher_getSliceAsString(slice_output, mod_bwav_data, 0, 4), "BWAV") != 0) {
            printf("Error in %s: Modded file is not a BWAV file. Skipping.\n", mod_dir_list[entry]);
            skipped_files++;
            continue;
        }
        if(strcmp(barspatcher_getSliceAsString(slice_output, og_bwav_data, 0, 4), "BWAV") != 0) {
            printf("Error in %s: Original file is not a BWAV file. Skipping.\n", mod_dir_list[entry]);
            skipped_files++;
            continue;
        }
        
        //Read byte order marks from both files
        //0 = little endian, 1 = big endian
        bool og_bwav_bom, mod_bwav_bom;
        
        if(barspatcher_getSliceAsInt16Sample(og_bwav_data, 0x04, 1) == -257) og_bwav_bom = 1;
        else og_bwav_bom = 0;
        if(barspatcher_getSliceAsInt16Sample(mod_bwav_data, 0x04, 1) == -257) mod_bwav_bom = 1;
        else mod_bwav_bom = 0;
        
        //Compare channel counts
        uint16_t og_bwav_chnum, mod_bwav_chnum;
        og_bwav_chnum = barspatcher_getSliceAsNumber(slice_output, og_bwav_data, 0x0E, 2, og_bwav_bom);
        mod_bwav_chnum = barspatcher_getSliceAsNumber(slice_output, mod_bwav_data, 0x0E, 2, mod_bwav_bom);
        
        if(og_bwav_chnum != mod_bwav_chnum) {
            printf("Error in %s: The modded BWAV file must have the same amount of channels as the original BWAV file. Skipping.\n", mod_dir_list[entry]);
            skipped_files++;
            continue;
        }
        
        //Read CRC32 hash from original file, used to find the location of the original file in the BARS file
        uint32_t og_bwav_crc32;
        uint8_t og_bwav_crc32_bytes[4];    
        
        barspatcher_getSlice(slice_output, og_bwav_data, 0x08, 4);
        memcpy(og_bwav_crc32_bytes, slice_output, 4);
        og_bwav_crc32 = barspatcher_getSliceAsNumber(slice_output, og_bwav_crc32_bytes, 0, 4, og_bwav_bom);
        
        //Size of BWAV file header to be written into BARS
        uint32_t patch_length = 0x10 + 0x4C*mod_bwav_chnum;
        if(patch_length > BARSPATCHER_MODBWAV_MEMBLOCK_SIZE) {
            printf("Error in %s: The patch is too big. Skipping.\nThis should never happen if you are correctly modding a game's audio tracks. Please make sure that all your files and paths are correct, and if the error repeats, please open a new issue in the repository of this program.\n", mod_dir_list[entry]);
            skipped_files++;
            continue;
        }
        
        
        //Search for the original BWAV file in BARS
        if(verbose) printf("%s: Original file hash: 0x%08X\n", mod_dir_list[entry], og_bwav_crc32);
        
        uint16_t patches_written = 0;
        uint16_t c;
        bool nomatch = 0;
        
        for(uint32_t bars_pos=0; bars_pos < bars_size; bars_pos++) {
            //Compare
            for(c = 0; c < 4; c++) {
                if(bars_data[bars_pos + c] != og_bwav_crc32_bytes[c]) {
                    nomatch = 1;
                    break;
                }
            }
            
            if(nomatch == 0) {
                //Found
                uint32_t bars_bwav_offset = bars_pos - 0x08;
                if(verbose) printf("Found at 0x%08X in BARS, ", bars_bwav_offset);
                
                if(bars_size - bars_bwav_offset < patch_length) {
                    if(!verbose) printf("Error in %s: ", mod_dir_list[entry]);
                    printf("not enough space for header in BARS file, is the BARS file valid?\n");
                    continue;
                }
                
                for(uint32_t i=0; i < patch_length; i++) {
                    bars_data[bars_bwav_offset + i] = mod_bwav_data[i];
                }
                
                if(verbose) printf("wrote patch.\n");
                patches_written++;
            }
            
            nomatch = 0;
        }
        
        
        if(patches_written > 0) patched_files++;
        else {
            skipped_files++;
            printf("%s: Not found in BARS file, skipped.\n", mod_dir_list[entry]);
        }
    }
    
    if(patched_files == 0) {
        printf("Error: All tracks were skipped, BARS file was not patched.\n");
        
        //Free everything that was previously allocated in this function
        free(bars_data);
        for(uint16_t i=0; mod_dir_list[i] != NULL; i++) free(mod_dir_list[i]);
        
        return 200;
    }
    
    
    //Write BARS output file
    ofile.open(bars_output_filename, std::ios::out | std::ios::binary | std::ios::trunc);
    if(!ofile.is_open()) {
        perror(bars_output_filename);
        
        //Free everything that was previously allocated in this function
        free(bars_data);
        for(uint16_t i=0; mod_dir_list[i] != NULL; i++) free(mod_dir_list[i]);
        
        return 249;
    }
    
    ofile.write((char*)bars_data, bars_size);
    
    //Check for write errors
    if(!ofile.good()) {
        perror(bars_output_filename);
        
        //Free everything that was previously allocated in this function
        free(bars_data);
        for(uint16_t i=0; mod_dir_list[i] != NULL; i++) free(mod_dir_list[i]);
        
        return 248;
    }
    
    ofile.close();
    
    
    printf("%d track%s patched, %d track%s skipped.\n", patched_files, (patched_files == 1 ? "" : "s"), skipped_files, (skipped_files == 1 ? "" : "s"));
    
    //Free everything
    free(bars_data);
    for(uint16_t i=0; mod_dir_list[i] != NULL; i++) free(mod_dir_list[i]);
    
    return (skipped_files > 99 ? 99 : skipped_files);
}
