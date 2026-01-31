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
    m_menu_settings   = new QMenu  ;
    m_action_signout  = new QAction;
    m_action_settings = new QAction;
    // add
    m_menu_add      = new QMenu  ;
    m_action_add    = new QAction;
    m_action_create = new QAction;

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
    connect(ui->button_settings, &QToolButton::clicked  , this, [&](){ this->on_show_settings_menu(); });
    connect(ui->button_add     , &QToolButton::clicked  , this, [&](){ this->on_show_add_menu     (); });
    connect(m_action_signout   , &QAction    ::triggered, this, [&](){ emit signout(); });
    connect(m_action_settings  , &QAction    ::triggered, this, [&](){ qDebug() << "m_action_settings"; });
    connect(m_action_add       , &QAction    ::triggered, this, [&](){ qDebug() << "m_action_add"     ; });
    connect(m_action_create    , &QAction    ::triggered, this, [&](){ qDebug() << "m_action_create"  ; });

    // 连接聊天列表点击事件
    connect(ui->page_list_chat, &QListWidget::itemClicked, this, &ChatWindow::on_chat_item_clicked);

    // 执行操作
    ui->navigate_button_chat->click();  // 初始化状态
    // 替换为可滑动窗口
    // ui->content_layout->removeWidget(ui->stacked_content_list);
    ui->content_layout->removeWidget(ui->widget_content_list);
    ui->content_layout->removeWidget(ui->stacked_content_detail);
    // m_split_content->addWidget(ui->stacked_content_list);
    m_split_content->addWidget(ui->widget_content_list);
    m_split_content->addWidget(ui->stacked_content_detail);
    m_split_content->setStretchFactor(0, 0); // 编号 0 的窗口拉伸因子为 0 // 整个窗口调整大小时左侧宽度不调整
    m_split_content->setStretchFactor(1, 1);
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
    // add
    m_action_add   ->setText(tr("Add as a friend or join a group"));
    m_action_create->setText(tr("Create a group"));
    m_menu_add->addAction(m_action_add   );
    m_menu_add->addAction(m_action_create);
    m_action_add   ->setEnabled(false); // TODO:功能未实现，暂时禁用
    m_action_create->setEnabled(false); // TODO:功能未实现，暂时禁用

    // 设置列表属性
    ui->page_list_chat->setSpacing(0);
    ui->page_list_chat->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    // m_split_content->setSizes(QList<int>{100, 100});
    ui->widget_content_list->resize(300, ui->widget_content_list->height());

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

    // debug - 初始化示例聊天消息
    {
        // 张三的聊天记录
        add_chat_message("张三", ItemMessageData{"", "张三", "你好，晚上有空吗?", "18:20", false});
        add_chat_message("张三", ItemMessageData{"", "我", "有空啊，什么事?", "18:22", true});
        add_chat_message("张三", ItemMessageData{"", "张三", "晚上一起吃饭吗?", "18:30", false});

        // 李四的聊天记录
        add_chat_message("李四", ItemMessageData{"", "我", "明天见!", "17:10", true});
        add_chat_message("李四", ItemMessageData{"", "李四", "好的，明天见!", "17:15", false});

        // 王五的聊天记录
        add_chat_message("王五", ItemMessageData{"", "王五", "周末有空吗?", "昨天 10:30", false});
        add_chat_message("王五", ItemMessageData{"", "王五", "一起打篮球", "昨天 10:31", false});

        // 赵六的聊天记录
        add_chat_message("赵六", ItemMessageData{"", "赵六", "[图片]", "周三", false});

        // 开发组的聊天记录
        add_chat_message("开发组", ItemMessageData{"", "组长", "大家注意一下", "1月27日 14:50", false});
        add_chat_message("开发组", ItemMessageData{"", "组长", "今天下午3点开会", "1月27日 14:51", false});
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
    if(m_menu_add) {
        delete m_menu_add;
        m_menu_add = nullptr;
    }
    if(m_action_add) {
        delete m_action_add;
        m_action_add = nullptr;
    }
    if(m_action_create) {
        delete m_action_create;
        m_action_create = nullptr;
    }
}

void ChatWindow::on_show_settings_menu() {
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
    QPoint pos = ui->button_settings->mapToGlobal(ui->button_settings->rect().bottomRight());
    int h = m_menu_settings->size().height();
    m_menu_settings->popup(QPoint(pos.x(), pos.y() - h));

    // m_menu_settings->exec(QCursor::pos()); // 在鼠标当前位置弹出
}

void ChatWindow::on_show_add_menu() {
    // 在按钮下方左侧弹出
    m_menu_add->adjustSize(); // 内存调整为实际大小
    QPoint pos = ui->button_add->mapToGlobal(ui->button_add->rect().bottomRight());
    int w = m_menu_add->size().width();
    m_menu_add->popup(QPoint(pos.x() - w, pos.y()));
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

void ChatWindow::on_chat_item_clicked(QListWidgetItem *item) {
    // 获取对应的 ItemChat Widget
    ItemChat *item_chat = qobject_cast<ItemChat*>(ui->page_list_chat->itemWidget(item));
    if (item_chat) {
        QString chat_id = item_chat->get_data().nickname;
        qDebug() << "Clicked chat item:" << chat_id;
        show_chat_messages(chat_id);
    }
}

void ChatWindow::show_chat_messages(const QString &chat_id) {
    // 有则切换，无则添加
    // 切换
    auto ite = m_message_windows.find(chat_id);
    if(ite != m_message_windows.end()) {
        ui->stacked_content_detail->setCurrentWidget(*ite);
        return;
    }
    // 添加
    MessageWindow*message_window = new MessageWindow;
    ui->stacked_content_detail->addWidget(message_window);
    ui->stacked_content_detail->setCurrentWidget(message_window);
    // 设置标题
    message_window->update_info(ChatroomInfo(chat_id));
    // 显示对应聊天的消息
    if (m_user_messages.contains(chat_id)) {
        const ChatroomMessages &chatroom_messages = m_user_messages[chat_id];
        message_window->update_data(chatroom_messages);
        ui->page_list_detail->scrollToBottom();  // 滚动到底部
    }
    else {
        message_window->update_data(ChatroomMessages());
    }
}

void ChatWindow::add_chat_message(const QString &chat_id, const ItemMessageData &data) {
    if (!m_user_messages.contains(chat_id)) {
        m_user_messages[chat_id] = QVector<ItemMessageData>();
    }
    m_user_messages[chat_id].append(data);

    MessageWindow *message_window = dynamic_cast<MessageWindow*>(ui->stacked_content_detail->currentWidget());
    if(message_window) {
        message_window->append_message(data);
    }
}

void ChatWindow::clear_chat_messages() {
    ui->page_list_detail->clear();
    m_user_messages.clear();
}
