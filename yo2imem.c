#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#define MAXSIZE 0x400

void processCmdLine(int argc, char **argv, char **filenameptr, char **basenameptr,
		    int * banksptr, int * lineptr)
{
  int opterr = 0;
  int c;

  while ((c=getopt(argc, argv, "f:b:c:l:")) != -1) {
    switch(c) {
    case 'f':
      *filenameptr = strdup(optarg);
      break;
    case 'b':
      *basenameptr = strdup(optarg);
      break;
    case 'c':
      *banksptr = atoi(optarg);
      break;
    case 'l':
      *lineptr = atoi(optarg);
      break;
    }
  }
  if (optind < argc && *filenameptr == NULL) {
    *filenameptr = strdup(argv[optind]);
  }
  if (*basenameptr == NULL) {
    *basenameptr = strdup("ibank");
  }
}

int main(int argc, char **argv) 
{
  unsigned char A[MAXSIZE];

  char * filename = NULL;
  char * basename = NULL;
  FILE * infile = stdin;
  int numbanks = 4;
  int bytesperline = 4;

  processCmdLine(argc, argv, &filename, &basename, &numbanks, &bytesperline);
  if (filename) {
    infile = fopen(filename, "r");
    if (infile == NULL) {
      fprintf(stderr, "Unable to open input file");
      exit(1);
    }
  }
  
  char * read;
  char line[120];
  char first[80];
  char bytes[80];
  size_t len = 0;

  int addr = 0;
  int faddr;
  int i;
  char * bytesptr;
  int byteslen;
  int abyte;

  while ((read = fgets(line, 120, infile)) != NULL) {
    faddr = 0;
    sscanf(line, "%s", first);
    if (strcmp(first, "|") == 0) {
      //printf("Non-address line\n");
      continue;
    }
    sscanf(line, "%x: %s", &faddr, bytes);
    byteslen = strlen(bytes);
    if (byteslen == 1) {
      byteslen = 0;
      bytes[0] = '\0';
    }
    assert(byteslen % 2 == 0);
    //printf("Address: 0x%x, bytes: '%s'\n", faddr, bytes);
    while (addr < faddr) {
      //printf("00 ");
      A[addr++] = 0;
    }
    bytesptr = bytes;
    for (i=0; i < byteslen/2; i++) {
      sscanf(bytesptr, "%2x", &abyte);
      //printf("%2.2x ", abyte);
      bytesptr += 2;
      A[addr++] = abyte & 0xFF;
    }
  }

  int Asize = addr;
  for (i=0; i<Asize; i++) {
    //printf("%2.2x ", A[i]);
  }
  A[Asize] = 0;

  FILE ** banks = (FILE **)malloc(numbanks * sizeof(FILE *));
  char bankname[32];
  
  for (i = 0; i < numbanks; i++) {
    sprintf(bankname, "%s%d.mem", basename ? basename : "bank", i);
    banks[i] = fopen(bankname, "w");
    if (banks[i] == NULL) {
      fprintf(stderr, "Unable to open bank file, exiting\n");
      exit(1);
    }
    fprintf(banks[i], "v2.0 raw\n");
  }

  addr = 0;
  int j;
  while (addr < Asize) {
    for (i = 0; i < numbanks; i++) {
      for (j = 0; j < bytesperline; j++) {
	fprintf(banks[i],"%2.2x",A[addr++]);
      }
      fprintf(banks[i], "\n");
      if (addr >= Asize) break;
    }
  }
}
