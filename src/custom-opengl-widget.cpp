#include "qtcpp/custom-opengl-widget.h"

#include <QOpenGLShader>
#include <QTimer>

namespace {

const char* kVertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec3 position;
    layout(location = 1) in vec3 color;

    uniform mat4 mvp;

    out vec3 vertexColor;

    void main() {
        gl_Position = mvp * vec4(position, 1.0);
        vertexColor = color;
    }
)";

const char* kFragmentShaderSource = R"(
    #version 330 core
    in vec3 vertexColor;
    out vec4 fragColor;

    void main() {
        fragColor = vec4(vertexColor, 1.0);
    }
)";

// Interleaved position (x, y, z) + color (r, g, b) for a single triangle.
constexpr GLfloat kVertexData[] = {
    // clang-format off
     0.0f,  0.6f, 0.0f,   1.0f, 0.2f, 0.2f,
    -0.6f, -0.4f, 0.0f,   0.2f, 1.0f, 0.2f,
     0.6f, -0.4f, 0.0f,   0.2f, 0.2f, 1.0f,
    // clang-format on
};

constexpr int kStride = 6 * sizeof(GLfloat);

}  // namespace

CustomOpenGLWidget::CustomOpenGLWidget(QWidget* parent)
    : QOpenGLWidget(parent) {}

CustomOpenGLWidget::~CustomOpenGLWidget() {
  // Destroy GL resources while the context is still current, as Qt docs
  // for QOpenGLWidget recommend.
  makeCurrent();
  m_vbo.destroy();
  delete m_program;
  doneCurrent();
}

void CustomOpenGLWidget::initializeGL() {
  initializeOpenGLFunctions();
  glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  m_program = new QOpenGLShaderProgram;
  m_program->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                     kVertexShaderSource);
  m_program->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                     kFragmentShaderSource);
  m_program->link();

  m_vbo.create();
  m_vbo.bind();
  m_vbo.allocate(kVertexData, sizeof(kVertexData));

  setupVertexAttribs();

  m_view.setToIdentity();
  m_view.translate(0.0f, 0.0f, -3.0f);

  // ~60 FPS animation tick, independent of mouse-driven rotation.
  auto* timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this,
          QOverload<>::of(&CustomOpenGLWidget::update));
  timer->start(16);
}

void CustomOpenGLWidget::setupVertexAttribs() {
  // Called from initializeGL(), so the GL context is current and this
  // object's own (protected, inherited) QOpenGLFunctions are already
  // initialized -- no need to go through QOpenGLContext::currentContext().
  m_vbo.bind();
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, kStride,
                        reinterpret_cast<void*>(0));
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, kStride,
                        reinterpret_cast<void*>(3 * sizeof(GLfloat)));
  m_vbo.release();
}

void CustomOpenGLWidget::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  m_rotationAngle +=
      m_angularSpeed + 0.4;  // small constant spin plus any drag-imparted speed
  m_angularSpeed *= 0.95;    // gently decay drag-imparted spin, like inertia

  m_model.setToIdentity();
  m_model.rotate(static_cast<float>(m_rotationAngle), m_rotationAxis);

  m_program->bind();
  m_program->setUniformValue("mvp", m_projection * m_view * m_model);

  m_vbo.bind();
  glDrawArrays(GL_TRIANGLES, 0, 3);
  m_vbo.release();

  m_program->release();
}

void CustomOpenGLWidget::resizeGL(int width, int height) {
  m_projection.setToIdentity();
  qreal aspect = height != 0
                     ? static_cast<qreal>(width) / static_cast<qreal>(height)
                     : 1.0;
  m_projection.perspective(45.0f, static_cast<float>(aspect), 0.1f, 100.0f);
}

void CustomOpenGLWidget::mousePressEvent(QMouseEvent* event) {
  m_lastPanPoint = event->pos();
}

void CustomOpenGLWidget::mouseMoveEvent(QMouseEvent* event) {
  QPoint delta = event->pos() - m_lastPanPoint;

  if (!delta.isNull()) {
    // Drag direction sets the rotation axis (perpendicular to the drag
    // in screen space); drag distance sets how fast it spins.
    m_rotationAxis = QVector3D(delta.y(), delta.x(), 0.0f).normalized();
    m_angularSpeed += QVector3D(delta.x(), delta.y(), 0.0f).length() * 0.1;
  }

  m_lastPanPoint = event->pos();
  update();
}
