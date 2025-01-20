#ifndef GRIDRENDERER_H
#define GRIDRENDERER_H

#include "renderer.h"

#include <QOpenGLBuffer>

class QObject;

class GridRenderer : public Renderer
{
    Q_OBJECT
public:
    explicit GridRenderer(QObject *parent = nullptr);

    void initializeGL() override;
    void paintGL(const QMatrix4x4 &VP) override;
signals:

private:
    QOpenGLBuffer m_vbo;
    int m_count = 0;
};

#endif // GRIDRENDERER_H
