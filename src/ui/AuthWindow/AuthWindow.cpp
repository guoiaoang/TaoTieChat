#include "AuthWindow.h"
#include "ui_AuthWindow.h"
#include <QGrpcChannelOptions>
#include <QGrpcHttp2Channel>
#include <QCheckBox>

AuthWindow::AuthWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::AuthWindow) {
    ui->setupUi(this);
    connect(ui->signin_button_signup, &QToolButton::clicked, this, [&](){ ui->auth_stacked->setCurrentWidget(ui->page_signup); });
    connect(ui->signup_button_signin, &QToolButton::clicked, this, [&](){ ui->auth_stacked->setCurrentWidget(ui->page_signin); });
    // connect(ui->signin_button_signin, &QToolButton::clicked, this, &AuthWindow::accepted);  // 登录成功显示主界面
    // connect(ui->signup_button_signup, &QToolButton::clicked, this, &AuthWindow::accepted);  // 注册成功直接登录
    connect(ui->signup_button_signup, &QToolButton::clicked, this, &AuthWindow::handle_signup);  // 注册成功直接登录
    connect(ui->signin_button_signin, &QToolButton::clicked, this, &AuthWindow::handle_signin);  // 登录成功显示主界面
    // 登陆页面清除错误提示
    connect(ui->signin_edit_username, &QLineEdit::textChanged, this, &AuthWindow::handle_page_signin_tips_clean);
    connect(ui->signin_edit_password, &QLineEdit::textChanged, this, &AuthWindow::handle_page_signin_tips_clean);
    // 注册页面清除错误提示
    connect(ui->signup_edit_username, &QLineEdit::textChanged, this, &AuthWindow::handle_page_signup_tips_clean);
    connect(ui->signup_edit_password, &QLineEdit::textChanged, this, &AuthWindow::handle_page_signup_tips_clean);
    connect(ui->signup_edit_confirm , &QLineEdit::textChanged, this, &AuthWindow::handle_page_signup_tips_clean);
    connect(ui->signup_edit_question, &QLineEdit::textChanged, this, &AuthWindow::handle_page_signup_tips_clean);
    connect(ui->signup_edit_answer  , &QLineEdit::textChanged, this, &AuthWindow::handle_page_signup_tips_clean);
    // 记住密码与自动登录选项
    connect(ui->signin_check_rember, &QCheckBox::checkStateChanged, this, [this](){
        if(!ui->signin_check_rember->isChecked()) {
            ui->signin_check_auto->setChecked(false);
        }
    });
    connect(ui->signin_check_auto  , &QCheckBox::checkStateChanged, this, [this](){
        if(ui->signin_check_auto->isChecked()) {
            ui->signin_check_rember->setChecked(true);
        }
    });

    // 不知道什么情况，自动生成的翻译有问题，应该为"登陆"却显示为"注册"
    ui->signup_button_signin->setText(ui->signin_button_signin->text());

    if(!NetworkClient::instance().init()) {
        qDebug().nospace().noquote() << "NetworkClient::instance().init() failed.";
    }

    // 暂时禁用/隐藏未完整实现的功能
    ui->signin_check_rember->setEnabled(false);
    ui->signin_check_auto  ->setEnabled(false);
    ui->signin_check_rember->hide();
    ui->signin_check_auto  ->hide();
}

AuthWindow::~AuthWindow() {
    if(ui) {
        delete ui;
        ui = nullptr;
    }
}

void AuthWindow::handle_page_signin_tips_error(const QString&message) {
    auto palette = ui->signin_label_info->palette();
    palette.setBrush(QPalette::WindowText, QBrush(QColor(Qt::red)));
    ui->signin_label_info->setPalette(palette);
    ui->signin_label_info->setText(message);
}

void AuthWindow::handle_page_signin_tips_clean() {
    ui->signin_label_info->clear();
}

void AuthWindow::handle_page_signup_tips_error(const QString&message) {
    auto palette = ui->signin_label_info->palette();
    palette.setBrush(QPalette::WindowText, QBrush(QColor(Qt::red)));
    ui->signup_label_info->setPalette(palette);
    ui->signup_label_info->setText(message);
}

void AuthWindow::handle_page_signup_tips_clean() {
    ui->signup_label_info->clear();
}

bool AuthWindow::handle_signup() {
    signup_params params_signup = std::make_shared<TaoTieChat::SignUpRequest>();
    params_signup->setUserId          (ui->signup_edit_username->text());
    params_signup->setPassword        (ui->signup_edit_password->text());
    params_signup->setConfirm         (ui->signup_edit_confirm ->text());
    params_signup->setSecurityQuestion(ui->signup_edit_question->text());
    params_signup->setSecurityAnswer  (ui->signup_edit_answer  ->text());
    return signup(params_signup);
}

bool AuthWindow::handle_signin() {
    signin_params params_signin = std::make_shared<TaoTieChat::SignInRequest>();
    params_signin->setUserId  (ui->signin_edit_username->text());
    params_signin->setPassword(ui->signin_edit_password->text());
    // 传递登录选项，用于保存登录信息
    std::shared_ptr<AuthOpt> auth_opt = std::make_shared<AuthOpt>();
    auth_opt->rember_password = ui->signin_check_rember->isChecked();
    auth_opt->auto_login      = ui->signin_check_auto  ->isChecked();
    return signin(params_signin, auth_opt);
}

bool AuthWindow::signup(signup_params params_signup) {
    auto callback = [this](signup_params params_signup, signup_reply reply_signup){
        // 账号注册成功
        if(reply_signup && reply_signup->hasResult() && reply_signup->result().hasSuccessCode()) {
            handle_signup_successd(params_signup);
            return;
        }
        // 账号注册失败
        QString error_info = tr("login failed!");
        if(reply_signup && reply_signup->hasResult() && reply_signup->result().hasErrorCode()) {
            auto code = reply_signup->result().errorCode();
            error_info = reply_signup->result().message();
        }
        handle_page_signup_tips_error(error_info);
    };
    return NetworkClient::instance().signup(params_signup, callback);
}

bool AuthWindow::signin(signin_params params_signin, std::shared_ptr<AuthOpt> auth_opt) {
    auto callback = [this, auth_opt](signin_params params_signin, signin_reply reply_signin) {
        // 用户登录成功
        if(reply_signin && reply_signin->hasResult() && reply_signin->result().hasSuccessCode()) {
            handle_signin_successd(params_signin, auth_opt);
            return;
        }
        // 用户登录失败
        QString error_info = tr("login failed!");
        if(reply_signin && reply_signin->hasResult() && reply_signin->result().hasErrorCode()) {
            auto code = reply_signin->result().errorCode();
            error_info = reply_signin->result().message();
        }
        handle_page_signin_tips_error(error_info);
    };
    return NetworkClient::instance().signin(params_signin, callback);
}

bool AuthWindow::handle_signup_successd(signup_params params_signup) {
    QMessageBox::information(this, tr("tips"), tr("SignUp success. close this dialog to SignIn."));

#ifndef _DEBUG
    // TODO:清除界面数据
#endif // _DEBUG

    // 创建用于登陆的参数  // 注册返回的信息不包含 token，因此需要使用密码登录
    signin_params params_signin = std::make_shared<TaoTieChat::SignInRequest>();
    params_signin->setUserId  (params_signup->userId  ());
    params_signin->setPassword(params_signup->password());
    return signin(params_signin);
}

bool AuthWindow::handle_signin_successd(signin_params params_signin, std::shared_ptr<AuthOpt> auth_opt) {
#ifndef DEBUG
    QMessageBox::information(this, tr("tips"), tr("login success. \nclose this dialog to show chat window."));
    // TODO:清除界面数据
#endif // _DEBUG

    // 主界面可能需要的参数
    // QString user_id    = reply_login->request().userId();
    // QString account_id = reply_login->accountId();
    // QString token      = reply_login->token    ();
    // qDebug().nospace().noquote() << "login success. reply:" << reply_login;

    if(auth_opt && auth_opt->rember_password) {
        // 保存登录记录
    }

    // this, &AuthWindow::accepted
    this->accepted();
    return true;
}
