#pragma once

#include "headers.h"
#include "Utils.h"
#include "Defines.h"

struct AuthOpt;

class Accounts : public QObject {
    Q_OBJECT
private:
    Accounts();
    Accounts(const Accounts&) = delete;
    Accounts& operator=(const Accounts&) = delete;
public:
    ~Accounts();
    static Accounts* instance();

public:
    // 设置当前账号信息及设置好友列表
    void set_current_account (              std::shared_ptr<ContactData::ItemData>   contact_data );
    void set_contact_list    (const QVector<std::shared_ptr<ContactData::ItemData>>& contact_datas);
    ;       std::shared_ptr<ContactData::ItemData > get_current_account() const;
    QVector<std::shared_ptr<ContactData::ItemData>> get_contact_list   () const;
    // 是否是联系人  // 可添加自己
    bool is_me    (const QString& account_id) const;
    bool is_friend(const QString& account_id) const;
    // 记住密码及自动登陆选项
    std::shared_ptr<AuthRequest> get_auth_request() const;
    void set_account_auth_opt(std::shared_ptr<AuthOpt> auth_opt);
    std::shared_ptr<AuthOpt> get_auth_opt(const QString&account_id) const;

    // 获取指定账号的用户信息
    bool set_contact(std::shared_ptr<ContactData::ItemData> contact_data);
    std::shared_ptr<ContactData::ItemData> get_contact(const QString&account_id) const;

private:
    ;       std::shared_ptr<ContactData::ItemData>  m_current_account = std::make_shared<ContactData::ItemData >();
    QVector<std::shared_ptr<ContactData::ItemData>> m_contact_datas;
    std::map<QString, std::shared_ptr<AuthOpt>> m_map_account_id_auth_opt;
    // std::map<QString, std::map<QString, ContactRequestData::ItemData>> m_map_contact_requests;
    // cache
    std::map<QString, std::shared_ptr<ContactData::ItemData>> m_map_all_contacts;
};

// 登陆附加选项  // 记住密码及自动登陆选项
struct AuthOpt{
    QString account_id;
    QString user_id   ;
    QString password  ;
    QString token     ;
    bool rember_password = false;
    bool auto_login      = false;
    QDateTime last_login;  // 最后登录时间
};

// TODO: 记录加密后的 token 而非密码
class AuthOptRW {
    static QString config_path() {
        static QString config_path = Utils::app_config_path();
        return config_path;
    }
    static QString m_user_section_prefix() { return "account:"; }
public:
    static bool save(std::shared_ptr<AuthOpt> auth_opt) {
        auto section_name = (m_user_section_prefix() + auth_opt->account_id).toStdString();

        Utils::create_dirs(Utils::get_dir(config_path()));  // 确保目录存在

        mINI::INIFile ini_file(config_path().toStdString());
        mINI::INIStructure ini_data;
        ini_file.read(ini_data);
        ini_data[section_name]["account_id"     ] = auth_opt->account_id.toStdString();
        ini_data[section_name]["user_id"        ] = auth_opt->user_id   .toStdString();
        ini_data[section_name]["token"          ] = auth_opt->rember_password ? auth_opt->token     .toStdString() : "";
        ini_data[section_name]["password"       ] = auth_opt->rember_password ? auth_opt->password  .toStdString() : "";
        ini_data[section_name]["rember_password"] = auth_opt->rember_password ? "true" : "false";
        ini_data[section_name]["auto_login"     ] = auth_opt->auto_login      ? "true" : "false";
        ini_data[section_name]["last_login"     ] = QDateTime::currentDateTimeUtc().toLocalTime().toString(Qt::ISODate).toStdString();

        // 当前账号信息置顶
        mINI::INIStructure last_data;
        last_data[section_name] = ini_data[section_name];
        for (auto const& ite_section : ini_data) {
            auto const& section_title = ite_section.first;
            if (section_title == section_name) { continue; }
            last_data[section_title] = ini_data[section_title];
        }

        // return ini_file.write(last_data);  // 保留原始的注释及顺序
        return ini_file.generate(last_data);  // 重新写入内容，不保留原始注释及顺序
    }

    static std::vector<std::shared_ptr<AuthOpt>> load() {
        std::vector<std::shared_ptr<AuthOpt>> auth_opts;

        mINI::INIFile ini_file(config_path().toStdString());
        mINI::INIStructure ini_data;
        ini_file.read(ini_data);

        for (auto const& ite_section : ini_data) {
            auto const& section_title = ite_section.first;
            if(section_title.starts_with(m_user_section_prefix().toUtf8().toStdString().data())) {
                auto auth_opt             = std::make_shared<AuthOpt>();
                auth_opt->account_id      = QString::fromStdString(section_title.substr(m_user_section_prefix().length()));  // 去掉 "user:" 前缀
                auto const& collection    = ite_section.second;
                auth_opt->account_id      = collection.has("account_id"     ) ? QString::fromStdString(collection.get("account_id")) : QString();
                auth_opt->user_id         = collection.has("user_id"        ) ? QString::fromStdString(collection.get("user_id"   )) : QString();
                auth_opt->token           = collection.has("token"          ) ? QString::fromStdString(collection.get("token"     )) : QString();
                auth_opt->password        = collection.has("password"       ) ? QString::fromStdString(collection.get("password"  )) : QString();
                auth_opt->rember_password = collection.has("rember_password") ? collection.get("rember_password") == "true" : false;
                auth_opt->auto_login      = collection.has("auto_login"     ) ? collection.get("auto_login"     ) == "true" : false;
                auth_opt->last_login      = collection.has("last_login"     ) ? QDateTime::fromString(QString::fromStdString(collection.get("last_login")), Qt::ISODate) : QDateTime{};
                auth_opts.push_back(auth_opt);
            }
        }

        return auth_opts;
    }
};
