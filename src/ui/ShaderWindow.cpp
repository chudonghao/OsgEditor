//
// Created by chudonghao on 2020/3/20.
//

#include "ShaderWindow.h"

#include "ui_ShaderWindow.h"

CShaderWindow::CShaderWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui)
{
    ui->setupUi(this);
}

CShaderWindow::~CShaderWindow()
{
    delete ui;
}

