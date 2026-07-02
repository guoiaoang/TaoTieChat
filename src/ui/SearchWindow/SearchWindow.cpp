#include "SearchWindow.h"
#include "ui_SearchWindow.h"

#include "NetworkClient.h"
#include "ItemModel.h"
#include "ItemDelegate.h"
#include "ContactWindow.h"

SearchWindow::SearchWindow(QWidget *parent)
    : QScrollArea(parent) , m_ui(std::make_unique<Ui::SearchWindow>()) {
    m_ui->setupUi(this);

    // new
    m_action_search  = std::make_unique<QAction>(this);
    // model/view
    m_users_model    = std::make_unique<ItemModel    >();
    m_users_delegate = std::make_unique<ItemDelegate >();
    m_user_window    = std::make_unique<ContactWindow>();

    // connect
    connect(m_ui->searchpage_lineedit_key     , &QLineEdit::textChanged, this, [this](){
        QString str_search_key = m_ui->searchpage_lineedit_key->text();
        str_search_key = str_search_key.trimmed();
        m_ui->searchpage_toolbutton_search->setEnabled(!str_search_key.isEmpty());
        // if(str_search_key.isEmpty()) {
        //     this->m_users_model->clean();  // 搜索词为空时清空搜索结果
        // }
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
    m_ui->searchpage_toolbutton_search->setEnabled(m_action_search->text().isEmpty());  // 搜索词为空时不允许点击搜索按钮
    // 设置应用级别的图标后就不需要再手动设置窗口级别的图标了
    // this->setWindowIcon(QApplication::windowIcon());

    // contact view/model/delegate
    m_ui->page_list_users->setModel(m_users_model.get());
    m_ui->page_list_users->setItemDelegate(m_users_delegate.get());
    // 使用 currentChanged 信号支持键盘上下键导航和鼠标点击
    // connect(ui->page_list_users->selectionModel(), &QItemSelectionModel::currentChanged, this, &ChatWindow::handle_contact_selection_changed);
    // 响应双击事件
    connect(m_ui->page_list_users, &QListView::doubleClicked, this, [this](const QModelIndex &index) {
        do{
            if(!index.isValid()) { break; }

            auto index_data = index.data(Qt::UserRole + 1);
            if(!index_data.canConvert<std::shared_ptr<ItemDelegate>>()) { break; }
            // if(!index_data.canConvert<std::shared_ptr<ItemDelegateSpacer>>()) { break; }
            if(!index_data.canConvert<std::shared_ptr<ItemDelegateContact>>()) { break; }

            auto item_provider = index_data.value<std::shared_ptr<ItemDelegateContact>>();
            if(!item_provider) { break; }

            const std::shared_ptr<ContactInfo>&contact_info = item_provider->get_contact_info();
            if (!contact_info || !m_user_window) { return; }
            this->m_user_window->show_contact(contact_info);
            Utils::widget_raise(this->m_user_window.get());
            return;
        } while(false);
        qDebug() << "canConvert error.";
    });

    // TODO:群组功能当前未实现
    m_ui->searchpage_widget_groups  ->hide();
    m_ui->searchpage_toolbutton_user->hide();  // 只有一个作用时可暂不显示标签

    // 设置回车键操作
    this->installEventFilter(this);
}

SearchWindow::~SearchWindow() {
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
            // 发生错误时要刷新显示为空列表
            this->m_users_model.reset();
            return;
        }

        qDebug().noquote().nospace() << "search_user reply : " << reply_search_user;

        if(reply_search_user->hasContactInfo() && reply_search_user->contactInfo().hasBaseInfo() && !reply_search_user->contactInfo().baseInfo().avatarId().isEmpty()) {
            auto file_id   = reply_search_user->contactInfo().baseInfo().avatarId();
            auto file_path = Database::instance().get_local_file_id_path(file_id);
            auto file_size = QFileInfo(file_path).size();
            // 没有本地记录但有头像 ID 时则从服务器下载完成后再显示
            if(file_id.isEmpty() || file_path.isEmpty() || file_size <= 0) {
                network_file_io_notify_callback callback_download_avatar_notify = [this, reply_search_user](const NetworkFileIONotify&notify) {
                    // qDebug().nospace().noquote() << "notify:" << notify;
                    // 下载时不需要更新界面
                    if((notify.ok == true && notify.status == FileStatusCode::FILE_STATUS_UPLOADED) || (notify.ok == false)) {
                        // 刷新显示
                        auto contacts = std::make_shared<ContactInfos>();
                        contacts->setContactInfos({reply_search_user->contactInfo()});
                        // 转换
                        std::vector<std::shared_ptr<ItemDelegate>> item_providers;
                        for(auto&ite : contacts->contactInfos()) {
                            item_providers.emplace_back(std::make_shared<ItemDelegateContact>(std::make_shared<ContactInfo>(ite), ENUMS::ItemGroupType::NONE));
                        }
                        this->m_users_model->reset(item_providers);
                    }
                };
                NetworkClient::instance().download_file(file_id, file_size, callback_download_avatar_notify);
                return;
            }
        }

        // 刷新显示
        auto contacts = std::make_shared<ContactInfos>();
        // 搜索结果不为空时才添加成员  // 不知道为什么始终不为空
        if(reply_search_user->hasContactInfo() && reply_search_user->contactInfo().hasBaseInfo() && !reply_search_user->contactInfo().baseInfo().accountId().isEmpty()) {
            contacts->setContactInfos({ reply_search_user->contactInfo() });
        }
        // this->m_users_model->set_contacts(contacts);
        {
            // 转换
            std::vector<std::shared_ptr<ItemDelegate>> item_providers;
            for(auto&ite : contacts->contactInfos()) {
                item_providers.emplace_back(std::make_shared<ItemDelegateContact>(std::make_shared<ContactInfo>(ite), ENUMS::ItemGroupType::NONE));
            }
            this->m_users_model->reset(item_providers);
        }
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

bool SearchWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == this && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {

            // 补充一个人性化体验：如果当前焦点在一个本来就要响应回车的控件上（比如别的 QPushButton），就别抢人家的回车了
            QWidget *focusWidget = this->focusWidget();
            if (focusWidget && qobject_cast<QAbstractButton*>(focusWidget) && focusWidget != m_ui->searchpage_toolbutton_search) {
                return QScrollArea::eventFilter(watched, event); // 放行
            }

            // 否则，触发我们的 QToolButton
            m_ui->searchpage_toolbutton_search->animateClick();
            return true; // 拦截
        }
    }
    return QScrollArea::eventFilter(watched, event);
}
