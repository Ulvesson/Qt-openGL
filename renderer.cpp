#include "renderer.h"

Renderer::Renderer(QObject *parent)
    : QObject{parent}
{}

void Renderer::compileShaders(const std::string &vertSrc, const std::string &fragSrc)
{
    m_program = new QOpenGLShaderProgram(this);
    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc.c_str()) ||
        !m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc.c_str())) {
        throw std::runtime_error(m_program->log().toStdString());
    }

    if (!m_program->link()) {
        throw std::runtime_error(m_program->log().toStdString());
    }

    m_matrixLocation = m_program->uniformLocation("mVP");
    m_program->release();
}
