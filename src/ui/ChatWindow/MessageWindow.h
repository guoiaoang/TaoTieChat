#pragma once

#include "Headers.h"
#include "data.h"
#include "Utils.h"

namespace Ui {
class MessageWindow;
}

// 单个聊天对话框
class MessageWindow : public QWidget {
    Q_OBJECT

public:
    explicit MessageWindow(QWidget *parent = nullptr);
    explicit MessageWindow(Contact contact, Messages messages, QWidget *parent = nullptr);
    ~MessageWindow();

// private:
    void update_contacts(Contact  contact );
    void update_messages(Messages messages);
    void append_message (Message  message );
    // void update_ui();

private:
    Ui::MessageWindow *ui         = nullptr;
    QSplitter         *m_splitter = nullptr;
    Contact  m_contact ;
    Messages m_messages;
};
