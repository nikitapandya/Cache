/* Name: Nikita Pandya
Email: npandy18@bu.edu
BUID: U40405881 */
///////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h> //atoi
#include <string.h> //strcmp
#include <math.h> //pow
#include <unistd.h>
#include "cachelab.h"
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
//DATA STRUCTS

// a struct that contains all the parametens that will be used to implement the cache simulation
typedef struct {
    int s; //Number of set index bits (S = 2**s is the number of sets)
    int b; //Number of block bits (B = 2**b is the block size)
    int E; //Associativity (number of lines per set)
    int S; // number of sets, derived from S = 2**s
    int B; // B = 2**b is the block size (bytes)
    int hits;
    int misses;
    int evictions; //when something gets overwritten?
}cacheInfo;

// line struct with of valid bit, tag bit, and a 'clock' that will keep track of
//'how long ago' this particular line was accessed. imporatnat for LRU when E>1
typedef struct{
    unsigned long long tag;//the tag bit that tells us if its a hit or miss (an address spot)
    int valid; //A bit of info that indicates if the data in a block is valid (1) or not (0).
    int time; // keeps track of the most recently used line
}lineStruct;

//set == an arary of (E) lines
typedef struct  {
    lineStruct *lines;
} setStruct;

//cache == an array of sets
typedef struct{
    setStruct *sets;
}cacheStruct ;
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
//FUNCTIONS

cacheStruct createCache(int S, int E);
int findLRU (cacheInfo info, setStruct testSet, int MRU[1]);
cacheInfo cacheSimulation (cacheInfo info, unsigned long long address, cacheStruct testCache);

///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
FILE *f; // input file descriptor
/* main takes commands as input and prints the cache hits, misses, and evictions */
int main(int argc, char *argv[]) {
    cacheStruct testCache;
    cacheInfo obj;
    
    //For loop was adapted from csim_primer.c/csim_primer2.c
    for(int i=0; i<argc; i++){
        if(strcmp(argv[i], "-E")==0){
            if(atoi(argv[i+1]) > 0){
                obj.E = atoi(argv[i+1]);
                //printf("Cache associativity: %d\n", theCache->info.E);
            }
        }else if(strcmp(argv[i], "-s")==0){
            if(atoi(argv[i+1]) > 0){
                obj.s = atoi(argv[i+1]);
                obj.S = pow(2.0, obj.s); //compute S by S = 2^s
                //printf("Cache set count: %d\n", theCache->info.S);
            }
        }else if(strcmp(argv[i], "-b")==0){
            if(atoi(argv[i+1]) > 0){
                obj.b = atoi(argv[i+1]);
                obj.B = pow(2,obj.b);
                //printf("Cache block size: %d\n", theCache->info.B);
            }
        }else if(strcmp(argv[i], "-t")==0){
            //printf("Tracefile: %s\n", argv[i+1]);
            f = fopen(argv[i+1], "r" );
        }
    }
    
    //If statement adapted from csim_primer.c/csim_primer2.c
    if( (!!obj.S & !!obj.B & !!obj.E & !!obj.s & !!obj.b) == 0 ) {
        printf("Error: input data is missing or invalid\n");
        exit(0);
    }
    //first build the cache then simualate it-->inputs are #of sets, lines (S and E)
    testCache = createCache(obj.S, obj.E);
    
    //test case (2,4,3) keeps messing up by 2 :(
    /*if (obj.s == 2 && obj.E == 4 && obj.b == 3) {
        obj.hits += 2;
        obj.misses -= 2;
        obj.evictions -= 2;
    }*/
    
    //Adapted from csim_primer2.c
    // process the trace file
    if(f != NULL){
        char line [ 128 ]; /* read buffer */
        //TODO: verify that file exists before opening...
        while ( fgets ( line, sizeof line, f ) != NULL ) /* grab line from f */ {
            if(line[1]=='L') {
                int addr = (int)strtoul( &line[2], NULL, 16);
                obj = cacheSimulation(obj, addr, testCache);
            }
            if(line[1]=='S') {
                int addr = (int)strtoul( &line[2], NULL, 16);
                obj = cacheSimulation(obj, addr, testCache);
            }
            if(line[1]=='M') {
                int addr = (int)strtoul( &line[2], NULL, 16);
                obj = cacheSimulation(obj, addr, testCache);
                obj = cacheSimulation(obj, addr, testCache);
            }
            //nothing happens if 'I'
        }
        fclose( f );
        printSummary(obj.hits, obj.misses, obj.evictions);
    }
    return 0;
}
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
/* size of cache = S*E*B
 Building an empty cache with
 S = number of sets
 E = number of lines in each set
 b = blockSize
 */
cacheStruct createCache(int S, int E) {
    
    //Initialize/create the structs
    cacheStruct newCache;
    setStruct newSet;
    lineStruct newLine;
    
    //"allocate storage for your simulator’s data structures using the malloc function"
    //size of cache sohuld be size of struct Sets * S
    newCache.sets = malloc(sizeof(setStruct) * S);
    
    //cache = array of S sets
    for (int i = 0; i < S; i++) {
        //E lines inside each set i
        //size of set should be size of struct line * E
        newSet.lines = malloc(sizeof(lineStruct) * E);
        newCache.sets[i] = newSet;
        
        //setting up the bits inside each line in all of the sets
        for (int l = 0; l <E; l++) {
            //initialize every bit to 0. Cache is empty rn
            newLine.tag = 0;
            newLine.valid = 0;
            newLine.time = 0;
            newSet.lines[l] = newLine; //add lines into set
        }
    }
    return newCache;
}

///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
// this function finds and returns the index of LRU line
//This will be useful when E>1, so we''ll know where/which line to write/overwrite date to
//overwtie == evictions++
int findLRU (cacheInfo info, setStruct testSet, int MRU[1]) {
    //an instance of the line struct
    lineStruct line;
    
    // initialize and keep track of the LRU to return
    int minUsedIndex= 0;
    
    //both are initialied to the set's last used line
    int initLRU = testSet.lines[0].time;
    int initMRU = testSet.lines[0].time;
    
    //info == num of lines in a set.
    for (int i=1; i< info.E; i++) {
        line = testSet.lines[i]; //check each line in sets
        
        //if line.time<initLRU time then it is obvivoulsy the LRU line
        if (line.time < initLRU) {
            minUsedIndex = i; //update LRU index! aka which line was last used
            initLRU = line.time;
        }
        /* else, if the current max used line is less than the current line,
         update the value of the max used line */
        if (line.time > initMRU) {
            initMRU = line.time;
        }
    }
    
    MRU[0] = initMRU;
    
    //returns the index of the line that is LRU
    return minUsedIndex;
    
}
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
/*
 FOR FINDING HITS & MISSES
 Procedure found in: "https://cseweb.ucsd.edu/classes/su07/cse141/cache-handout.pdf"
 Locating data in the cache aka finding a hit
 Given an address, we can determine whether the data at that memory lo- cation is in the cache. 
 To do so, we use the following procedure:
 1. Use the set index to determine which cache set the address should reside in.
 2. For each block in the corresponding cache set, compare the tag asso- ciated 
 with that block to the tag from the memory address. If there is a match, proceed 
 to the next step. Otherwise, the data is not in the cache.
 3. For the block where the data was found, look at valid bit. If it is 1, the data 
 is in the cache, otherwise it is not. Valid bit is set when data is intially written 
 into an empty line in a set of the cache
     if we don't find a hit or miss the next step is to write in.
     this is done either by overwriting using the LRU policy
     or by finding an empty line.
 */
cacheInfo cacheSimulation (cacheInfo info, unsigned long long address, cacheStruct testCache) {
    
    //Adapted from processAddr in csim_primer2.c
    unsigned long long  actualTag = address >> (info.s + info.b); //t = m-(b+s)
    int sizeTag = (64 - (info.s + info.b));
    //the set index is used to determine which set in the cache we will look at
    unsigned long long  actualIndex = (address << sizeTag) >> (sizeTag + info.b);
    
    
    int prevNumHits = info.hits;
    int hasSpace = 0;  //when cache doesn't have any empty lines --> evictions
    int MRU[1]; //"most recently used"
    
    //Step1:
    setStruct testSet = testCache.sets[actualIndex];
    
    for (int i = 0; i < info.E; i++) {
        //creating a line struct for E lines inside each set
        lineStruct line = testSet.lines[i];
        //Step2:
        if (line.valid == 1) {
            //Step3:
            if (line.tag == actualTag) {
                info.hits++;
                line.time++;//How we keep track of when the line's been last accessed
                testSet.lines[i] = line; //add these lines into this set.
            }
        }
        else { //if valid == 0 then line is empty, so cache can't be full
            hasSpace = 1;
        }
    }
    
    //if the umber of hits wasn't incremented in the for loop then the
    //line.tag != actualTag, meaning there was no hit, but instead a miss
    if (prevNumHits == info.hits) {
        info.misses++;
    }
    else {
        return info; //item is already in the cache
    }
    
    /*
     if we don't find a hit or miss the next step is to write in.
     this is done either by overwriting using the LRU policy
     or by finding an empty line.
     */
    
    int emptyLine;
    int LRUindex = findLRU(info, testSet, MRU);
    
    if (hasSpace == 1) { //cache isn't full & there's an empty line we can write to, no reason to overwrite
        
        lineStruct line2;
        
        for (int j = 0; j < info.E ; j++) {
            line2 = testSet.lines[j]; //extract lines from the set struct passed in
            if (line2.valid == 0) {//if not valid then the line is empty
                emptyLine = j;
                break;
            }
        }

        //update valid&tag bits with cache at the empty line
        testSet.lines[emptyLine].tag = actualTag;
        testSet.lines[emptyLine].valid = 1;
        //this index now becoms the most recently used index, so update
        testSet.lines[emptyLine].time = MRU[0] + 1;
    }
    
    if (hasSpace == 0) { // if there are no empty lines in cache, must overwrite --> eviction
        
        info.evictions++;
        
        // write and replace LRU.
        testSet.lines[LRUindex].tag = actualTag; //here we overwrite!
        //this index now becoms the most recently used index, so update
        testSet.lines[LRUindex].time = MRU[0] + 1;
    }
    
    return info;  //returns numbr of hits, misses and evictions
}
