#include "tables.h"

#define REGRECORDS 32
#define OPRECORDS 28

static Register RTable[REGRECORDS] = {
	{"zero", "00000"}, 
	{"at", "00001"}, 
	{"v0", "00010"}, 
	{"v1", "00011"}, 
	{"a0", "00100"}, 
	{"a1", "00101"}, 
	{"a2", "00110"}, 
	{"a3", "00111"}, 
	{"t0", "01000"}, 
	{"t1", "01001"}, 
	{"t2", "01010"}, 
	{"t3", "01011"}, 
	{"t4", "01100"}, 
	{"t5", "01101"}, 
	{"t6", "01110"}, 
	{"t7", "01111"}, 
	{"s0", "10000"}, 
	{"s1", "10001"}, 
	{"s2", "10010"}, 
	{"s3", "10011"},
	{"s4", "10100"}, 
	{"s5", "10101"}, 
	{"s6", "10110"}, 
	{"s7", "10111"}, 
	{"t8", "11000"}, 
	{"t9", "11001"},
	{"k0", "11010"},
	{"k1", "11011"},
	{"gp", "11100"},
	{"sp", "11101"},
	{"fp", "11110"},
	{"ra", "11111"}
};

static OP OpTable[OPRECORDS] = {
	{"lw", "100011", RO}, // also RL version
	{"sw", "101011",RO}, 
	{"lui", "001111", RI}, 
	{"add", "100000", RRR}, 
	{"addi", "001000", RRI}, 
	{"addu", "100001", RRR}, 
	{"addiu", "001001", RRI}, 
	{"and", "100100", RRR}, 
	{"andi", "001100", RRI},  
	{"nor", "100111", RRR}, 
	{"sll", "000000", SA}, 
	{"slt", "101010", RRR}, 
	{"slti", "001010", RRI}, 
	{"sra", "000011", SA}, 
	{"srav", "000111", RRR}, 
	{"sub", "100010", RRR},
	{"beq", "000100", RRO}, 
	{"blez", "000110", BO}, 
	{"bgtz", "000111", BO}, 
	{"bne", "000101", RRO}, 
	{"j", "000010", J}, 
	
	{"mul", "000010", RRR},
	{"move", "100001", RR}, 
	{"blt", "000101", RRO}, 
	{"la", "001000", RL}, 
	{"li", "001001", RI},
	{"nop", "000000", N}, 
	{"syscall", "001100", N}
};

LTable labels = {0, NULL}; // table of labels

OP opNotFound = {NULL, NULL, NUL}; // empty op

void addLabel(char* n, uint32_t a) {
	// increase size of table
	labels.size = labels.size + 1;
	
	// allocate for new label
	size_t strLen = strlen(n);
	void *ret = realloc(labels.table, labels.size * sizeof(Label));
	if(ret == NULL) {
		printf("Out of memory\n");
		exit(1);
	}
	labels.table = ret;
	
	// allocate for label name
	labels.table[labels.size-1].name = malloc((strLen+1) * sizeof(char));
	if(labels.table[labels.size-1].name == NULL) {
		printf("Out of memory\n");
		exit(1);
	}
	
	// copy label name
	memcpy(labels.table[labels.size-1].name, n, strLen);
	labels.table[labels.size-1].name[strLen] = '\0';
	
	// copy address
	labels.table[labels.size-1].addr = a;
}

Register findReg(char* n) {
	int index = 0;
	int cmp = strcmp(n, RTable[index].name);
	while(cmp != 0 && index < REGRECORDS-1) {
		index = index + 1;
		cmp = strcmp(n, RTable[index].name);
	}
	return RTable[index];
}

OP findOp(char* n) {
	int index = 0;
	int cmp = strcmp(n, OpTable[index].name);
	while(cmp != 0 && index < OPRECORDS-1) {
		index = index + 1;
		cmp = strcmp(n, OpTable[index].name);
	}
	// op not found
	if(index == OPRECORDS - 1) {
		return opNotFound;
	}
	return OpTable[index];
}

Label* findLabel(char* n) {
	int index = 0;
	int cmp = strcmp(n, labels.table[index].name);
	while(cmp != 0 && index < labels.size) {
		index = index + 1;
		cmp = strcmp(n, labels.table[index].name);
	}
	// label not found
	if(index == labels.size) {
		return NULL;
	}
	return labels.table + index;
}

void freeLTable() {
	// free names
	for(int i=0; i < labels.size; i++) {
		free(labels.table[i].name);
	}
	// free table
	free(labels.table);
	labels.table = NULL;
}

void printLTable(FILE *Out) {
	char* curName = "z"; // last printed
	char* nextName = "a"; // next to print
	
	// while another label to print
	while(strcmp(curName, nextName) !=0){
		curName = nextName; // set last printed
		nextName = "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"; // reasonably low string
		
		// find next alphabetical string
		for(int i = 0; i < labels.size; i++) {
			if(strcmp(labels.table[i].name, curName) > 0 && strcmp(labels.table[i].name, nextName) < 0) {
				nextName = labels.table[i].name;
			}
		}
		// if next string found
		if(strcmp("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz", nextName) !=0) {
			// get label
			Label *l = findLabel(nextName);
			
			// print label
			fprintf(Out, "0x%08x:\t%s\n", l->addr, l->name);
		}
	}
}
