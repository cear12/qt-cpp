// widgets_demo.cpp - Exercises the headers that need a real QApplication
// (widgets, item views, graphics scene, a GL widget, a background thread
// that creates a widget). Run under `-platform offscreen` in CI/headless
// environments (see .github/workflows/ci.yml) -- nothing here needs an
// actual visible display, but QApplication itself requires *some*
// platform plugin to be selected.
#include <QApplication>
#include <QEventLoop>
#include <QGraphicsScene>
#include <QModelIndex>
#include <QPixmap>
#include <QPushButton>
#include <QStandardItemModel>
#include <QStringList>
#include <QStyleOptionViewItem>
#include <QTableView>
#include <QTimer>
#include <iostream>
#include <memory>

#include "qtcpp/algorithm-strategy.h"
#include "qtcpp/animated-graphic-scene.h"
#include "qtcpp/composite-for-ui-component.h"
#include "qtcpp/custom-delegate.h"
#include "qtcpp/custom-opengl-widget.h"
#include "qtcpp/custom-plugin-widget.h"
#include "qtcpp/model-view-delegate.h"
#include "qtcpp/multi-threading-widget-creation.h"

namespace {

void runStrategyDemo() {
  std::cout
      << "\n=== Strategy pattern image filters (algorithm-strategy.h) ===\n";

  ImageProcessor processor;
  QPixmap image(64, 64);
  image.fill(Qt::white);
  processor.setImage(image);

  QStringList filters = processor.availableFilters();
  std::cout << "  Available filters: " << filters.join(", ").toStdString()
            << "\n";

  QPixmap blurred = processor.applyFilter(0);
  QPixmap custom =
      processor.applyCustomFilter([](const QPixmap& in) { return in; });

  bool ok =
      filters.size() == 2 && !blurred.isNull() && custom.size() == image.size();
  std::cout << "  [" << (ok ? "PASS" : "FAIL")
            << "] filters registered and applied without error\n";
}

void runCompositeDemo() {
  std::cout
      << "\n=== Composite UI builder (composite-for-ui-component.h) ===\n";

  bool clicked = false;
  UIBuilder builder;
  builder.addLabel("Title")
      .beginPanel(PanelComponent::Horizontal)
      .addButton("OK", [&clicked]() { clicked = true; })
      .addButton("Cancel")
      .endPanel()
      .addSpacer();

  auto root = builder.build();
  std::cout << "  Root panel child count: " << root->childCount()
            << " (expected 2: title label + button row)\n";
  std::cout << "  Root sizeHint: " << root->sizeHint().width() << "x"
            << root->sizeHint().height() << "\n";

  // Drive the "OK" button's real click handler through the actual
  // QPushButton, not by calling the std::function directly -- this
  // exercises the ButtonComponent -> QPushButton::clicked wiring for
  // real. The button lives inside the nested horizontal panel, but
  // QWidget::findChildren() recurses through the whole descendant tree,
  // so there's no need to walk PanelComponent's own tree manually.
  auto buttons = root->widget()->findChildren<QPushButton*>();
  if (!buttons.isEmpty()) {
    buttons.first()->click();
  }
  std::cout << "  [" << (clicked ? "PASS" : "FAIL")
            << "] button click handler fired\n";
}

void runModelViewDelegateDemo() {
  std::cout << "\n=== Model/View/Delegate (model-view-delegate.h) ===\n";

  PersonModel model;
  QTableView view;
  view.setModel(&model);
  view.setItemDelegate(new PersonDelegate(&view));

  std::cout << "  rowCount=" << model.rowCount()
            << " columnCount=" << model.columnCount() << "\n";
  std::cout << "  cell(0, Name)="
            << model.data(model.index(0, PersonModel::NameColumn))
                   .toString()
                   .toStdString()
            << "\n";

  bool ok =
      model.rowCount() == 3 && model.columnCount() == PersonModel::ColumnCount;
  std::cout << "  [" << (ok ? "PASS" : "FAIL")
            << "] model dimensions correct\n";
}

void runStarDelegateDemo() {
  std::cout << "\n=== Star rating delegate (custom-delegate.h) ===\n";

  // The original file's fatal bug (top-level `QTableView *view = ...;
  // view->setItemDelegate(...)` statements sitting directly in the
  // header) is fixed by putting real usage here instead.
  QStandardItemModel model(3, 1);
  model.setData(model.index(0, 0), 3);
  model.setData(model.index(1, 0), 0);
  model.setData(model.index(2, 0), 5);

  QTableView view;
  view.setModel(&model);
  auto* delegate = new StarDelegate(&view);
  view.setItemDelegate(delegate);

  std::cout
      << "  StarDelegate sizeHint: "
      << delegate->sizeHint(QStyleOptionViewItem(), QModelIndex()).width()
      << "x"
      << delegate->sizeHint(QStyleOptionViewItem(), QModelIndex()).height()
      << "\n";
  std::cout
      << "  [PASS] StarDelegate constructed and attached to a QTableView\n";
}

void runPluginWidgetDemo() {
  std::cout << "\n=== Designer plugin (custom-plugin-widget.h) ===\n";

  CustomWidgetPlugin plugin;
  plugin.initialize(nullptr);

  std::cout << "  name: " << plugin.name().toStdString() << "\n";
  std::cout << "  group: " << plugin.group().toStdString() << "\n";
  std::cout << "  isContainer: " << (plugin.isContainer() ? "true" : "false")
            << "\n";
  std::cout << "  isInitialized: "
            << (plugin.isInitialized() ? "true" : "false") << "\n";

  std::unique_ptr<QWidget> widget(plugin.createWidget(nullptr));
  auto* custom = qobject_cast<CustomWidget*>(widget.get());
  bool ok =
      custom != nullptr && custom->caption() == QStringLiteral("Custom Widget");
  std::cout << "  [" << (ok ? "PASS" : "FAIL")
            << "] createWidget() returns a real, working CustomWidget\n";
}

void runOpenGlWidgetDemo() {
  std::cout << "\n=== OpenGL widget (custom-opengl-widget.h) ===\n";
  // Constructed but not shown: under a headless/offscreen platform there
  // may be no real GL context available, so initializeGL()/paintGL()
  // (which only run once the widget is actually shown/painted) are not
  // exercised here. This proves linking and construction only -- see the
  // repository README.
  CustomOpenGLWidget widget;
  Q_UNUSED(widget)
  std::cout << "  [PASS] CustomOpenGLWidget constructed OK (not shown -- see "
               "README)\n";
}

void runAnimatedGraphicsDemo() {
  std::cout << "\n=== Animated graphics item (animated-graphic-scene.h) ===\n";

  QGraphicsScene scene;
  auto* item = new AnimatedGraphicsItem();
  scene.addItem(item);

  std::cout << "  boundingRect: " << item->boundingRect().width() << "x"
            << item->boundingRect().height() << "\n";
  std::cout << "  [PASS] item constructed and added to a QGraphicsScene\n";
}

void runWidgetCreationDemo(QApplication& app) {
  std::cout << "\n=== Cross-thread widget creation "
               "(multi-threading-widget-creation.h) ===\n";

  WidgetCreator creator;
  WorkerThread worker;
  bool created = false;

  QObject::connect(&worker, &WorkerThread::requestWidgetCreation, &creator,
                   &WidgetCreator::createWidgetSafely);
  QObject::connect(&creator, &WidgetCreator::widgetCreated, &app,
                   [&created]() { created = true; });

  QEventLoop loop;
  QObject::connect(&worker, &QThread::finished, &loop, &QEventLoop::quit);
  QTimer::singleShot(3000, &loop, &QEventLoop::quit);  // safety timeout

  worker.start();
  loop.exec();

  // Give the queued createWidgetSafely() call a moment to actually run
  // on the GUI thread after the worker signals completion.
  QEventLoop drain;
  QTimer::singleShot(50, &drain, &QEventLoop::quit);
  drain.exec();

  std::cout << "  [" << (created ? "PASS" : "FAIL")
            << "] widget created safely on the GUI thread\n";
}

}  // namespace

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  std::cout << "qt-cpp widgets demo\n";

  runStrategyDemo();
  runCompositeDemo();
  runModelViewDelegateDemo();
  runStarDelegateDemo();
  runPluginWidgetDemo();
  runOpenGlWidgetDemo();
  runAnimatedGraphicsDemo();
  runWidgetCreationDemo(app);

  return 0;
}
