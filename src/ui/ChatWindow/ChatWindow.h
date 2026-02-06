#pragma once

#include <QtDebug>
#include <QDialog>
#include <QSplitter>
#include <QMenu>
#include <QAction>
#include <QListWidgetItem>
#include <QScrollBar>
#include <QMenu>
#include <QAction>
#include <QMap>

#include "MessageWindow.h"
#include "ContentModel.h"
#include "ContentDelegate.h"

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QDialog {
    Q_OBJECT

public:
    explicit ChatWindow(QWidget *parent = nullptr);
    ~ChatWindow();

private:
    // 显示设置菜单
    void on_show_settings_menu();
    // 显示 创建/添加 好友/群组 菜单
    void on_show_add_menu();

    // // 聊天室管理
    // 添加到聊天室列表
    void add_chat_room(Content content);
    // 点击聊天室中的一个对话
    void on_chat_room_item_clicked(QListWidgetItem *item);
    // 显示指定聊天室的对话
    void show_chat_room_messages(Content content);

    // // 联系人管理
    // 添加到联系人列表
    void add_content(Content content);
    // 点击联系人列表中的一个联系人
    void on_content_item_clicked(QListWidgetItem *item);
    // 显示指定联系人的信息
    void show_content_info(Content content);

signals:
    void signout();

private:
    Ui::ChatWindow *ui;
    QSplitter      *m_split_content   = nullptr;
    QMenu          *m_menu_settings   = nullptr;
    QAction        *m_action_signout  = nullptr;
    QAction        *m_action_settings = nullptr;
    QMenu          *m_menu_add        = nullptr;
    QAction        *m_action_add      = nullptr;
    QAction        *m_action_create   = nullptr;

    QMap<Content, MessageWindow*> m_message_windows;  // 聊天室 ID 与对应窗口的绑定
    UserMessages  m_user_messages;
    // model/view/delegate
    ContentModel    *m_content_model    = nullptr;
    ContentDelegate *m_content_delegate = nullptr;
};
