#pragma once

// A minimal Qt Designer custom-widget plugin, wrapping a small real
// CustomWidget so the plugin has something genuine to hand back from
// createWidget() rather than a placeholder.
//
// NOTE: the original version of this file declared every
// QDesignerCustomWidgetInterface override but only ever defined domXml()
// -- the other nine (isContainer, isInitialized, icon, group, includeFile,
// name, toolTip, whatsThis, createWidget, initialize) were left as bare
// declarations with no implementation anywhere, and createWidget() would
// have needed to return a `new CustomWidget(parent)` for a class
// (CustomWidget) that didn't exist anywhere in this repository. An abstract
// class (which this was, with nine unimplemented pure virtuals inherited
// from QDesignerCustomWidgetInterface) can never actually be instantiated,
// so this plugin could never have worked even once fully compiled. Fixed by
// implementing every method and adding a real (if simple) CustomWidget.
#include <QtUiPlugin/QDesignerCustomWidgetInterface>
#include <QIcon>
#include <QLabel>
#include <QObject>
#include <QString>
#include <QWidget>

// The actual widget this plugin exposes to Qt Designer: a small colored
// panel with a centered caption label.
class CustomWidget : public QWidget {
  Q_OBJECT

 public:
  explicit CustomWidget(QWidget* parent = nullptr);

  QString caption() const;
  void setCaption(const QString& text);

 private:
  QLabel* m_label;
};

class CustomWidgetPlugin : public QObject,
                           public QDesignerCustomWidgetInterface {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface")
  Q_INTERFACES(QDesignerCustomWidgetInterface)

 public:
  explicit CustomWidgetPlugin(QObject* parent = nullptr);

  bool isContainer() const override;
  bool isInitialized() const override;
  QIcon icon() const override;
  QString domXml() const override;
  QString group() const override;
  QString includeFile() const override;
  QString name() const override;
  QString toolTip() const override;
  QString whatsThis() const override;
  QWidget* createWidget(QWidget* parent) override;
  void initialize(QDesignerFormEditorInterface* core) override;

 private:
  bool m_initialized;
};
