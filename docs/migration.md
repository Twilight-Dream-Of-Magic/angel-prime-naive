# Migration Guide

## Existing clients

No migration is required. The legacy Wilson pipeline remains:

```cpp
auto result = candidate(p)
            | upload_factorial_state()
            | bind_quotient_view()
            | download_wilson(policy);
```

## Opting into the joint consumer

Add the semantic header and bind the certified native coordinate before the
new Download operation:

```cpp
#include "angel/joint_wilson.hpp"

using namespace angel::prime;

JointWilsonPolicy policy{};
auto result = candidate(p)
            | upload_factorial_state()
            | bind_native_factorial()
            | project_wilson_jointly(policy);
```

The result adds detailed deterministic operation and peak-space accounting.
It does not change the native state or the existing observation types.

## Exact integer Download

The full arbitrary-precision path remains separate:

```cpp
auto native = candidate(p)
            | upload_factorial_state()
            | bind_native_factorial();
auto exact = native | derive_exact_factorial();
auto integer = exact | download_exact_factorial();
auto wilson = exact | observe_wilson_from_exact();
```

Use this path only when the complete integer or an independent exact oracle is
needed. The joint modular path does not materialize the integer.

## Error handling

Invalid stage transitions fail to compile. Invalid bindings and policy values
throw `std::invalid_argument`; polynomial resource limits return the existing
ordinary `ResourceLimit` alternative.
