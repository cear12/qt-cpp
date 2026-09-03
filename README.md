# qt-cpp

A collection of Qt6/C++20 patterns and idioms: Strategy, Composite, Proxy,
Factory, Observer (via native signals/slots), Command (via QUndoStack),
Model/View/Delegate, a state machine, cross-thread widget creation, a
minimal OpenGL widget, a Qt Designer plugin, QML type registration, and a
small threaded TCP server. Each header demonstrates one pattern in isolation
rather than a single application.

## Local verification status -- please read this first

**None of this has been compiled locally.** It was cleaned up in a sandbox
with no Qt installation and no root access to add one. Every fix described
below was made by careful reading, not by a compiler catching the mistake,
so **CI (`.github/workflows/ci.yml`) is the first real compiler this code
will see.** Please check its result before relying on any of this, and
treat `src/client_server.cpp` and `include/qtcpp/proxy-for-service-layer.h`
as the two highest-priority files to review first -- they have the most
substantive logic changes (see "Bugs found and fixed" below).

## Layout

| Path | Contents |
|---|---|
| `include/qtcpp/*.h` | One pattern per header, mostly self-contained (inline-implemented). |
| `src/*.cpp` | Implementations for the headers that declare methods without defining them inline: `client_server`, `custom-delegate`, `custom-opengl-widget`, `custom-plugin-widget`, `qml-cpp-custom-type-integration`. |
| `examples/console_demo.cpp` | Runs every header that doesn't need a display (Factory, Undo/Redo, State Machine, Observer, metaprogramming property access, async processing, the QML-exposed `BackEnd` type, network classes construction-only). |
| `examples/widgets_demo.cpp` | Runs every header that needs `QApplication` (Strategy, Composite, Model/View/Delegate, the star-rating delegate, the Designer plugin, the OpenGL widget, the animated graphics item, cross-thread widget creation). Needs a display or `-platform offscreen`. |
| `examples/client_server_demo.cpp` | A real loopback test of `client_server.h`: starts the server, connects a plain `QTcpSocket`, and checks the computed result comes back over the wire. |

## Bugs found and fixed

Several of these files could not have compiled, or could not have worked
correctly even if they did, in the state this repo was in. Fixed here:

- **`client_server.h`/`.cpp`** (the most substantial fix): `MyServer` never
  overrode `incomingConnection()`, so accepted TCP connections were never
  turned into `MyClient` objects at all -- the server would `listen()`
  successfully and then silently do nothing with anyone who connected.
  `MyClient`/`MyServer` had no `Q_OBJECT` and used the old string-based
  `SIGNAL()`/`SLOT()` `connect()` overload for methods that weren't even
  declared under a `slots:` section, so those connections would fail at
  runtime even once compiling. One `connect()` wired a signal
  (`Result()`) that doesn't exist on `QTcpSocket` -- a copy/paste leftover.
  `MyTask` (a `QRunnable`) declared a `Q_SIGNALS` section, but `QRunnable`
  is not a `QObject` and cannot have signals at all. And `MyTask::run()`
  computed a result and then never emitted it (the `emit` line was
  commented out), so the one thing the whole example exists to demonstrate
  -- getting a result from a background thread back to the client -- never
  actually happened. All fixed; `examples/client_server_demo.cpp` is a real
  end-to-end test of the fixed chain.
- **`custom-opengl-widget.h`**: the file was truncated mid-function --
  `initializeGL()` started and never closed, and `paintGL()`, `resizeGL()`,
  the mouse handlers, `setupVertexAttribs()`, and the shader source strings
  it referenced didn't exist anywhere. Completely rewritten as a real
  (simple) rotating-triangle shader/VBO widget.
- **`custom-plugin-widget.h`**: nine of the ten `QDesignerCustomWidgetInterface`
  overrides (everything except `domXml()`) were declared but never defined,
  and `createWidget()` was meant to return a `CustomWidget` that didn't
  exist anywhere in the repo. An abstract class with unimplemented pure
  virtuals can never be instantiated, so this plugin could never have
  worked. Fixed by implementing every method and adding a small real
  `CustomWidget`.
- **`custom-delegate.h`**, **`multi-threading-widget-creation.h`**,
  **`qml-cpp-custom-type-integration.h`**: each ended with real usage code
  (`view->setItemDelegate(...)`, `QObject::connect(...)`,
  `qmlRegisterType<BackEnd>(...)`) sitting as bare statements directly in
  the header, outside any function -- not legal at namespace scope in C++,
  so none of these headers could compile if included anywhere. That usage
  now lives in the `examples/` demos, where it belongs (and actually runs).
  `multi-threading-widget-creation.h` also called a
  `performHeavyCalculation()` that was never declared anywhere; it now has
  a real (stand-in) implementation.
- **`animated-graphic-scene.h`**: animated a property named `"pos"` via
  `QPropertyAnimation` on a class that inherited `QObject` + `QGraphicsItem`
  directly. Plain `QGraphicsItem` has no such `Q_PROPERTY` (it deliberately
  isn't a `QObject`), so the animation would silently do nothing at
  runtime. Fixed by inheriting `QGraphicsObject` instead -- Qt's
  purpose-built combination of the two with real, animatable
  `pos`/`x`/`y`/etc. properties.
- **`state-machine-controller.h`**: the constructor took a `QWidget* parent`
  but never passed it to `QObject()`, so the parent argument was silently
  discarded regardless of what callers passed in. Fixed by taking (and
  forwarding) a `QObject*`, which is also the more accurate type.
- Every header was missing at least some of its `#include`s (several were
  missing nearly all of them) and had no include guard / `#pragma once` --
  they read like excerpts from a larger, already-fully-included codebase
  rather than standalone files. All fixed; every header in this repo is now
  independently includable.

## Honesty notes

- **`proxy-for-service-layer.h`**'s `QPromise` usage was adjusted
  defensively: rather than moving a `QPromise` into a lambda capture (the
  original approach, which may or may not be valid depending on the exact
  Qt6 minor version -- `QPromise`'s move/copy guarantees have shifted
  across Qt6 releases), results are reported through a `std::shared_ptr<QPromise<T>>`
  captured by value, which sidesteps that question entirely. Please
  double-check this file specifically once CI can compile it.
- **Network code is not exercised against a real server.** `AdvancedNetworkManager`
  and `NetworkDataService`/`CachedDataService` talk to `https://api.example.com/...`,
  which doesn't exist; `examples/console_demo.cpp` only constructs these
  classes, it never calls `makeRequest`/`fetchData` for real, to avoid a
  demo that depends on network access being available (and to avoid a
  30-second timeout on every CI run).
- **The OpenGL widget is constructed but never shown** in
  `examples/widgets_demo.cpp`, since a headless/offscreen platform may have
  no real GL context available, and `initializeGL()`/`paintGL()` only run
  once a widget is actually shown/painted. This proves linking and
  construction, not rendering.
- **The Feistel-style "toy round function" caveat doesn't apply here** (that's
  `cryptographic-techniques-cpp`), but in the same spirit: `BlurFilter`/`SepiaFilter`
  in `algorithm-strategy.h` are illustrative composition-mode tricks, not a
  real blur/sepia convolution kernel.

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/examples/console_demo
./build/examples/widgets_demo         # needs a display, or -platform offscreen
./build/examples/client_server_demo
```

Requires Qt6 (Core, Widgets, Gui, Network, Concurrent, OpenGL,
OpenGLWidgets, StateMachine, Qml, UiPlugin) and a C++20 compiler. No Catch2
test suite in this repo -- the three demos above, each printing PASS/FAIL
per check, are what CI runs as the verification step.

## Building in Visual Studio

With three executable targets (`console_demo`, `widgets_demo`,
`client_server_demo`) and no CMakePresets.json, Visual Studio's Open
Folder / CMake integration has no default startup item configured.
Pressing **Debug/Run** (not Build) then pops a blocking "Select Startup
Item" dialog -- easy to mistake for the project failing to build, even
though **Build > Build All** (Ctrl+Shift+B) succeeds regardless of
what's selected there (assuming Qt6 is discoverable -- see the local
verification note above). `CMakePresets.json` sets
`CMAKE_VS_STARTUP_PROJECT` to `console_demo`, the one demo that needs no
display, so Debug/Run works immediately too; pick `widgets_demo` or
`client_server_demo` from the dropdown next to the Run button to debug
either of the other two.
