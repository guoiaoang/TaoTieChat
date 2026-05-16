#include "AuthWindow.h"
#include "ui_AuthWindow.h"

AuthWindow::AuthWindow(QWidget *parent)
    : QDialog(parent), m_ui(std::make_shared<Ui::AuthWindow>()) {
    m_ui->setupUi(this);
    connect(m_ui->signin_button_signup, &QToolButton::clicked, this, [&](){ m_ui->auth_stacked->setCurrentWidget(m_ui->page_signup); });
    connect(m_ui->signup_button_signin, &QToolButton::clicked, this, [&](){ m_ui->auth_stacked->setCurrentWidget(m_ui->page_signin); });
    // connect(m_ui->signin_button_signin, &QToolButton::clicked, this, &AuthWindow::accepted);  // 登录成功显示主界面
    // connect(m_ui->signup_button_signup, &QToolButton::clicked, this, &AuthWindow::accepted);  // 注册成功直接登录
    connect(m_ui->signup_button_signup, &QToolButton::clicked, this, &AuthWindow::handle_signup);  // 注册成功直接登录
    connect(m_ui->signin_button_signin, &QToolButton::clicked, this, &AuthWindow::handle_signin);  // 登录成功显示主界面
    // 登陆页面清除错误提示
    connect(m_ui->signin_edit_username, &QLineEdit::textChanged, this, &AuthWindow::handle_page_signin_tips_clean);
    connect(m_ui->signin_edit_password, &QLineEdit::textChanged, this, &AuthWindow::handle_page_signin_tips_clean);
    // 注册页面清除错误提示
    connect(m_ui->signup_edit_username, &QLineEdit::textChanged, this, &AuthWindow::handle_page_signup_tips_clean);
    connect(m_ui->signup_edit_password, &QLineEdit::textChanged, this, &AuthWindow::handle_page_signup_tips_clean);
    connect(m_ui->signup_edit_confirm , &QLineEdit::textChanged, this, &AuthWindow::handle_page_signup_tips_clean);
    connect(m_ui->signup_edit_question, &QLineEdit::textChanged, this, &AuthWindow::handle_page_signup_tips_clean);
    connect(m_ui->signup_edit_answer  , &QLineEdit::textChanged, this, &AuthWindow::handle_page_signup_tips_clean);
    // 记住密码与自动登录选项
    connect(m_ui->signin_check_rember, &QCheckBox::checkStateChanged, this, [this](){
        if(!m_ui->signin_check_rember->isChecked()) {
            m_ui->signin_check_auto->setChecked(false);
        }
    });
    connect(m_ui->signin_check_auto  , &QCheckBox::checkStateChanged, this, [this](){
        if(m_ui->signin_check_auto->isChecked()) {
            m_ui->signin_check_rember->setChecked(true);
        }
    });

    // 不知道什么情况，自动生成的翻译有问题，应该为"登陆"却显示为"注册"
    m_ui->signup_button_signin->setText(m_ui->signin_button_signin->text());

    if(!NetworkClient::instance().init()) {
        qDebug().nospace().noquote() << "NetworkClient::instance().init() failed.";
    }

    // 加载登录记录
    auto auth_opts = AuthOptRW::load();
    if(!auth_opts.empty()) {
        // 默认选择第一条记录
        auto auth_opt = auth_opts.front();
        m_ui->signin_edit_username->setText(auth_opt->user_id);
        m_ui->signin_edit_password->setText(auth_opt->password);
        m_ui->signin_check_rember->setChecked(auth_opt->rember_password);
        m_ui->signin_check_auto  ->setChecked(auth_opt->auto_login     );
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
    if(m_ui) { m_ui.reset(); }
}

void AuthWindow::handle_page_signin_tips_error(const QString&message) {
    auto palette = m_ui->signin_label_info->palette();
    palette.setBrush(QPalette::WindowText, QBrush(QColor(Qt::red)));
    m_ui->signin_label_info->setPalette(palette);
    m_ui->signin_label_info->setText(message);
}

void AuthWindow::handle_page_signin_tips_clean() {
    m_ui->signin_label_info->clear();
}

void AuthWindow::handle_page_signup_tips_error(const QString&message) {
    auto palette = m_ui->signup_label_info->palette();
    palette.setBrush(QPalette::WindowText, QBrush(QColor(Qt::red)));
    m_ui->signup_label_info->setPalette(palette);
    m_ui->signup_label_info->setText(message);
}

void AuthWindow::handle_page_signup_tips_clean() {
    m_ui->signup_label_info->clear();
}

bool AuthWindow::handle_signup() {
    signup_params params_signup = std::make_shared<TaoTieChat::SignUpRequest>();
    params_signup->setUserId          (m_ui->signup_edit_username->text());
    params_signup->setPassword        (m_ui->signup_edit_password->text());
    params_signup->setConfirm         (m_ui->signup_edit_confirm ->text());
    params_signup->setSecurityQuestion(m_ui->signup_edit_question->text());
    params_signup->setSecurityAnswer  (m_ui->signup_edit_answer  ->text());
    return signup(params_signup);
}

bool AuthWindow::handle_signin() {
    signin_params params_signin = std::make_shared<TaoTieChat::SignInRequest>();
    TaoTieChat::AuthRequest auth;
    auth.setUserId  (m_ui->signin_edit_username->text());
    auth.setPassword(m_ui->signin_edit_password->text());
    params_signin->setRequestUserAuth(std::move(auth));
    // 传递登录选项，用于保存登录信息
    std::shared_ptr<AuthOpt> auth_opt = std::make_shared<AuthOpt>();
    auth_opt->user_id         = m_ui->signin_edit_username->text     ();
    auth_opt->password        = m_ui->signin_edit_password->text     ();
    auth_opt->rember_password = m_ui->signin_check_rember ->isChecked();
    auth_opt->auto_login      = m_ui->signin_check_auto   ->isChecked();
    return signin(params_signin, auth_opt);
}

bool AuthWindow::signup(const signup_params& params_signup) {
    status_signup_unblock(false);
    auto callback = [this](const signup_params& params_signup, const signup_reply& reply_signup){
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

bool AuthWindow::signin(const signin_params& params_signin, const std::shared_ptr<AuthOpt>& auth_opt) {
    // 1. 执行操作前禁用按钮
    // 2. 执行失败后立即解除禁用状态
    // 3. 执行成功时不在这里进行状态解除，防止登陆过程过快导致用户误以为窗口闪烁的问题，便于用户看到登陆过程
    status_signin_unblock(false);
    auto callback = [this, auth_opt](const signin_params& params_signin, const signin_reply& reply_signin) {
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

bool AuthWindow::handle_signup_successd(const signup_params& params_signup) {
    QMessageBox::information(this, tr("tips"), tr("SignUp success. close this dialog to SignIn."));

    // TODO:清除界面数据

    // 创建用于登陆的参数  // 注册返回的信息不包含 token，因此需要使用密码登录
    signin_params params_signin = std::make_shared<TaoTieChat::SignInRequest>();
    TaoTieChat::AuthRequest auth;
    auth.setUserId  (params_signup->userId  ());
    auth.setPassword(params_signup->password());
    params_signin->setRequestUserAuth(std::move(auth));
    return signin(params_signin, std::make_shared<AuthOpt>(AuthOpt{
        .user_id         = params_signup->userId  (),
        .password        = params_signup->password(),
        .rember_password = false,
        .auto_login      = false,
    }));
}

bool AuthWindow::handle_signin_successd(const signin_params& params_signin, const signin_reply& reply_signin, const std::shared_ptr<AuthOpt>& auth_opt) {
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

    // 将用户信息保存到全局状态中
    std::shared_ptr<ContactInfo> contact_info = std::make_shared<ContactInfo>();
    ContactInfoBase base_info;
    base_info.setAccountId(reply_signin->accountId());
    base_info.setUserId   (auth.userId());
    contact_info->setBaseInfo(base_info);
    ContactInfoSelf self_info;
    self_info.setPassword(auth.password());
    self_info.setToken   (reply_signin->token());
    contact_info->setSelfInfo(self_info);
    Accounts::instance().set_current_account(contact_info);

    // 实际上登陆消息瞬间完成，延迟 1 秒再显示主界面，防止用户误以为登陆窗口闪烁的错误判断
    QTimer::singleShot(1000, this, [this]() {
        status_signin_unblock(true);
        this->accepted();
    });
    return true;
}

void AuthWindow::status_signin_unblock(bool unblock) {
    m_ui->signin_edit_username->setEnabled(unblock);
    m_ui->signin_edit_password->setEnabled(unblock);
    m_ui->signin_check_rember ->setEnabled(unblock);
    m_ui->signin_check_auto   ->setEnabled(unblock);
    m_ui->signin_button_signin->setEnabled(unblock);
    if(unblock) {
        handle_page_signin_tips_clean();
    }
    else {
        handle_page_signin_tips_error(tr("logining..."));
    }
}

void AuthWindow::status_signup_unblock(bool unblock) {
    m_ui->signup_edit_username->setEnabled(unblock);
    m_ui->signup_edit_password->setEnabled(unblock);
    m_ui->signup_edit_confirm ->setEnabled(unblock);
    m_ui->signup_edit_question->setEnabled(unblock);
    m_ui->signup_edit_answer  ->setEnabled(unblock);
    m_ui->signup_button_signup->setEnabled(unblock);
    if(unblock) {
        handle_page_signup_tips_clean();
    }
    else {
        handle_page_signup_tips_error(tr("signing up..."));
    }
}
