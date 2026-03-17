#pragma once

#include "Headers.h"
#include "Utils.h"
#include "NetworkClient.h"
// #include <QGrpcHttp2Channel>

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
    bool handle_user_register();
    bool handle_user_login();
    bool handle_user_register_successd(QSharedPointer<UserRegisterParameters> params_register);
private:
    bool user_register(QSharedPointer<UserRegisterParameters> params_register);
    bool user_login(QSharedPointer<UserLoginParameters> params_login);

private:
    Ui::AuthWindow *ui = nullptr;
    std::unique_ptr<Client> m_client;
    std::shared_ptr<QGrpcHttp2Channel> m_channel;
};
