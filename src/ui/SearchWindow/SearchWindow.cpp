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

bool SearchWindow::do_search(const QString&key) {
    QString str_search_key = key.trimmed();
    if(str_search_key.isEmpty()) {
        return false;
    }

    search_user(str_search_key);
    search_group(str_search_key);
    return false;
}

bool SearchWindow::search_user(const QString&key) {
    QString str_search_key = key.trimmed();
    if(str_search_key.isEmpty()) {
        return false;
    }

    NetworkClient::instance();
    return false;
}

bool SearchWindow::search_group(const QString&key) {
    QString str_search_key = key.trimmed();
    if(str_search_key.isEmpty()) {
        return false;
    }
    return false;
}
