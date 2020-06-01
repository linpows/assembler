#include "tables.h"

#ifndef CONVERT_H
#define CONVERT_H

/**
 * prints r type
 */
void rInstruction(char *opcode, char *func, char *rs, char *rt, char *rd, int shamt, FILE *Out);

/**
 * prints i type
 */
void iInstruction(char *opcode, char *rs, char *rt, int immediate, FILE *Out);

/**
 * prints j type
 */
void jInstruction(char *opcode, int immediate, FILE *Out);

/**
 * prints word
 */
void printWord(int bin, FILE *Out);

/**
 * prints asciiz
 */
void printString(char string[], size_t strLen, FILE *Out);

/**
 * converts num to binary
 */
void getBin(int num, char *str, int padding);

/**
 * converts binary to num
 */
int getDec(char *bin);

#endif
