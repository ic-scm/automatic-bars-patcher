//Standard PC frontend for automatic BARS patcher
//Copyright (C) 2020 I.C.

#include <iostream>
#include <fstream>
#include <cstring>

#define BARSPATCHER_VERSION_PC
#include "../bars-patcher-core/bars-patcher.h"

int main(int argc, char** args) {
    if(argc < 2 || strcmp(args[1], "--help") == 0 || strcmp(args[1], "-h") == 0) {
        printf("Automatic BARS Patcher %s\nCopyright (C) 2020 I.C.\nThis program is free software, see the license file for more information.\n\nUsage: auto_bars_patcher [options...]\n\n", barspatcher_getVersionString());
        printf("Options:\n--og-stream-dir [directory path] - Directory with original unmodified BWAV files\n--mod-stream-dir [directory path] - Directory with modified BWAV files\n--og-bars-file [file path] - Original unmodified BARS file\n--bars-output-file [file path] - Location for the patched BARS file\n");
        
        return 0;
    }
    
    //Command line options
    const char* opts[] = {"-og-stream-dir","-mod-stream-dir","-og-bars-file","-bars-output-file"};
    const char* opts_alt[] = {"--og-stream-dir","--mod-stream-dir","--og-bars-file","--bars-output-file"};
    const unsigned int optcount = 4;
    const bool optrequiredarg[optcount] = {1,1,1,1};
    bool  optused  [optcount];
    char* optargstr[optcount];
    
    //Parse command line options
    for(int a=1;a<argc;a++) {
        int vOpt = -1;
        //Compare cmd arg against each known option
        for(unsigned int o=0;o<optcount;o++) {
            if( strcmp(args[a], opts[o]) == 0 || strcmp(args[a], opts_alt[o]) == 0 ) {
                //Matched
                vOpt = o;
                break;
            }
        }
        //No match
        if(vOpt < 0) {std::cerr << "Unknown option '" << args[a] << "'.\n"; return 1;}
        //Mark the options as used
        optused[vOpt] = 1;
        //Read the argument for the option if it requires it
        if(optrequiredarg[vOpt]) {
            if(a+1 < argc) {
                optargstr[vOpt] = args[++a];
            } else {
                std::cerr << "Option " << opts[vOpt] << " requires an argument.\n";
                return 1;
            }
        }
    }
    
    //Check options
    if(!(optused[0] && optused[1] && optused[2] && optused[3])) {
        std::cout << "All directory and file path options must be used.\n";
        return 1;
    }
    
    unsigned char bars_res;
    bars_res = barspatcher_run(optargstr[0], optargstr[1], optargstr[2], optargstr[3]);
    
    if(bars_res != 0) {
        printf("BARS patch error. (%d, %s)\n", bars_res, barspatcher_getErrorString(bars_res));
        return 2;
    }
    
    return 0;
}
