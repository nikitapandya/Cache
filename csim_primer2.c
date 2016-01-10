#include <stdint.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <math.h> 
#include "cachelab.h"

// data structures
typedef struct{
  uint8_t valid;
  uint64_t tag;
} CacheEntry;

typedef struct{
  uint32_t S;
  uint32_t B;
  uint32_t E;
  uint32_t ls;
  uint32_t lb;
} CacheInfo;

typedef struct{
  uint32_t hits;
  uint32_t misses;
  uint32_t evictions;
} CacheStats;

typedef struct{
  CacheEntry* entries;
  CacheInfo info;
  CacheStats stats;
} CacheObject;
typedef CacheObject* CacheRef;

// functions 
uint8_t getAddrFromLine(uint32_t* addr, char* line);
void processAddr( uint32_t addr, CacheRef c);
void updateCacheEntry(uint64_t tag, uint64_t set, CacheRef c);

int main(int argc, char *argv[])
{
  FILE *f; 
  CacheRef theCache = calloc(1, sizeof(CacheObject));
  // process command line input
  for(int i=0; i<argc; i++){
    if(strcmp(argv[i], "-E")==0){
      if(atoi(argv[i+1]) > 0){
        theCache->info.E = atoi(argv[i+1]);
        printf("Cache associativity: %d\n", theCache->info.E);
      }
    }else if(strcmp(argv[i], "-s")==0){
      if(atoi(argv[i+1]) > 0){
        theCache->info.ls = atoi(argv[i+1]);
        theCache->info.S = pow(2,theCache->info.ls);
        printf("Cache set count: %d\n", theCache->info.S);
      }
    }else if(strcmp(argv[i], "-b")==0){
      if(atoi(argv[i+1]) > 0){
        theCache->info.lb = atoi(argv[i+1]);
        theCache->info.B = pow(2,theCache->info.lb);
        printf("Cache block size: %d\n", theCache->info.B);
      }
    }else if(strcmp(argv[i], "-t")==0){
      printf("Tracefile: %s\n", argv[i+1]);
      f = fopen ( argv[i+1], "r" );
    }
  }
  // check that we got necessary input
  if( (!!theCache->info.S & 
       !!theCache->info.B & 
       !!theCache->info.E & 
       !!theCache->info.ls & 
       !!theCache->info.lb) == 0 ) { 
    printf("Error: input data is missing or invalid\n");
    exit(0); 
  }

  // allocate space for cache entries
  theCache->entries = calloc( theCache->info.S * theCache->info.E, 
                              sizeof(CacheEntry));

  // process the trace file
  if(f != NULL){
    char line [ 128 ]; /* read buffer */
    //TODO: verify that file exists before opening...
    while ( fgets ( line, sizeof line, f ) != NULL ) /* grab line from f */
      { 
         uint32_t addr;
         if( getAddrFromLine(&addr,line) ){
           printf("Processing line: %s", line);
           processAddr(addr, theCache);
           if(line[1]=='M') processAddr(addr, theCache); 
         } 
         else printf("Skipping line: %s\n", line);
      }
    fclose( f );
    printSummary(theCache->stats.hits, 
                 theCache->stats.misses, 
                 theCache->stats.evictions);
    printf("Warning: these result are only valid when E = 1\n");
  }
  // clean up
  free(theCache->entries);
  free(theCache);
  return 1;
}

uint8_t 
getAddrFromLine( uint32_t* addr, char* line) {
  if( line[0] == 'I' ) return 0;
  *addr = (uint32_t)strtoul( &line[2], NULL, 16);
  return 1;
}

void
processAddr(uint32_t addr, CacheRef c){
  uint64_t setmask = (1<<c->info.ls)-1;  
  uint64_t set = (addr >> c->info.lb) & setmask;
  uint64_t tag = addr >> (c->info.lb+c->info.ls);
  updateCacheEntry(tag, set, c);
  return;
}

void 
updateCacheEntry(uint64_t tag, uint64_t set, CacheRef c){
  //printf("t:%llx s:%llx h:%d m:%d\n", tag,set,*h,*m);
  if( c->entries[set].valid == 1 && c->entries[set].tag == tag ){
    printf("hit!\n");
    c->stats.hits++;
  }else{
    printf("miss!\n");
    c->entries[set].valid = 1;
    c->entries[set].tag = tag;
    c->stats.misses++;
  }
  return;
}
