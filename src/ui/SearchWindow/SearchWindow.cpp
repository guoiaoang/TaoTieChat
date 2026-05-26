#include "SearchWindow.h"
#include "ui_SearchWindow.h"

SearchWindow::SearchWindow(QWidget *parent)
    : QScrollArea(parent) , m_ui(std::make_unique<Ui::SearchWindow>()) {
    m_ui->setupUi(this);

    // new
    m_action_search  = std::make_unique<QAction>(this);
    // model/view
    m_users_model    = std::make_unique<ContactModel   >();
    m_users_delegate = std::make_unique<ContactDelegate>();
    m_user_window    = std::make_unique<ContactWindow  >();

    // connect
    connect(m_ui->searchpage_lineedit_key     , &QLineEdit::textChanged, this, [this](){
        QString str_search_key = m_ui->searchpage_lineedit_key->text();
        str_search_key = str_search_key.trimmed();
        m_ui->searchpage_toolbutton_search->setEnabled(!str_search_key.isEmpty());
        if(str_search_key.isEmpty()) {
            this->m_users_model->clean();  // 搜索词为空时清空搜索结果
        }
    });
    connect(m_ui->searchpage_toolbutton_search, &QToolButton::triggered, this, [this](){
        // qDebug().nospace().noquote() << "searchpage_toolbutton_search triggered";
        do_search(m_ui->searchpage_lineedit_key->text());
    });
    // 用于同步显示搜索词
    connect(m_ui->searchpage_lineedit_key, &QLineEdit::textChanged, this, &SearchWindow::SearchEditTextChanged);

    // set
    m_action_search->setText(m_ui->searchpage_toolbutton_search->text());
    m_ui->searchpage_toolbutton_search->setDefaultAction(m_action_search.get());
    // 设置应用级别的图标后就不需要再手动设置窗口级别的图标了
    // this->setWindowIcon(QApplication::windowIcon());

    // contact view/model/delegate
    m_ui->page_list_users->setModel(m_users_model.get());
    m_ui->page_list_users->setItemDelegate(m_users_delegate.get());
    // 使用 currentChanged 信号支持键盘上下键导航和鼠标点击
    // connect(ui->page_list_users->selectionModel(), &QItemSelectionModel::currentChanged, this, &ChatWindow::handle_contact_selection_changed);
    // 响应双击事件
    connect(m_ui->page_list_users, &QListView::doubleClicked, this, [this](const QModelIndex &index) {
        const std::shared_ptr<ContactInfo>&contact_info = index.data(Qt::UserRole).value<std::shared_ptr<ContactInfo>>();
        this->m_user_window->show_contact(contact_info);
        Utils::widget_raise(this->m_user_window.get());
    });

    // TODO:群组功能当前未实现
    m_ui->searchpage_widget_groups  ->hide();
    m_ui->searchpage_toolbutton_user->hide();  // 只有一个作用时可暂不显示标签
}

SearchWindow::~SearchWindow() {
    if(m_action_search ) { m_action_search .reset(); }
    if(m_users_model   ) { m_users_model   .reset(); }
    if(m_users_delegate) { m_users_delegate.reset(); }
    if(m_user_window   ) { m_user_window   .reset(); }
    if(m_ui            ) { m_ui            .reset(); }
}

bool SearchWindow::do_search(const QString&keyword) {
    // get_search_params(); //  应该封装为函数，当结果返回时对比搜索词一致时才显示
    QString str_search_keyword = keyword.trimmed();
    if(str_search_keyword.isEmpty()) {
        return false;
    }

    // TODO:任务队列，同时搜索
    search_user (str_search_keyword);
    search_group(str_search_keyword);
    return false;
}

bool SearchWindow::search_user(const QString&keyword) {
    QString str_search_keyword = keyword.trimmed();
    if(str_search_keyword.isEmpty()) {
        return false;
    }

    // 本地搜索 + 网络搜索
    // local_search_user();
    search_user_params params_search_user = std::make_shared<TaoTieChat::SearchUserRequest>();
    params_search_user->setUserId(str_search_keyword);
    auto network_search_user_callback = [this](const search_user_params&param_search_user, const search_user_reply&reply_search_user) {
        ErrorCode error_code    = ErrorCode::ERROR_UNSET;
        QString   error_message = tr("network_search_user recv failed.");
        Result    result;

        // 检验错误
        do{
            if(!reply_search_user) {
                error_code = ErrorCode::ERROR_RECV;
                break;
            }
            if(!reply_search_user->hasResult()) {
                error_code = ErrorCode::ERROR_RECV;
                break;
            }
            result = reply_search_user->result();
            if(result.hasErrorCode()) {
                error_code    = result.errorCode();
                error_message = result.message();
            }
        } while(false);
        // 处理错误
        if(error_code != ErrorCode::ERROR_UNSET) {
            qDebug().noquote().nospace() << "network search users error, code : " << error_code << ", message : " << error_message;
            return;
        }

        // 刷新显示
        auto contacts = std::make_shared<ContactInfos>();
        contacts->setContactInfos({reply_search_user->contactInfo()});
        this->m_users_model->set_contacts(contacts);
    };
    NetworkClient::instance().search_user(params_search_user, network_search_user_callback);
    return false;
}

bool SearchWindow::search_group(const QString&keyword) {
    QString str_search_keyword = keyword.trimmed();
    if(str_search_keyword.isEmpty()) {
        return false;
    }

    // 本地搜索 + 网络搜索
    // local_search_groups();
    NetworkClient::instance();
    return false;
}
