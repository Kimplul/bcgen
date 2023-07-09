#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "bcode.h"

#include "gen/head.inc"

enum bcode_insn {
	END,
#include "gen/enum.inc"
};

static void append(struct compile_state *cs, enum bcode_insn label, bcval_t imm)
{
	assert(cs->labels);

	if (cs->pc == cs->size) {
		cs->size *= 2;
		/* better memory checks could be a good idea */
		void *p = realloc(cs->s.op, cs->size * sizeof(*cs->s.op));
		assert(p);

		p = realloc(cs->s.imm, cs->size * sizeof(*cs->s.imm));
		assert(p);
	}

	struct run_state *s = &cs->s;
	s->op[cs->pc] = cs->labels[label];
	s->imm[cs->pc] = imm;
	cs->pc++;
}

#define RELOC cs->pc
#define PUSH_OP(label) append(cs, label, (bcval_t){ .r = 0 })
#define PUSH_IMM_OP(label, imm) append(cs, label, (bcval_t){ .r = imm })
#define PUSH_DIMM_OP(label, dimm) append(cs, label, (bcval_t){ .d = dimm })

#include "gen/select.inc"
#include "gen/macro.inc"

#define IMM s.imm[pc].r
#define SIMM s.imm[pc].s
#define FIMM s.imm[pc].f
#define DIMM s.imm[pc].d

#define NEXT_INSN goto *s.op[++pc]
#define JUMP(i) goto *s.op[pc = (i)]

static ubcval_t _run(struct compile_state *cs, bool init)
{
	static void *labels[] = {
		[END] = &&end,
#include "gen/static.inc"
	};

	if (init) {
		cs->labels = labels;
		return 0;
	}

#include "gen/regs.inc"
#include "gen/extra.inc"

	const struct run_state s = cs->s;
	size_t pc = 0;
	JUMP(pc);

#include "gen/body.inc"

end: /* we assume there's always at least one general register */
	return r0;
}

ubcval_t run(struct compile_state *cs)
{
	return _run(cs, false);
}

void init(struct compile_state *cs)
{
	cs->labels = NULL;
	cs->size = 16;

	cs->s.imm = malloc(cs->size * sizeof(*cs->s.imm));
	assert(cs->s.imm);

	cs->s.op = malloc(cs->size * sizeof(*cs->s.op));
	assert(cs->s.op);

	cs->pc = 0;
	_run(cs, true);
}

void end(struct compile_state *cs)
{
	PUSH_OP(END);
}

bcval_t label(struct compile_state *cs)
{
	return (bcval_t){.r = cs->pc};
}

void patch(struct compile_state *cs, bcreloc_t reloc, bcval_t imm)
{
	cs->s.imm[reloc] = imm;
}

void destroy(struct compile_state *cs)
{
	free(cs->s.op);
	free(cs->s.imm);
}
