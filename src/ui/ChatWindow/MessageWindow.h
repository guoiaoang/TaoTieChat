#pragma once

#include <QWidget>
#include <QSplitter>
#include "ItemMessage.h"

namespace Ui {
class MessageWindow;
}

class MessageWindow : public QWidget {
    Q_OBJECT

public:
    explicit MessageWindow(QWidget *parent = nullptr);
    explicit MessageWindow(
        const ChatroomMessages&chatroom_messages,
        QWidget *parent = nullptr);
    ~MessageWindow();

// private:
    void update_info(const ChatroomInfo&chatroom_info);
    void update_data(const ChatroomMessages &chatroom_messages);
    void update_ui();
    void append_message(const ItemMessageData&item_message_data);

private:
    Ui::MessageWindow *ui;
    QSplitter         *m_splitter = nullptr;
    ChatroomInfo       m_chatroom_info;
    ChatroomMessages   m_chatroom_messages;
};
