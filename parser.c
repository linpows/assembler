#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <unistd.h>
#include "parser.h"
#include "token.h"

void parseFile(FILE *fptr, int pass, FILE *Out) {
	char line[MAX_LINE_LENGTH + 1]; // read line
	uint32_t currentAddr = 0x00000000; // current address
	char *tokPtr = NULL; // pointer to next token
	char *gotLine = NULL; // return of fgets
	char *token = NULL; // token
	int inData = 0; // 1 if in data section
	
	// loop continues until next line is null
	while(1) {
		if((gotLine = fgets(line, MAX_LINE_LENGTH, fptr)) == NULL)
			break;
		line[MAX_LINE_LENGTH] = 0; // 0 terminate

		tokPtr = line; // set tokPtr

		// loops til end of line or certain token is reached
		while(1) {
			// get token
			token = getToken(tokPtr, " \n\t$,", &tokPtr);

			// skip comments
			if(token == NULL || *token == '#') {
				free(token);
				break;
			}

			// reached data
			else if(strcmp(token, ".data") == 0) {
				currentAddr = 0x00002000;
				inData = 1;
				if(pass == 3) {
					fprintf(Out, "\n");
				}
			}
			// reached text
			else if(strcmp(token, ".text") == 0) { 
				currentAddr = 0x00000000;
				inData = 0;
			}

			// Get labels in first pass
			if(pass == 1) {
				if(inData == 0) {
					OP op = findOp(token);
					// increment current address according to operation
					if(op.type != NUL) {
						if(strcmp(token, "blt") == 0) {
							currentAddr = currentAddr + 8;
							free(token);
							break;
						}
						else {
							currentAddr = currentAddr + 4;
							free(token);
							break;
						}
					}
				}
				// if token has ':', then it is a label
				if(strstr(token, ":")) {
					// address of instruction
					if(inData == 0) {
						// take out ':'
						size_t tokLen = strlen(token);
						token[tokLen - 1] = '\0';

						// add label to table
						addLabel(token, currentAddr);
					}
					// in data
					else {
						char *varTok = NULL; // token of variable
						char *varTokPtr = tokPtr; // pointer to rest of string

						// is num
						if(strstr(tokPtr, ".word")) {
							// is repeated array
							if(strstr(varTokPtr, ":")) {
								// get number
								varTok = getToken(varTokPtr, ":", &varTokPtr);

								// get frequency/size of array
								int freq = atoi(varTokPtr);

								// take out ':'
								size_t tokLen = strlen(token);
								token[tokLen - 1] = '\0';

								// add label to table
								addLabel(token, currentAddr);

								// increase current address
								currentAddr += (freq * 4);
								free(varTok);
								free(token);
								break;
							}
							// is array
							else if(strstr(varTokPtr, ",")) {
								// get first num
								varTok = getToken(varTokPtr, ":", &varTokPtr);

								int freq = 1; // size of array
								while(strstr(varTokPtr, ",")) {
									varTok = getToken(varTokPtr, ",", &varTokPtr);
									freq++;
									free(varTok);
								}
								
								// take out ':'
								size_t tokLen = strlen(token);
								token[tokLen - 1] = '\0';

								// add label to table
								addLabel(token, currentAddr);

								// increase current address
								currentAddr += (freq * 4);
								free(token);
								break;
							}

							// not array
							else {
								// take out ':'
								size_t tokLen = strlen(token);
								token[tokLen - 1] = '\0';

								// add label to hash table
								addLabel(token, currentAddr);
								
								// increase current address
								currentAddr = currentAddr + 4;
								free(token);
								break;
							}
						}

						// is string
						else if(strstr(tokPtr, ".asciiz")) {
							// Store the string in varTok
							varTok = getToken(varTokPtr, "\"", &varTokPtr);
							free(varTok);
							varTok = getToken(varTokPtr, "\"", &varTokPtr);
							
							// take out ':' from token
							size_t tokLen = strlen(token);
							token[tokLen - 1] = '\0';

							// add label to hash table
							addLabel(token, currentAddr);

							// increase current address
							size_t strLen = strlen(varTok);
							currentAddr += strLen + 1;
							
							// check if padding needed
							gotLine = fgets(line, MAX_LINE_LENGTH, fptr);
							line[MAX_LINE_LENGTH] = 0;
							int lineLen = strlen(line);
							// check if next line has string
							if(gotLine == NULL || strstr(line, ".asciiz") == NULL) {
								line[0] = '#';// set to ignore
								fseek(fptr, -lineLen, SEEK_CUR);// rewind to last line
								
								// calculate padding
								size_t padding = 0;
								if (currentAddr % 4 != 0) {
									padding = 4 - currentAddr % 4;
								}
								
								// increase current address
								currentAddr = currentAddr + padding;
							}
							// continue to next string
							tokPtr = line;
							
							free(varTok);
							free(token);
							break;
						}
					}
				}
			}

			// interpret text in next pass
			else if(pass == 2) {
				// get to text
				if (inData == 0) {	
					// makes sure token isn't null when getting op				
					if(token == NULL) {
						free(token);
						break;
					}

					// find instruction
					OP op = findOp(token);
					
					// increase current address
					if(op.type != NUL) {
						if(strcmp(token, "blt") == 0) {
							currentAddr = currentAddr + 8;
						}
						else {
							currentAddr = currentAddr + 4;
						}
					}

					
					if(strcmp(token, "syscall") == 0) {
						fprintf(Out, "00000000000000000000000000001100\n");
					}
					else if(strcmp(token, "nop") == 0) {
						fprintf(Out, "00000000000000000000000000000000\n");
					}
					// if not a supported operation
					else if(op.type == NUL) {
						free(token);
						continue;
					}
					else if(op.type == J) { // j
						char *targetTok = NULL;
						
						// get target
						targetTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						
						// find label
						Label *label = findLabel(targetTok);
							
						// get address << 2
						char addrBin[33];
						getBin((label->addr)/4, addrBin, 32);
							
						// fit into 26 bits
						char upperBits[27];
						for (int i = 6; i < 32; i++) {
							upperBits[i-6] = addrBin[i];
						}
						upperBits[26] = '\0';
						int imm = getDec(upperBits);
						
						// print instruction
						jInstruction(op.bits, imm, Out);
						
						free(targetTok);
					}
					else if(op.type == RI) { // lui, li
						char *rtTok = NULL;
						char *immTok = NULL;
						
						rtTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						immTok = getToken(tokPtr, " $,\n\t()", &tokPtr);
						int imm = 0;
						
						// lui
						if(strcmp(op.name, "lui") == 0){
							// get num
							char addrBin[33];
							getBin(atoi(immTok), addrBin, 32);
							
							// fit in 16 bits
							char upperBits[17];
							for (int i = 16; i < 32; i++) {
								upperBits[i-16] = addrBin[i];
							}
							upperBits[16] = '\0';
								
							imm = getDec(upperBits);
						}
						else {
							imm = atoi(immTok);
						}
						
						// print instruction
						iInstruction(op.bits, "zero", rtTok, imm, Out);
						
						free(rtTok);
						free(immTok);
					}
					else if(op.type == RO) { // lw, sw
						char *rtTok = NULL;
						char *rsTok = NULL;
						char *offTok = NULL;
						
						rtTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						offTok = getToken(tokPtr, " $,\n\t()", &tokPtr);
						rsTok = getToken(tokPtr, " $,\n\t()", &tokPtr);
						
						// second lw case
						if(strcmp(op.name, "lw") == 0 && !isdigit(offTok[0]) && offTok[0] != '-') {
							// find label
							Label *label = findLabel(offTok);
							
							// get address
							char addrBin[33];
							getBin(label->addr, addrBin, 32);
							
							// fit in 16 bits
							char upperBits[17];
							for (int i = 16; i < 32; i++) {
								upperBits[i-16] = addrBin[i];
							}
							upperBits[16] = '\0';
							int offset = getDec(upperBits);
							
							// print instruction
							iInstruction(op.bits, "zero", rtTok, offset, Out);
						}
						else {
							// get offset
							int offset = atoi(offTok);
							
							// print instruction
							iInstruction(op.bits, rsTok, rtTok, offset, Out);
						}
						
						free(rtTok);
						free(rsTok);
						free(offTok);
					}
					else if(op.type == RL) { // la
						char *rdTok = NULL;
						char *labelTok = NULL;
						
						rdTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						labelTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						
						// find label
						Label *label = findLabel(labelTok);
							
						// get address
						char addrBin[33];
						getBin(label->addr, addrBin, 32);
							
						// fit in 16 bits
						char upperBits[17];
						for (int i = 16; i < 32; i++) {
							upperBits[i-16] = addrBin[i];
						}
						upperBits[16] = '\0';
						int imm = getDec(upperBits);
						
						// print instruction
						iInstruction(op.bits, "zero", rdTok, imm, Out);
						
						free(rdTok);
						free(labelTok);
					}
					else if(op.type == RR) { // move
						char *rdTok = NULL;
						char *rsTok = NULL;
						
						rdTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						rsTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						
						// print instruction
						rInstruction("000000", op.bits, "zero", rsTok, rdTok, 0, Out);
						
						free(rdTok);
						free(rsTok);
					}
					else if(op.type == RRI) { // addi, addui, andi, slti
						char *rtTok = NULL;
						char *rsTok = NULL;
						char *immTok = NULL;
						
						rtTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						rsTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						immTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						
						// get immediate
						int imm = atoi(immTok);
						
						// print instruction
						iInstruction(op.bits, rsTok, rtTok, imm, Out);
						
						free(rtTok);
						free(rsTok);
						free(immTok);
					}
					else if(op.type == RRO) { // beq, bne, blt
						char *rtTok = NULL;
						char *rsTok = NULL;
						char *labelTok = NULL;
						
						rsTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						rtTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						labelTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						
						// find label
						Label *label = findLabel(labelTok);
							
						// get address
						char addrBin[33];
						getBin((label->addr - currentAddr)/4, addrBin, 32);
							
						// fit in 16 bits
						char upperBits[17];
						for (int i = 16; i < 32; i++) {
							upperBits[i-16] = addrBin[i];
						}
						upperBits[16] = '\0';
						int imm = getDec(upperBits);
						
						// print instruction
						if(strcmp(op.name, "blt") == 0) {// blt special
							rInstruction("000000", "101010", rsTok, rtTok, "at", 0, Out);
							iInstruction(op.bits, "at", "zero", imm, Out);
						}
						else {
							iInstruction(op.bits, rsTok, rtTok, imm, Out);
						}
						
						free(rtTok);
						free(rsTok);
						free(labelTok);
					}
					else if(op.type == RRR) { // add, addu, and, mul, nor, slt, srav, sub
						char *rdTok = NULL;
						char *rsTok = NULL;
						char *rtTok = NULL;
						
						rdTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						rsTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						rtTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						
						// print instruction
						if(strcmp(op.name, "mul") == 0) { // mul special
							rInstruction("011100", op.bits, rsTok, rtTok, rdTok, 0, Out);
						}
						else if(strcmp(op.name, "srav") == 0) { // srav special
							rInstruction("000000", op.bits, rtTok, rsTok, rdTok, 0, Out);
						}
						else {
							rInstruction("000000", op.bits, rsTok, rtTok, rdTok, 0, Out);
						}
						
						free(rtTok);
						free(rsTok);
						free(rdTok);
					}
					else if(op.type == SA) { // sll, sra
						char *rdTok = NULL;
						char *rtTok = NULL;
						char *saTok = NULL;
						
						rdTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						rtTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						saTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						
						// get shift amount
						int sa = atoi(saTok);
						
						// print instruction
						rInstruction("000000", op.bits, "zero", rtTok, rdTok, sa, Out);
						
						free(rtTok);
						free(saTok);
						free(rdTok);
					}
					else if(op.type == BO) { // blez, bgtz
						char *rsTok = NULL;
						char *labelTok = NULL;
						
						rsTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						labelTok = getToken(tokPtr, " $,\n\t", &tokPtr);
						
						// find label
						Label *label = findLabel(labelTok);
							
						// get address
						char addrBin[33];
						getBin((label->addr - currentAddr)/4, addrBin, 32);
						
						// fit in 16 bits
						char upperBits[17];
						for (int i = 16; i < 32; i++) {
							upperBits[i-16] = addrBin[i];
						}
						upperBits[16] = '\0';
						int imm = getDec(upperBits);
						
						// print instruction
						iInstruction(op.bits, rsTok, "zero", imm, Out);
						
						free(rsTok);
						free(labelTok);
					}
				}
			}
			// print data
			else if(pass == 3) {
				// reach data part
				if(inData == 1){
					char *varTok = NULL; // token for variable
					char *varTokPtr = tokPtr; // pointer to rest of line

					// is num
					if(strstr(tokPtr, ".word")) {
						int varValue; // value of variable

						// is repeated array
						if(strstr(varTokPtr, ":")) {
							// put num in varTok
							varTok = getToken(varTokPtr, ":", &varTokPtr);

							// get array size
							int freq = atoi(varTokPtr);

							// get value of num
							sscanf(varTok, "%*s %d", &varValue);

							// print repeatedly
							for (int i = 0; i < freq; i++) {
								printWord(varValue, Out);
							}
							free(varTok);
						}
						// is array
						else if(strstr(varTokPtr, ",")) {
								// put first num in varTok
								varTok = getToken(varTokPtr, ",", &varTokPtr);
								
								// get value and print
								sscanf(varTok, "%*s %d", &varValue);
								printWord(varValue, Out);

								// repeat for rest of numbers
								while(strstr(varTokPtr, ",")) {
									free(varTok);
									varTok = getToken(varTokPtr, ",", &varTokPtr);
									sscanf(varTok, "%d", &varValue);
									printWord(varValue, Out);
								}
								
								// print last number
								sscanf(varTokPtr, "%d", &varValue);
								printWord(varValue, Out);
								free(varTok);
						}
						// not an array
						else {
							// get value and print
							sscanf(varTokPtr, "%*s %d", &varValue);
							printWord(varValue, Out);
						}
					}
					// is string
					else if (strstr(tokPtr, ".asciiz")) {
						char *strBld = NULL; // string builder
						size_t bldLen = 0; // length of string builder including \0
						
						// while line includes asciiz
						while(strstr(line, ".asciiz")) {
							// put string in varTok
							varTok = getToken(varTokPtr, "\"", &varTokPtr);
							free(varTok);
							varTok = getToken(varTokPtr, "\"", &varTokPtr);
							
							// get length and add to bldLen
							size_t strLen = strlen(varTok);
							strLen++;
							bldLen += strLen;
							
							// allocate for string
							void *ret = realloc(strBld, bldLen * sizeof(char));
							if(ret == NULL) {
								printf("Out of memory\n");
								exit(1);
							}
							strBld = ret;
							
							// put string in end of strBld
							memcpy((strBld + bldLen - strLen), varTok, strLen - 1);
							*(strBld + bldLen - 1) = 0;
						
							// get next line
							if ((gotLine = fgets(line, MAX_LINE_LENGTH, fptr)) == NULL) {
								break;
							}
							line[MAX_LINE_LENGTH] = 0;
							int lineLen = strlen(line);
							
							// check if next line has string
							if(strstr(line, ".asciiz") == NULL) {
								line[0] = '#'; // ignore
								tokPtr = line; // reset tokPtr
								fseek(fptr, -lineLen, SEEK_CUR); // rewind to last line
							} 
							// not string, so end string building and prepare for next line
							else {
								free(token);
								token = getToken(line, " \n\t$,", &tokPtr);
								free(varTok);
								varTokPtr = tokPtr;
							}
						}
						
						// double checking string built
						if(strBld != NULL) {
							// print string block
							printString(strBld, bldLen, Out);
							free(strBld);
						}
						free(varTok);
					}
				}
			}
			free(token);
		}
	}
}
