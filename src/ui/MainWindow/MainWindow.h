#pragma once

#include "Headers.h"
#include "data.h"
#include "Utils.h"
#include "AuthWindow.h"
#include "ChatWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;  // 用于调试

private:
    void on_init();                // 延迟初始化，用于在 qt 的消息循环 app.exec() 中触发
    void on_show_widget_change();  // 切换显示页面时触发，用于自动修改窗口大小及标题
    bool check_login();            // 登陆检查
    bool show_window(QWidget*widget);
    bool show_auth_window();  // 显示登陆窗口
    bool show_chat_window();  // 显示主窗口

private:
    Ui::MainWindow *ui            = nullptr;
    AuthWindow     *m_auth_window = nullptr;
    ChatWindow     *m_chat_window = nullptr;
};
