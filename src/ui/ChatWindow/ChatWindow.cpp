#include "ChatWindow.h"
#include "ui_ChatWindow.h"

// #include <QListView>
// #include <QListWidget>
#include <QtDebug>
#include <QButtonGroup>

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
    connect(ui->navigate_button_chat   , &QToolButton::clicked  , this, [&](){
        ui->stacked_content_list->setCurrentWidget(ui->stacked_page_chat   );
        ui->navigate_button_chat   ->setEnabled(false);
        ui->navigate_button_contact->setEnabled(true );
    });
    connect(ui->navigate_button_contact, &QToolButton::clicked  , this, [&](){
        ui->stacked_content_list->setCurrentWidget(ui->stacked_page_content);
        ui->navigate_button_chat   ->setEnabled(true );
        ui->navigate_button_contact->setEnabled(false);
    });
    connect(ui->navigate_button_menu   , &QToolButton::clicked  , this, [&](){ this->on_show_menu(); });
    connect(m_action_signout           , &QAction    ::triggered, this, [&](){ emit signout(); });
    connect(m_action_settings          , &QAction    ::triggered, this, [&](){ qDebug() << "m_action_settings"; });

    // 执行操作
    ui->navigate_button_chat->click();  // 初始化状态
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

    // 设置列表属性
    ui->page_list_chat->setSpacing(0);
    ui->page_list_chat->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // TODO:设置最小宽度为 ItemChat 宽度 + 滚动条宽度
    // ui->stacked_content_list->setMinimumWidth(ItemChat().minimumWidth() + ui->page_list_chat->verticalScrollBar()->sizeHint().width());

    // debug - 添加示例聊天数据
    {
        ItemChatData item1;
        item1.nickname = "张三";
        item1.lastMessage = "晚上一起吃饭吗?";
        item1.timestamp = "18:30";
        item1.unreadCount = 3;
        add_chat_item(item1);

        ItemChatData item2;
        item2.nickname = "李四";
        item2.lastMessage = "好的,明天见!";
        item2.timestamp = "17:15";
        item2.unreadCount = 0;
        add_chat_item(item2);

        ItemChatData item3;
        item3.nickname = "王五";
        item3.lastMessage = "周末有空吗?一起打篮球";
        item3.timestamp = "昨天";
        item3.unreadCount = 1;
        add_chat_item(item3);

        ItemChatData item4;
        item4.nickname = "赵六";
        item4.lastMessage = "[图片]";
        item4.timestamp = "周三";
        item4.unreadCount = 0;
        add_chat_item(item4);

        ItemChatData item5;
        item5.nickname = "开发组";
        item5.lastMessage = "大家注意一下,今天下午3点开会";
        item5.timestamp = "1月27日";
        item5.unreadCount = 10;
        add_chat_item(item5);
    }

    // // debug - content list
    // for(int i = 0; i < 50; ++i){
    //     ui->page_list_content->addItem("content list " + QString::number(i));
    // }

    // debug - 添加示例联系人数据
    {
        ItemContentData item1;
        item1.nickname = "张三";
        add_content_item(item1);

        ItemContentData item2;
        item2.nickname = "李四";
        add_content_item(item2);

        ItemContentData item3;
        item3.nickname = "王五";
        add_content_item(item3);

        ItemContentData item4;
        item4.nickname = "赵六";
        add_content_item(item4);

        ItemContentData item5;
        item5.nickname = "开发组";
        add_content_item(item5);
    }
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

void ChatWindow::add_chat_item(const ItemChatData &data) {
    QListWidgetItem *list_widget_item = new QListWidgetItem(ui->page_list_chat);
    ItemChat        *itemWidget       = new ItemChat(data, ui->page_list_chat);
    list_widget_item->setSizeHint(itemWidget->sizeHint());
    ui->page_list_chat->setItemWidget(list_widget_item, itemWidget);
}

void ChatWindow::add_content_item(const ItemContentData &data) {
    QListWidgetItem *list_widget_item = new QListWidgetItem(ui->page_list_content);
    ItemContent     *itemWidget       = new ItemContent(data, ui->page_list_content);
    list_widget_item->setSizeHint(itemWidget->sizeHint());
    ui->page_list_content->setItemWidget(list_widget_item, itemWidget);
}

void ChatWindow::clear_chat_list() {
    ui->page_list_chat->clear();
}

void ChatWindow::clear_content_list() {
    ui->page_list_chat->clear();
}
