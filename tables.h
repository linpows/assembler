#ifndef TABLES_H
#define TABLES_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

struct _Register
{
	char* name;
	char* bits;
};
typedef struct _Register Register;

enum _Type {N, J, RI, RO, RL, RR, RRI, RRO, RRR, SA, BO, NUL};
typedef enum _Type Type;

struct _OP
{
	char* name;
	char* bits;
	Type type;
};
typedef struct _OP OP;

struct _Label
{
	char* name;
	uint32_t addr;
};
typedef struct _Label Label;

struct _LTable
{
	int size;
	Label* table;
};
typedef struct _LTable LTable;

/**
 * Finds register with name n in register table
 */
Register findReg(char* n);

/**
 * Finds operation with name n in operation table
 */
OP findOp(char* n);

/**
 * Finds label with name n in label table
 */
Label* findLabel(char* n);

/**
 * adds Label l to label table
 */
void addLabel(char* name, uint32_t addr);

/**
 * frees LTable
 */
void freeLTable();

/**
 * prints LTable in alphabetical order
 */
void printLTable(FILE *Out);

#endif
