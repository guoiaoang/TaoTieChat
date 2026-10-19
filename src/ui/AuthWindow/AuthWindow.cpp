#include "AuthWindow.h"
#include "ui_AuthWindow.h"

#include "NetworkClient.h"
#include "Accounts.h"
#include "Database.h"
#include "SignalInner.h"

AuthWindow::AuthWindow(QWidget *parent)
    : QDialog(parent), m_ui(std::make_shared<Ui::AuthWindow>()) {
    m_ui->setupUi(this);
    connect(m_ui->signin_button_signup, &QToolButton::clicked, this, [&](){ m_ui->auth_stacked->setCurrentWidget(m_ui->page_signup); });
    connect(m_ui->signup_button_signin, &QToolButton::clicked, this, [&](){ m_ui->auth_stacked->setCurrentWidget(m_ui->page_signin); });
    // connect(m_ui->signin_button_signin, &QToolButton::clicked, this, &AuthWindow::accepted);  // 登录成功显示主界面
    // connect(m_ui->signup_button_signup, &QToolButton::clicked, this, &AuthWindow::accepted);  // 注册成功直接登录
    connect(m_ui->signup_button_signup, &QToolButton::clicked, this, &AuthWindow::slot_signup);  // 注册成功直接登录
    connect(m_ui->signin_button_signin, &QToolButton::clicked, this, &AuthWindow::slot_signin);  // 登录成功显示主界面
    // 登陆页面清除错误提示
    connect(m_ui->signin_combo_username, &QComboBox::currentTextChanged, this, &AuthWindow::slot_page_signin_tips_clean);
    connect(m_ui->signin_edit_password , &QLineEdit::textChanged       , this, &AuthWindow::slot_page_signin_tips_clean);
    // 注册页面清除错误提示
    connect(m_ui->signup_edit_username, &QLineEdit::textChanged, this, &AuthWindow::slot_page_signup_tips_clean);
    connect(m_ui->signup_edit_password, &QLineEdit::textChanged, this, &AuthWindow::slot_page_signup_tips_clean);
    connect(m_ui->signup_edit_confirm , &QLineEdit::textChanged, this, &AuthWindow::slot_page_signup_tips_clean);
    connect(m_ui->signup_edit_question, &QLineEdit::textChanged, this, &AuthWindow::slot_page_signup_tips_clean);
    connect(m_ui->signup_edit_answer  , &QLineEdit::textChanged, this, &AuthWindow::slot_page_signup_tips_clean);
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
    connect(m_ui->signin_combo_username, &QComboBox::currentTextChanged, this, [this](const QString& currentText){
        for(const auto& auth_opt : m_auth_opts) {
            if(auth_opt->user_id == currentText) {
                m_ui->signin_edit_password ->setText   (auth_opt->password       );
                m_ui->signin_check_rember  ->setChecked(auth_opt->rember_password);
                m_ui->signin_check_auto    ->setChecked(auth_opt->auto_login     );
                // break;
                return;  // 用户名唯一，找到第一条记录后直接返回
            }
        }
        // 没有找到记录，清空界面数据
        m_ui->signin_edit_password ->clear();
        m_ui->signin_check_rember ->setChecked(false);
        m_ui->signin_check_auto   ->setChecked(false);
    });
    connect(m_ui->auth_stacked, &QStackedWidget::currentChanged, this, [this](int index){
        ;    if(m_ui->auth_stacked->widget(index) == m_ui->page_signin) { setWindowTitle(tr("Sign In")); }
        else if(m_ui->auth_stacked->widget(index) == m_ui->page_signup) { setWindowTitle(tr("Sign Up")); }
    });

    m_ui->signin_combo_username->lineEdit()->setPlaceholderText(m_ui->signin_combo_username->placeholderText());
    m_ui->signin_combo_username->lineEdit()->setClearButtonEnabled(true);
    // 编译时需要选择 `ALL_BUILD` 来触发 qm 文件的完全重新生成，否则可能会仅复制原先的内容而没有实际更新
    // // ~~不知道什么情况，自动生成的翻译有问题，应该为"登陆"却显示为"注册"~~
    // m_ui->signup_button_signin->setText(m_ui->signin_button_signin->text());

    // 设置信息提示的 Label 隐藏时依然保留大小占位，防止显示信息与清空信息显示时造成的其他控件位置的变化
    auto sizePolicy = m_ui->signin_label_info->sizePolicy();
    sizePolicy.setRetainSizeWhenHidden(true);
    m_ui->signin_label_info->setSizePolicy(sizePolicy);
    m_ui->signin_label_info->setVisible(false); // 初始状态下先隐藏

    if(!NetworkClient::instance()->init()) {
        qDebug().nospace().noquote() << "NetworkClient::instance()->init() failed.";
    }

    // 加载登录记录
    m_auth_opts = AuthOptRW::load();
    for(auto const& auth_opt : m_auth_opts) {
        m_ui->signin_combo_username->addItem(auth_opt->user_id);
    }
    m_ui->signin_combo_username->setCurrentIndex(0);

    // 执行自动登陆
    for(auto const& auth_opt : m_auth_opts) {
        if(auth_opt->auto_login) {
            // 自动登录
            signin_params params_signin = std::make_shared<TaoTieChat::SignInRequest>();
            TaoTieChat::AuthRequest auth;
            auth.setUserId  (auth_opt->user_id );
            auth.setPassword(auth_opt->password);
            // auth.setToken(auth_opt->token   );
            params_signin->setRequestUserAuth(std::move(auth));
            // 修改界面显示
            m_ui->signin_combo_username->setCurrentText(auth_opt->user_id );
            // 执行操作
            signin(params_signin, auth_opt);
            break;  // 只自动登录第一条记录
        }
    }
}

AuthWindow::~AuthWindow() {
}

void AuthWindow::clear_login_info() {
    m_ui->signin_combo_username->clear();
}

void AuthWindow::reload_login_config(bool auto_login) {
    // 1. 清除页面数据
    clear_login_info();
    // 加载登录记录
    m_auth_opts = AuthOptRW::load();
    for(auto const& auth_opt : m_auth_opts) {
        m_ui->signin_combo_username->addItem(auth_opt->user_id);
    }
    m_ui->signin_combo_username->setCurrentIndex(0);

    // 执行自动登陆
    for(auto const& auth_opt : m_auth_opts) {
        if(auth_opt->auto_login && auto_login) {
            // 自动登录
            signin_params params_signin = std::make_shared<TaoTieChat::SignInRequest>();
            TaoTieChat::AuthRequest auth;
            auth.setUserId  (auth_opt->user_id );
            auth.setPassword(auth_opt->password);
            // auth.setToken(auth_opt->token   );
            params_signin->setRequestUserAuth(std::move(auth));
            // 修改界面显示
            m_ui->signin_combo_username->setCurrentText(auth_opt->user_id );
            // 执行操作
            signin(params_signin, auth_opt);
            break;  // 只自动登录第一条记录
        }
    }
}

void AuthWindow::view() {
    static Utils::FontIcons font_icon =  Utils::FontIcons().update();
    QApplication::setWindowIcon(font_icon.icon.profile);
    // 新用户从注册页面直接登陆时又点击了记住密码，此时需要重置页面为登陆页面及加载登陆配置，然后才能显示
    m_ui->auth_stacked->setCurrentWidget(m_ui->page_signin);
    reload_login_config(false);
    show();
}

void AuthWindow::slot_page_signin_tips_show(TipsLevel level, const QString&message) {
    // clear() 会导致布局调整，设置为空字符串则不会
    // if (level == TipsLevel::CLEAN) { m_ui->signin_label_info->clear(); return; }

    auto tips_color = QColor(Qt::green);
    ;    if (level == TipsLevel::SUCCEEDED  ) { tips_color = QColor(Qt::green     ); }
    else if (level == TipsLevel::INFORMATION) { tips_color = QColor(Qt::darkYellow); }
    else if (level == TipsLevel::FAILED     ) { tips_color = QColor(Qt::red       ); }

    auto palette = m_ui->signin_label_info->palette();
    palette.setBrush(QPalette::WindowText, QBrush(tips_color));
    m_ui->signin_label_info->setPalette(palette);
    m_ui->signin_label_info->setText(level == TipsLevel::CLEAN ? QString{} : message);
    m_ui->signin_label_info->setVisible(true);  // 确保显示
}

void AuthWindow::slot_page_signin_tips_clean() {
    // m_ui->signin_label_info->clear();
    // m_ui->signin_label_info->setVisible(false);  // 需要在初始化时设置为隐藏时不移除空间占用
    slot_page_signin_tips_show(TipsLevel::CLEAN);
}

void AuthWindow::slot_page_signup_tips_show(TipsLevel level, const QString&message) {
    // clear() 会导致布局调整，设置为空字符串则不会
    // if (level == TipsLevel::CLEAN) { m_ui->signup_label_info->clear(); return; }

    auto tips_color = QColor(Qt::green);
    ;    if (level == TipsLevel::SUCCEEDED  ) { tips_color = QColor(Qt::green     ); }
    else if (level == TipsLevel::INFORMATION) { tips_color = QColor(Qt::darkYellow); }
    else if (level == TipsLevel::FAILED     ) { tips_color = QColor(Qt::red       ); }

    auto palette = m_ui->signup_label_info->palette();
    palette.setBrush(QPalette::WindowText, QBrush(tips_color));
    m_ui->signup_label_info->setPalette(palette);
    m_ui->signup_label_info->setText(level == TipsLevel::CLEAN ? QString{} : message);
}

void AuthWindow::slot_page_signup_tips_clean() {
    // m_ui->signup_label_info->clear();
    slot_page_signup_tips_show(TipsLevel::CLEAN);
}

bool AuthWindow::slot_signup() {
    signup_params params_signup = std::make_shared<TaoTieChat::SignUpRequest>();
    params_signup->setUserId          (m_ui->signup_edit_username->text());
    params_signup->setPassword        (m_ui->signup_edit_password->text());
    params_signup->setConfirm         (m_ui->signup_edit_confirm ->text());
    params_signup->setSecurityQuestion(m_ui->signup_edit_question->text());
    params_signup->setSecurityAnswer  (m_ui->signup_edit_answer  ->text());
    return signup(params_signup);
}

bool AuthWindow::slot_signin() {
    signin_params params_signin = std::make_shared<TaoTieChat::SignInRequest>();
    TaoTieChat::AuthRequest auth;
    auth.setUserId  (m_ui->signin_combo_username->currentText());
    auth.setPassword(m_ui->signin_edit_password ->text       ());
    params_signin->setRequestUserAuth(std::move(auth));
    // 传递登录选项，用于保存登录信息
    std::shared_ptr<AuthOpt> auth_opt = std::make_shared<AuthOpt>();
    auth_opt->user_id         = m_ui->signin_combo_username->currentText();
    auth_opt->password        = m_ui->signin_edit_password ->text       ();
    auth_opt->rember_password = m_ui->signin_check_rember  ->isChecked  ();
    auth_opt->auto_login      = m_ui->signin_check_auto    ->isChecked  ();
    return signin(params_signin, auth_opt);
}

bool AuthWindow::signup(signup_params params_signup) {
    status_signup_unblock(false);
    auto callback = [this](signup_params params_signup, signup_reply reply_signup){
        status_signup_unblock(true);
        // 账号注册成功
        if(reply_signup && reply_signup->hasResult() && reply_signup->result().hasSuccessCode()) {
            slot_signup_successd(params_signup);
            return;
        }
        // 账号注册失败
        QString error_info = tr("signup failed!");
        if(reply_signup && reply_signup->hasResult() && reply_signup->result().hasErrorCode()) {
            auto code = reply_signup->result().errorCode();
            // error_info = reply_signup->result().message();

            // 注册界面是否需要修改返回错误代码?
            static QMap<ErrorCode, ErrorCode> s_map_codes = {
                // { ErrorCode::AUTH_ID_EMPTY    ,  ErrorCode::USER_ID_EMPTY  },
                // { ErrorCode::AUTH_METHOD_EMPTY,  ErrorCode::PASSWORD_EMPTY },
            };
            if (s_map_codes.contains(code)) {
                error_info = Utils::Transform::to_string(s_map_codes.value(code));
            }
            else if (Utils::Transform::s_map_error_code_strings.contains(code)) {
                error_info = Utils::Transform::to_string(code);
            }
            else {
                error_info = reply_signup->result().message();
            }
        }
        slot_page_signup_tips_show(TipsLevel::FAILED, error_info);
    };
    return NetworkClient::instance()->signup(params_signup, callback);
}

bool AuthWindow::signin(signin_params params_signin, std::shared_ptr<AuthOpt> auth_opt) {
    // 尝试抢占管道，防止多开登录同一账号导致的状态混乱问题，抢占成功返回 true，抢占失败返回 false
    if(!Utils::OS::try_grab_account_pipe(auth_opt->user_id)) {
        // 账号已登陆
        slot_page_signin_tips_show(TipsLevel::FAILED, tr("User %1 already Logined!").arg(auth_opt->user_id));
        return false;
    }

    // 1. 执行操作前禁用按钮
    // 2. 执行失败后立即解除禁用状态
    // 3. 执行成功时不在这里进行状态解除，防止登陆过程过快导致用户误以为窗口闪烁的问题，便于用户看到登陆过程
    status_signin_unblock(false);
    auto callback = [this, auth_opt](signin_params params_signin, signin_reply reply_signin) {
        // 用户登录成功
        if(reply_signin && reply_signin->hasResult() && reply_signin->result().hasSuccessCode()) {
            slot_signin_successd(params_signin, reply_signin, auth_opt);
            return;
        }

        // 用户登录失败
        status_signin_unblock(true);
        QString error_info = tr("signin failed!");
        if(reply_signin && reply_signin->hasResult() && reply_signin->result().hasErrorCode()) {
            auto code = reply_signin->result().errorCode();
            // ;    if(code == ErrorCode::AUTH_ID_EMPTY    ) { error_info = tr("User id empty"); }
            // else if(code == ErrorCode::AUTH_METHOD_EMPTY) { error_info = tr("Password empty"); }
            // // else if(code == ErrorCode::ERROR_INIT    ) { error_info = tr(""); }
            // else { error_info = reply_signin->result().message(); }

            // 登陆界面修改返回错误代码为用户名、密码为空/错误
            static QMap<ErrorCode, ErrorCode> s_map_codes = {
                { ErrorCode::AUTH_ID_EMPTY    ,  ErrorCode::USER_ID_EMPTY  },
                { ErrorCode::AUTH_METHOD_EMPTY,  ErrorCode::PASSWORD_EMPTY },
            };
            if (s_map_codes.contains(code)) {
                error_info = Utils::Transform::to_string(s_map_codes.value(code));
            }
            else if (Utils::Transform::s_map_error_code_strings.contains(code)) {
                error_info = Utils::Transform::to_string(code);
            }
            else {
                error_info = reply_signin->result().message();
            }
        }
        slot_page_signin_tips_show(TipsLevel::FAILED, error_info);
    };
    return NetworkClient::instance()->signin(params_signin, callback);
}

bool AuthWindow::slot_signup_successd(signup_params params_signup) {
    // QMessageBox::information(this, tr("tips"), tr("SignUp success. close this dialog to SignIn."));

    // 清除界面数据
    m_ui->signup_edit_username->clear();
    m_ui->signup_edit_password->clear();
    m_ui->signup_edit_confirm ->clear();
    m_ui->signup_edit_question->clear();
    m_ui->signup_edit_answer  ->clear();

    // // "账号注册成功！"
    // slot_page_signup_tips_show(TipsLevel::SUCCEEDED, tr("Account registered successfully!"));
    // return true;

    // 自动返回登陆窗口
    m_ui->auth_stacked->setCurrentWidget(m_ui->page_signin);
    m_ui->signin_combo_username->setEditText(params_signup->userId());
    m_ui->signin_edit_password->setText(params_signup->password());
    slot_page_signin_tips_show(TipsLevel::SUCCEEDED, tr("Account registered successfully!"));
    return true;

    // 自动登陆

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

bool AuthWindow::slot_signin_successd(signin_params params_signin, signin_reply reply_signin, std::shared_ptr<AuthOpt> auth_opt) {
    // 优先使用登录成功的数据覆盖 auth_opt
    if(auth_opt) {
        // 先复制原始的登录信息
        if (params_signin && params_signin->hasRequestUserAuth()) {
            auth_opt->account_id = params_signin->requestUserAuth().hasAccountId() ? params_signin->requestUserAuth().accountId() : "";
            auth_opt->user_id    = params_signin->requestUserAuth().hasUserId   () ? params_signin->requestUserAuth().userId   () : "";
            auth_opt->token      = params_signin->requestUserAuth().hasToken    () ? params_signin->requestUserAuth().token    () : "";
            auth_opt->password   = params_signin->requestUserAuth().hasPassword () ? params_signin->requestUserAuth().password () : "";
        }
        // 从返回参数中强制覆盖
        if(reply_signin && !reply_signin->accountId().isEmpty()) {
            auth_opt->account_id = reply_signin->accountId();
        }
        if(reply_signin && !reply_signin->token().isEmpty()) {
            auth_opt->token = reply_signin->token();
        }
        // 保存登录记录
        AuthOptRW::save(auth_opt);
    }

    // 将用户信息保存到全局状态中
    std::shared_ptr<ContactData::ItemData> contact_data = std::make_shared<ContactData::ItemData>();
    contact_data->account_id = auth_opt->account_id;
    contact_data->user_id    = auth_opt->user_id;
    contact_data->password   = auth_opt->password;
    contact_data->token      = auth_opt->token;
    Accounts::instance()->set_current_account(contact_data);
    Accounts::instance()->set_account_auth_opt(auth_opt);

    // 初始化配置目录
    Database::instance()->set_config_dir(Utils::user_home_path());

    // 实际上登陆消息瞬间完成，延迟 1 秒再显示主界面，防止用户误以为登陆窗口闪烁的错误判断
    QTimer::singleShot(1000, this, [this]() {
        status_signin_unblock(true);
        // this->accepted();
        this->accepted();  // 关闭窗口
        emit SignalInner::instance()->accepted();  // 发送信号

        // 清除登陆页面信息
        // 需要在切换窗口显示后清除，而非收到消息时立即清除
        slot_page_signin_tips_clean();
        slot_page_signup_tips_clean();
        if (!m_ui->signin_check_rember->isChecked()) {
            m_ui->signin_edit_password->setText(QString{});
        }
    });

    return true;
}

void AuthWindow::status_signin_unblock(bool unblock) {
    m_ui->signin_combo_username->setEnabled(unblock);
    m_ui->signin_edit_password ->setEnabled(unblock);
    m_ui->signin_check_rember  ->setEnabled(unblock);
    m_ui->signin_check_auto    ->setEnabled(unblock);
    m_ui->signin_button_signin ->setEnabled(unblock);
    if(unblock) {
        slot_page_signin_tips_clean();
    }
    else {
        slot_page_signin_tips_show(TipsLevel::INFORMATION, tr("logining..."));
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
        slot_page_signup_tips_clean();
    }
    else {
        slot_page_signup_tips_show(TipsLevel::INFORMATION, tr("signing up..."));
    }
}
