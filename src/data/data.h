#pragma once

#include <QObject>
#include <QVariant>
#include <QString>
#include <QVector>
#include <QMap>
#include <QSharedPointer>
// #include <iostream>
// #include <vector>
// #include <map>
// #include <memory>

// ========================================================================== //
// // 定义声明

struct ContactData;
struct MessageData;
struct AccountData;

// 一个联系人
typedef QSharedPointer<ContactData> Contact;
// 一条消息
typedef QSharedPointer<MessageData> Message;
// 一个登陆用户
typedef QSharedPointer<AccountData> Account;

// 单用户的所有联系人
typedef QVector<Contact> ContactsData;
typedef QSharedPointer<ContactsData> Contacts;

// 单用户的单个聊天室的聊天记录  // 与某人/群组的一个聊天框的内容
typedef QVector<Message> MessagesData;
typedef QSharedPointer<MessagesData> Messages;

// 单用户的所有聊天室的所有聊天数据  // QMap<QString 群组 ID, QVector<ItemMessageData*>*所有消息>
typedef QMap<std::string, Messages> UserMessagesData;
typedef QSharedPointer<UserMessagesData> UserMessages;

// 所有登陆用户的所有信息 // 包含所有登陆用户信息/好友信息/聊天内容
typedef QMap<std::string, Account> AccountsData;
typedef QSharedPointer<AccountsData> Accounts;

// ========================================================================== //
// // 数据内容

// 聊天室/群组/联系人信息
struct ContactData{
    Q_GADGET  // 使结构体支持元对象系统（反射、枚举转换）

public:
    enum class Type{
        UNINIT,
        FRIEND,  // 朋友/个人
        GROUP,   // 群组
        SELF     // 自己
    };
    Q_ENUM(Type)

    enum class Role{
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
    Q_ENUM(Role)

    QMap<Role, QVariant> datas;
};
typedef QMap<ContactData::Role, QVariant> ContactDatas;

// 消息内容，单条消息
struct MessageData {
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

// 单个登陆账号信息  // 用户信息/好友列表/消息列表
struct AccountData{
    Contact      user    ;  // 当前用户
    Contacts     contacts;  // 好友及群组列表
    UserMessages messages;  // 消息列表 // 相同好友可能有多个群聊
};

// ========================================================================== //
// // 管理操作

// 聊天室/群组/联系人信息
class ContactManager{
public:
    static Contact create() {
        return QSharedPointer<ContactData>(new ContactData);
    }

    // 表头
    // 1. `QMap` 和 `QVariant` 不是 `constexpr` 类型，不能使用 `static constexpr` 修饰
    // 2. `return {...}` 返回的是临时对象，不能使用 `static const ReturnType&` 引用修饰
    static const QMap<ContactData::Role, QString>& headers() {
        using MR = ContactData::Role;
        static const QMap<MR, QString> s_headers =
            {
             { MR::ID      , "ID"       },
             { MR::TYPE    , "Type"     },
             { MR::Avatar  , "Avatar"   },
             { MR::NickName, "NickName" },
             { MR::Message , "Message"  },
             { MR::UNREAD  , "Unread"   },
             { MR::Ext     , "Ext"      },
             };
        return s_headers;
    }

    // headers_size 用于 model/view 视图获取标题行数
    static int headers_size() {
        return headers().size();  // 数据成员个数
    }

private:
    Contact m_contact;

public:
    // FIIXME:BUGS:传入 nullptr 时没有维护该对象 `Contact contact; ContactManager(contact)`
    // 传入非 nullptr 时能够正常赋值
    ContactManager(Contact contact = nullptr)
        : m_contact(ContactManager::create()) {
        // assert(contact == nullptr, "")
        replace(contact);  // 保存维护对象。不能是 update_data()
    }

    // update == update_data : 更新维护对象内的数据，不更改维护对象; replace: 更改维护对象。

    // 替换维护的对象
    // 只有传入维护对象的构造函数需要调用该函数。当前类属于临时的类，一般情况下不需要更改维护对象
    void replace(Contact contact = nullptr) {
        // 既然是替换，那么即使 Contact 的内容为 nullptr，那么也需要维护该指针
        m_contact = contact; // ? contact : QSharedPointer<ContactData>(new ContactData);
        // 为防意外，需要对维护指针创建内存
        if(m_contact == nullptr) {
            m_contact = ContactManager::create();
        }
    }
    // 更新维护对象的内容
    void update(Contact contact = nullptr) {
        ContactData* contact_data = contact ? contact.data() : ContactManager::create().data();
        *m_contact.data() = *contact_data;
    }
    // 构造
    ContactManager(
        QString id = QString(), ContactData::Type type = ContactData::Type::UNINIT,
        QString avatar = QString(), QString nickname = QString(), QString message = QString(),
        int unread = 0, void *point = nullptr
    ) : ContactManager(ContactManager::create()) {
        // 没有传入维护对象指针，则不需要替换默认生成的维护对象
        update(id, type, avatar, nickname, message, unread, point);
    }
    // 执行  // 这里使用新建的对象而不是对默认生成的对象直接操作，主要是为了方便维护
    void update(
        QString id = QString(), ContactData::Type type = ContactData::Type::UNINIT,
        QString avatar = QString(), QString nickname = QString(), QString message = QString(),
        int unread = 0, void *point = nullptr) {
        // 创建数据对象
        Contact contact = ContactManager::create();
        // 保存参数到数据对象
        using MR = ContactData::Role;
        QVariant vtype;
        vtype.setValue(type);
        QVariant vpoint;
        vpoint.setValue(point);
        contact->datas[MR::ID      ] = id      ;
        contact->datas[MR::TYPE    ] = vtype   ;
        contact->datas[MR::Avatar  ] = avatar  ;
        contact->datas[MR::NickName] = nickname;
        contact->datas[MR::Message ] = message ;
        contact->datas[MR::UNREAD  ] = unread  ;
        contact->datas[MR::Ext     ] = vpoint  ;
        // 调用更新
        update(contact);
    }

public:
    Contact      &contact      () { return m_contact             ; }
    ContactDatas &contact_datas() { return contact()->datas      ; }
    int           contact_size () { return contact_datas().size(); }

    // data 用于 model/view 视图自动获取内容 // 传入非 Role 值
    QVariant value(int index) {
        using MR = ContactData::Role;
        MR role = MR(int(MR::Begin) + 1 + index);
        if(int(role) >= int(MR::Begin) + 1 && int(role) < int(MR::End)
            && index >= 0 && index < contact_size()) {
            return value(role);
        }
        return QVariant();
    }

    // data 用于 model/view 视图自动获取内容 // 传入 Role 值
    QVariant value(ContactData::Role role) {
        auto&date = contact_datas();
        auto item = date.find(role);
        if(item != date.end()) {
            return item.value();
        }
        return QVariant();
    }

    // header 用于 model/view 视图获取标题名称 // 传入非 Role 值
    QVariant header(int index) {
        using MR = ContactData::Role;
        MR role = MR(int(MR::Begin) + 1 + index);
        if(int(role) >= int(MR::Begin) && int(role) < int(MR::End)
            && index >= 0 && index < headers().size()) {
            return header(role);
        }
        return QVariant();
    }

    // header 用于 model/view 视图获取标题名称 // 传入 Role 值
    QVariant header(ContactData::Role role) {
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
    QString id      () { return value(ContactData::Role::ID      ).toString(); }
    QString avatar  () { return value(ContactData::Role::Avatar  ).toString(); }
    QString nickname() { return value(ContactData::Role::NickName).toString(); }
    QString message () { return value(ContactData::Role::Message ).toString(); }
    int     unread  () { return value(ContactData::Role::UNREAD  ).toInt   (); }
    ContactData::Type type    () { return value(ContactData::Role::TYPE    ).value<ContactData::Type>(); }
    void*   ext     () { return value(ContactData::Role::Ext     ).value<void*>(); }
};

class AccountsManager{
public:
    AccountsManager* instance() {
        static AccountsManager s_accounts;
        return &s_accounts;
    }
private:
    AccountsManager(){}
    ~AccountsManager(){}
    // AccountsManager *Accounts(const Accounts*) = delete;
    // AccountsManager *operator=(const Accounts*) = delete;
public:
    // Account account(const QString user_id);
public:
    Accounts m_accounts;
};
