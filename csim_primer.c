//
// CS210 Fall 2015
// Cache Lab Primer
//
// The intention of this primer is to bootstrap the cachelab by
// providing the initial ability to 1) parse the command line arguments, and
// 2) read in the tracefile line-by-line. 

#include <stdio.h> // printf, fgets, gputs
#include <stdlib.h> //  atoi 
#include <string.h> // strcmp
#include <math.h> // pow
#include "cachelab.h"

FILE *f; // input file descriptor  
 
int main(int argc, char *argv[])
{
  // Command line arguments are retrieved through the argv[] array. Each
  // entry contains a pointer to a string that contains an argument.
  //
  // The first entry in argv points to the program name:
  //    *argv[0] = program name
  //    *argv[1] = first argument 
  //    *argv[2] = second argument 
  //    *argv[n] = nth argument 
  //
  printf( "executable: %s \n", argv[0] );
  //
  // Example for the following line:
  // ./csim-ref -s 4 -E 1 -b 4 -t mufile.log
  //    *argv[0] = "csim-ref" 
  //    *argv[1] = "-s" 
  //    *argv[2] = "4" 
  //    *argv[3] = "-E" 
  //    *argv[4] = "1" 
  //    *argv[5] = "-b" 
  //    *argv[6] = "4" 
  //    *argv[7] = "-t" 
  //    *argv[8] = "mufile.log" 
  //
  // The integer 'argc' contains a count of the entries of argv[]
  printf("There are %d arguments\n", argc-1);

  // The following code extracts the '-E', '-s', '-b' and '-t' values from the
  // input arguments, and prints the corresponding cache properties  to the
  // screen.
  // 
  // c-standard library helper functions:
  // int strcmp(string1, string2); //compare two string
  // int atoi(string);  //convert string to integer

  for(int i=0; i<argc; i++){
    printf( ">%s\n", argv[i] );
    if(strcmp(argv[i], "-E")==0){
      if(atoi(argv[i+1]) > 0){
        printf("Cache associativity: %d\n", atoi(argv[i+1]));
      }
    }else if(strcmp(argv[i], "-s")==0){
      if(atoi(argv[i+1]) > 0){
        printf("Cache set count: %d\n", (int)(pow(2.0,atoi(argv[i+1]))));
      }
    }else if(strcmp(argv[i], "-b")==0){
      if(atoi(argv[i+1]) > 0){
        printf("Cache block size: %d\n", (int)(pow(2.0,atoi(argv[i+1]))));
      }
    }else if(strcmp(argv[i], "-t")==0){
      printf("Tracefile: %s\n", argv[i+1]);
      // Here we open the trace file 
      f = fopen ( argv[i+1], "r" );
    }
  }
  // Next I read through the trace file line-by-line, and print out each line
  if(f != NULL){
    char line [ 128 ]; /* read buffer */
    while ( fgets ( line, sizeof line, f ) != NULL ) /* read a line from f */
      { fputs ( line, stdout ); /* print the line */ }
    fclose ( f );
  }
  return 0;
}
