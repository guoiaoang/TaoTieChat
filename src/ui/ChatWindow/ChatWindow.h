#pragma once

#include <QtDebug>
#include <QDialog>
#include <QSplitter>
#include <QMenu>
#include <QAction>
#include <QListWidgetItem>
#include <QScrollBar>
#include "ItemChat.h"

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QDialog {
    Q_OBJECT

public:
    explicit ChatWindow(QWidget *parent = nullptr);
    ~ChatWindow();

private:
    void on_show_menu();
    void add_chat_item(const ItemChatData &data);
    void clear_chat_list();

signals:
    void signout();

private:
    Ui::ChatWindow *ui;
    QSplitter      *m_split_content   = nullptr;
    QMenu          *m_menu_settings   = nullptr;
    QAction        *m_action_signout  = nullptr;
    QAction        *m_action_settings = nullptr;
};
