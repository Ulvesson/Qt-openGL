#include "gridrenderer.h"

namespace {
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
void main(void)
{
    gl_FragColor = vec4(1);
}
)";

} // namespace

GridRenderer::GridRenderer(QObject *parent) : Renderer{parent} {}

void GridRenderer::initializeGL() {
    QOpenGLExtraFunctions::initializeOpenGLFunctions();
    compileShaders(vertexSource, fragmentSource);

    m_vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    std::vector<float> vertices;
    const float gridSize = 1.0;
    const float noCells  = 10.0;

    for (float n = 0; n < noCells; n += gridSize) {
        // Vertical
        vertices.push_back(n);
        vertices.push_back(0);
        vertices.push_back(0);
        vertices.push_back(1);

        vertices.push_back(n);
        vertices.push_back(noCells);
        vertices.push_back(0);
        vertices.push_back(1);

        // Horizontal
        vertices.push_back(0);
        vertices.push_back(n);
        vertices.push_back(0);
        vertices.push_back(1);

        vertices.push_back(noCells);
        vertices.push_back(n);
        vertices.push_back(0);
        vertices.push_back(1);
    }

    m_count = vertices.size() / 2;
    int size = vertices.size() * sizeof(float);

    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(&vertices[0], size);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    m_vbo.release();
}

void GridRenderer::paintGL(const QMatrix4x4 &VP) {
    if (!m_initialized) {
        initializeGL();
        m_initialized = true;
    }
    m_program->bind();
    m_program->setUniformValue(m_matrixLocation, VP);

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    glDrawArrays(GL_LINES, 0, m_count);
    m_program->release();
}
