#pragma once

#include <QDebug>
#include <QString>
#include <QMap>
#include <QSharedPointer>
#include <functional>

template<class Base>
class RegistryFactory {
public:
    // 注册函数：利用模板自动匹配不同的构造参数
    // 注意：
    //    这里的 `template<typename T>` 不能被省略，
    //    也不能使用 `template<class RegistryPatternBase>` 替换，
    //    否则 `RegistryPatternBase` 有虚函数时会报 `无法实例化抽象类`.
    template<typename T, typename... Args>
    static std::function<QSharedPointer<Base>(Args...)> register_type(const QString& name) {
        auto& map = get_map<Args...>();
        map[name] = [](Args... args) -> QSharedPointer<Base> {
            return QSharedPointer<T>::create(std::forward<Args>(args)...);
        };
        return map[name];
    }

    template<typename... Args>
    static QSharedPointer<Base> create(const QString& name, Args... args) {
        auto& map = get_map<Args...>();
        if (map.contains(name)) {
            return map[name](std::forward<Args>(args)...);
        }

        // 友好错误提示：打印出尝试匹配的参数类型
        qDebug() << "[Factory Error] Type '" << name << "' not found for argument profile.";
        return nullptr;
    }

private:
    template<typename... Args>
    using Creator = std::function<QSharedPointer<Base>(Args...)>;

    // 核心：为每种参数组合生成一个独立的 Map 存储空间
    template<typename... Args>
    static QMap<QString, Creator<Args...>>& get_map() {
        static QMap<QString, Creator<Args...>> s_map;
        return s_map;
    }
};

#define REG_FAC_CONCAT_HIDDEN(a, b, c, d) a##b##c##d
#define REG_FAC_CONCAT(a, b, c, d) REG_FAC_CONCAT_HIDDEN(a, b, c, d)
// BASE:基类，IMPL:之类，KEY:不带引号且有较多限制的字符串
// 若需更灵活的字符串可手动创建而非调用该宏
#define REG_FAC(BASE, IMPL, KEY) \
static auto REG_FAC_CONCAT(gs_, BASE, KEY, __LINE__) = \
    RegistryFactory<BASE>::register_type<IMPL>(#KEY);
// 示例：(`class ContactFriend : public ContactBase {}`)
//     REG_FAC(ContactBase, ContactFriend, contact_friend)
// 手动创建：
//    static auto gs_ContactBase_ContactFriend_contact_friend =
//        RegistryFactory<ContactBase>::register_type<ContactFriend>("contact_friend");


// class Shape {
// public:
//     virtual ~Shape() = default;
//     virtual void draw(const std::string& args) = 0;
// };
//
// class Circle : public Shape {
// public:
//     ;    Circle(                       )          { std::cout << "Circle created"                         << std::endl; }
//     ;    Circle(int w, int h           )          { std::cout << "Circle " << w << "x" << h << " created" << std::endl; }
//     ;    Circle(const std::string& name)          { std::cout << "Circle " << name << " created"          << std::endl; }
//     void draw  (const std::string& args) override { std::cout << "Drawing Circle with args: " << args     << std::endl; }
// };
//
// int main() {
//     RegistryFactory<Shape>::register_type<Circle             >("_circle_void"   );
//     RegistryFactory<Shape>::register_type<Circle, int, int   >("_circle_int_int");
//     RegistryFactory<Shape>::register_type<Circle, std::string>("_circle_string" );
//     // 有参创建需要明确指出与注册函数想匹配的参数类型，否则自动转换大概率失败
//     auto v1 = RegistryFactory<Shape>::create("_circle_void");
//     auto i1 = RegistryFactory<Shape>::create("_circle_int_int", 10, 20);  // ok
//     auto i2 = RegistryFactory<Shape>::create("_circle_int_int", 10.5  );  // error: double 不能直接匹配 int
//     auto s1 = RegistryFactory<Shape>::create("_circle_string" , std::string("Sirius"));  // ok
//     auto s2 = RegistryFactory<Shape>::create("_circle_string" , "Polaris"            );  // error: const char* 不能直接匹配 std::string
//     if(v1) { v1->draw("void"   ); } else { std::cerr << "Failed to create Circle with void        args." << std::endl; }
//     if(i1) { i1->draw("10, 20" ); } else { std::cerr << "Failed to create Circle with int, int    args." << std::endl; }
//     if(i2) { i2->draw("10.5"   ); } else { std::cerr << "Failed to create Circle with double      args." << std::endl; }
//     if(s1) { s1->draw("Sirius" ); } else { std::cerr << "Failed to create Circle with string      args." << std::endl; }
//     if(s2) { s2->draw("Polaris"); } else { std::cerr << "Failed to create Circle with const char* args." << std::endl; }
//     return 0;
// }
//
// // 输出：
// // > g++ --std=c++17 RegistryPatternDemo.cpp -o RegistryPatternDemo
// // > RegistryPatternDemo.exe
// // Circle created
// // Circle 10x20 created
// // [Factory Error] Type '_circle_int_int' not found for argument profile.
// // Circle Sirius created
// // [Factory Error] Type '_circle_string' not found for argument profile.
// // Drawing Circle with args: void
// // Drawing Circle with args: 10, 20
// // Failed to create Circle with double      args.
// // Drawing Circle with args: Sirius
// // Failed to create Circle with const char* args.
// // >

#include <QObject>
#include <QtDebug>
#include <QUuid>
#include <QString>
#include <QVector>
#include <QMap>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QDirListing>
#include <QFileInfo>
#include <QIcon>
#include <QPixmap>
#include <QBitmap>
#include <QImage>
#include <QtSvg/QSvgGenerator>
#include <QDateTime>

namespace RegistryFactoryContact{
#define RFC_ADD_MEMBER(T, NAME, VALUE) \
private: \
    T m_##NAME = T{}; \
    public: \
    virtual void set_##NAME(const T&t) { m_##NAME = t; } \
    virtual T get_##NAME() const { return m_##NAME; }

#define RFC_DISABLE_MEMBER(T, NAME, VALUE) \
public: \
    void set_##NAME(const T&t) = delete; \
    T get_##NAME() const = delete;

// 联系人  // 基础信息
class ContactBase{
public:
    ContactBase() {}
    virtual ~ContactBase() {}

private:
    RFC_ADD_MEMBER(bool       , starred    , bool       {}) // 星标
    RFC_ADD_MEMBER(bool       , silent     , bool       {}) // 静音/勿扰/免提醒
    RFC_ADD_MEMBER(QString    , account_id , QString    {}) // 账号 ID  // 唯一，不可修改，不显示
    RFC_ADD_MEMBER(QString    , user_id    , QString    {}) // 唯一，可修改，用于查找、添加好友等操作
    RFC_ADD_MEMBER(QString    , nick_name  , QString    {}) // 昵称
    RFC_ADD_MEMBER(QString    , notes      , QString    {}) // 备注
    RFC_ADD_MEMBER(QString    , description, QString    {}) // 描述
    RFC_ADD_MEMBER(QVariant   , avatar     , QVariant   {}) // 头像路径或图片
};

// 好友
class ContactFriend : public ContactBase{
public:
    ContactFriend() {}
    ~ContactFriend() override {}

    RFC_ADD_MEMBER(QString    , gender     , QString    {}) // 性别
    RFC_ADD_MEMBER(QString    , moments    , QString    {}) // 朋友圈  // ？？
    RFC_ADD_MEMBER(QString    , signature  , QString    {}) // 个性签名
    RFC_ADD_MEMBER(QString    , from       , QString    {}) // 添加来源
    RFC_ADD_MEMBER(QStringList, region     , QStringList{}) // 地区     // 不同层级
    RFC_ADD_MEMBER(QStringList, phones     , QStringList{}) // 手机号   // 可以有多个
    RFC_ADD_MEMBER(QStringList, emails     , QStringList{}) // 邮箱     // 可以有多个
    RFC_ADD_MEMBER(QStringList, tags       , QStringList{}) // 标签     // 可以有多个
    RFC_ADD_MEMBER(QStringList, groups     , QStringList{}) // 共同群聊 // 可以有多个 // group_ids
};
REG_FAC(ContactBase, ContactFriend, contact_friend)

// 群聊
class ContactGroup : public ContactBase{
public:
    ContactGroup() {}
    ~ContactGroup() override {}
};
REG_FAC(ContactBase, ContactGroup, contact_group)

// 企业员工
class ContactEmployee : public ContactFriend{
public:
    ContactEmployee() {}
    ~ContactEmployee() override {}
    RFC_ADD_MEMBER(bool       , employee   , bool       {}) // 雇员/企业联系人
    RFC_ADD_MEMBER(QString    , employer   , QString    {}) // 雇主/企业名称
};
REG_FAC(ContactBase, ContactEmployee, contact_employee)

#undef RFC_ADD_MEMBER
#undef RFC_DISABLE_MEMBER
}

namespace RegistryFactoryMessage{
// 消息内容，单条消息
class MessageBase {
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
    Sender  sender = Sender::FRIEND;
    Type    type   = Type::Text;
    QString avatar  ;
    QString nickname;
    QString contact ;
    QString time    ;
};

// 消息内容
class MessageContent : public MessageBase{
public:
};

// 自发送的消息
class MessageSelf : public MessageBase{
public:
};

// 朋友发送的消息
class MessageFriend : public MessageBase{
public:
};

// 显示时间
class MessageTime : public MessageBase{
public:
};

// 显示图片
class MessageImage : public MessageBase{
public:
};

// 显示视频
class MessageVideo : public MessageBase{
public:
};

}

// using ContactData = RegistryFactoryContact::ContactBase;
// using MessageBase = RegistryFactoryMessage::MessageBase;
