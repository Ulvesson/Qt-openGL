#include "rendertoy.h"

#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLExtraFunctions>
#include <QWheelEvent>
#include <QScreen>

namespace {

constexpr float halfW = 1.0f / 2.0f;

static GLfloat const vertices[] = {
    0.0f, 0.0f, 0, 1,
    1.0f, 0.0f, 0, 1,
    1.0f, 1.0f, 0, 1
};

static GLfloat const instanceData[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    2.0f, 0.0f,
    3.0f, 0.0f,
    4.0f, 0.0f,
    0.0f, 2.0f,
    1.0f, 2.0f,
    2.0f, 2.0f,
    3.0f, 2.0f,
    4.0f, 2.0f,
};

std::string vertexSource = R"(
#version 430
layout(location=0) in vec4 vertex;
layout(location=1) in vec2 pos;
uniform highp mat4 mVP;

void main(void)
{
    vec4 p = vec4(vertex.xy + pos, 0, 1);
    gl_Position = mVP * p;
}
)";

std::string fragmentSource = R"(
uniform mediump vec4 color;
void main(void)
{
    gl_FragColor = color;
}
)";

void MessageCallback( GLenum source,
                GLenum type,
                GLuint id,
                GLenum severity,
                GLsizei length,
                const GLchar* message,
                const void* userParam )
{
    fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}
}

RenderToy::RenderToy(QWidget* parent)
    : QOpenGLWidget(parent)
    , m_grid(new GridRenderer(this))
{
    setMouseTracking(true);
    auto sc = screen();
    qDebug() << "PixelRatio:" << sc->devicePixelRatio();
    qDebug() << "DPY x:" << sc->logicalDotsPerInchX();
    qDebug() << "DPY x:" << sc->logicalDotsPerInchY();
}

RenderToy::~RenderToy()
{
    m_vbo.destroy();
    m_vao.destroy();
}

QSize RenderToy::sizeHint() const
{
    return QSize(800,800);
}

void RenderToy::initializeGL()
{
    QOpenGLExtraFunctions::initializeOpenGLFunctions();
    glEnable              ( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( MessageCallback, 0 );
    glClearColor(.0f, .0f, .0f, 1.f);
    glDisable(GL_CULL_FACE);
}

void RenderToy::resizeGL(int width, int height)
{
    updateProjection(width, height);
}

void RenderToy::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    if (!m_initialized) {
        initTriangle();
        m_initialized = true;
    }

    m_grid->paintGL(m_proj * m_view);

    drawTriangle();
}

void RenderToy::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() > 0) {
        m_scale *= 2.0f;
    }
    else {
        m_scale *= 0.5f;
    }

    if (m_scale < 0.1f) {
        m_scale = 0.1;
    }
    update();
}

void RenderToy::mouseMoveEvent(QMouseEvent *event)
{
    auto clip = dcToClipspace(event->pos());
    QMatrix4x4 vp = m_proj * m_view;
    auto m = vp.inverted();
    auto p = m * QVector4D(clip, 0, 1);
    qDebug() << p.x() << ", " << p.y();
}

void RenderToy::initTriangle()
{
    m_vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_vbo.create();
    m_vbo.bind();
    int size = sizeof(vertices) * sizeof(GLfloat);
    m_vbo.allocate(vertices, size);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    m_vbo.release();

    m_vboPos.create();
    m_vboPos.bind();
    m_vboPos.allocate(instanceData, sizeof(instanceData));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribDivisor(1,1);
    m_vboPos.release();

    m_program = new QOpenGLShaderProgram(this);
    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexSource.c_str()) ||
        !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSource.c_str())) {
        throw std::runtime_error(m_program->log().toStdString());
    }

    if (!m_program->link()) {
        throw std::runtime_error(m_program->log().toStdString());
    }

    m_matrixLocation = m_program->uniformLocation("mVP");
    m_colorLocation = m_program->uniformLocation("color");
    m_program->release();
}

void RenderToy::drawTriangle()
{
    QMatrix4x4 vp = m_proj * m_view;
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_program->bind();
    m_program->setUniformValue(m_matrixLocation, vp);
    m_program->setUniformValue(m_colorLocation, QColor(0, 255, 0, 255));

    glDrawArraysInstanced(GL_TRIANGLES, 0, 4, sizeof(vertices) / 4);
    if (glGetError() != GL_NO_ERROR) {
        throw std::runtime_error("GL error");
    }
}

QVector2D RenderToy::dcToClipspace(const QPoint &pos)
{
    float x = pos.x() * (1.0f / width()) * 2 - 1.0f;
    float y = (height() - pos.y()) * (1.0f / height()) * 2 - 1.0f;
    return QVector2D(x, y);
}

void RenderToy::updateProjection(int width, int height)
{
    constexpr float size = 10.0;
    float aspect = static_cast<float>(width) / static_cast<float>(height);

    m_proj.setToIdentity();
    m_view.setToIdentity();

    if (aspect >= 1.0) {
        m_proj.ortho(0, size, 0, size / aspect, 0.0f, 1.0f);
    }
    else {
        m_proj.ortho(0, size * aspect, 0, size, 1, -1);
    }
}
