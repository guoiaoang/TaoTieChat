#include "AuthWindow.h"
#include "ui_AuthWindow.h"

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
    // ui->signin_check_rember->setEnabled(false);
    // ui->signin_check_auto  ->setEnabled(false);
    // ui->signin_check_rember->hide();
    // ui->signin_check_auto  ->hide();

    // 加载登录记录
    auto auth_opts = AuthOptRW::load();
    if(!auth_opts.empty()) {
        // 默认选择第一条记录
        auto auth_opt = auth_opts.front();
        ui->signin_edit_username->setText(auth_opt->user_id);
        ui->signin_edit_password->setText(auth_opt->password);
        ui->signin_check_rember->setChecked(auth_opt->rember_password);
        ui->signin_check_auto  ->setChecked(auth_opt->auto_login     );
        if(auth_opt->auto_login) {
            // 自动登录
            signin_params params_signin = std::make_shared<TaoTieChat::SignInRequest>();
            TaoTieChat::AuthRequest auth;
            auth.setUserId  (auth_opt->user_id);
            auth.setPassword(auth_opt->password);
            // auth.setToken   (auth_opt->token  );
            params_signin->setRequestUserAuth(std::move(auth));
            signin(params_signin, auth_opt);
        }
    }
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
    TaoTieChat::AuthRequest auth;
    auth.setUserId  (ui->signin_edit_username->text());
    auth.setPassword(ui->signin_edit_password->text());
    params_signin->setRequestUserAuth(std::move(auth));
    // 传递登录选项，用于保存登录信息
    std::shared_ptr<AuthOpt> auth_opt = std::make_shared<AuthOpt>();
    auth_opt->user_id         = ui->signin_edit_username->text     ();
    auth_opt->rember_password = ui->signin_check_rember ->isChecked();
    auth_opt->auto_login      = ui->signin_check_auto   ->isChecked();
    return signin(params_signin, auth_opt);
}

bool AuthWindow::signup(signup_params params_signup) {
    status_signup_unblock(false);
    auto callback = [this](signup_params params_signup, signup_reply reply_signup){
        status_signup_unblock(true);
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
    // 1. 执行操作前禁用按钮
    // 2. 执行失败后立即解除禁用状态
    // 3. 执行成功时不在这里进行状态解除，防止登陆过程过快导致用户误以为窗口闪烁的问题，便于用户看到登陆过程
    status_signin_unblock(false);
    auto callback = [this, auth_opt](signin_params params_signin, signin_reply reply_signin) {
        // 用户登录成功
        if(reply_signin && reply_signin->hasResult() && reply_signin->result().hasSuccessCode()) {
            handle_signin_successd(params_signin, reply_signin, auth_opt);
            return;
        }

        // 用户登录失败
        status_signin_unblock(true);
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

    // TODO:清除界面数据

    // 创建用于登陆的参数  // 注册返回的信息不包含 token，因此需要使用密码登录
    signin_params params_signin = std::make_shared<TaoTieChat::SignInRequest>();
    TaoTieChat::AuthRequest auth;
    auth.setUserId  (params_signup->userId  ());
    auth.setPassword(params_signup->password());
    params_signin->setRequestUserAuth(std::move(auth));
    return signin(params_signin);
}

bool AuthWindow::handle_signin_successd(signin_params params_signin, signin_reply reply_signin, std::shared_ptr<AuthOpt> auth_opt) {
    // 主界面可能需要的参数
    // QString user_id    = reply_login->request().userId();
    // QString account_id = reply_login->accountId();
    // QString token      = reply_login->token    ();
    // qDebug().nospace().noquote() << "login success. reply:" << reply_login;

    if(auth_opt) {
        // auth_opt->user_id = params_signin->requestUserAuth().userId();
        auth_opt->token      = auth_opt->rember_password ? reply_signin->token() : QString();
        auth_opt->password   = auth_opt->rember_password ? params_signin->requestUserAuth().password() : QString();
        // 保存登录记录
        AuthOptRW::save(auth_opt);
    }

    const TaoTieChat::AuthRequest auth = (params_signin && params_signin->hasRequestUserAuth())
                                         ? params_signin->requestUserAuth()
                                         : TaoTieChat::AuthRequest();

    ContactManager cm(ContactManager::create());
    cm.set_user_id   (auth.hasUserId  () ? auth.userId  () : QString());
    cm.set_password  (auth.hasPassword() ? auth.password() : QString());
    cm.set_account_id(reply_signin->accountId());
    cm.set_token     (reply_signin ->token   ());
    cm.set_contact   (cm.contact             ());
    Accounts::instance().set_current_account(cm.contact());

    // 实际上登陆消息瞬间完成，延迟 1 秒再显示主界面，防止用户误以为登陆窗口闪烁的错误判断
    QTimer::singleShot(1000, this, [this]() {
        status_signin_unblock(true);
        this->accepted();
    });
    return true;
}

void AuthWindow::status_signin_unblock(bool unblock) {
    ui->signin_edit_username->setEnabled(unblock);
    ui->signin_edit_password->setEnabled(unblock);
    ui->signin_check_rember ->setEnabled(unblock);
    ui->signin_check_auto   ->setEnabled(unblock);
    ui->signin_button_signin->setEnabled(unblock);
    if(unblock) {
        handle_page_signin_tips_clean();
    }
    else {
        handle_page_signin_tips_error(tr("logining..."));
    }
}

void AuthWindow::status_signup_unblock(bool unblock) {
    ui->signup_edit_username->setEnabled(unblock);
    ui->signup_edit_password->setEnabled(unblock);
    ui->signup_edit_confirm ->setEnabled(unblock);
    ui->signup_edit_question->setEnabled(unblock);
    ui->signup_edit_answer  ->setEnabled(unblock);
    ui->signup_button_signup->setEnabled(unblock);
    if(unblock) {
        handle_page_signup_tips_clean();
    }
    else {
        handle_page_signup_tips_error(tr("signing up..."));
    }
}
