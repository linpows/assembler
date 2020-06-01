#include "tables.h"
#include "convert.h"
#include "token.h"

#ifndef PARSER_H
#define PARSER_H

#define MAX_LINE_LENGTH 256

/**
 * parses input
 */
void parseFile(FILE *fptr, int pass, FILE *Out);

#endif
