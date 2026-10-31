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

**Timing.** A monotonic clock in nanoseconds, a separate wall clock, and a sleep to a deadline, so the host's notion of time does not depend on which standard library it happened to be built against.

## What it does not

No ECS, no rendering, no audio device. Audio talks to the operating system too, and the boundary here is the concern rather than the fact of being OS-specific.

No file I/O beyond resolving standard paths, since reading files is `liara-assets`. No logical input mapping.

## What exists today

The create and destroy lifecycle, the shutdown surface, the clock, and windows with their events. A window is opened through SDL3, reports its size in pixels, describes itself to a renderer through the tagged `liara_native_window_t`, and produces close and resize events that `liara_platform_poll_event` returns one at a time. A close request on the last open window of an instance makes `liara_platform_quit_requested` report a stop, as SIGINT does. Input devices are designed and not yet written.

`src/platform.cpp` is the ABI shim and holds nothing else. Behind it, `Instance`, `Window`, `VideoSubsystem`, `Events`, `Shutdown` and `Time` each hold one concern. The `manifest.json` `dev` entry tracks the unreleased ABI, as every module does until ABI 1.0.0 is tagged.

## Where to go next

The [API reference](https://liara-engine.liara-engine-documentation.workers.dev/liara-platform/latest/api/) is generated from the headers in `liara-interfaces`, which is where this module's contract lives. The internal C++ behind it is private and deliberately undocumented here.
