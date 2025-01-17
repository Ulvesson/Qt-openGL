#include "rendertoy.h"

#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLExtraFunctions>
#include <QWheelEvent>

namespace {

static GLfloat const triangleVertices[] = {
    0.0f,  -1.0f,  0.0f,
    -1.f, 1.0f, 0.0f,
    1.0f,  1.0f, 0.0f
};

static GLfloat const instanceData[] = {
    0.0f, 0.0f,
    2.0f, 0.0f,
    -2.0, 0.0f,
    0.0f, 2.0f,
    2.0f, 2.0f,
    -2.0, 2.0f,
    0.0f, -2.0f,
    2.0f, -2.0f,
    -2.0, -2.0f,
};

std::string vertexSource = R"(
#version 430
layout(location=0) in vec4 vertex;
layout(location=1) in vec2 pos;
uniform highp mat4 matrix;

void main(void)
{
    vec4 p = vec4(vertex.xy + pos, 0, 1);
    gl_Position = matrix * p;
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
{
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
}

void RenderToy::resizeGL(int w, int h)
{
    float target_width = 10.f;
    float target_height = 10.f;
    float A = target_width / target_height; // target aspect ratio
    float V = static_cast<float>(w) / static_cast<float>(h);

    m_proj.setToIdentity();

    if (V >= A) {
        // wide viewport, use full height
        m_proj.ortho(-V/A * target_width/2.0f, V/A * target_width/2.0f, -target_height/2.0f, target_height/2.0f, 1, -1);
    } else {
        // tall viewport, use full width
        m_proj.ortho(-target_width/2.0f, target_width/2.0f, -A/V*target_height/2.0f, A/V*target_height/2.0f, 1, -1);
    }
}

void RenderToy::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    if (!m_initialized) {
        initTriangle();
        m_initialized = true;
    }

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

void RenderToy::initTriangle()
{
    m_vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_vbo.create();
    m_vbo.bind();
    int size = 9 * sizeof(GLfloat);
    m_vbo.allocate(triangleVertices, size);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
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

    m_matrixLocation = m_program->uniformLocation("matrix");
    m_colorLocation = m_program->uniformLocation("color");
    m_program->release();
}

void RenderToy::drawTriangle()
{
    QMatrix4x4 view;
    view.setToIdentity();
    view.translate(0, 0);
    QMatrix4x4 vp =m_proj * view;

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    m_program->bind();
    m_program->setUniformValue(m_matrixLocation, vp);
    m_program->setUniformValue(m_colorLocation, QColor(0, 255, 0, 255));

    glDrawArraysInstanced(GL_TRIANGLES, 0, 3, sizeof(instanceData));
    if (glGetError() != GL_NO_ERROR) {
        throw std::runtime_error("GL error");
    }
}
