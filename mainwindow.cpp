#include "mainwindow.h"
#include "rendertoy.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setCentralWidget(new RenderToy(this));
}

MainWindow::~MainWindow() {}
