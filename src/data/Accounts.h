#pragma once

#include "headers.h"
#include "Utils.h"
#include "Defines.h"

struct AuthOpt;

class Accounts : public QObject{
    Q_OBJECT
private:
    Accounts();
    Accounts(const Accounts&) = default;
    Accounts& operator=(const Accounts&) = default;
public:
    ~Accounts();
    static Accounts& instance();

public:
    // 设置当前账号信息及设置好友列表
    void set_current_account (const std::shared_ptr<ContactInfo >& contact_info );
    void set_contact_list    (const std::shared_ptr<ContactInfos>& contact_infos);
    std::shared_ptr<ContactInfo > get_current_account() const;
    std::shared_ptr<ContactInfos> get_contact_list   () const;
    std::shared_ptr<AuthRequest > get_auth_request   () const;
    // 是否是联系人  // 可添加自己
    bool is_self   (const std::shared_ptr<ContactInfo >& contact_info) const;
    bool is_contact(const std::shared_ptr<ContactInfo >& contact_info) const;
    // 记住密码及自动登陆选项
    void set_account_auth_opt(const QString&account_id, const std::shared_ptr<AuthOpt>& auth_opt);
    std::shared_ptr<AuthOpt     > get_auth_opt       (const QString&account_id) const;

signals:
    void signal_account_changing(const std::shared_ptr<ContactInfo >& contact_info );
    void signal_account_changed ();
    void signal_contact_list_changing(const std::shared_ptr<ContactInfos>& contact_infos);
    void signal_contact_list_changed ();
    // void signal_contact_changed  (const std::shared_ptr<ContactInfo >& contact_info );

private:
    std::shared_ptr<ContactInfo>  m_current_account = std::make_shared<ContactInfo >();
    std::shared_ptr<ContactInfos> m_contact_infos   = std::make_shared<ContactInfos>();
    std::map<QString, std::shared_ptr<AuthOpt>> m_map_account_id_auth_opt;
};

// 登陆附加选项  // 记住密码及自动登陆选项
struct AuthOpt{
    QString user_id ;
    QString password;
    QString token   ;
    bool rember_password = false;
    bool auto_login      = false;
};

class AuthOptRW {
    static QString config_path() {
        static QString config_path = Utils::app_config_path();
        return config_path;
    }
    static QString m_username_prefix() { return "user:"; }
public:
    static bool save(const std::shared_ptr<AuthOpt> & auth_opt) {
        auto user_section     = m_username_prefix() + auth_opt->user_id;
        auto user_section_str = user_section.toStdString();
        auto user_section_sz  = user_section_str.c_str();

        Utils::create_dirs(Utils::get_dir(config_path()));  // 确保目录存在

        mINI::INIFile file(config_path().toStdString());
        mINI::INIStructure ini;
        file.read(ini);
        ini[user_section_sz]["token"          ] = auth_opt->token   .toStdString();
        ini[user_section_sz]["password"       ] = auth_opt->password.toStdString();
        ini[user_section_sz]["rember_password"] = auth_opt->rember_password ? "true" : "false";
        ini[user_section_sz]["auto_login"     ] = auth_opt->auto_login      ? "true" : "false";
        file.write(ini);
        return true;
    }

    static std::vector<std::shared_ptr<AuthOpt>> load() {
        std::vector<std::shared_ptr<AuthOpt>> auth_opts;

        mINI::INIFile file(config_path().toStdString());
        mINI::INIStructure ini;
        file.read(ini);

        for (auto const& ite_section : ini) {
            auto const& section    = ite_section.first;
            if(section.starts_with(m_username_prefix().toUtf8().toStdString().data())) {
                auto auth_opt             = std::make_shared<AuthOpt>();
                auth_opt->user_id         = QString::fromStdString(section.substr(5));  // 去掉 "user:" 前缀
                auto const& collection    = ite_section.second;
                auth_opt->token           = collection.has("token"          ) ? QString::fromStdString(collection.get("token")) : QString();
                auth_opt->password        = collection.has("password"       ) ? QString::fromStdString(collection.get("password")) : QString();
                auth_opt->rember_password = collection.has("rember_password") ? collection.get("rember_password") == "true" : false;
                auth_opt->auto_login      = collection.has("auto_login"     ) ? collection.get("auto_login"     ) == "true" : false;
                auth_opts.push_back(auth_opt);
            }
        }

        return auth_opts;
    }
};
