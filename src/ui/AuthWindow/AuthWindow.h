#pragma once

#include "headers.h"
#include "Utils.h"
#include "Defines.h"

struct AuthOpt;  // Accounts.h

namespace Ui {
class AuthWindow;
}

class AuthWindow : public QDialog {
    Q_OBJECT

public:
    enum class TipsLevel : short {
        CLEAN       = 0,
        SUCCEEDED   = 1,
        INFORMATION = 2,
        FAILED      = 3,  // ERROR 宏与 Windows SKD 中的定义冲突
    };
    Q_ENUM(TipsLevel)

public:
    explicit AuthWindow(QWidget *parent = nullptr);
    ~AuthWindow();

public:
    void view();

private slots:
    void slot_page_signin_tips_show(TipsLevel level, const QString&message = QString{});
    void slot_page_signin_tips_clean();
    void slot_page_signup_tips_show(TipsLevel level, const QString&message = QString{});
    void slot_page_signup_tips_clean();
    //
    bool slot_signup();
    bool slot_signin();
    bool slot_signup_successd(signup_params params_signup);
    bool slot_signin_successd(signin_params params_signin, signin_reply reply_signin, std::shared_ptr<AuthOpt> auth_opt);
    // 账号信息修改时更新头像 ID
    void slot_account_changed(std::shared_ptr<ContactData::ItemData> contact_data);

private:
    bool signup(signup_params params_signup);
    bool signin(signin_params params_signin, std::shared_ptr<AuthOpt> auth_opt);

    void status_signin_unblock(bool unblock = true);
    void status_signup_unblock(bool unblock = true);

    void clear_login_info();
    // 重新读取配置并设置到页面  // auto_login 是否执行自动登陆
    void reload_login_config(bool auto_login = true);

private:
    std::shared_ptr<Ui::AuthWindow   > m_ui     ;
    std::unique_ptr<AccountClient    > m_client ;
    std::shared_ptr<QGrpcHttp2Channel> m_channel;
    std::vector<std::shared_ptr<AuthOpt>> m_auth_opts;
};
