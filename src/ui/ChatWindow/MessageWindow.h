#pragma once

#include <QWidget>
#include <QSplitter>

#include "data.h"

namespace Ui {
class MessageWindow;
}

// 单个聊天对话框
class MessageWindow : public QWidget {
    Q_OBJECT

public:
    explicit MessageWindow(QWidget *parent = nullptr);
    explicit MessageWindow(Content content, Messages messages, QWidget *parent = nullptr);
    ~MessageWindow();

// private:
    void update_contents(Content  content );
    void update_messages(Messages messages);
    void append_message (Message  message );
    // void update_ui();

private:
    Ui::MessageWindow *ui         = nullptr;
    QSplitter         *m_splitter = nullptr;
    Content  m_content ;
    Messages m_messages;
};
