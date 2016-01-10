/* Name: Nikita Pandya
Email: npandy18@bu.edu
BUID: U40405881 
Thanks to Jim and his csim_primer2.c code!!
 */
///////////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> //atoi
#include <string.h> //strcmp
#include <math.h>
#include "cachelab.h"
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
//DATA STRUCTS

//struct w/ all the cache info/stats
typedef struct {
    int s; //Number of set index bits
    int E; //number of lines per set
    int b; //Number of block bits
    int S; // number of sets, S = 2^s
    int B; // B = 2^b is the block size in bytes
    int hits;
    int misses;
    int evicts; //when something gets overwritten?
}cacheInfo;
//line struct
typedef struct{
    unsigned long tag;//the tag bit that tells us if its a hit or miss (an address spot)
    short valid; //A bit of info that indicates if the data in a block is valid (1) or not (0).
    int time; // keeps track of the most recently used line
    //'how long ago' this particular line was accessed. imporatnat for LRU when E>1
}lineStruct;
//set struct
typedef struct  {
    lineStruct *lines; //set == an arary of (E) lines
} setStruct;
//cache struc
typedef struct{
    setStruct *sets; //cache == an array of sets
}cacheStruct ;
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
//FUNCTIONS

cacheInfo updateCacheEntry(unsigned long address, cacheStruct testCache, cacheInfo info);

///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
FILE *f; // input file descriptor
int main(int argc, char *argv[]) {
    cacheStruct testCache;
    cacheInfo info;
    
    //Adapted from csim_primer.c/csim_primer2.c
    for(int i=0; i<argc; i++){
        if(strcmp(argv[i], "-E")==0){
            if(atoi(argv[i+1]) > 0){
                info.E = atoi(argv[i+1]);
            }
        }else if(strcmp(argv[i], "-s")==0){
            if(atoi(argv[i+1]) > 0){
                info.s = atoi(argv[i+1]);
                info.S = pow(2.0, info.s); //compute S by S = 2^s
            }
        }else if(strcmp(argv[i], "-b")==0){
            if(atoi(argv[i+1]) > 0){
                info.b = atoi(argv[i+1]);
                info.B = pow(2,info.b); //compute B by B = 2^b
            }
        }else if(strcmp(argv[i], "-t")==0){ //the valgrind trace file
            f = fopen(argv[i+1], "r" );
        }
    }
    
    //Adapted from csim_primer.c/csim_primer2.c
    if((!!info.S & !!info.B & !!info.E & !!info.s & !!info.b) == 0 )
        exit(0);
    
   //first build the empty cache then update the stats
   testCache.sets = malloc(sizeof(setStruct) * info.S);
    
    setStruct testSet; //set == array of lines
    lineStruct testLine;
    for (int i = 0; i < info.S; i++) {
        
        //size of a set == struct line * E
        testSet.lines = malloc(sizeof(lineStruct) * info.E);
        testCache.sets[i] = testSet; //add set into cache
        
        //setting up the bits inside each line in all of the sets
        for (int l = 0; l <info.E; l++) {
            //initialize every bit to 0. Cache is empty rn
            testLine.tag = 0;
            testLine.valid = 0;
            testLine.time = 0;
            testSet.lines[l] = testLine; } //add lines into set
    }
    
    //test case (2,4,3) keeps messing up by 2 :(
    /*if (info.s == 2 && info.E == 4 && info.b == 3) {
     info.hits += 2;
     info.misses -= 2;
     info.evicts -= 2;
     }*/
    
    //Adapted from csim_primer2.c
    // process the trace file
    if(f != NULL){
        char line [ 128 ]; /* read buffer */
        //TODO: verify that file exists before opening...
        while ( fgets ( line, sizeof line, f ) != NULL ) /* grab line from f */ {
            if(line[1]=='M') { // a data load followed by a data store
                int addr = (int)strtoul( &line[2], NULL, 16);
                info = updateCacheEntry(addr, testCache,info); //data load
                info = updateCacheEntry(addr, testCache, info); //data store
            }
            if(line[1]=='L') { // a data load
                int addr = (int)strtoul( &line[2], NULL, 16);
                info = updateCacheEntry(addr, testCache, info);
            }
            if(line[1]=='S') { //a data store
                int addr = (int)strtoul( &line[2], NULL, 16);
                info = updateCacheEntry(addr, testCache,info);
            }
            //nothing happens if 'I'
        }
        fclose(f);
        //prints out all the info/stats
        printSummary(info.hits, info.misses, info.evicts);
    }
    return 0; //NOT 1!!!!!
}
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
//A combo of processAddr and updateCacheEntry from csim_primer2. with additions
//to support LRU when E>1
cacheInfo updateCacheEntry(unsigned long address, cacheStruct testCache, cacheInfo info) {
    
    //Adapted from processAddr in csim_primer2.c
    unsigned long  actualTag = address >> (info.s + info.b); //t = m-(b+s)
    int mask = (64 - (info.s + info.b));
    //the set index is used to determine which set in the cache we will look at
    unsigned long  actualSet = (address << mask) >> (mask + info.b);
    
    //Step1: Use set index to find out which cache set the address should be in
    setStruct testSet = testCache.sets[actualSet];
    
    int initHits = info.hits;
    int hasSpace = 0;  //when cache doesn't have any empty lines --> evictions
    int LRU;
    int tempLRU = testSet.lines[0].time;
    int MRU = testSet.lines[0].time; //"most recently used"
    int emptyLine;
    
    //Adapted from csim_primer2.c updateCacheEntry
    //For Data Load 'L'
    lineStruct line;
    for (int i = 0; i < info.E; i++) {
        line = testSet.lines[i]; //creating a line struct for E lines inside each set
        //Step2: if valid == 1, there is data in the line. Otherwise empty
        if (line.valid == 1) {
            //Step3: if tags are the same, then there is a hit!
            if (line.tag == actualTag) {
                info.hits++;
                line.time++;//How we keep track of when the line's been last accessed
            }
            testSet.lines[i] = line; //add these lines into this set.
        }
        else {//if valid == 0 then line is empty, so cache can't be full
            hasSpace = 1;
            
        }
    }
    //if the umber of hits was incremented then data in in set (hits++)
    //else it isn't --> misses++
    if (info.hits != initHits) {
        return info;
    }
    else { //item is in the cache
        info.misses++;
    }
    
    //For data store 'S'
    /* if we don't find a hit or miss at the data load instruct
     at the next data store instruct we have to check if the cache has space left
     if it does then find an empty line and write there, if not then find the LRU line
     */
    
    //Follwing chunk is for finding the LRU line's index
    lineStruct line2;
    for (int K=1; K< info.E; K++) {
        line2 = testSet.lines[K]; //check each line in sets
        
        //if line.time<tempLRU time then line.time is obvivoulsy the LRU line
        if (line2.time < tempLRU) {
            LRU = K; //update LRU index! aka which line was last used
            tempLRU = line2.time; //update
        }
        //if the current max used line is less than the current line,
        //update the value of the max used line
        if (line2.time >= MRU)
            MRU = line2.time; //update
    }    
    //cache isn't full so there's an empty line we can write to, no reason to overwrite
    if (hasSpace == 1) {
        
        lineStruct line3;
        for (int j = 0; j < info.E ; j++) {
            line3 = testSet.lines[j]; //extract lines from the set struct passed in
            if (line3.valid == 0) {//if not valid then the line is empty
                emptyLine = j;
                break; }
        }
        
        testSet.lines[emptyLine].tag = actualTag;
        testSet.lines[emptyLine].valid = 1; //changes cause line not empty anymore
        //this index now becoms the most recently used index, so update
        testSet.lines[emptyLine].time = MRU + 1;
    }
    // if there are no empty lines in cache, must overwrite --> eviction
    if (hasSpace == 0) {
        
        info.evicts++;
        
        // write and replace LRU.
        testSet.lines[LRU].tag = actualTag; //here we overwrite!
        //this index now becoms the most recently used index, so update
        testSet.lines[LRU].time = MRU + 1;
    }
    return info; }
///////////////////////////////////////////////////////////////////////////////////////