#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "tables.h"

/**
 * main function, expects inFile outFile or inFile outFile -symbols
 */
int main(int argc, char *argv[]) {

	// incorrect num args
	if(argc != 3 && argc != 4) {
		printf("Incorrect number of arguments!");
	}
	else {
		// Open and check files
		FILE *In;
		In = fopen(argv[1], "r");
		if(In == NULL) {
			printf("Input file could not be opened.");
			exit(1);
		}

		FILE *Out;
		Out = fopen(argv[2], "w");
		if(Out == NULL) {
			printf("Output file could not opened.");
			exit(1);
		}

		int passNumber = 1; // number of passes
		
		// pass 1
		parseFile(In, passNumber, Out);
		
		// no symbols
		if(argc == 3) {
			// pass 2
			rewind(In);
			passNumber = 2;
			parseFile(In, passNumber, Out);
			
			// pass 3
			rewind(In);
			passNumber = 3;
			parseFile(In, passNumber, Out);
		}
		// print symbols
		else {
			printLTable(Out);
		}

		// Close files
		fclose(In);
		fclose(Out);
		
		// free label table
		freeLTable();

		return 0;
	}
}
