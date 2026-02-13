#pragma once

#include "Utils.h"
// #include "RegistryFactory.h"

// ========================================================================== //
// // 定义声明

struct ContactData ;
struct MessageData ;  // 仅消息内容
struct ChatroomData;  // 消息内容及群组附加信息
struct AccountData ;

// 一个联系人
using Contact  = QSharedPointer<ContactData>;
// 一条消息
using Message  = QSharedPointer<MessageData>;
// 一个聊天室
using Chatroom = QSharedPointer<ChatroomData>;
// 一个登陆用户
using Account  = QSharedPointer<AccountData>;

// 单用户的所有联系人
// 使用 QList 保持插入顺序，而不是 QMap 的自动排序
using ContactsData = QList<Contact>;
using Contacts     = QSharedPointer<ContactsData>;

// 单用户的单个聊天室的聊天记录  // 与某人/群组的一个聊天框的内容
using MessagesData = QVector<Message>;
using Messages     = QSharedPointer<MessagesData>;

// 单用户的所有聊天室的所有聊天数据
// QMap<QString char_room_id, Messages>
using UserMessagesData = QMap<QString, Messages>;
using UserMessages     = QSharedPointer<UserMessagesData>;

// 所有登陆用户的所有信息 // 包含所有登陆用户信息/好友信息/聊天内容
// QMap<QString user_id, Contact user_info>;
using AccountsData = QMap<QString, Account>;
using Accounts     = QSharedPointer<AccountsData>;

// ========================================================================== //
// // 数据内容

// 聊天室/群组/联系人信息
struct ContactData{
    Q_GADGET  // 使结构体支持元对象系统（反射、枚举转换）

public:
    // 预定义内容：
    //            头像,     昵称,    性别,             星标,            勿扰,
    // Profile picture, nickname, gender, starred contacts, do not disturb(Silent),
    //      账号ID,   地区,   备注,         电话,  邮箱,  标签,         描述,
    // account ID, region, notes, phone number, email, tags, description,
    //   朋友圈,          共同群聊,           个性签名,            好友来源/来自
    // Moments, shared group chats, personal signature, source of friends/from
    //     消息,             未读
    // messages, unread messages
    enum class Role{
        Begin = Qt::ItemDataRole::UserRole,  // 开始标记
        Starred  ,   // 星标
        Silent   ,   // 静音/勿扰/免提醒
        AccountID,   // 账号 ID  // 唯一，不可修改，不显示
        Userid   ,   // 唯一，可修改，用于查找、添加好友等操作
        Avatar   ,   // 头像图片路径  // Profile photo 倾向于使用本人真实照片，强调写实；Avatar 则是网络虚拟化身
        Pixmap   ,   // 头像图片
        Nickname ,   // 昵称
        Notes    ,   // 对该用户的备注
        Desc     ,   // 描述
        Gender   ,   // 性别
        Moments  ,   // 朋友圈  // ？？
        Sign     ,   // 个性签名
        From     ,   // 添加来源
        Region   ,   // 地区     // 不同层级
        Phones   ,   // 电话     // 可以有多个
        Emails   ,   // 邮箱     // 可以有多个
        Tags     ,   // 标签     // 可以有多个
        Groups   ,   // 共同群聊 // 可以有多个 // group_ids
        // 基础数据
        // TYPE    ,  // 对话类型？?
        // 下面的内容可用于个人信息页面的信息显示
        // // 下面的内容可用于消息列表显示
        // // 可能有重复群聊，应该归属于 Chat room, 不应与联系人关联
        // Message ,  // 最后一条消息 // 用于预览  // TODO: 需要移动到 chat room
        // Unread  ,  // 是否有未读消息
        // 扩展及其他
        // Ext     ,  // 扩展，可用于保存自定义数据
        End        // 结束标记
    };
    Q_ENUM(Role)

    QMap<QString, QVariant> datas;
};
using ContactDatas = QMap<QString, QVariant>;

// 消息内容，单条消息
struct MessageData {
    // 发送者类型
    enum class Sender{
        SELF,
        FRIEND  // 非自己  // 好友及群聊中的其他人等
    };
    // 消息类型
    enum class Type{
        TIME,  // 显示为时间戳
        Text,  // 显示消息内容
        IMAGE, // 显示图片
        VIDEO  // 显示视频
    };
    //
    Sender  sender = Sender::SELF;
    Type    type   = Type::Text;
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

    static QString to_string(ContactData::Role role) {
        return QMetaEnum::fromType<ContactData::Role>().valueToKey(quint64(role));  // quint64 = long long unsigned int
    }
    static QString to_key   (ContactData::Role role) { return to_string(role).toUpper(); }

    // 表头
    // 1. `QMap` 和 `QVariant` 不是 `constexpr` 类型，不能使用 `static constexpr` 修饰
    // 2. `return {...}` 返回的是临时对象，不能使用 `static const ReturnType&` 引用修饰
    static const QMap<ContactData::Role, QString>& headers() {
        static const QMap<ContactData::Role, QString> s_headers = [&](){
            QMap<ContactData::Role, QString> headers;
            QMetaEnum meta_enum = QMetaEnum::fromType<ContactData::Role>();
            for(int role = int(ContactData::Role::Begin); role < int(ContactData::Role::End); ++role) {
                headers[ContactData::Role(role)] = meta_enum.valueToKey(role);
            }
            return headers;
        }();
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
        const QString account_id     = QString(),
        const QPixmap &avatar_pixmap = QPixmap(),
        const QString avatar_path    = QString(),
        const QString nickname       = QString(),
        const QString notes          = QString()
    ) : ContactManager(ContactManager::create()) {
        // 没有传入维护对象指针，则不需要替换默认生成的维护对象
        update(account_id, avatar_pixmap, avatar_path, nickname, notes);
    }
    // 执行  // 这里使用新建的对象而不是对默认生成的对象直接操作，主要是为了方便维护
    void update(
        const QString account_id     = QString(),
        const QPixmap &avatar_pixmap = QPixmap(),
        const QString avatar_path    = QString(),
        const QString nickname       = QString(),
        const QString notes          = QString()
    ) {
        ContactManager cm(ContactManager::create());
        cm.set_account_id   (account_id   );
        cm.set_avatar_pixmap(avatar_pixmap);
        cm.set_avatar_path  (avatar_path  );
        cm.set_nick_name    (nickname     );
        cm.set_notes        (notes        );
        update(cm.contact());
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
        return contact_datas()[to_key(role)];
        // QString key = to_key(role);
        // auto&datas = contact_datas();
        // if(datas.contains(key)) {
        //     return datas[key];
        // }
        // return QVariant();
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

    QVariant &get(const ContactData::Role&role) { return contact_datas()[to_key(role)]; }
    // Calling QVariant::toString() on an unsupported variant returns an empty string.
    bool        get_starred    (                             ) { return get(ContactData::Role::Starred  ).toBool      (); }
    bool        get_silent     (                             ) { return get(ContactData::Role::Silent   ).toBool      (); }
    QString     get_account_id (                             ) { return get(ContactData::Role::AccountID).toString    (); }
    QString     get_user_id    (                             ) { return get(ContactData::Role::Userid   ).toString    (); }
    QString     get_avatar_path(                             ) { return get(ContactData::Role::Avatar   ).toString    (); }
    QString     get_nick_name  (                             ) { return get(ContactData::Role::Nickname ).toString    (); }
    QString     get_notes      (                             ) { return get(ContactData::Role::Notes    ).toString    (); }
    QString     get_description(                             ) { return get(ContactData::Role::Desc     ).toString    (); }
    QString     get_gender     (                             ) { return get(ContactData::Role::Gender   ).toString    (); }
    QString     get_moments    (                             ) { return get(ContactData::Role::Moments  ).toString    (); }
    QString     get_signature  (                             ) { return get(ContactData::Role::Sign     ).toString    (); }
    QString     get_from       (                             ) { return get(ContactData::Role::From     ).toString    (); }
    QStringList get_region     (                             ) { return get(ContactData::Role::Region   ).toStringList(); }
    QStringList get_phones     (                             ) { return get(ContactData::Role::Phones   ).toStringList(); }
    QStringList get_emails     (                             ) { return get(ContactData::Role::Emails   ).toStringList(); }
    QStringList get_tags       (                             ) { return get(ContactData::Role::Tags     ).toStringList(); }
    QStringList get_groups     (                             ) { return get(ContactData::Role::Groups   ).toStringList(); }
    void        set_starred    (const bool       &starred    ) {        get(ContactData::Role::Starred  ) = starred     ; }
    void        set_silent     (const bool       &silent     ) {        get(ContactData::Role::Silent   ) = silent      ; }
    void        set_account_id (const QString    &account_id ) {        get(ContactData::Role::AccountID) = account_id  ; }
    void        set_user_id    (const QString    &user_id    ) {        get(ContactData::Role::Userid   ) = user_id     ; }
    void        set_avatar_path(const QString    &avatar_path) {        get(ContactData::Role::Avatar   ) = avatar_path ; }
    void        set_nick_name  (const QString    &nick_name  ) {        get(ContactData::Role::Nickname ) = nick_name   ; }
    void        set_notes      (const QString    &notes      ) {        get(ContactData::Role::Notes    ) = notes       ; }
    void        set_description(const QString    &description) {        get(ContactData::Role::Desc     ) = description ; }
    void        set_gender     (const QString    &gender     ) {        get(ContactData::Role::Gender   ) = gender      ; }
    void        set_moments    (const QString    &moments    ) {        get(ContactData::Role::Moments  ) = moments     ; }
    void        set_signature  (const QString    &signature  ) {        get(ContactData::Role::Sign     ) = signature   ; }
    void        set_from       (const QString    &from       ) {        get(ContactData::Role::From     ) = from        ; }
    void        set_region     (const QStringList&region     ) {        get(ContactData::Role::Region   ) = region      ; }
    void        set_phones     (const QStringList&phones     ) {        get(ContactData::Role::Phones   ) = phones      ; }
    void        set_emails     (const QStringList&emails     ) {        get(ContactData::Role::Emails   ) = emails      ; }
    void        set_tags       (const QStringList&tags       ) {        get(ContactData::Role::Tags     ) = tags        ; }
    void        set_groups     (const QStringList&groups     ) {        get(ContactData::Role::Groups   ) = groups      ; }
    QPixmap get_avatar_pixmap() {
        return get(ContactData::Role::Pixmap).value<QPixmap>();
    }
    void set_avatar_pixmap(const QPixmap &avatar_pixmap) {
        QVariant avatar_variant;
        avatar_variant.setValue(avatar_pixmap);
        get(ContactData::Role::Pixmap) = avatar_variant;
    }
};

// ========================================================================== //

class AccountsManager{
public:
    AccountsManager* instance() {
        static AccountsManager s_accounts;
        return &s_accounts;
    }
private:
    AccountsManager(){}
    ~AccountsManager(){}
    AccountsManager(const Accounts&) = delete;
    AccountsManager *operator=(const Accounts&) = delete;
public:
    Accounts    &accounts      () { return m_accounts; }
    Account      account       (const QString&user_id) {
        if(!accounts()->contains(user_id)) {
            accounts()->operator [](user_id) = Account();
        }
        return accounts()->operator [](user_id);
    }
    Contact      user_info     (const QString&user_id) { return account(user_id)->user    ; }
    Contacts     user_contacts (const QString&user_id) { return account(user_id)->contacts; }
    UserMessages user_messages (const QString&user_id) { return account(user_id)->messages; }
    void         remove_account(const QString&user_id) { accounts()->remove(user_id); }
public:
    // 从单用户的所有信息中查找指定 id 的好友
    static Contact contact(Account account, QString contact_id) {
        if(account == nullptr || account->contacts == nullptr) {
            return Contact();
        }
        for(const auto &contact : *account->contacts) {
            if(contact && ContactManager(contact).get_account_id() == contact_id) {
                return contact;
            }
        }
        return Contact();
    }
    // 从好友列表中获取第 index 个好友
    static Contact contact(Contacts contacts, int index) {
        if(contacts == nullptr || index < 0 || index >= contacts->size()) {
            return Contact();
        }
        return contacts->at(index);
    }
    // // 获取 ID  // 多此一举，没必要
    // static QString id(Contact contact) { return ContactManager(contact).id(); }
public:
    Accounts m_accounts;
};
