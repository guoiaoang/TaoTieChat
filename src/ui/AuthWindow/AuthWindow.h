#pragma once

#include "Headers.h"
#include "Utils.h"
#include "NetworkClient.h"

namespace Ui {
class AuthWindow;
}

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
    bool handle_signin_successd(signin_params params_signin);
private:
    bool signup(signup_params params_signup);
    bool signin(signin_params params_signin);

private:
    Ui::AuthWindow *ui = nullptr;
    std::unique_ptr<Client> m_client;
    std::shared_ptr<QGrpcHttp2Channel> m_channel;
};
