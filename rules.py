g = BCGen(r = 7, f = 6, head='gbreg_t t = 0;')
g.rule('addr',   '_RRR__', 'R0 = R1 + R2;')
g.rule('addr_f', '_FFF__', 'F0 = F1 + F2;')
g.rule('bltr',   'rRR__I', 'if (R0 < R1) JUMP(IMM);')
