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

#include "ItemChat.h"
#include "ItemContent.h"
#include "ItemMessage.h"
#include "MessageWindow.h"

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QDialog {
    Q_OBJECT

public:
    explicit ChatWindow(QWidget *parent = nullptr);
    ~ChatWindow();

private:
    void on_show_settings_menu();
    void on_show_add_menu();
    void add_chat_item   (const ItemChatData &data);
    void add_content_item(const ItemContentData &data);
    void clear_chat_list   ();
    void clear_content_list();

    // 聊天消息管理
    void on_chat_item_clicked(QListWidgetItem *item);
    void show_chat_messages(const QString &chat_id);
    void add_chat_message(const QString &chat_id, const ItemMessageData &data);
    void clear_chat_messages();

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

    QMap<QString, MessageWindow*> m_message_windows;  // 聊天室 ID 与对应窗口的绑定
    UserMessages m_user_messages;
};
