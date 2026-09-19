---
title: liara-platform
description: What the platform module owns, why one repository holds every operating-system detail, and what exists today.
sidebar:
  label: Overview
  order: 0
---

The reference implementation of the platform interface. It is the only place in the engine allowed to know which operating system it is running on.

That confinement is the whole point. "Linux and Windows are both first-class" is a claim that either holds in one repository or degenerates into an `#ifdef` in every other one, and this is the repository where it holds.

## What it owns

**Window management.** Creation, resizing, fullscreen, and exposure of the native handle the renderer needs to create its surface. SDL3 is the backend, and no SDL type crosses the module boundary.

**Input devices.** Keyboard, mouse, gamepad, reported as physical device state and physical events. The module does not know what an action is; mapping a physical input to a logical action is a consumer's concern.

**OS signals and shutdown requests.** SIGINT, SIGTERM, the Windows console control events and the window's close button, which are deliberately one thing at the interface. The [module page](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/modules/platform/) explains why shutdown is polled rather than delivered through a callback, and the three constraints that follow from it.

**Timing.** A monotonic clock and high-resolution counters, so the host's notion of time does not depend on which standard library it happened to be built against.

## What it does not

No ECS, no rendering, no audio device. Audio talks to the operating system too, and the boundary here is the concern rather than the fact of being OS-specific.

No file I/O beyond resolving standard paths, since reading files is `liara-assets`. No logical input mapping.

## What exists today

A skeleton. The module reports itself through `liara_platform_info()` and `liara_platform_abi_version()`, `src/` holds one file, and the contract in `liara-interfaces` declares those two functions and nothing more.

There is no window, no event pump, no clock, and no SDL3 dependency. That is deliberate rather than unfinished: the repository exists so the launcher can discover the module and negotiate its ABI version, and the rest of the interface is designed when its first implementation is written, which is [v0.1](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/roadmap/v0-1/).

The `manifest.json` targets ABI 0.2.1, matching `liara-core` and `liara-renderer`. It moves when they move.

## Where to go next

The [API reference](https://liara-engine.liara-engine-documentation.workers.dev/liara-platform/latest/api/) is generated from the headers in `liara-interfaces`, which is where this module's contract lives. The internal C++ behind it is private and deliberately undocumented here.
