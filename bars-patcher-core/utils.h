//Generic slicing functions for BARS patcher and BWAV readers
//Copyright (C) 2020 I.C.

//Bool endian: 0 = little endian, 1 = big endian

#pragma once

//Get slice of data
//output is expected to have at least [length] bytes of memory allocated
unsigned char* barspatcher_getSlice(unsigned char* output, const unsigned char* data, unsigned long start, unsigned long length) {
    for(unsigned int i=0; i < length; i++) {
        output[i] = data[i + start];
    }
    return output;
}

//Get slice and convert it to a number
//output is expected to have at least [length] bytes of memory allocated
unsigned long barspatcher_getSliceAsNumber(unsigned char* output, const unsigned char* data, unsigned long start, unsigned long length, bool endian) {
    //Limit to 32-bit number
    if(length>4) length = 4;
    
    unsigned long number = 0;
    
    unsigned char* bytes = barspatcher_getSlice(output, data, start, length);
    
    unsigned char pos;
    
    if(endian) {
        pos = length - 1; //Read as big endian
    } else {
        pos = 0; //Read as little endian
    }
    
    unsigned long pw = 1; //Multiply by 1,256,65536...
    for(unsigned int i=0; i < length; i++) {
        if(i > 0) pw *= 256;
        number += bytes[pos] * pw;
        
        if(endian) pos--;
        else pos++;
    }
    return number;
}

//Get slice as signed 16 bit number
signed int barspatcher_getSliceAsInt16Sample(const unsigned char* data, unsigned long start, bool endian) {
    unsigned long number = 0;
    unsigned char bytes[2] = {data[start], data[start+1]};
    unsigned char little = bytes[endian];
    signed   char big = bytes[!endian];
    number = little + big*256;
    return number;
}

//Get slice as a null terminated string
//output is expected to have at least [length+1] bytes of memory allocated
char* barspatcher_getSliceAsString(unsigned char* output, const unsigned char* data, unsigned long start, unsigned long length) {
    unsigned char slicestr[length+1];
    unsigned char* bytes = barspatcher_getSlice(output, data, start, length);
    
    for(unsigned int i=0; i < length; i++) {
        slicestr[i] = bytes[i];
        if(slicestr[i] == '\0') slicestr[i]=' ';
    }
    
    slicestr[length]='\0';
    
    for(unsigned int i=0; i < length+1; i++) {
        output[i] = slicestr[i];
    }
    
    return (char*)output;
}
