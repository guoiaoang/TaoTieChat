#pragma once

#include <QWidget>

// 聊天列表项数据结构
struct ItemChatData {
    QString avatar;      // 头像路径或标识
    QString nickname;    // 昵称
    QString lastMessage; // 最后一条消息内容
    QString timestamp;   // 时间戳
    int unreadCount;     // 未读消息数

    ItemChatData() : unreadCount(0) {}
};

namespace Ui {
class ItemChat;
}

class ItemChat : public QWidget {
    Q_OBJECT

public:
    explicit ItemChat(QWidget *parent = nullptr);
    explicit ItemChat(const ItemChatData data, QWidget *parent = nullptr);
    ~ItemChat();

    // 获取数据
    const ItemChatData& get_data() const { return m_data; }

private:
    void update_data(const ItemChatData &data);
    void update_ui();

private:
    Ui::ItemChat *ui;
    ItemChatData  m_data;
};
