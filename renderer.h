#ifndef RENDERER_H
#define RENDERER_H

#include <QObject>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

class Renderer : public QObject, protected QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    explicit Renderer(QObject *parent = nullptr);

    virtual void paintGL(const QMatrix4x4 &VP) = 0;

protected:
    virtual void initializeGL() = 0;
    void compileShaders(const std::string &vertSrc, const std::string &fragSrc);

protected:
    bool m_initialized = false;
    QOpenGLShaderProgram *m_program = nullptr;
    QOpenGLVertexArrayObject m_vao;
    int m_matrixLocation = 0;
};

#endif // RENDERER_H
