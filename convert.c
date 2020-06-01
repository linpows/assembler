#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <unistd.h>
#include "convert.h"

// Write out the R-Type instruction
void rInstruction(char *opBits, char *func, char *rs, char *rt, char *rd, int shamt, FILE *Out) {
	Register rdReg = findReg(rd);
	Register rsReg = findReg(rs);
	Register rtReg = findReg(rt);
		
	char shamtBin[6];

	// Convert shamt to binary and put in shamtBin as a char*
	getBin(shamt, shamtBin, 5);

	// Print out the instruction to the file
	fprintf(Out, "%s%s%s%s%s%s\n", opBits, rsReg.bits, rtReg.bits, rdReg.bits, shamtBin, func);
}

// Write out the I-Type instruction
void iInstruction(char *opcode, char *rs, char *rt, int immediateNum, FILE *Out) {
	Register rsReg = findReg(rs);
	Register rtReg = findReg(rt);
	
	char immediate[17];

	// Convert immediate to binary
	getBin(immediateNum, immediate, 16);

	// Print out the instruction to the file
	fprintf(Out, "%s%s%s%s\n", opcode, rsReg.bits, rtReg.bits, immediate);
}

// Write out the J-Type instruction
void jInstruction(char *opcode, int immediate, FILE *Out) {
	// Convert immediate to binary
	char immediateStr[27];
	getBin(immediate, immediateStr, 26);

	// Print out instruction to file
	fprintf(Out, "%s%s\n", opcode, immediateStr);
}

// Write out the variable in binary
void printWord(int bin, FILE *Out) {

	for (int k = 31; k >= 0; k--) {
		fprintf(Out, "%c", (bin & (1 << k)) ? '1' : '0');
	}

	fprintf(Out, "\n");
}

// Write out the ascii string
void printString(char string[], size_t strLen, FILE *Out) {

	// Separate the string, and put each four characters in an element of an array of strings
	int num = strLen / 4;
	if ((strLen % 4) > 0)
		num++;

	char *ptr;
	ptr = &string[0];

	// Create an array of strings which separates each 4-char string
	char **seperate;
	seperate = malloc(num * sizeof(char*));
	if (seperate == NULL) {
		fprintf(Out, "Out of memory\n");
		exit(1);
	}
	
	
	char *clear = "0000"; // initializer for partition

	//allocate partition
	for (int i = 0; i < num; i++) {
		seperate[i] = malloc(4 * sizeof(char));
		if (seperate[i] == NULL) {
			fprintf(Out, "Out of memory\n");
			exit(1);
		}
		memcpy(seperate[i], clear, 4);
	}

	//copy string
	int count = 0;
	for (size_t i = 0; i < strLen; i++) {
		char c = ptr[0];
		seperate[i / 4][i % 4] = c;
		ptr++;
		count++;
	}

	// print binary
	size_t index = 0;
	for (int i = 0; i < num; i++) {
		for (int j = 0; j < 4; j++) {
			index++;
			char c = seperate[i][j];
			for (int k = 7; k >= 0; k--) {
				if(index < strLen){
					fprintf(Out, "%c", (c & (1 << k)) ? '1' : '0');
				}
				else {
					fprintf(Out, "%c", '0');
				}
			}
		}

		fprintf(Out, "\n");
	}

	// Deallocate partition
	for (int i = 0; i < num; i++) {
		free(seperate[i]);
	}
	free(seperate);
	seperate = NULL;
}

void getBin(int num, char *str, int padding) {

	*(str + padding) = '\0';

	long pos;
	if (padding == 5)
		pos = 0x10;
	else if (padding == 16)
		pos = 0x8000;
	else if (padding == 26)
		pos = 0x2000000;
	else if (padding == 32)
		pos = 0x80000000;

	long mask = pos << 1;
	while (mask >>= 1)
		*str++ = !!(mask & num) + '0';
}

// Convert a binary string to a decimal value
int getDec(char *bin) {

	int  b, k, m, n;
	int  len, sum = 0;

	// Length - 1 to accomodate for null terminator
	len = strlen(bin) - 1;

	// Iterate the string
	for(k = 0; k <= len; k++) {

		// Convert char to numeric value
		n = (bin[k] - '0');

		// Check the character is binary
		if ((n > 1) || (n < 0))  {
			return 0;
		}

		for(b = 1, m = len; m > k; m--)
			b *= 2;

		// sum it up
		sum = sum + n * b;
	}

	return sum;
}

