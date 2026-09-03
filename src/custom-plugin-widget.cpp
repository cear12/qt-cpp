#include "qtcpp/custom-plugin-widget.h"

#include <QDesignerFormEditorInterface>
#include <QVBoxLayout>

CustomWidget::CustomWidget(QWidget* parent) : QWidget(parent) {
  m_label = new QLabel(tr("Custom Widget"), this);
  m_label->setAlignment(Qt::AlignCenter);

  auto* layout = new QVBoxLayout(this);
  layout->addWidget(m_label);

  setAutoFillBackground(true);
  QPalette pal = palette();
  pal.setColor(QPalette::Window, QColor(230, 240, 255));
  setPalette(pal);
}

QString CustomWidget::caption() const { return m_label->text(); }

void CustomWidget::setCaption(const QString& text) { m_label->setText(text); }

CustomWidgetPlugin::CustomWidgetPlugin(QObject* parent)
    : QObject(parent), m_initialized(false) {}

bool CustomWidgetPlugin::isContainer() const { return false; }

bool CustomWidgetPlugin::isInitialized() const { return m_initialized; }

QIcon CustomWidgetPlugin::icon() const { return QIcon(); }

QString CustomWidgetPlugin::domXml() const {
  return QLatin1String(
      "<ui language=\"c++\">\n"
      " <widget class=\"CustomWidget\" name=\"customWidget\">\n"
      "  <property name=\"geometry\">\n"
      "   <rect>\n"
      "    <x>0</x>\n"
      "    <y>0</y>\n"
      "    <width>100</width>\n"
      "    <height>100</height>\n"
      "   </rect>\n"
      "  </property>\n"
      " </widget>\n"
      "</ui>\n");
}

QString CustomWidgetPlugin::group() const {
  return QLatin1String("My Widgets");
}

QString CustomWidgetPlugin::includeFile() const {
  return QLatin1String("qtcpp/custom-plugin-widget.h");
}

QString CustomWidgetPlugin::name() const {
  return QLatin1String("CustomWidget");
}

QString CustomWidgetPlugin::toolTip() const {
  return QLatin1String("A simple custom widget");
}

QString CustomWidgetPlugin::whatsThis() const {
  return QLatin1String(
      "A minimal QWidget subclass, wrapped as a Qt Designer plugin.");
}

QWidget* CustomWidgetPlugin::createWidget(QWidget* parent) {
  return new CustomWidget(parent);
}

void CustomWidgetPlugin::initialize(QDesignerFormEditorInterface* /*core*/) {
  if (m_initialized) return;
  m_initialized = true;
}
