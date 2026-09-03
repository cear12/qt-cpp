// console_demo.cpp - Exercises every header in this repo that doesn't need
// a display: Factory, Undo/Redo, State Machine, Signals/Slots+Observer,
// metaprogramming property access, the QML-exposed BackEnd type, and
// (construction only, no real network I/O) the network-facing classes.
// Uses QCoreApplication, so it also runs under CI with no display server.
#include <QCoreApplication>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>
#include <cmath>
#include <iostream>

#include "qtcpp/advanced-network-manager.h"
#include "qtcpp/async-processor.h"
#include "qtcpp/metaprogramming.h"
#include "qtcpp/proxy-for-service-layer.h"
#include "qtcpp/qml-cpp-custom-type-integration.h"
#include "qtcpp/signals-slots-observer.h"
#include "qtcpp/state-machine-controller.h"
#include "qtcpp/template-magic-factory.h"
#include "qtcpp/undo-redo-command.h"

namespace {

void runFactoryDemo() {
  std::cout << "\n=== Factory (template-magic-factory.h) ===\n";
  ObjectFactory<Shape> factory;
  factory.registerType<Circle>("Circle", 5.0);
  factory.registerType<Rectangle>("Rectangle", 10.0, 20.0);

  auto circle = factory.create("Circle");
  auto rect = factory.create("Rectangle");
  auto missing = factory.create("Triangle");

  std::cout << "  Circle area:    " << (circle ? circle->area() : -1.0)
            << " (expected ~78.54)\n";
  std::cout << "  Rectangle area: " << (rect ? rect->area() : -1.0)
            << " (expected 200)\n";
  std::cout << "  Unregistered type returns null: "
            << (missing == nullptr ? "yes" : "NO (bug)") << "\n";

  bool circleOk = circle && std::abs(circle->area() - 78.53981633974483) < 1e-9;
  bool rectOk = rect && std::abs(rect->area() - 200.0) < 1e-9;
  std::cout << "  [" << ((circleOk && rectOk && !missing) ? "PASS" : "FAIL")
            << "] factory results correct\n";
}

void runUndoRedoDemo() {
  std::cout << "\n=== Undo/Redo (undo-redo-command.h) ===\n";
  Document doc;
  QUndoStack undoStack;

  undoStack.push(new InsertTextCommand(&doc, "Hello "));
  undoStack.push(new InsertTextCommand(&doc, "World!"));
  std::cout << "  After two inserts: \"" << doc.content().toStdString()
            << "\"\n";

  undoStack.undo();
  std::cout << "  After one undo:    \"" << doc.content().toStdString()
            << "\"\n";

  undoStack.push(new ReplaceTextCommand(&doc, "Replaced"));
  std::cout << "  After replace:     \"" << doc.content().toStdString()
            << "\"\n";

  undoStack.undo();
  undoStack.undo();
  std::cout << "  After two more undos: \"" << doc.content().toStdString()
            << "\" (expected empty)\n";

  bool ok = doc.content().isEmpty();
  undoStack.redo();
  undoStack.redo();
  undoStack.redo();
  ok = ok && doc.content() == QStringLiteral("Replaced");
  std::cout << "  [" << (ok ? "PASS" : "FAIL")
            << "] undo/redo round-trip correct\n";
}

void runPropertyWrapperDemo() {
  std::cout
      << "\n=== Metaprogramming property access (metaprogramming.h) ===\n";
  DataSource source;
  PropertyWrapper<DataSource> wrapper(&source);

  // Dynamic Qt property (not a declared Q_PROPERTY -- QObject supports
  // both, and PropertyWrapper's setProperty/property work on either).
  wrapper.setProperty("label", QString("demo-source"));
  QString label = wrapper.property<QString>("label");
  std::cout << "  Dynamic property round-trip: \"" << label.toStdString()
            << "\"\n";
  std::cout << "  ["
            << (label == QStringLiteral("demo-source") ? "PASS" : "FAIL")
            << "] property get/set correct\n";

  bool signalFired = false;
  wrapper.connectToSignal([&signalFired]() { signalFired = true; });
  emit source.dataChanged(
      1);  // manually fire, rather than waiting a full second for the timer
  std::cout << "  [" << (signalFired ? "PASS" : "FAIL")
            << "] connectToSignal wiring fired\n";
}

void runAsyncProcessorDemo() {
  std::cout << "\n=== Async processing (async-processor.h) ===\n";

  AsyncProcessor processor;
  bool completed = false;
  QString resultText;

  QObject::connect(&processor, &AsyncProcessor::operationCompleted,
                   [&](const QString& result) {
                     completed = true;
                     resultText = result;
                   });

  processor.watchFuture(
      processor.processDataAsync(QByteArray("some demo payload bytes")));

  QEventLoop loop;
  QObject::connect(&processor, &AsyncProcessor::operationCompleted, &loop,
                   &QEventLoop::quit);
  QTimer::singleShot(4000, &loop, &QEventLoop::quit);  // safety timeout
  loop.exec();

  std::cout << "  result: " << resultText.toStdString() << "\n";
  std::cout << "  [" << (completed ? "PASS" : "FAIL")
            << "] async operation completed and was watched correctly\n";
}

void runNetworkConstructionCheck() {
  std::cout << "\n=== Network classes (construction only, no real I/O -- see "
               "README) ===\n";
  AdvancedNetworkManager networkManager;
  NetworkDataService realService;
  Q_UNUSED(networkManager)
  std::cout << "  AdvancedNetworkManager, NetworkDataService constructed OK "
               "(not exercised: no live server)\n";

  // CachedDataService takes ownership of an IDataService -- prove the
  // Proxy pattern's wiring (unique_ptr<IDataService> -> concrete
  // NetworkDataService) at least type-checks and constructs.
  auto owned = std::make_unique<NetworkDataService>();
  CachedDataService cached(std::move(owned));
  Q_UNUSED(cached)
  std::cout << "  CachedDataService wraps a real IDataService instance OK\n";
}

void runBackEndDemo() {
  std::cout
      << "\n=== QML-exposed type (qml-cpp-custom-type-integration.h) ===\n";
  BackEnd backend;
  backend.setUserName("ole");
  backend.processData("  hello from qml  ");
  backend.processData("second item");

  std::cout << "  userName: " << backend.userName().toStdString() << "\n";
  std::cout << "  dataList[0]: " << backend.getData(0).toString().toStdString()
            << "\n";
  std::cout << "  dataList[1]: " << backend.getData(1).toString().toStdString()
            << "\n";
  std::cout << "  out-of-range getData(5) is invalid: "
            << (!backend.getData(5).isValid() ? "yes" : "NO (bug)") << "\n";

  registerQmlTypes();  // the line that used to be a bare, non-compiling
                       // statement in the header
  std::cout << "  registerQmlTypes() called OK\n";
}

}  // namespace

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);

  std::cout << "qt-cpp console demo (headless-safe subset)\n";

  runFactoryDemo();
  runUndoRedoDemo();
  runPropertyWrapperDemo();
  runAsyncProcessorDemo();
  runNetworkConstructionCheck();
  runBackEndDemo();

  std::cout << "\n=== State machine + Observer (state-machine-controller.h, "
               "signals-slots-observer.h) ===\n";
  std::cout << "  (runs on the event loop for a few seconds)\n";

  StateMachineController controller;
  QObject::connect(&controller, &StateMachineController::statusChanged,
                   [](const QString& status) {
                     std::cout << "  [state machine] " << status.toStdString()
                               << "\n";
                   });
  QTimer::singleShot(200, &controller, &StateMachineController::triggerNext);
  QTimer::singleShot(400, &controller, &StateMachineController::triggerNext);
  QTimer::singleShot(600, &controller, &StateMachineController::triggerReset);

  DataSource source;
  Observer observer1("Observer1");
  Observer observer2("Observer2");
  QObject::connect(&source, &DataSource::dataChanged, &observer1,
                   &Observer::onDataChanged);
  QObject::connect(&source, &DataSource::milestone, &observer2,
                   &Observer::onMilestone);

  QTimer::singleShot(3500, &app, &QCoreApplication::quit);
  return app.exec();
}
