#include "AuthWindow.h"
#include "ui_AuthWindow.h"
#include <QGrpcChannelOptions>
#include <QGrpcHttp2Channel>


AuthWindow::AuthWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::AuthWindow) {
    ui->setupUi(this);
    connect(ui->signin_button_signup, &QToolButton::clicked, this, [&](){ ui->auth_stacked->setCurrentWidget(ui->page_signup); });
    connect(ui->signup_button_signin, &QToolButton::clicked, this, [&](){ ui->auth_stacked->setCurrentWidget(ui->page_signin); });
    // connect(ui->signin_button_signin, &QToolButton::clicked, this, &AuthWindow::accepted);  // 登录成功显示主界面
    // connect(ui->signup_button_signup, &QToolButton::clicked, this, &AuthWindow::accepted);  // 注册成功直接登录
    connect(ui->signin_button_signin, &QToolButton::clicked, this, &AuthWindow::handle_user_login);  // 登录成功显示主界面
    connect(ui->signup_button_signup, &QToolButton::clicked, this, &AuthWindow::handle_user_register);  // 注册成功直接登录

    // 不知道什么情况，自动生成的翻译有问题，应该为"登陆"却显示为"注册"
    ui->signup_button_signin->setText(ui->signin_button_signin->text());

    if(!NetworkClient::instance().init()) {
        qDebug().nospace().noquote() << "NetworkClient::instance().init() failed.";
    }
}

AuthWindow::~AuthWindow() {
    delete ui;
    ui = nullptr;
}

bool AuthWindow::handle_user_register() {
    QSharedPointer<UserRegisterParameters> params_register = QSharedPointer<UserRegisterParameters>(new UserRegisterParameters);
    params_register->user_id           = ui->signup_edit_username->text();
    params_register->password          = ui->signup_edit_password->text();
    params_register->confirm           = ui->signup_edit_confirm ->text();
    params_register->security_question = ui->signup_edit_question->text();
    params_register->security_answer   = ui->signup_edit_answer  ->text();
    return user_register(params_register);
}

bool AuthWindow::handle_user_login() {
    QSharedPointer<UserLoginParameters> params_login = QSharedPointer<UserLoginParameters>(new UserLoginParameters);
    params_login->user_id = ui->signin_edit_username->text();
    params_login->password = ui->signin_edit_password->text();
    return user_login(params_login);
}

bool AuthWindow::user_register(user_register_params params_register) {
    auto callback = [this](int error_code, const QString&error_message, user_register_params params_register, user_register_reply reply_register){
        if(error_code != 0) {
            qDebug().nospace().noquote() << "user_register error, code : " << error_code << ", message : " << error_message;
            return;
        }
        // 账号注册成功
        handle_user_register_successd(params_register);
    };
    return NetworkClient::instance().user_register(params_register, callback);
}

bool AuthWindow::user_login(QSharedPointer<UserLoginParameters> params_login) {
    qDebug().noquote().nospace() << __FUNCSIG__;
    return false;
}

bool AuthWindow::handle_user_register_successd(QSharedPointer<UserRegisterParameters> params_register) {
    QMessageBox::information(this, tr("tips"), tr("register success. close this dialog to login."));
#ifndef _DEBUG
    // TODO:清除界面数据
#endif // _DEBUG
    QSharedPointer<UserLoginParameters> params_login = QSharedPointer<UserLoginParameters>(new UserLoginParameters);
    params_login->user_id = params_register->user_id;
    params_login->password = params_register->password;
    return user_login(params_login);
}
