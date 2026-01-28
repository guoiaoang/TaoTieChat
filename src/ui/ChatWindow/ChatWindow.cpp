#include "ChatWindow.h"
#include "ui_ChatWindow.h"

// #include <QListView>
// #include <QListWidget>
#include <QtDebug>

ChatWindow::ChatWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::ChatWindow) {
    ui->setupUi(this);
    // // connect(ui->button_chat, &QToolButton::clicked, this, &ChatWindow::accept);

    // 创建对象
    m_split_content   = new QSplitter;
    m_menu_settings   = new QMenu;
    m_action_signout  = new QAction;
    m_action_settings = new QAction;

    // 连接消息
    connect(ui->navigate_button_chat   , &QToolButton::clicked  , this, [&](){ ui->stacked_content_list->setCurrentWidget(ui->stacked_page_chat   ); });
    connect(ui->navigate_button_contact, &QToolButton::clicked  , this, [&](){ ui->stacked_content_list->setCurrentWidget(ui->stacked_page_content); });
    connect(ui->navigate_button_menu   , &QToolButton::clicked  , this, [&](){ this->on_show_menu(); });
    connect(m_action_signout           , &QAction    ::triggered, this, [&](){ emit signout(); });
    connect(m_action_settings          , &QAction    ::triggered, this, [](){ qDebug() << "m_action_settings"; });

    // 执行操作
    // 替换为可滑动窗口
    ui->content_layout->removeWidget(ui->stacked_content_list);
    ui->content_layout->removeWidget(ui->stacked_content_detail);
    m_split_content->addWidget(ui->stacked_content_list);
    m_split_content->addWidget(ui->stacked_content_detail);
    ui->chat_layout->replaceWidget(ui->content_widget, m_split_content);

    // 设置菜单
    // ui->navigate_button_menu->setMenu(m_menu_settings);
    // ui->navigate_button_menu->setPopupMode(QToolButton::InstantPopup);        // 点击按钮立即弹出菜单  // 带下三角
    // // ui->navigate_button_menu->setPopupMode(QToolButton::MenuButtonPopup);  // 分隔符 + 下三角
    // // ui->navigate_button_menu->setPopupMode(QToolButton::DelayedPopup);     // 带下三角
    m_action_settings->setText(tr("settings"));
    m_action_settings->setDisabled(true); // TODO:功能未实现，暂时禁用
    m_action_signout->setText(tr("Signout"));
    m_menu_settings->addAction(m_action_settings);
    m_menu_settings->addSeparator();
    m_menu_settings->addAction(m_action_signout);

    // debug
    ui->page_list_chat   ->addItem("chat list");
    ui->page_list_content->addItem("content list");
}

ChatWindow::~ChatWindow() {
    delete ui;
    ui = nullptr;
    if(m_split_content) {
        delete m_split_content;
        m_split_content = nullptr;
    }
    if(m_menu_settings) {
        delete m_menu_settings;
        m_menu_settings = nullptr;
    }
    if(m_action_signout) {
        delete m_action_signout;
        m_action_signout = nullptr;
    }
    if(m_action_settings) {
        delete m_action_settings;
        m_action_settings = nullptr;
    }
}

void ChatWindow::on_show_menu() {
    // qDebug() << "navigate_button_menu";

    // // 在按钮上方弹出
    // QSize menuSize = m_menu_settings->sizeHint();
    // m_menu_settings->adjustSize(); // 内存调整为实际大小
    // QSize finalSize = m_menu_settings->size();
    // QPoint pos = ui->navigate_button_menu->mapToGlobal(QPoint(0, 0));
    // pos.setY(pos.y() - menuSize.height());
    // m_menu_settings->popup(pos);

    // 在按钮上方弹出
    m_menu_settings->adjustSize(); // 内存调整为实际大小
    QPoint pos = ui->navigate_button_menu->mapToGlobal(ui->navigate_button_menu->rect().bottomRight());
    int h = m_menu_settings->size().height();
    m_menu_settings->popup(QPoint(pos.x(), pos.y() - h));

    // m_menu_settings->exec(QCursor::pos()); // 在鼠标当前位置弹出
}
