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
    bool handle_user_register();
    bool handle_user_login   ();
    bool handle_user_register_successd(user_register_params params_register);
    bool handle_user_login_successd   (user_login_reply     peply_login    );
private:
    bool user_register(user_register_params params_register);
    bool user_login   (user_login_params    params_login   );

private:
    Ui::AuthWindow *ui = nullptr;
    std::unique_ptr<Client> m_client;
    std::shared_ptr<QGrpcHttp2Channel> m_channel;
};
