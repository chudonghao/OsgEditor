//
// Created by chudonghao on 2020/3/20.
//

#ifndef OSGEDITOR_SRC_UI_SHADERWINDOW_H
#define OSGEDITOR_SRC_UI_SHADERWINDOW_H

#include <QMainWindow>

namespace Ui
{
class ShaderWindow;
} // namespace Ui

class CShaderWindow : public QMainWindow
{
Q_OBJECT;
public:
    explicit CShaderWindow(QWidget *parent);

    ~CShaderWindow() override;

private:
    using Ui = ::Ui::ShaderWindow;
    Ui *ui{};
};



#endif //OSGEDITOR_SRC_UI_SHADERWINDOW_H
