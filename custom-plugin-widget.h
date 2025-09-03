class CustomWidgetPlugin : public QObject, public QDesignerCustomWidgetInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface")
    Q_INTERFACES(QDesignerCustomWidgetInterface)
    
public:
    explicit CustomWidgetPlugin(QObject *parent = nullptr);
    
    bool isContainer() const override;
    bool isInitialized() const override;
    QIcon icon() const override;
    QString domXml() const override;
    QString group() const override;
    QString includeFile() const override;
    QString name() const override;
    QString toolTip() const override;
    QString whatsThis() const override;
    QWidget *createWidget(QWidget *parent) override;
    void initialize(QDesignerFormEditorInterface *core) override;
    
private:
    bool m_initialized;
};

QString CustomWidgetPlugin::domXml() const {
    return QLatin1String("<ui language=\"c++\">\n"
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
