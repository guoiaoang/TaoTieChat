#pragma once

#include "Headers.h"
#include "data.h"
#include "Utils.h"
#include "NetworkClient.h"
#include "Accounts.h"

#include <mini/ini.h>   // cpp

namespace Ui {
class AuthWindow;
}

// 附加选项
struct AuthOpt{
    QString user_id;
    QString password;
    QString token;
    bool rember_password = false;
    bool auto_login = false;
};

class AuthOptRW {
    static QString config_path() {
        static QString config_path = Utils::app_config_path();
        return config_path;
    }
public:
    static bool save(const std::shared_ptr<AuthOpt> & auth_opt) {
        auto user_section = "user_" + auth_opt->user_id;
        auto user_section_str = user_section.toStdString();
        auto user_section_sz = user_section_str.c_str();

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
            if(section.starts_with("user_")) {
                auto auth_opt = std::make_shared<AuthOpt>();
                auth_opt->user_id = QString::fromStdString(section.substr(5));  // 去掉 "user_" 前缀
                auto const& collection = ite_section.second;
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

class AuthWindow : public QDialog{
    Q_OBJECT

public:
    using Client = TaoTieChat::AccountService::Client;

    explicit AuthWindow(QWidget *parent = nullptr);
    ~AuthWindow();

private slots:
    void handle_page_signin_tips_error(const QString&message);
    void handle_page_signin_tips_clean();
    void handle_page_signup_tips_error(const QString&message);
    void handle_page_signup_tips_clean();
    //
    bool handle_signup();
    bool handle_signin();
    bool handle_signup_successd(signup_params params_signup);
    bool handle_signin_successd(signin_params params_signin, signin_reply reply_signin, std::shared_ptr<AuthOpt> auth_opt = nullptr);  // nullptr 不能使用引用
private:
    bool signup(signup_params params_signup);
    bool signin(signin_params params_signin, std::shared_ptr<AuthOpt> auth_opt = nullptr);

    void status_signin_block();
    void status_signin_unblock();
    void status_signup_block();
    void status_signup_unblock();

private:
    Ui::AuthWindow *ui = nullptr;
    std::unique_ptr<Client> m_client;
    std::shared_ptr<QGrpcHttp2Channel> m_channel;
};
