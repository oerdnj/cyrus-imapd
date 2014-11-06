/*
 * grammar.h
 *
 *  Created on: Nov 6, 2014
 *      Author: James Cassell
 */

#ifndef SIEVE_GRAMMAR_H_
#define SIEVE_GRAMMAR_H_

#include "strarray.h"
#include "varlist.h"

EXPORTED int is_identifier(char *s);
EXPORTED char *parse_string(const char *s, variable_list_t *vars);

#endif /* SIEVE_GRAMMAR_H_ */
