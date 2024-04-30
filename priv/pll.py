#!/usr/bin/env python3

def f(mul:int, div1:int, div2:int=1, pre:int=1, x:float=12.0, want:float=157.5) -> tuple[float, float]:
    assert mul in range(16, 321)
    assert div1 in range(1, 8)
    assert div2 in range(1, 8)
    ret = x / pre
    assert ret >= 5.0
    ret *= mul
    assert ret >=  750.0
    assert ret <= 1600.0
    ret /= (div1 * div2)
    err = abs(ret - want) / want
    return (ret, err)


syspll = None
for pre in (1, 2):
    if syspll: break
    for mul in range(16, 321):
        if syspll: break
        for div2 in range(1, 8):
            if syspll: break
            for div1 in range(1, 8):
                if syspll: break
                try:
                    z = f(pre=pre, mul=mul, div1=div1, div2=div2)
                    if z == (157.5, 0.0):
                        syspll = z
                except AssertionError as _:
                    pass

# (157.5, 0.0) 1 105 4 2
assert syspll
(mhz, _) = tuple(syspll)
print(mhz / 11)

# bool clock_configure( \
#    enum clock_index clk_index, uint32_t src, uint32_t auxsrc, uint32_t  src_freq, uint32_t freq)

# clock_configure(clock_index::clk_sys, 157500000, 14318182)