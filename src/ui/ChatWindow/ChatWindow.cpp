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
    // 点击了聊天室按钮
    connect(ui->navigate_button_chat_room, &QToolButton::clicked  , this, [&](){
        ui->stacked_content_list->setCurrentWidget(ui->stacked_page_chat_room);
        ui->navigate_button_chat_room->setEnabled(false);
        ui->navigate_button_contact  ->setEnabled(true );
    });
    // 点击了联系人按钮
    connect(ui->navigate_button_contact, &QToolButton::clicked  , this, [&](){
        ui->stacked_content_list->setCurrentWidget(ui->stacked_page_content);
        ui->navigate_button_chat_room->setEnabled(true );
        ui->navigate_button_contact  ->setEnabled(false);
    });
    // 点击了设置按钮
    connect(ui->button_settings, &QToolButton::clicked  , this, [&](){ this->on_show_settings_menu(); });
    // 点击了添加按钮
    connect(ui->button_add     , &QToolButton::clicked  , this, [&](){ this->on_show_add_menu     (); });
    // 各种菜单事件
    connect(m_action_signout   , &QAction    ::triggered, this, [&](){ emit signout(); });
    connect(m_action_settings  , &QAction    ::triggered, this, [&](){ qDebug() << "m_action_settings"; });
    connect(m_action_add       , &QAction    ::triggered, this, [&](){ qDebug() << "m_action_add"     ; });
    connect(m_action_create    , &QAction    ::triggered, this, [&](){ qDebug() << "m_action_create"  ; });
    // 连接聊天列表点击事件
    // connect(ui->page_list_chat_room, &QListView::itemClicked, this, &ChatWindow::on_chat_room_item_clicked);

    // 执行操作
    ui->navigate_button_chat_room->click();  // 初始化状态
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
    ui->page_list_chat_room->setSpacing(0);
    // 设置按像素滚动列表内容
    ui->page_list_chat_room->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    // m_split_content->setSizes(QList<int>{100, 100});
    ui->widget_content_list->resize(300, ui->widget_content_list->height());

    // TODO:设置最小宽度为 ItemChat 宽度 + 滚动条宽度
    // ui->stacked_content_list->setMinimumWidth(ItemChat().minimumWidth() + ui->page_list_chat->verticalScrollBar()->sizeHint().width());

    // content view/model/delegate
    m_content_model = new ContentModel;
    ui->page_list_content->setModel(m_content_model);
    m_content_delegate = new ContentDelegate;
    ui->page_list_content->setItemDelegate(m_content_delegate);
    // content test data
    {
        Content content = QSharedPointer<TypeContent>(new TypeContent);
        content->set_data("Content ID 01", TypeContent::Type::FRIEND, "Content Avatar 01", "Content NickName 01", "Last Message 01");
        // content->datas()[TypeContent::ModelRole::ID      ] = "Content ID 01"      ;
        // content->datas()[TypeContent::ModelRole::Avatar  ] = "Content Avatar 01"  ;
        // content->datas()[TypeContent::ModelRole::NickName] = "Content NickName 01";
        // content->datas()[TypeContent::ModelRole::Message ] = "Last Message 01"    ;
        add_content(content);
        add_content(QSharedPointer<TypeContent>(new TypeContent("Content ID 02", TypeContent::Type::FRIEND, "Content Avatar 02", "Content NickName 02", "Last Message 02")));
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
    if(m_content_model) {
        delete m_content_model;
        m_content_model = nullptr;
    }
    if(m_content_delegate) {
        delete m_content_delegate;
        m_content_delegate = nullptr;
    }
}

void ChatWindow::on_show_settings_menu() {
    // 在按钮上方弹出
    m_menu_settings->adjustSize(); // 内存调整为实际大小
    QPoint pos = ui->button_settings->mapToGlobal(ui->button_settings->rect().bottomRight());
    int h = m_menu_settings->size().height();
    m_menu_settings->popup(QPoint(pos.x(), pos.y() - h));
}

void ChatWindow::on_show_add_menu() {
    // 在按钮下方左侧弹出
    m_menu_add->adjustSize(); // 内存调整为实际大小
    QPoint pos = ui->button_add->mapToGlobal(ui->button_add->rect().bottomRight());
    int w = m_menu_add->size().width();
    m_menu_add->popup(QPoint(pos.x() - w, pos.y()));
}

void ChatWindow::add_chat_room(Content content) {
    // 添加聊天室到聊天室列表控件
    // QListWidgetItem *list_widget_item = new QListWidgetItem(ui->page_list_chat);
    // ItemChat        *itemWidget       = new ItemChat(data, ui->page_list_chat);
    // list_widget_item->setSizeHint(itemWidget->sizeHint());
    // ui->page_list_chat->setItemWidget(list_widget_item, itemWidget);
}

void ChatWindow::on_chat_room_item_clicked(QListWidgetItem *item) {
    // // 获取对应的 ItemChat Widget
    // ItemChat *item_chat = qobject_cast<ItemChat*>(ui->page_list_chat->itemWidget(item));
    // if (item_chat) {
    //     QString chat_id = item_chat->get_data().nickname;
    //     qDebug() << "Clicked chat item:" << chat_id;
    //     show_chat_messages(chat_id);
    // }
}

void ChatWindow::show_chat_room_messages(Content content) {
    // // 有则切换，无则添加
    // // 切换
    // auto ite = m_message_windows.find(chat_id);
    // if(ite != m_message_windows.end()) {
    //     ui->stacked_content_detail->setCurrentWidget(*ite);
    //     return;
    // }
    // // 添加
    // MessageWindow*message_window = new MessageWindow;
    // ui->stacked_content_detail->addWidget(message_window);
    // ui->stacked_content_detail->setCurrentWidget(message_window);
    // // 设置标题
    // message_window->update_info(ChatroomInfo(chat_id));
    // // 显示对应聊天的消息
    // if (m_user_messages.contains(chat_id)) {
    //     const ChatroomMessages &chatroom_messages = m_user_messages[chat_id];
    //     message_window->update_data(chatroom_messages);
    //     ui->page_list_detail->scrollToBottom();  // 滚动到底部
    // }
    // else {
    //     message_window->update_data(ChatroomMessages());
    // }
}

void ChatWindow::add_content(Content content) {
    // QListWidgetItem *list_widget_item = new QListWidgetItem(ui->page_list_content);
    // ItemContent     *itemWidget       = new ItemContent(data, ui->page_list_content);
    // list_widget_item->setSizeHint(itemWidget->sizeHint());
    // ui->page_list_content->setItemWidget(list_widget_item, itemWidget);
    m_content_model->add_content(content);
}

void ChatWindow::on_content_item_clicked(QListWidgetItem *item) {
    // // 获取对应的 ItemChat Widget
    // ItemChat *item_chat = qobject_cast<ItemChat*>(ui->page_list_chat->itemWidget(item));
    // if (item_chat) {
    //     QString chat_id = item_chat->get_data().nickname;
    //     qDebug() << "Clicked chat item:" << chat_id;
    //     show_chat_messages(chat_id);
    // }
}

void ChatWindow::show_content_info(Content content) {
}
