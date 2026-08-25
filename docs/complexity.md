# Time and Space Complexity

This ledger distinguishes the native state model, the ordinary external
algorithm, and full ordinary materialization. It does not relabel a succinct
description as a free bit-level computation.

Let:

- `p` be the Wilson candidate and `n = p - 1`;
- `b = ceil(log2(n + 1))` be the input bit length;
- `s = ceil(sqrt(n))` be the external block width;
- `m` be a cyclic state order, `c` its cycle multiplicity, and `J` a requested
  jet horizon;
- `M(s)` be the cost of multiplying degree-`s` polynomials over the selected
  modular/CRT engine.

## Factorial and Wilson pipeline

| Stage | Time | Additional live space | What is actually produced |
|---|---:|---:|---|
| `upload_factorial_state` | `Theta(b)` native ledger steps | `Theta(b)` | Succinct q-Pochhammer program, certified principal-jet state, and evidence |
| `bind_quotient_view` | `O(1)` in the fixed 64-bit implementation | `O(1)` | Descriptor bound to complete state identity; no state payload |
| `download_wilson` coordinate | `Theta(s)` materialized factor-scale objects | `Theta(s)` scalar coordinate | Block polynomial/evaluation coordinate |
| `download_wilson` algorithm | `O(M(s) log s + s)` ring operations | current tree implementation `O(s log s)` coefficients | Exact `n! mod p`, Wilson decision, ledger, integrity witness |

With schoolbook polynomial multiplication, the conservative bound becomes
`O(s^2 log s) = O(n log n)` ring operations. With the fast transform/CRT path,
`M(s)` can be quasi-linear, giving a soft-`O(s)` arithmetic-operation regime.
That does not erase modulus-bit costs, CRT constants, allocations, or policy
limits.

The external coordinate has exactly

```text
s + 1 + floor(n / s) + (n mod s)
```

scalar slots in the frozen implementation. This is `Theta(sqrt(n))`, not
polylogarithmic. The code and result explicitly mark the former polylog claim
as rejected.

### Native-coordinate repair

SDK 1.1.0 changes where the external algorithm obtains `n`: it loads the rank
from the certified native factorial coordinate and verifies that the
coefficient is one. The modular algorithm, coordinate, polynomial engine, and
resource policy are otherwise unchanged. Therefore the optimized Wilson
path keeps the same asymptotic T-S bounds shown above. The additional binding
replay is at most linear in the succinct program description.

### Exact arbitrary-precision replay

Let

```text
B = bit_length(n!) = Theta(n log n)
```

and let `w=32` be the limb width of the included owned big integer.

| Exact stage | Time | Live/result space |
|---|---:|---:|
| sequential derivation | `Theta(sum(k log k)/w) = Theta(n^2 log n / w)` limb updates | `Theta(B/w)` limbs |
| independent product tree with schoolbook multiplication | `O((B/w)^2)` limb-product accumulations | `O(B/w)` live limbs |
| decimal download by repeated division | `O((B/w) * decimal_digits(n!))` limb divisions | `Theta(B)` output bits |
| exact Wilson remainder after derivation | `Theta(B/w)` limb scans | `O(1)` additional words |

The strict path runs both factorial derivations and compares the full values.
Its current upper bound is dominated by the schoolbook product-tree audit and
decimal conversion, not by the succinct native state. This extra work is
separately charged in `ExactFactorialLedger` and is not included in the
optimized Wilson headline.

Any implementation that downloads every bit or decimal digit of `n!` has an
unconditional `Omega(B)` output-time and output-space cost. The SDK therefore
keeps exact materialization as an explicit bounded policy path.

## Cyclic boundary pipeline

For fixed-width 64-bit values, normalization, upload, presentation transport,
native quotient, same-frame continuation, order closure, native checkpoint,
and order download each use `O(1)` word operations and `O(1)` state-sized
space. In a variable-width bit model, copying and validating an order costs at
least `Omega(b)` bits.

The primitive closure operation is a dense reference audit, not a native
constant-time primitive. Its implementation applies `c*m` factors and updates
two `(J+1) x m` tensors. Its bound is:

```text
time  = O(c * m^2 * J^2) modular updates
space = O(m * J) modular coefficients
```

At the exact ramification horizon `J = c`, this is
`O(c^3 * m^2)` time and `O(c*m)` coefficient space. The runtime ledger records
the actual dense coefficient and update counts.

## Factorial-derived cyclic structure

For order `m`, the primitive-period reference evaluator performs exactly
`m(m-1)` modular coefficient updates and keeps two `m`-coefficient vectors.
The independently derived Ramanujan audit costs
`O(sqrt(m) + m*omega(m))` fixed-width number-theory work.

Optional normalized-action materialization, valuation reattachment, and the
reference kernel calculation each use `O(m^2)` fixed-width operations and
`O(m)` coefficient space. The valuation replay records exactly
`2m(m-1)` modular updates.

Thus a fully materialized cyclic response is `Theta(m^2)` word work and
`Theta(m)` response/live coefficient space. With input length
`L=ceil(log2(m+1))`, those are exponential in `L`; the seven-word descriptor
does not change that execution fact.

## Public wrapper overhead

Each newly produced opaque handle performs one model allocation. Copying a
handle copies a `shared_ptr` in `O(1)` time/space; it does not copy, merge, or
compress the frozen state. Exceptions are used for invalid pipeline stages.

## Information-theoretic boundary

This implementation has not reached an `O(log p)` external Wilson algorithm.
Its ordinary external coordinate is `Theta(sqrt(p))`, so it is above the input
and one-bit-decision information boundary.

The native factorial state is not an ordinary binary expansion of `n!`.
The new exact path can download all digits, but honestly pays the
`Theta(n log n)`-bit output requirement and the additional arbitrary-precision
audit work. The optimized Wilson path avoids that download and keeps its prior
T-S main order.

`results/complexity_probe.txt`, generated by `build_and_test.sh`, reports the
actual object sizes, native ledger steps, payload bytes, external scalar slots,
dense reference updates, and one non-normative wall-clock sample for the local
compiler and machine.
