#ifndef RENDERTOY_H
#define RENDERTOY_H

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>

class QOpenGLShader;
class QOpenGLShaderProgram;

class RenderToy : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
public:
    explicit RenderToy(QWidget* parent = nullptr);
    ~RenderToy();

    QSize sizeHint() const override;

// QOpenGLWidget interface
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void wheelEvent(QWheelEvent *event) override;

private:
    void initTriangle();
    void drawTriangle();

private:
    bool m_initialized = false;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QOpenGLBuffer m_vboPos;
    QOpenGLShaderProgram* m_program;
    QMatrix4x4 m_proj;
    float m_scale = 10.0f;
    int m_matrixLocation;
    int m_colorLocation;

};

#endif // RENDERTOY_H
