---
title: About Liara Platform
description: Everything the Liara Engine needs from the operating system, behind one interface.
sidebar:
    order: 0
---

# liara-platform

> Everything the Liara Engine needs from the operating system, behind one interface.

Window management, input devices, OS signals and timing. Confining all of it here is what makes "Linux and Windows are both first-class" a property of one repository instead of an `#ifdef` scattered through all of them.

It is replaceable on purpose. A platform module built on GLFW, on winit, or directly on Wayland and Win32 is a legitimate substitute, and keeping the windowing backend behind the C interface is precisely what keeps that true. Its only dependency today is [liara-interfaces](https://github.com/liara-engine/liara-interfaces).

## Status

**Skeleton.** The module reports itself through `liara_platform_info()` and does nothing else: there is no window, no event pump, no clock, and no SDL3 dependency yet. That is enough for the launcher to discover it and negotiate its ABI version, which is the one thing worth proving before the real work starts.

The window, the input devices, the poll-only shutdown flag and the monotonic clock arrive with [v0.1](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/roadmap/v0-1/), and their entry points are designed then rather than now — an interface designed ahead of its first implementation is the mistake [Adding and removing a module](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/modules/lifecycle/) warns about.

## Documentation

- **[Module guide](https://liara-engine.liara-engine-documentation.workers.dev/liara-platform/latest/guides/)**: what this module is for and what it will not contain.
- **[API reference](https://liara-engine.liara-engine-documentation.workers.dev/liara-platform/latest/api/)**: generated from the headers.
- **Architecture**: the meta repository's [architecture](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/architecture/) and [modules](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/modules/platform/) pages.
- **Workflow**: [contributing](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/guides/contributing/).

## Building it on its own

Usually you do not. This repository is consumed through the workspace superbuild, or through `find_package(LiaraPlatform)` from an install.

Two things make a bare clone awkward, and both are deliberate. It has no `CMakePresets.json`, because presets are generated into the workspace from a template in the meta repository. And it fails to configure without `Liara::Interfaces`, with a message saying so.

To build it alone against an installed contract:

```bash
git clone https://github.com/liara-engine/liara-platform.git
cd liara-platform

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_PREFIX_PATH=/path/to/liara-interfaces/install
cmake --build build
ctest --test-dir build --output-on-failure
```

From v0.1 this also needs SDL3, which vcpkg resolves through the workspace manifest.

## Consuming it

```cmake
find_package(LiaraPlatform REQUIRED)
target_link_libraries(my_module PRIVATE Liara::Platform)
```

Always through the `Liara::Platform` alias rather than the bare target name, so that a build compiles against exactly what an external consumer gets.

## License

[MIT](https://liara-engine.liara-engine-documentation.workers.dev/liara/latest/about/license/).
