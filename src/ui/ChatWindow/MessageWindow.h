#pragma once

#include "headers.h"
#include "Utils.h"
#include "Defines.h"

namespace Ui {
class MessageWindow;
}

// 单个聊天对话框
class MessageWindow : public QWidget {
    Q_OBJECT

public:
    explicit MessageWindow(QWidget *parent = nullptr);
    // explicit MessageWindow(const std::shared_ptr<ContactInfo>&contact_info, Messages messages, QWidget *parent = nullptr);
    ~MessageWindow();

// private:
    void update_contacts(const std::shared_ptr<ContactInfo>&contact_info);
    // void update_messages(Messages messages);
    // void append_message (Message  message );
    // void update_ui();

private:
    std::shared_ptr<Ui::MessageWindow> m_ui          ;
    std::shared_ptr<QSplitter        > m_splitter    ;
    std::shared_ptr<ContactInfo      > m_contact_info;
    // Messages           m_messages;
};
