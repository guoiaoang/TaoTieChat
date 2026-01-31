#pragma once

#include <QWidget>

namespace Ui {
class ItemMessage;
}

struct ItemMessageData {
    enum class Type{
        FRIEND,
        SELF,
        TIME
    } type = Type::FRIEND;
    QString avatar;
    QString nickname;
    QString content;
    QString time;
    ItemMessageData(){}
    // ItemMessageData(const QString&time) : type(Type::TIME), time(time) {}
    // ItemMessageData(
    //     const Type&type, const QString&avatar,
    //     const QString&nickname, const QString&content,
    //     const QString&time
    //     ) : type(type), avatar(avatar),
    //     nickname(nickname), content(content), time(time) {}
    ItemMessageData(
        const QString&avatar, const QString&nickname,
        const QString&content, const QString&time, bool is_self
    ) : avatar(avatar), nickname(nickname), content(content), time(time),
        type(is_self ? Type::SELF : Type::FRIEND)    {}
};

struct ChatroomInfo{
    enum class Type{
        FRIEND,  // 与朋友一对一聊天
        SELF,  // 自己
        GROUP  // 群组
    } type = Type::GROUP;
    QString title;  // 聊天室标题
    QString avatar;
    // 组成员
    ChatroomInfo(){}
    ChatroomInfo(const QString&title, const QString&avatar = "", const Type&type = Type::GROUP)
        : type(type), title(title), avatar(avatar) {}
};

typedef QVector<ItemMessageData> ChatroomMessages;  // 单聊天框的聊天记录
typedef QMap<QString, QVector<ItemMessageData>> UserMessages; // 单用户的聊天数据

class ItemMessage : public QWidget {
    Q_OBJECT

public:
    explicit ItemMessage(QWidget *parent = nullptr);
    explicit ItemMessage(const ItemMessageData&data, QWidget *parent = nullptr);
    ~ItemMessage();

private:
    void update_data(const ItemMessageData &data);
    void update_ui();

private:
    Ui::ItemMessage *ui;
    ItemMessageData  m_data;
};
