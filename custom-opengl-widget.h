class CustomOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
    
public:
    explicit CustomOpenGLWidget(QWidget *parent = nullptr);
    
protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    
private:
    void setupVertexAttribs();
    
    QOpenGLShaderProgram *m_program;
    QOpenGLBuffer m_vbo;
    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;
    QMatrix4x4 m_model;
    
    QPoint m_lastPanPoint;
    QVector3D m_rotationAxis;
    qreal m_angularSpeed;
};

void CustomOpenGLWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    
    // Создаем и компилируем шейдеры
    m_program = new QOpenGLShaderProgram;
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->link();
    
    // Настраиваем геометрию
    setupVertexAttribs();
    
    // Запускаем таймер для анимации
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&CustomOpenGLWidget::update));
    timer->start(16); // ~60 FPS
}
