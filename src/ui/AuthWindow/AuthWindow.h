#pragma once

#include "headers.h"
#include "Utils.h"
#include "NetworkClient.h"
#include "Accounts.h"

namespace Ui {
class AuthWindow;
}

class AuthWindow : public QDialog{
    Q_OBJECT

public:
    using Client = TaoTieChat::AccountService::Client;

    explicit AuthWindow(QWidget *parent = nullptr);
    ~AuthWindow();

    void view();

private slots:
    void handle_page_signin_tips_error(const QString&message);
    void handle_page_signin_tips_clean();
    void handle_page_signup_tips_error(const QString&message);
    void handle_page_signup_tips_clean();
    //
    bool handle_signup();
    bool handle_signin();
    bool handle_signup_successd(const signup_params&params_signup);
    bool handle_signin_successd(const signin_params& params_signin, const signin_reply& reply_signin, const std::shared_ptr<AuthOpt>& auth_opt);
private:
    bool signup(const signup_params& params_signup);
    bool signin(const signin_params& params_signin, const std::shared_ptr<AuthOpt>& auth_opt);

    void status_signin_unblock(bool unblock = true);
    void status_signup_unblock(bool unblock = true);

private:
    std::shared_ptr<Ui::AuthWindow   > m_ui     ;
    std::unique_ptr<Client           > m_client ;
    std::shared_ptr<QGrpcHttp2Channel> m_channel;
    std::vector<std::shared_ptr<AuthOpt>> m_auth_opts;
};
