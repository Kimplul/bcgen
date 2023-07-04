#ifndef BCGEN_BCODE_H
#define BCGEN_BCODE_H

#include <stddef.h>

typedef unsigned long gbreg_t;
typedef double fbreg_t;
typedef size_t breloc_t;

typedef union {
	gbreg_t g;
	fbreg_t f;
} breg_t;

struct run_state {
	void* (*op);
	breg_t *imm;
};

struct compile_state {
	struct run_state s;
	void* (*labels);
	size_t size;
	size_t pc;
};

#include "gen/select.h"

gbreg_t run(struct compile_state *cs);
void init(struct compile_state *cs);
void destroy(struct compile_state *cs);
void patch(struct compile_state *cs, breloc_t reloc, breg_t imm);

#endif /* BCGEN_BCODE_H */
