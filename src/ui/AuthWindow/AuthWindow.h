#pragma once

#include "Headers.h"
#include "data.h"
#include "Utils.h"
#include "NetworkClient.h"
#include "Accounts.h"

namespace Ui {
class AuthWindow;
}

// 附加选项
struct AuthOpt{
    bool rember_password = false;
    bool auto_login = false;
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

private:
    Ui::AuthWindow *ui = nullptr;
    std::unique_ptr<Client> m_client;
    std::shared_ptr<QGrpcHttp2Channel> m_channel;
};
