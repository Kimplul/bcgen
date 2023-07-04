#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "bcode.h"

enum bcode_insn {
	END,
#include "gen/enum.inc"
};

static void append(struct compile_state *cs, enum bcode_insn label, breg_t imm)
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
#define PUSH_OP(label) append(cs, label, (breg_t){ .g = 0 })
#define PUSH_IMM_OP(label, imm) append(cs, label, (breg_t){ .g = imm })
#define PUSH_DIMM_OP(label, dimm) append(cs, label, (breg_t){ .f = dimm })

#include "gen/select.inc"
#include "gen/macro.inc"

#define IMM s.imm[pc].g
#define DIMM s.imm[pc].f
#define NEXT_INSN goto *s.op[pc++]
#define JUMP(i) goto *s.op[pc = (i)]

static gbreg_t _run(struct compile_state *cs, bool init)
{
	static void *labels[] = {
		[END] = &&end,
#include "gen/static.inc"
	};

	if (init) {
		cs->labels = labels;
		return 0;
	}

#include "gen/head.inc"
#include "gen/regs.inc"

	const struct run_state s = cs->s;
	size_t pc = 0;
	JUMP(pc);
#include "gen/body.inc"

end: /* we assume there's always at least one general register */
	return r0;
}

gbreg_t run(struct compile_state *cs)
{
	return _run(cs, false);
}

void init(struct compile_state *cs)
{
	cs->labels = NULL;
	cs->s.imm = NULL;
	cs->s.op = NULL;
	cs->size = 0;
	cs->pc = 0;
	_run(cs, true);
}

void destroy(struct compile_state *cs)
{
	free(cs->s.op);
	free(cs->s.imm);
}
