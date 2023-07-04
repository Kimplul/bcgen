# Byte code generator

Separate project for now, but essentially create a program that takes some input
(for example)
```
addr(_GGG_)
{
	R0 = R1 + R2;
	return NEXT_INSN;
}

addi(_GGi_)
{
	R0 = R1 + IMM;
	return NEXT_INSN;
}

beqi(RGi__)
{
	if (R0 == IMM)
		return BRANCH;

	return NEXT_INSN;
}
```

and generates implementations with registers fixed and some kind of muxing
compilation functions that selects the correct implementation from some runtime
`compile_addr(1, 2, 3);`.
