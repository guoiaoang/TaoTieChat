#include "SearchWindow.h"
#include "ui_SearchWindow.h"

SearchWindow::SearchWindow(QWidget *parent)
    : QScrollArea(parent) , ui(new Ui::SearchWindow) {
    ui->setupUi(this);

    // new
    m_action_search = new QAction(this);

    // connect
    connect(ui->searchpage_lineedit_key     , &QLineEdit::textChanged, this, [this](){
        QString str_search_key = ui->searchpage_lineedit_key->text();
        str_search_key = str_search_key.trimmed();
        ui->searchpage_toolbutton_search->setEnabled(!str_search_key.isEmpty());
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
}

SearchWindow::~SearchWindow() {
    if(m_action_search) {
        delete m_action_search;
        m_action_search = nullptr;
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
    params_search_users->setUsersKeyword(str_search_keyword);
    auto network_search_users_callback = [](const search_users_params&param_search_users, const search_users_reply&reply_search_users) {
        using ec = TaoTieChat::ErrorCodeGadget::ErrorCode;
        using TaoTieChat::Result;
        using TaoTieChat::UserBaseInfo;

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
        QList<UserBaseInfo> reply_users_info = reply_search_users->userInfo();
        if(reply_users_info.empty()) {
            // clean_users_list();
            qDebug().noquote().nospace() << "network search users results is empty.";
            return;
        }
        for(auto&user_info : reply_users_info) {
            // 构造、清除后添加到搜索结果列表
            // user_info->usetId();
            qDebug().noquote().nospace() << "user_info : " << user_info;
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
