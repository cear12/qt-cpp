#pragma once

// A minimal modern-OpenGL (shader + VBO) QOpenGLWidget: a rotating colored
// triangle, draggable with the mouse to change the rotation axis/speed.
//
// NOTE: the original version of this file declared the class, then defined
// initializeGL() inline right after it -- and the function body (and the
// file) simply stopped mid-statement with no closing brace, no
// vertexShaderSource/fragmentShaderSource that it referenced, and no
// definitions at all for paintGL(), resizeGL(), the mouse handlers, or
// setupVertexAttribs(). It could not have compiled in any state this repo
// has ever been in. This is a complete, from-scratch (but deliberately
// simple) rewrite -- see custom-opengl-widget.cpp -- and, per the
// repository README, has not been compiled locally since this sandbox has
// no OpenGL/Qt installation; CI is the first real compiler it will see.
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QPoint>
#include <QVector3D>

class CustomOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit CustomOpenGLWidget(QWidget* parent = nullptr);
    ~CustomOpenGLWidget() override;

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    void setupVertexAttribs();

    QOpenGLShaderProgram* m_program = nullptr;
    QOpenGLBuffer m_vbo{QOpenGLBuffer::VertexBuffer};
    QMatrix4x4 m_projection;
    QMatrix4x4 m_view;
    QMatrix4x4 m_model;

    QPoint m_lastPanPoint;
    QVector3D m_rotationAxis{0.0f, 1.0f, 0.0f};
    qreal m_angularSpeed = 0.0;
    qreal m_rotationAngle = 0.0;
};
