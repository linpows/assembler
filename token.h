#ifndef TOKEN_H
#define TOKEN_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/**
 * returns delimited token from in and sets out to rest of string
 * does not modify in
 */
static inline char *getToken(char *in, char *delim, char **out) {
	int len; // length
	char *ptr, *tptr, *token; // pointers and token

	len = strspn(in, delim); // get length of token
	ptr = (in + len);
	  
	tptr = strpbrk(ptr, delim); // get end of token
	if(tptr == NULL) {
		return(NULL);
	}
	len = tptr - ptr; // true length
	  
	*out = tptr + 1; // set out

	// allocate token
	token = malloc((len + 1) * sizeof(char));
	if(token == NULL) {
		return(NULL);
	}
	// copy token
	memcpy(token, ptr, len);
	token[len] = '\0';
	
	return(token);
}


#endif 
