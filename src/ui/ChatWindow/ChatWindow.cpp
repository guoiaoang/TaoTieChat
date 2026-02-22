#include "ChatWindow.h"
#include "ui_ChatWindow.h"

ChatWindow::ChatWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::ChatWindow) {
    ui->setupUi(this);
    // // connect(ui->button_chat, &QToolButton::clicked, this, &ChatWindow::accept);

    // 创建对象
    m_split_contact   = new QSplitter;
    m_menu_settings   = new QMenu  ;
    m_action_signout  = new QAction;
    m_action_settings = new QAction;
    // add
    m_menu_add      = new QMenu  ;
    m_action_add    = new QAction;
    m_action_create = new QAction;
    // model/view
    m_contact_model    = new ContactModel;
    m_contact_delegate = new ContactDelegate;

    // 连接消息
    // 点击了聊天室按钮
    connect(ui->navigate_button_chat_room, &QToolButton::clicked  , this, [&](){
        ui->stacked_contact_list->setCurrentWidget(ui->stacked_page_chat_room);
        ui->navigate_button_chat_room->setEnabled(false);
        ui->navigate_button_contact  ->setEnabled(true );
    });
    // 点击了联系人按钮
    connect(ui->navigate_button_contact, &QToolButton::clicked  , this, [&](){
        ui->stacked_contact_list->setCurrentWidget(ui->stacked_page_contact);
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
    // // 使用 currentChanged 信号支持键盘上下键导航和鼠标点击
    // connect(ui->page_list_contact->selectionModel(), &QItemSelectionModel::currentChanged, this, &ChatWindow::on_contact_selection_changed);

    // 执行操作
    ui->navigate_button_chat_room->click();  // 初始化状态
    // 替换为可滑动窗口
    // ui->contact_layout->removeWidget(ui->stacked_contact_list);
    ui->contact_layout->removeWidget(ui->widget_contact_list);
    ui->contact_layout->removeWidget(ui->stacked_contact_detail);
    // m_split_contact->addWidget(ui->stacked_contact_list);
    m_split_contact->addWidget(ui->widget_contact_list);
    m_split_contact->addWidget(ui->stacked_contact_detail);
    m_split_contact->setStretchFactor(0, 0); // 编号 0 的窗口拉伸因子为 0 // 整个窗口调整大小时左侧宽度不调整
    m_split_contact->setStretchFactor(1, 1);
    ui->chat_layout->replaceWidget(ui->contact_widget, m_split_contact);

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
    // m_split_contact->setSizes(QList<int>{100, 100});
    ui->widget_contact_list->resize(300, ui->widget_contact_list->height());

    // TODO:设置最小宽度为 ItemChat 宽度 + 滚动条宽度
    // ui->stacked_contact_list->setMinimumWidth(ItemChat().minimumWidth() + ui->page_list_chat->verticalScrollBar()->sizeHint().width());

    // contact view/model/delegate
    ui->page_list_contact->setModel(m_contact_model);
    ui->page_list_contact->setItemDelegate(m_contact_delegate);
    // 使用 currentChanged 信号支持键盘上下键导航和鼠标点击
    connect(ui->page_list_contact->selectionModel(), &QItemSelectionModel::currentChanged, this, &ChatWindow::on_contact_selection_changed);

#ifdef DEBUG
    // Dev::ChatWindowDev(this, parent);
    for(auto contact : *m_local_user_data->get_contacts()) {
        add_contact(contact);
    }
#endif // DEBUG
}

ChatWindow::~ChatWindow() {
    delete ui;
    ui = nullptr;
    if(m_split_contact) {
        delete m_split_contact;
        m_split_contact = nullptr;
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
    if(m_contact_model) {
        delete m_contact_model;
        m_contact_model = nullptr;
    }
    if(m_contact_delegate) {
        delete m_contact_delegate;
        m_contact_delegate = nullptr;
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

void ChatWindow::on_chat_room_item_selected(const QModelIndex &index) {
    qDebug() << "index : " << index;
}

void ChatWindow::on_chat_room_item_activated(const QModelIndex &index) {
    qDebug() << "index : " << index;
}

void ChatWindow::add_chat_room(Contact contact) {
    qDebug() << "contact : " << ContactManager(contact).get_nick_name();
}

void ChatWindow::show_chat_room_messages(Contact contact) {
    qDebug() << "contact : " << ContactManager(contact).get_nick_name();
}

void ChatWindow::add_contact(Contact contact) {
    // qDebug() << "contact : " << ContactManager(contact).get_nick_name();
    m_contact_model->add_contact(contact);
}

void ChatWindow::on_contact_selection_changed(const QModelIndex &current, const QModelIndex &previous) {
    // 索引值需要有效
    if (!current.isValid()) {
        return;
    }
    // 能够获取到有效的联系人信息
    Contact contact = m_contact_model->contact(current);
    if (!contact) {
        return;
    }
    // 查找是否已经添加
    ContactWindow *contact_window_update = nullptr;
    for(int idx = 0; idx < ui->stacked_contact_detail->count(); ++idx) {
        ContactWindow*contact_window_already = dynamic_cast<ContactWindow*>(ui->stacked_contact_detail->widget(idx));
        if(!contact_window_already) {
            break;
        }
        Contact contact_already = contact_window_already->contact();
        if(ContactManager(contact_already).get_account_id() == ContactManager(contact).get_account_id()) {
            contact_window_update = contact_window_already;
            break;
        }
    }
    // 如果已经添加则更新信息
    if(contact_window_update) {
        contact_window_update->update(contact);
    }
    // 如果没有则添加
    else {
        contact_window_update = new ContactWindow(contact, this);
        ui->stacked_contact_detail->addWidget(contact_window_update);
    }
    // 显示
    ui->stacked_contact_detail->setCurrentWidget(contact_window_update);
}
