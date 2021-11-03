#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>

#define MAXSIZE 0x800
#define MAXLINE 120
#define FILE_SUFFIX ".mem"

int verbose = 0;
/*
  Usage: yo2dmem [-p outprefix -b banks -l linebytes -v -h] inputfile.yo
 */
void processCmdLine(int argc, char **argv, char **filenameptr, char **basenameptr,
		    int * banksptr, int * lineptr, int * verbose)
{
  int opterr = 0;
  int c;

  if (*verbose) {
    printf("verbose on entry\n");
  }
  while ((c=getopt(argc, argv, "p:b:l:hv")) != -1) {
    switch(c) {
    case 'p':
      *basenameptr = strdup(optarg);
      break;
    case 'b':
      *banksptr = atoi(optarg);
      break;
    case 'l':
      *lineptr = atoi(optarg);
      break;
    case 'v':
      printf("verbose option");
      *verbose = 1;
      break;
    case 'h':
      printf("Usage: %s  [-p outprefix -b banks -l linebytes -v -h] inputfile.yo\n", argv[0]);
      exit(0);
    default:
      abort();
    }
  }
  // After the loop, the optind should be at the index beyond the option arguments
  // If the user has properly specified an input filename, optind should still be < argc
  // If it is not, then print a usage message and get out, just like with a help option.
  if (optind >= argc) {
      printf("Usage: %s  [-p outprefix -b banks -l linebytes -v -h] inputfile.yo\n", argv[0]);
      exit(1);
  }
  *filenameptr = strdup(argv[optind]);
  
  if (*verbose) {
    printf("infile: %s, prefix: %s, banks: %d, line: %d, optind: %d\n", *filenameptr, *basenameptr, *banksptr, *lineptr, optind);
  }
}

/* 
 * Process the given .yo file, converting the object format into an array of unsigned
 * character bytes in mem, up to the maximum size of the memory, given by memsize.
 * Returns the byte offset at the greatest extent in the memory.
 */
int yo2bytes(FILE * infile, unsigned char mem[], int memsize)
{
  char * retval;        // return value for file fgets
  int faddr;            // for any given line in the file, the Y86 address label
  int byteslen;         // length of string with hex assembled bytes
  char line[MAXLINE];   // string for full line for the file
  char first[MAXLINE];  // string for file line starting with first non-shitespace character
  char bytes[MAXLINE];  // string for the remainder of the line after the Y86 address

  int addr = 0;
  char * bytesptr;
  int abyte;

  int i;                // loop index

  // read from the .yo file a line at a time
  
  while ((retval = fgets(line, MAXLINE, infile)) != NULL) {
    
    // Determine if the line read fails to have an address, and so has no bytes to
    // assemble into the memory image.  This is true if the first non-whitespace
    // character is '|'.
    
    sscanf(line, "%s", first);
    if (strcmp(first, "|") == 0) {
      continue;
    }

    // Now read address-containing line to parse out the address from the rest
    // where bytes contains the remainder starting with the first non-whitespace
    // and ending at the trailing whitespace.
    
    faddr = 0;
    sscanf(line, "%x: %s", &faddr, bytes);

    // Assembler directives are on a line with no assembled bytes, then a '|',
    // then whitespace.  So the above sscanf would end with just the '|'
    // so make bytes an empty string before continuing.
    
    byteslen = strlen(bytes);
    if (byteslen == 1) {
      byteslen = 0;
      bytes[0] = '\0';
    }
    assert(byteslen % 2 == 0);
    //printf("Address: 0x%x, bytes: '%s'\n", faddr, bytes);
    while (addr < faddr) {
      //printf("00 ");
      mem[addr++] = 0;
    }
    bytesptr = bytes;
    for (i=0; i < byteslen/2; i++) {
      sscanf(bytesptr, "%2x", &abyte);
      //printf("%2.2x ", abyte);
      bytesptr += 2;
      mem[addr++] = abyte & 0xFF;
    }
  }
  int Asize = addr;
  for (i=0; i<Asize; i++) {
    //printf("%2.2x ", A[i]);
  }
  mem[Asize] = 0;
  return Asize;
}

void mem2files(unsigned char mem[], int memsize, char * prefix, int numbanks, int bytesperline, int endian)
{
  char suffix[] = FILE_SUFFIX;
  FILE ** banks = (FILE **)malloc(numbanks * sizeof(FILE *));
  char bankname[MAXLINE];
  int i, j;
  unsigned int value[2];

  for (i = 0; i < numbanks; i++) {
    sprintf(bankname, "%s%d%s", prefix, i, suffix);
    banks[i] = fopen(bankname, "w");
    if (banks[i] == NULL) {
      fprintf(stderr, "Unable to open bank file, exiting\n");
      exit(1);
    }
    fprintf(banks[i], "v2.0 raw\n");
  }

  assert(numbanks == 2 && bytesperline == 4 && endian);

  int addr = 0;
  while (addr < memsize) {

    for (i = 0; i < numbanks; i++) {
      if (endian) {
        assert(bytesperline == 4);

        value[i] = (mem[addr+3] & 0xff) << 24;
        value[i] |= (mem[addr+2] & 0xff) << 16;
        value[i] |= (mem[addr+1] & 0xff) << 8;
        value[i] |= (mem[addr] & 0xff);

        fprintf(banks[i], "%8.8X\n", value[i]);
        addr += 4;

      } else {
        for (j = 0; j < bytesperline; j++) {
          fprintf(banks[i],"%2.2X",mem[addr++]);
        }
        fprintf(banks[i], "\n");
      }
      if (addr >= memsize) break;
    }
  }
  for (i=0; i < numbanks; i++) {
    fclose(banks[i]);
  }
}

int main(int argc, char **argv) 
{
  unsigned char A[MAXSIZE];

  char * filename = NULL;
  char * basename = NULL;
  FILE * infile = NULL;
  int numbanks = 0;
  int bytesperline = 0;

  processCmdLine(argc, argv, &filename, &basename, &numbanks, &bytesperline, &verbose);
  assert(filename != NULL);
  int filenamelen = strlen(filename);
  if ((filename[filenamelen-3] != '.') || (filename[filenamelen-2] != 'y') 
      || (filename[filenamelen-1] != 'o') || ((infile = fopen(filename, "r")) == NULL)) {
    fprintf(stderr, "Bad input file: %s\n", filename);
    printf("infile: %s, prefix: %s, banks: %d, line: %d\n", filename, basename, numbanks, bytesperline);
    printf("Usage: %s  [-p outprefix -b banks -l linebytes -v -h] inputfile.yo\n", argv[0]);
    exit(1);
  }
  
  int Asize;
  int i;
  for (i = 0; i < MAXSIZE; i++)
    A[i] = '\0';
  Asize = yo2bytes(infile, A, MAXSIZE);
  Asize = (Asize + 7) & ~0x7;

  if (basename == NULL || strlen(basename) == 0) {
    basename = "bank";
  }
  if (bytesperline == 0 || numbanks == 0) {
    char dmem_prefix[MAXLINE];

    strncpy(dmem_prefix, "d", MAXLINE);
    strncat(dmem_prefix, basename, MAXLINE);
    
    mem2files(A, Asize, dmem_prefix, 2, 4, 1);

  } else {
    //mem2files(A, Asize, basename, numbanks, bytesperline, 0);
  }
}
