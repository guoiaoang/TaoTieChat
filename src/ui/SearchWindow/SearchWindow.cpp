#include "SearchWindow.h"
#include "ui_SearchWindow.h"

SearchWindow::SearchWindow(QWidget *parent)
    : QScrollArea(parent) , ui(new Ui::SearchWindow) {
    ui->setupUi(this);

    // new
    m_action_search = new QAction(this);
    // model/view
    m_users_model    = new ContactModel   ;
    m_users_delegate = new ContactDelegate;
    m_user_window    = new ContactWindow  ;

    // connect
    connect(ui->searchpage_lineedit_key     , &QLineEdit::textChanged, this, [this](){
        QString str_search_key = ui->searchpage_lineedit_key->text();
        str_search_key = str_search_key.trimmed();
        ui->searchpage_toolbutton_search->setEnabled(!str_search_key.isEmpty());
        if(str_search_key.isEmpty()) {
            this->m_users_model->clean();  // 搜索词为空时清空搜索结果
        }
    });
    connect(ui->searchpage_toolbutton_search, &QToolButton::triggered, this, [this](){
        // qDebug().nospace().noquote() << "searchpage_toolbutton_search triggered";
        do_search(ui->searchpage_lineedit_key->text());
    });
    // 用于同步显示搜索词
    connect(ui->searchpage_lineedit_key, &QLineEdit::textChanged, this, &SearchWindow::SearchEditTextChanged);

    // set
    m_action_search->setText(ui->searchpage_toolbutton_search->text());
    ui->searchpage_toolbutton_search->setDefaultAction(m_action_search);
    // 设置应用级别的图标后就不需要再手动设置窗口级别的图标了
    // this->setWindowIcon(QApplication::windowIcon());

    // contact view/model/delegate
    ui->page_list_users->setModel(m_users_model);
    ui->page_list_users->setItemDelegate(m_users_delegate);
    // 使用 currentChanged 信号支持键盘上下键导航和鼠标点击
    // connect(ui->page_list_users->selectionModel(), &QItemSelectionModel::currentChanged, this, &ChatWindow::handle_contact_selection_changed);
    // 响应双击事件
    connect(ui->page_list_users, &QListView::doubleClicked, this, [this](const QModelIndex &index) {
        // QString user_id = index.data((int)ContactData::Role::Userid).toString();  // ok
        // // index 包含了被双击项的行、列及模型指针
        // QString data = index.data(Qt::DisplayRole).toString();
        // qDebug() << "双击了第" << index.row() << "行，内容为：" << data << ", user_id : " << user_id;
        Contact contact = index.data((int)ContactData::Role::Contact).value<Contact>();
        this->m_user_window->update(contact);
        Utils::widget_raise(this->m_user_window);
    });

    // 群组功能当前未实现
    ui->searchpage_widget_groups  ->hide();
    ui->searchpage_toolbutton_user->hide();  // 只有一个作用时可暂不显示标签
}

SearchWindow::~SearchWindow() {
    if(m_action_search) {
        delete m_action_search;
        m_action_search = nullptr;
    }
    if(m_users_model) {
        delete m_users_model;
        m_users_model = nullptr;
    }
    if(m_users_delegate) {
        delete m_users_delegate;
        m_users_delegate = nullptr;
    }
    if(m_user_window) {
        delete m_user_window;
        m_user_window = nullptr;
    }
    if(ui) {
        delete ui;
        ui = nullptr;
    }
}

bool SearchWindow::do_search(const QString&keyword) {
    // get_search_params(); //  应该封装为函数，当结果返回时对比搜索词一致时才显示
    QString str_search_keyword = keyword.trimmed();
    if(str_search_keyword.isEmpty()) {
        return false;
    }

    // TODO:任务队列，同时搜索
    search_users (str_search_keyword);
    search_groups(str_search_keyword);
    return false;
}

bool SearchWindow::search_users(const QString&keyword) {
    QString str_search_keyword = keyword.trimmed();
    if(str_search_keyword.isEmpty()) {
        return false;
    }

    // 本地搜索 + 网络搜索
    // local_search_users();
    search_users_params params_search_users = std::make_shared<TaoTieChat::SearchUsersRequest>();
    params_search_users->setUserId(str_search_keyword);
    params_search_users->setUserFilter(QString());
    auto network_search_users_callback = [this](const search_users_params&param_search_users, const search_users_reply&reply_search_users) {
        using ec = TaoTieChat::ErrorCodeGadget::ErrorCode;
        using TaoTieChat::Result;
        using TaoTieChat::Contact;
        using TaoTieChat::Contacts;

        //
        ec      error_code    = ec::UNSET;
        QString error_message = tr("network_search_users recv faied.");
        Result  result;

        // 检验错误
        do{
            if(!reply_search_users) {
                error_code = ec::ERROR_RECV;
                break;
            }
            if(!reply_search_users->hasResult()) {
                error_code = ec::ERROR_RECV;
                break;
            }
            result = reply_search_users->result();
            if(result.hasErrorCode()) {
                error_code    = result.errorCode();
                error_message = result.message();
            }
        } while(false);
        // 处理错误
        if(error_code != ec::UNSET) {
            qDebug().noquote().nospace() << "network search users error, code : " << error_code << ", message : " << error_message;
            return;
        }

        // 暂不校验搜索词，直接刷新显示结果
        this->m_users_model->clean();  // 显示结果前必须首先清空
        QList<Contact> reply_users_info = reply_search_users->contacts().contact();
        if(reply_users_info.empty()) {
            qDebug().noquote().nospace() << "network search users results is empty.";
            return;
        }
        for(auto&user_info : reply_users_info) {
            // 构造、清除后添加到搜索结果列表
            qDebug().noquote().nospace() << "user_info : " << user_info;

            ContactManager cm(ContactManager::create());
            cm.set_user_id    (user_info.userId  ());
            cm.set_nick_name  (user_info.nickname());  // 当前没有设置昵称
            cm.set_avatar_path(user_info.avatar  ());  // 当前没有设置头像
            cm.set_signature  (user_info.sign    ());
            cm.set_emails     (user_info.emails  ());
            // cm.set_region     (user_info.region  ());  // TODO:添加或修复
            // cm.set_birthday   (user_info.birthday());  // TODO:添加或修复
            // cm.set_gender     (user_info.gender  ());  // TODO:添加或修复
            cm.set_phones     (user_info.phones  ());
            cm.set_contact    (cm.contact()); // 先手动保存自身
            this->m_users_model->add_contact(cm.contact());
        }
    };
    NetworkClient::instance().search_users(params_search_users, network_search_users_callback);
    return false;
}

bool SearchWindow::search_groups(const QString&keyword) {
    QString str_search_keyword = keyword.trimmed();
    if(str_search_keyword.isEmpty()) {
        return false;
    }

    // 本地搜索 + 网络搜索
    // local_search_groups();
    NetworkClient::instance();
    return false;
}
