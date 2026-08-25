# Migration Guide

## Cyclic boundary API

| Historical call | Semantic API |
|---|---|
| `SpecificationGate::encode_canonical` | `EncodedOrder::canonical` |
| `SpecificationGate::normalize` | `normalize` |
| `ThreeGateBoundaryProtocol::upload` | `encoded | upload(authority)` |
| `change_presentation` | `state | change_presentation(target)` |
| `native_quotient` | `state | quotient_to(order)` |
| `native_same_frame_continue` | `state | continue_to(horizon)` |
| `native_export` | `state | export_checkpoint()` |
| `native_import` | `checkpoint | import_checkpoint()` |
| `close_order_observation` | `state | observe_order()` |
| `reference_close_primitive_observation` | `state | observe_primitive(horizon)` |
| `download` | `closed | download()` |

The former static god class has no public replacement. Each operation belongs
to its module and is valid only for the correct left-hand state type.

## Factorial/Wilson API

The R60 convenience function is replaced by visible stages:

```cpp
auto state = candidate(p) | upload_factorial_state();
auto view = state | bind_quotient_view();
auto result = view | download_wilson(policy);
```

This makes it impossible to confuse the external quotient view with the
complete R57 state and exposes the exact point where ordinary projection
begins.

The signature is unchanged in SDK 1.1.0. Its implementation now loads the
factor count from the native factorial result coordinate. Applications that
need explicit evidence can use:

```cpp
auto native = candidate(p)
            | upload_factorial_state()
            | bind_native_factorial();
auto result = native | project_wilson_from_native(policy);
```

For full arbitrary-precision replay:

```cpp
auto exact = native | derive_exact_factorial();
auto integer = exact | download_exact_factorial();
auto wilson = exact | observe_wilson_from_exact();
```

## Behavioral audit API

The behavioral reference mathematics is now in
`angel::boundary::behavior`. Names are shortened to `MooreMachine`,
`minimal_partition`, `observation_is_resumable`, and `certify_stability`.

## Error handling

Historical optional-returning operations map to `std::invalid_argument` in the
operator pipeline. This keeps chaining readable while preserving rejection.
Applications that need non-throwing control flow can catch at the pipeline
boundary and convert to their local result type.
