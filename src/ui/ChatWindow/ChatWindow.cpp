#include "ChatWindow.h"
#include "ui_ChatWindow.h"

ChatWindow::ChatWindow(QWidget *parent)
    : QMainWindow(parent), m_ui(std::make_shared<Ui::ChatWindow>()) {
    m_ui->setupUi(this);
    // 创建对象
    m_menu_settings    = std::make_shared<QMenu  >();
    m_action_signout   = std::make_shared<QAction>();
    m_action_settings  = std::make_shared<QAction>();
    // add
    m_menu_add         = std::make_shared<QMenu  >();
    m_action_add       = std::make_shared<QAction>();
    m_action_create    = std::make_shared<QAction>();
    // model/view
    m_contact_model    = std::make_shared<ContactModel   >();
    m_contact_delegate = std::make_shared<ContactDelegate>();
    // search
    // m_search_window = std::make_shared<SearchWindow>();  // 需要时再创建

    // 连接消息
    // 点击了头像按钮
    connect(m_ui->navigate_button_avatar, &QToolButton::clicked  , this, [&]() {
        // 重建后再显示
        if( m_contact_window) { m_contact_window.reset(); }
        if(!m_contact_window) { m_contact_window = std::make_shared<ContactWindow>(Accounts::instance().get_current_account()); }
        m_contact_window->show();

        // 先显示窗口再更新内容，避免网络请求较慢时窗口无响应
        auto&user_id = Accounts::instance().get_current_account()->baseInfo().userId();
        // 更新成功，刷新显示
        // update(params_update_profile->contact());
        search_user_params params_search_user = std::make_shared<TaoTieChat::SearchUserRequest>();
        params_search_user->setRequestUserAuth(*Accounts::instance().get_auth_request().get());
        params_search_user->setUserId(user_id);
        NetworkClient::instance().search_user(params_search_user, [this](const search_user_params&params_search_user, const search_user_reply&reply_search_user) {
            if(!reply_search_user || !reply_search_user->result().hasSuccessCode()) { return; }
            m_contact_window->update(std::make_shared<ContactInfo>(reply_search_user->contactInfo()));
        });
    });
    // 点击了聊天室按钮
    connect(m_ui->navigate_button_chat_room, &QToolButton::clicked  , this, [&](){
        m_ui->stacked_contact_list     ->setCurrentWidget(m_ui->stacked_page_chat_room);
        m_ui->navigate_button_chat_room->setEnabled(false);
        m_ui->navigate_button_contact  ->setEnabled(true );
    });
    // 点击了联系人按钮
    connect(m_ui->navigate_button_contact, &QToolButton::clicked  , this, [&](){
        m_ui->stacked_contact_list     ->setCurrentWidget(m_ui->stacked_page_contact);
        m_ui->navigate_button_chat_room->setEnabled(true );
        m_ui->navigate_button_contact  ->setEnabled(false);
    });
    // 点击了设置按钮
    connect(m_ui->button_settings  , &QToolButton::clicked, this, [&](){ this->on_show_settings_menu(); });
    // 点击了添加按钮
    connect(m_ui->button_add       , &QToolButton::clicked, this, [&](){ this->on_show_add_menu     (); });
    // 各种菜单事件
    connect(m_action_signout .get(), &QAction::triggered, this, [&](){ emit signout(); });
    connect(m_action_settings.get(), &QAction::triggered, this, [&](){ qDebug() << "m_action_settings"; });
    connect(m_action_add     .get(), &QAction::triggered, this, [&](){ qDebug() << "m_action_add"     ; });
    connect(m_action_create  .get(), &QAction::triggered, this, [&](){ qDebug() << "m_action_create"  ; });
    // 连接聊天列表点击事件
    // connect(m_ui->page_list_chat_room, &QListView::itemClicked, this, &ChatWindow::on_chat_room_item_clicked);
    // // 使用 currentChanged 信号支持键盘上下键导航和鼠标点击
    // connect(m_ui->page_list_contact->selectionModel(), &QItemSelectionModel::currentChanged, this, &ChatWindow::on_contact_selection_changed);
    // 用于同步显示搜索词
    // connect(m_search_window, &SearchWindow::SearchEditTextChanged, m_ui->edit_search, &QLineEdit::setText);

    // 执行操作
    m_ui->chat_splitter->setStretchFactor(0, 0); // 编号 0 的窗口拉伸因子为 0 // 整个窗口调整大小时左侧宽度不调整
    m_ui->chat_splitter->setStretchFactor(1, 1);

    // 设置图标
    static Utils::UtilsFontIcons ufi;
    auto button_set = [](QToolButton* button, const QIcon& icon, const QSize& size = QSize(36, 36)) {
        // QToolButton 默认显示模式为图标 + 文字，因此直接设置时会导致使用默认的小图标
        button->setIcon     (icon);
        button->setIconSize (size);
        // 设置按钮大小为图标大小，避免按钮过大导致布局不美观
        button->setFixedSize(size);
        // 默认不显示边框
        button->setAutoRaise(true);
        // 只显示图标，默认是图标 + 文字，因此如果不设置，按钮会显示一个默认的小图标
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        // 设置悬浮提示词
        button->setToolTip(button->text());
    };
    button_set(m_ui->navigate_button_chat_room, ufi.icon_chat_room);
    button_set(m_ui->navigate_button_contact  , ufi.icon_contacts );
    button_set(m_ui->button_settings          , ufi.icon_settings );
    // tooltips
    m_ui->navigate_button_avatar->setIcon     (ufi.icon_profile);
    m_ui->navigate_button_avatar->setIconSize (ufi.size_avatar );
    m_ui->navigate_button_avatar->setFixedSize(ufi.size_avatar );
    m_ui->navigate_button_avatar->setToolTip  (m_ui->navigate_button_avatar->text());
    // 控件居中对齐
    m_ui->navigate_layout        ->setAlignment(m_ui->navigate_button_avatar   , Qt::AlignHCenter);
    m_ui->navigate_layout_contact->setAlignment(m_ui->navigate_button_chat_room, Qt::AlignHCenter);
    m_ui->navigate_layout_contact->setAlignment(m_ui->navigate_button_contact  , Qt::AlignHCenter);
    m_ui->navigate_layout        ->setAlignment(m_ui->button_settings          , Qt::AlignHCenter);
    // window icon
    // 设置应用级别的图标后就不需要再手动设置窗口级别的图标了
    QApplication::setWindowIcon(ufi.icon_profile);

    // 设置菜单
    // m_ui->navigate_button_menu->setMenu(m_menu_settings);
    // m_ui->navigate_button_menu->setPopupMode(QToolButton::InstantPopup);        // 点击按钮立即弹出菜单  // 带下三角
    // // m_ui->navigate_button_menu->setPopupMode(QToolButton::MenuButtonPopup);  // 分隔符 + 下三角
    // // m_ui->navigate_button_menu->setPopupMode(QToolButton::DelayedPopup);     // 带下三角
    m_action_settings->setText     (tr("settings"));
    m_action_settings->setDisabled (true); // TODO:功能未实现，暂时禁用
    m_action_signout ->setText     (tr("Signout"));
    m_menu_settings  ->addAction   (m_action_settings.get());
    m_menu_settings  ->addSeparator();
    m_menu_settings  ->addAction   (m_action_signout.get());
    // add
    m_action_add   ->setText   (tr("Add as a friend or join a group"));
    m_action_create->setText   (tr("Create a group"));
    m_menu_add     ->addAction (m_action_add   .get());
    m_menu_add     ->addAction (m_action_create.get());
    m_action_add   ->setEnabled(false); // TODO:功能未实现，暂时禁用
    m_action_create->setEnabled(false); // TODO:功能未实现，暂时禁用

    // 设置列表属性
    m_ui->page_list_chat_room->setSpacing(0);
    // 设置按像素滚动列表内容
    m_ui->page_list_chat_room->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_ui->page_list_contact  ->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    // m_split_contact->setSizes(QList<int>{100, 100});
    m_ui->widget_contact_list->resize(300, m_ui->widget_contact_list->height());

    // TODO:设置最小宽度为 ItemChat 宽度 + 滚动条宽度
    // m_ui->stacked_contact_list->setMinimumWidth(ItemChat().minimumWidth() + m_ui->page_list_chat->verticalScrollBar()->sizeHint().width());

    // contact view/model/delegate
    m_ui->page_list_contact->setModel       (m_contact_model   .get());
    m_ui->page_list_contact->setItemDelegate(m_contact_delegate.get());
    // 使用 currentChanged 信号支持键盘上下键导航和鼠标点击
    connect(m_ui->page_list_contact->selectionModel(), &QItemSelectionModel::currentChanged, this, &ChatWindow::handle_contact_selection_changed);

    //
    // m_search_window->setWindowFlag(Qt::Window);
    // m_search_window->setAttribute(Qt::WA_DeleteOnClose);  // 关闭窗口时自动销毁内存（防止内存泄漏）
    // m_search_window->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
    // m_search_window->setParent(this);

    // 点击编辑框时使用弹窗替代
    m_ui->edit_search->installEventFilter(this);

    // 设置要初始化显示的页面
    m_ui->navigate_button_chat_room->click();  // 初始化状态
}

ChatWindow::~ChatWindow() {
    if(m_contact_window  ) { m_contact_window  .reset(); }
    if(m_menu_settings   ) { m_menu_settings   .reset(); }
    if(m_action_signout  ) { m_action_signout  .reset(); }
    if(m_action_settings ) { m_action_settings .reset(); }
    if(m_menu_add        ) { m_menu_add        .reset(); }
    if(m_action_add      ) { m_action_add      .reset(); }
    if(m_action_create   ) { m_action_create   .reset(); }
    if(m_contact_model   ) { m_contact_model   .reset(); }
    if(m_contact_delegate) { m_contact_delegate.reset(); }
    if(m_search_window   ) { m_search_window   .reset(); }
    if(m_ui              ) { m_ui              .reset(); }
}

void ChatWindow::on_show_settings_menu() {
    // 在按钮上方弹出
    m_menu_settings->adjustSize(); // 内存调整为实际大小
    QPoint pos = m_ui->button_settings->mapToGlobal(m_ui->button_settings->rect().bottomRight());
    int h = m_menu_settings->size().height();
    m_menu_settings->popup(QPoint(pos.x(), pos.y() - h));
}

void ChatWindow::on_show_add_menu() {
    // 在按钮下方左侧弹出
    m_menu_add->adjustSize(); // 内存调整为实际大小
    QPoint pos = m_ui->button_add->mapToGlobal(m_ui->button_add->rect().bottomRight());
    int w = m_menu_add->size().width();
    m_menu_add->popup(QPoint(pos.x() - w, pos.y()));
}

void ChatWindow::on_chat_room_item_selected(const QModelIndex &index) {
    qDebug() << "index : " << index;
}

void ChatWindow::on_chat_room_item_activated(const QModelIndex &index) {
    qDebug() << "index : " << index;
}

void ChatWindow::add_chat_room(const std::shared_ptr<ContactInfo>&contact_info) {
    // qDebug() << "contact : " << ContactManager(contact).get_nick_name();
}

void ChatWindow::show_chat_room_messages(const std::shared_ptr<ContactInfo>&contact_info) {
    // qDebug() << "contact : " << ContactManager(contact).get_nick_name();
}

void ChatWindow::add_contact(const std::shared_ptr<ContactInfo>&contact_info) {
    // qDebug() << "contact : " << ContactManager(contact).get_nick_name();
    m_contact_model->add_contact(contact_info);
}

void ChatWindow::handle_contact_selection_changed(const QModelIndex &current, const QModelIndex &previous) {
    // 索引值需要有效
    if (!current.isValid()) {
        return;
    }
    // 能够获取到有效的联系人信息
    const std::shared_ptr<ContactInfo>&contact_info = m_contact_model->get_contact(current);
    if (!contact_info) {
        return;
    }
    // 查找是否已经添加
    ContactWindow *contact_window_update = nullptr;  // 需要更新的窗口
    for(int idx = 0; idx < m_ui->stacked_contact_detail->count(); ++idx) {
        ContactWindow*contact_window_already = dynamic_cast<ContactWindow*>(m_ui->stacked_contact_detail->widget(idx));
        if(!contact_window_already) {
            break;
        }
        const std::shared_ptr<ContactInfo>&contact_info_already = contact_window_already->get_contact();
        if(contact_info->baseInfo().accountId() == contact_info_already->baseInfo().accountId() &&
           contact_info->baseInfo().userId   () == contact_info_already->baseInfo().userId   ()) {
            contact_window_update = contact_window_already;  // 找到需要更新的窗口
            break;
        }
    }
    // 如果已经添加则更新信息
    if(contact_window_update) {
        contact_window_update->update(contact_info);
    }
    // 如果没有则添加
    else {
        contact_window_update = new ContactWindow(contact_info, this);
        m_ui->stacked_contact_detail->addWidget(contact_window_update);
    }
    // 显示
    m_ui->stacked_contact_detail->setCurrentWidget(contact_window_update);
}

bool ChatWindow::eventFilter(QObject *obj, QEvent *event) {
    if(obj == m_ui->edit_search) {
        if(event->type() == QEvent::MouseButtonRelease) {
            if(!m_search_window) {
                m_search_window = std::make_shared<SearchWindow>();
                // 用于同步显示搜索词
                connect(m_search_window.get(), &SearchWindow::SearchEditTextChanged, m_ui->edit_search, &QLineEdit::setText);
            }

            Utils::widget_raise(m_search_window.get());
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}
