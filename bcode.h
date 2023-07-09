#ifndef BCGEN_BCODE_H
#define BCGEN_BCODE_H

#include <stddef.h>

typedef unsigned long ubcval_t;
typedef signed long sbcval_t;
typedef float fbcval_t;
typedef double dbcval_t;
typedef size_t bcreloc_t;

typedef union {
	ubcval_t r;
	sbcval_t s;
	fbcval_t f;
	dbcval_t d;
} bcval_t;

struct run_state {
	void* (*op);
	bcval_t *imm;
};

struct compile_state {
	struct run_state s;
	void* (*labels);
	size_t size;
	size_t pc;
};

#include "gen/select.h"

ubcval_t run(struct compile_state *cs);
void init(struct compile_state *cs);
void end(struct compile_state *cs);
void destroy(struct compile_state *cs);

bcval_t label(struct compile_state *cs);
void patch(struct compile_state *cs, bcreloc_t reloc, bcval_t imm);

#endif /* BCGEN_BCODE_H */
