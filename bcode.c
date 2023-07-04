#include <stdbool.h>
#include "bcode.h"

enum bcode_insn {
	END,
#include "gen/enum.inc"
};

static void append(struct compile_state *cs, enum bcode_insn label, breg_t imm)
{
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

static void _run(struct compile_state *cs, bool init)
{
	static void *labels[] = {
		[END] = &&end,
#include "gen/static.inc"
	};

	if (init) {
		cs->labels = labels;
		return;
	}

#include "gen/head.inc"
#include "gen/regs.inc"

	const struct run_state s = cs->s;
	size_t pc = 0;
	JUMP(pc);
#include "gen/body.inc"

end:
	return;
}

void run(struct compile_state *cs)
{
	_run(cs, false);
}

void init(struct compile_state *cs)
{
	_run(cs, true);
}

void destroy(struct compile_state *cs)
{
}
