#pragma once

#include <QVariant>
#include <QString>
#include <QVector>
#include <QMap>
#include <QSharedPointer>
// #include <iostream>
// #include <vector>
// #include <map>
// #include <memory>

// // 聊天室/群组/联系人信息
// struct TypeContact {
//     enum class Type{
//         FRIEND,  // 朋友/个人
//         GROUP,   // 群组
//         SELF     // 自己
//     };
//     Type type = Type::FRIEND;
//     QString id         ;  // 对话 ID  // 唯一标识
//     // QString title   ;  // 聊天室标题
//     QString avatar     ;  // 头像路径或标识
//     QString nickname   ;  // 昵称
//     QString lastMessage;  // 最后一条消息内容
//     QString timestamp  ;  // 时间戳
//     int unreadCount;   // 未读消息数
//     // model/view
//     enum class ModelRole{
//         Begin = Qt::ItemDataRole::UserRole,  // 开始标记
//         // ID      ,  // 不必展示
//         TYPE    ,  // 对话类型
//         Avatar  ,  // 头像
//         NickName,  // 昵称
//         Message ,  // 消息
//         UNREAD  ,  // 是否有未读消息
//         End        // 结束标记
//     };
// };

// 消息内容，单条消息
struct TypeMessage {
    // 发送者
    enum class Sender{
        SELF,
        FRIEND
    };
    // 消息类型
    enum class Type{
        TIME,  // 显示为时间戳
        Text,  // 显示消息内容
        IMAGE, // 显示图片
        VIDEO  // 显示视频
    };
    //
    Sender sender = Sender::SELF;
    Type   type   = Type::Text;
    QString avatar  ;
    QString nickname;
    QString contact ;
    QString time    ;
};

struct TypeContact;
struct TypeMessage;

// 一个联系人
typedef QSharedPointer<TypeContact> Contact;
// 一条消息
typedef QSharedPointer<TypeMessage> Message;

// 所有联系人
typedef QVector<Contact> TypeContacts;
typedef QSharedPointer<TypeContacts> Contacts;

// 单聊天框的聊天记录  // 与某人/群组的一个聊天框的内容
typedef QVector<Message> TypeMessages;
typedef QSharedPointer<TypeMessages> Messages;

// 单用户的聊天数据  // QMap<QString 群组 ID, QVector<ItemMessageData*>*所有消息>
typedef QMap<std::string, Messages> TypeUserMessages;
typedef QSharedPointer<TypeUserMessages> UserMessages;

// ========================================================================== //

struct TypeContact{
    enum class Type{
        UNINIT,
        FRIEND,  // 朋友/个人
        GROUP,   // 群组
        SELF     // 自己
    };
    enum class ModelRole{
        Begin = Qt::ItemDataRole::UserRole,  // 开始标记
        ID      ,  // 不必展示
        TYPE    ,  // 对话类型
        Avatar  ,  // 头像
        NickName,  // 昵称
        Message ,  // 消息
        UNREAD  ,  // 是否有未读消息
        Ext     ,  // 扩展，可用于保存自定义数据
        End        // 结束标记
    };
    // Q_ENUM(ModelRole) // not for `enum class`?

private:
    QMap<ModelRole, QVariant> m_datas;

public:
    // 构造
    TypeContact(QString id = QString(), Type type = Type::UNINIT,
                    QString avatar = QString(), QString nickname = QString(), QString message = QString(),
                    int unread = 0, void *point = nullptr
    ) {
        set_data(id, type, avatar, nickname, message, unread, point);
    }

    void set_data(
        QString id = QString(), Type type = Type::UNINIT,
        QString avatar = QString(), QString nickname = QString(), QString message = QString(),
        int unread = 0, void *point = nullptr
    ) {
        datas()[ModelRole::ID      ] = id      ;
        // datas()[ModelRole::TYPE ] = type    ;
        datas()[ModelRole::Avatar  ] = avatar  ;
        datas()[ModelRole::NickName] = nickname;
        datas()[ModelRole::Message ] = message ;
        datas()[ModelRole::UNREAD  ] = unread  ;
        // datas()[ModelRole::Ext  ] = point   ;
        QVariant variant_type;
        variant_type.setValue(type);
        datas()[ModelRole::TYPE  ] = variant_type;
        QVariant variant_point;
        variant_point.setValue(point);
        datas()[ModelRole::Ext   ] = variant_point;
    }

    // 表头
    // 1. `QMap` 和 `QVariant` 不是 `constexpr` 类型，不能使用 `static constexpr` 修饰
    // 2. `return {...}` 返回的是临时对象，不能使用 `static const ReturnType&` 引用修饰
    static const QMap<ModelRole, QVariant>& headers() {
        static const QMap<ModelRole, QVariant> s_headers =
            {
                { ModelRole::ID      , "ID"       },
                { ModelRole::TYPE    , "Type"     },
                { ModelRole::Avatar  , "Avatar"   },
                { ModelRole::NickName, "NickName" },
                { ModelRole::Message , "Message"  },
                { ModelRole::UNREAD  , "Unread"   },
                { ModelRole::Ext     , "Ext"      },
            };
        return s_headers;
    }

    QMap<ModelRole, QVariant>&datas() { return m_datas; }
    // data 用于 model/view 视图自动获取内容 // 传入非 ModelRole 值
    QVariant data(int index) {
        ModelRole role = ModelRole(int(ModelRole::Begin) + 1 + index);
        if(int(role) >= int(ModelRole::Begin) + 1 && int(role) < int(ModelRole::End)
            && index >= 0 && index < m_datas.size()) {
            return data(role);
        }
        return QVariant();
    }
    // data 用于 model/view 视图自动获取内容 // 传入 ModelRole 值
    QVariant data(ModelRole role) {
        auto item = m_datas.find(role);
        if(item != m_datas.end()) {
            return item.value();
        }
        return QVariant();
    }

    // headers_size 用于 model/view 视图获取标题行数
    static int headers_size() {
        return headers().size();  // 数据成员个数
    }

    // header 用于 model/view 视图获取标题名称 // 传入非 ModelRole 值
    static QVariant header(int index) {
        ModelRole role = ModelRole(int(ModelRole::Begin) + 1 + index);
        if(int(role) >= int(ModelRole::Begin) && int(role) < int(ModelRole::End)
            && index >= 0 && index < headers().size()) {
            return header(role);
        }
        return QVariant();
    }
    // header 用于 model/view 视图获取标题名称 // 传入 ModelRole 值
    static QVariant header(ModelRole role) {
        // 建议先保存 `headers()` 结果再对比，
        // 否则 `headers()` 非 static 修饰的情况下会再次创建 `headers()` 对象
        auto&s_headers = headers();
        auto item = s_headers.find(role);
        if(item != s_headers.end()) {
            return item.value();
        }
        return QVariant();
    }

    // Calling QVariant::toString() on an unsupported variant returns an empty string.
    QString id      () { return data(ModelRole::ID      ).toString(); }
    Type    type    () { return data(ModelRole::TYPE    ).value<Type >(); }
    QString avatar  () { return data(ModelRole::Avatar  ).toString(); }
    QString nickname() { return data(ModelRole::NickName).toString(); }
    QString message () { return data(ModelRole::Message ).toString(); }
    int     unread  () { return data(ModelRole::UNREAD  ).toInt   (); }
    void*   ext     () { return data(ModelRole::Ext     ).value<void*>(); }
};
