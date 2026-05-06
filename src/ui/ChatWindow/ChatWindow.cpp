#include "ChatWindow.h"
#include "ui_ChatWindow.h"

ChatWindow::ChatWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::ChatWindow) {
    ui->setupUi(this);
    // // connect(ui->button_chat, &QToolButton::clicked, this, &ChatWindow::accept);

    // 创建对象
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
    // search
    // m_search_window = new SearchWindow;

    // 连接消息
    // 点击了头像按钮
    connect(ui->navigate_button_avatar, &QToolButton::clicked  , this, [&]() {
        if(m_contact_window) {
            delete m_contact_window;
            m_contact_window = nullptr;
        }
        if(!m_contact_window) {
            m_contact_window = new ContactWindow(Accounts::instance().get_current_account());
        }
        m_contact_window->show();
    });
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
    // 用于同步显示搜索词
    // connect(m_search_window, &SearchWindow::SearchEditTextChanged, ui->edit_search, &QLineEdit::setText);

    // 执行操作
    ui->navigate_button_chat_room->click();  // 初始化状态
    ui->chat_splitter->setStretchFactor(0, 0); // 编号 0 的窗口拉伸因子为 0 // 整个窗口调整大小时左侧宽度不调整
    ui->chat_splitter->setStretchFactor(1, 1);

    // 设置图标
    const  QSize  size_avatar(48, 48);
    const  QSize  size_button(36, 36);
    static QColor icon_color          = this->palette().color(QPalette::WindowText);
    static QColor icon_color_disabled = this->palette().color(QPalette::Disabled, QPalette::WindowText);
    static QIcon  icon_profile            = Utils::to_icon("󰙄", icon_color);
    static QIcon  icon_chat_room_normal   = Utils::to_icon("󰻞", icon_color);
    static QIcon  icon_chat_room_disabled = Utils::to_icon("󰻞", icon_color_disabled);
    static QIcon  icon_contacts_normal    = Utils::to_icon("󰖸", icon_color);
    static QIcon  icon_contacts_disabled  = Utils::to_icon("󰖸", icon_color_disabled);
    static QIcon  icon_settings           = Utils::to_icon("", icon_color);
    static QIcon  icon_chat_room;
    static QIcon  icon_contacts;
    icon_chat_room.addPixmap(icon_chat_room_normal  .pixmap(size_button), QIcon::Normal  , QIcon::Off);
    icon_chat_room.addPixmap(icon_chat_room_disabled.pixmap(size_button), QIcon::Disabled, QIcon::Off);
    icon_contacts .addPixmap(icon_contacts_normal   .pixmap(size_button), QIcon::Normal  , QIcon::Off);
    icon_contacts .addPixmap(icon_contacts_disabled .pixmap(size_button), QIcon::Disabled, QIcon::Off);
    auto button_set = [](QToolButton* button, const QIcon& icon, const QSize& size = QSize(36, 36)) {
        // QToolButton 默认显示模式为图标 + 文字，因此直接设置时会导致使用默认的小图标
        button->setIcon(icon);
        button->setIconSize(size);
        // 设置按钮大小为图标大小，避免按钮过大导致布局不美观
        button->setFixedSize(size);
        // 默认不显示边框
        button->setAutoRaise(true);
        // 只显示图标，默认是图标 + 文字，因此如果不设置，按钮会显示一个默认的小图标
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        // 设置悬浮提示词
        button->setToolTip(button->text());
    };
    button_set(ui->navigate_button_chat_room, icon_chat_room);
    button_set(ui->navigate_button_contact  , icon_contacts );
    button_set(ui->button_settings          , icon_settings );
    // tooltips
    ui->navigate_button_avatar->setIcon(icon_profile);
    ui->navigate_button_avatar->setIconSize(size_avatar);
    ui->navigate_button_avatar->setFixedSize(size_avatar);
    ui->navigate_button_avatar->setToolTip(ui->navigate_button_avatar->text());
    // 控件居中对齐
    ui->navigate_layout        ->setAlignment(ui->navigate_button_avatar   , Qt::AlignHCenter);
    ui->navigate_layout_contact->setAlignment(ui->navigate_button_chat_room, Qt::AlignHCenter);
    ui->navigate_layout_contact->setAlignment(ui->navigate_button_contact  , Qt::AlignHCenter);
    ui->navigate_layout        ->setAlignment(ui->button_settings          , Qt::AlignHCenter);
    // window icon
    // 设置应用级别的图标后就不需要再手动设置窗口级别的图标了
    // this->setWindowIcon(icon_profile);
    QApplication::setWindowIcon(icon_profile);

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
    ui->page_list_contact  ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    // m_split_contact->setSizes(QList<int>{100, 100});
    ui->widget_contact_list->resize(300, ui->widget_contact_list->height());

    // TODO:设置最小宽度为 ItemChat 宽度 + 滚动条宽度
    // ui->stacked_contact_list->setMinimumWidth(ItemChat().minimumWidth() + ui->page_list_chat->verticalScrollBar()->sizeHint().width());

    // contact view/model/delegate
    ui->page_list_contact->setModel(m_contact_model);
    ui->page_list_contact->setItemDelegate(m_contact_delegate);
    // 使用 currentChanged 信号支持键盘上下键导航和鼠标点击
    connect(ui->page_list_contact->selectionModel(), &QItemSelectionModel::currentChanged, this, &ChatWindow::handle_contact_selection_changed);

    //
    // m_search_window->setWindowFlag(Qt::Window);
    // m_search_window->setAttribute(Qt::WA_DeleteOnClose);  // 关闭窗口时自动销毁内存（防止内存泄漏）
    // m_search_window->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
    // m_search_window->setParent(this);

    // 点击编辑框时使用弹窗替代
    ui->edit_search->installEventFilter(this);

#ifdef DEBUG
    for(auto contact : *m_local_user_data->get_contacts()) {
        add_contact(contact);
    }
#endif // DEBUG
}

ChatWindow::~ChatWindow() {
    if(m_contact_window  ) { delete m_contact_window  ; m_contact_window   = nullptr; }
    if(m_menu_settings   ) { delete m_menu_settings   ; m_menu_settings    = nullptr; }
    if(m_action_signout  ) { delete m_action_signout  ; m_action_signout   = nullptr; }
    if(m_action_settings ) { delete m_action_settings ; m_action_settings  = nullptr; }
    if(m_menu_add        ) { delete m_menu_add        ; m_menu_add         = nullptr; }
    if(m_action_add      ) { delete m_action_add      ; m_action_add       = nullptr; }
    if(m_action_create   ) { delete m_action_create   ; m_action_create    = nullptr; }
    if(m_contact_model   ) { delete m_contact_model   ; m_contact_model    = nullptr; }
    if(m_contact_delegate) { delete m_contact_delegate; m_contact_delegate = nullptr; }
    if(m_search_window   ) { delete m_search_window   ; m_search_window    = nullptr; }
    if(ui                ) { delete ui                ; ui                 = nullptr; }
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

void ChatWindow::handle_contact_selection_changed(const QModelIndex &current, const QModelIndex &previous) {
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

bool ChatWindow::eventFilter(QObject *obj, QEvent *event) {
    if(obj == ui->edit_search) {
        if(event->type() == QEvent::MouseButtonRelease) {
            if(!m_search_window) {
                m_search_window = new SearchWindow;
                // 用于同步显示搜索词
                connect(m_search_window, &SearchWindow::SearchEditTextChanged, ui->edit_search, &QLineEdit::setText);
            }

            Utils::widget_raise(m_search_window);
            return true;
        }
    }
    return QDialog::eventFilter(obj, event);
}
