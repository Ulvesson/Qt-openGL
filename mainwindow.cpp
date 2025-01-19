#include "mainwindow.h"
#include "rendertoy.h"

#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setCentralWidget(new RenderToy(this));
    setStatusBar(new QStatusBar(this));
}

MainWindow::~MainWindow() {}
