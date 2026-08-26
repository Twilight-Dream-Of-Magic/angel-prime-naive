# Joint Time-Space Ledger

## Classification

```text
result type                         local joint Pareto improvement
old time                            O(M(sqrt(n)) log n + sqrt(n))
new time                            O(M(sqrt(n/2)) log n + sqrt(n/2)) + O(1)
old space                           O(sqrt(n) log n) live coefficients
new space                           O(sqrt(n/2) log n) live coefficients
time asymptotically improved        NO
space asymptotically improved       NO
both reduced in one implementation YES
sqrt(n) coordinate eliminated      NO
full n! materialized               NO
Wilson consumed native n!          YES, through certified rank/coefficient
Angel state changed                NO
```

The asymptotic class is unchanged. Completion is therefore classified under
the strict local-improvement branch: deterministic work, ring additions, ring
multiplications, modular reductions, coefficient updates, allocation count,
peak live coefficient/limb space, and materialized coordinate size are all
strictly lower on the continuous audit interval. The fixed increasing release
sequence independently exhibits the same direction.

## Mathematical reduction

For `m=n+1` and `h=floor(n/2)`:

\[
n!\equiv
\begin{cases}
(-1)^h(h!)^2 & n\text{ even},\\
(-1)^h(h!)^2(h+1) & n\text{ odd}
\end{cases}
\pmod m.
\]

The implementation computes only `h! mod m`. It uses no inverse and follows
the same exact flow for prime, composite, even, and zero-divisor moduli.

## Instrumented old/new accounting

The old and new paths are replayed through the same exact polynomial engine
and the same counter conventions. The old audit deliberately materializes the
point array, value array, complete evaluation product tree, and sibling
remainders that the supplied implementation retains. The new audit generates
points at leaves, streams each value into one scalar residue, processes the two
top-level ranges sequentially, retires completed branches immediately, starts
Horner evaluation from the leading coefficient, and evaluates bottom-level
leaf points directly without degree-one monic remainders. Neither audit
materializes `n!`.

The corrected baseline coordinate count is
`block_coefficients + 2 * full_blocks`: the reconstructed baseline owns
both an evaluation-point array and a block-value array. Tail factors are
streamed time events, not persistent coordinate slots.

The detailed CSV records old and new values for:

```text
ring additions
ring multiplications
modular reductions
coefficient updates
limb products
limb additions
polynomial multiplications
schoolbook coefficient products
NTT butterflies
CRT mixed-radix digits
monic remainders
Horner coefficient steps
temporary polynomial count
temporary big-integer count
allocation count
copied-byte upper bound
peak live coefficients
peak live limbs
materialized coordinate count
native state nodes rewritten
```

All counters use checked accumulation. Counter overflow is an explicit failure;
no saturating value is relabelled as an exact upper bound.

## Fixed increasing sequence

| Candidate | Old work | New work | T ratio | Old peak | New peak | S ratio | Old coordinate | New coordinate | C ratio |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 257 | 807 | 383 | 0.47459727 | 258 | 84 | 0.32558140 | 49 | 9 | 0.18367347 |
| 1,021 | 2,661 | 1,251 | 0.47012401 | 534 | 178 | 0.33333333 | 95 | 18 | 0.18947368 |
| 4,093 | 9,570 | 4,297 | 0.44900731 | 1,141 | 372 | 0.32602980 | 191 | 41 | 0.21465969 |
| 16,381 | 35,811 | 16,068 | 0.44868895 | 2,420 | 795 | 0.32851240 | 383 | 91 | 0.23759791 |
| 65,521 | 137,686 | 61,142 | 0.44406839 | 5,107 | 1,686 | 0.33013511 | 767 | 181 | 0.23598435 |
| 100,003 | 208,616 | 92,350 | 0.44267937 | 6,434 | 2,111 | 0.32810071 | 948 | 224 | 0.23628692 |
| 250,003 | 514,444 | 226,954 | 0.44116366 | 10,481 | 3,460 | 0.33012117 | 1,500 | 354 | 0.23600000 |
| 500,009 | 1,013,841 | 449,704 | 0.44356462 | 26,073 | 4,998 | 0.19169256 | 2,121 | 501 | 0.23620934 |
| 1,000,003 | 1,527,021 | 979,902 | 0.64170827 | 35,604 | 18,800 | 0.52803056 | 3,000 | 708 | 0.23600000 |

Sequence extrema:

```text
T_new/T_old min = 0.44116366
T_new/T_old max = 0.64170827
S_new/S_old min = 0.19169256
S_new/S_old max = 0.52803056
C_new/C_old min = 0.18367347
C_new/C_old max = 0.23759791
ring-addition ratio min/max = 0.33565784 / 0.68492896
ring-multiplication ratio min/max = 0.33601614 / 0.68503434
coefficient-update ratio min/max = 0.34072971 / 0.68462337
allocation ratio min/max = 0.61379310 / 0.69635628
```

Every displayed ratio is below one. These finite ratios are not fitted into a
new asymptotic theorem.

## Detailed representative row

For `m=1,000,003`, `n=1,000,002`:

| Counter | Old | New |
|---|---:|---:|
| ring additions | 1,843,270 | 1,262,509 |
| ring multiplications | 1,845,271 | 1,264,074 |
| modular reductions | 1,845,271 | 1,264,074 |
| coefficient updates | 1,835,155 | 1,256,390 |
| limb products | 1,845,271 | 1,264,074 |
| limb additions | 1,843,270 | 1,262,509 |
| polynomial multiplications | 2,038 | 1,450 |
| schoolbook coefficient products | 693,709 | 437,116 |
| NTT butterflies | 307,200 | 276,480 |
| CRT mixed-radix digits | 18,080 | 11,960 |
| monic remainders | 1,996 | 704 |
| Horner coefficient steps | 999 | 900 |
| temporary polynomials | 5,994 | 3,527 |
| temporary big integers | 0 | 0 |
| allocations | 7,993 | 4,937 |
| copied-byte upper bound | 303,680 | 185,680 |
| peak live limbs | 35,604 | 18,800 |
| materialized point values | 999 | 0 |
| materialized block values | 999 | 0 |
| native state nodes rewritten | 0 | 0 |

## Continuous audit

The executable regression suite checks every candidate from 2 through 2,048:

```text
residue equality                    PASS
new deterministic work < old       PASS for all 2,047 inputs
new ring additions < old            PASS for all 2,047 inputs
new ring multiplications < old       PASS for all 2,047 inputs
new modular reductions < old         PASS for all 2,047 inputs
new coefficient updates < old        PASS for all 2,047 inputs
new allocation count < old           PASS for all 2,047 inputs
new peak coefficient/limb bound < old PASS for all 2,047 inputs
new coordinate < old                 PASS for all 2,047 inputs
```

The public API comparison additionally covers candidates 2 through 1,024 and
structured square, prime-power, semiprime, multi-factor, and block-boundary
fixtures. Exact arbitrary-precision checks cover `0!`, `1!`, `20!`, `100!`,
and `1000!`.

## Required integrity counters

```text
native state nodes rewritten 0
ordinary feedback            0
fixed-width truncation       0
full factorial materialized  0
```

The fixed-word Wilson chart rejects unsupported range through typed limits; it
does not truncate. The separate exact factorial path retains owned
arbitrary-precision integer support and is accounted independently.
