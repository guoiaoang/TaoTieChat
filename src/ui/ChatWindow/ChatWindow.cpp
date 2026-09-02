#include "ChatWindow.h"
#include "ui_ChatWindow.h"

#include "NetworkClient.h"
#include "Accounts.h"
#include "Database.h"
#include "ContactWindow.h"
#include "SearchWindow.h"
#include "ItemModel.h"
#include "ItemDelegate.h"
#include "MessageWindow.h"

ChatWindow::ChatWindow(QWidget *parent) : QMainWindow(parent) {}

ChatWindow::~ChatWindow() {
    release_resources();
}

void ChatWindow::reinit() {
    release_resources();
    init();
}

void ChatWindow::release_resources() {
    m_ui.reset();
    // 创建对象
    m_menu_settings         .reset();
    m_action_settings       .reset();
    m_action_rember_password.reset();
    m_action_auto_login     .reset();
    m_action_signout        .reset();
    m_action_exit           .reset();
    // add
    m_menu_add     .reset();
    m_action_add   .reset();
    m_action_create.reset();
    // model/view
    m_contact_model   .reset();
    m_contact_delegate.reset();
    // 退出时需要关闭及重置的页面
    m_search_window .reset();  // 搜素页面
    m_contact_window.reset();  // 点击头像时显示的个人信息页面
    //
    m_cache_self_info  .reset();
    m_cache_self_avatar.reset();
    m_chatroom_proxy   .reset();

    // 清除好友关系等
    // Accounts::instance().set_current_account(std::make_shared<ContactInfo>());  // 登录时会使用该内容，不能在这里清除
    Accounts::instance().set_contact_list(std::make_shared<ContactInfos>());

    // 单客户端单账号：退出登录时释放管道，允许下一次登录
    Utils::OS::try_grab_account_pipe(Accounts::instance().get_current_account()->baseInfo().userId(), false);

    // 断开实时同步的连接网络
    // NetworkClient::instance().reset_connection();
    NetworkClient::instance().disconnect_sync(Accounts::instance().get_current_account()->baseInfo().accountId());
}

void ChatWindow::init() {
    // 重新初始化界面及数据
    m_ui = std::make_shared<Ui::ChatWindow>();
    m_ui->setupUi(this);
    // 创建对象
    m_menu_settings    = std::make_shared<QMenu  >();
    m_action_settings  = std::make_shared<QAction>();
    m_action_rember_password = std::make_shared<QAction>();
    m_action_auto_login      = std::make_shared<QAction>();
    m_action_signout   = std::make_shared<QAction>();
    m_action_exit      = std::make_shared<QAction>();
    // add
    m_menu_add         = std::make_shared<QMenu  >();
    m_action_add       = std::make_shared<QAction>();
    m_action_create    = std::make_shared<QAction>();
    // model/view/proxy
    m_chatroom_model   = std::make_shared<ChatroomModel>();
    m_contact_model    = std::make_shared<ContactModel >();
    m_contact_delegate = std::make_shared<ItemDelegate >();
    m_chatroom_proxy   = std::make_shared<ChatroomProxy>();

    // 连接消息
    connect(m_ui->navigate_button_avatar   , &QToolButton::clicked, this, &ChatWindow::slot_button_navigate_avatar ); // 点击了头像按钮
    connect(m_ui->navigate_button_chat_room, &QToolButton::clicked, this, &ChatWindow::slot_button_navigate_message); // 点击了聊天室按钮
    connect(m_ui->navigate_button_contact  , &QToolButton::clicked, this, &ChatWindow::slot_button_navigate_contact); // 点击了联系人按钮
    connect(m_ui->button_settings          , &QToolButton::clicked, this, [&](){ this->on_show_settings_menu(); }); // 点击了设置按钮
    connect(m_ui->button_add               , &QToolButton::clicked, this, [&](){ this->on_show_add_menu     (); }); // 点击了添加按钮
    // 各种菜单事件
    connect(m_action_signout .get(), &QAction::triggered, this, [&](){
        release_resources();
        emit signout();
    });
    connect(m_action_exit    .get(), &QAction::triggered, this, [&](){ QApplication::exit(0); });
    connect(m_action_settings.get(), &QAction::triggered, this, [&](){ qDebug() << "m_action_settings"; });
    connect(m_action_add     .get(), &QAction::triggered, this, [&](){ qDebug() << "m_action_add"     ; });
    connect(m_action_create  .get(), &QAction::triggered, this, [&](){ qDebug() << "m_action_create"  ; });
    // 连接聊天列表点击事件
    // connect(m_ui->page_list_chat_room, &QListView::itemClicked, this, &ChatWindow::on_chat_room_item_clicked);
    // // 使用 currentChanged 信号支持键盘上下键导航和鼠标点击
    // connect(m_ui->page_list_contact->selectionModel(), &QItemSelectionModel::currentChanged, this, &ChatWindow::on_contact_selection_changed);
    // 用于同步显示搜索词
    // connect(m_search_window, &SearchWindow::SearchEditTextChanged, m_ui->edit_search, &QLineEdit::setText);
    connect(&Accounts     ::instance(), &Accounts     ::signal_account_changed   , this, &ChatWindow::current_accounut_info_changed);
    connect(&NetworkClient::instance(), &NetworkClient::signal_file_id_downloaded, this, &ChatWindow::slot_file_id_downloaded      );
    connect(m_action_rember_password.get(), &QAction::triggered, this, [&](){
        auto current_account = Accounts::instance().get_current_account();
        if (!current_account) { return; }
        // 创建副本，不要修改原先的数据，防止恢复为保存登陆信息时数据丢失
        std::shared_ptr<AuthOpt> auth_opt = std::make_shared<AuthOpt>(*Accounts::instance().get_auth_opt(current_account->baseInfo().accountId()).get());
        if (!auth_opt) { return; }
        // 保存数据
        auth_opt->rember_password = m_action_rember_password->isChecked();
        auth_opt->auto_login      = m_action_rember_password->isChecked() && m_action_auto_login->isChecked();  // 记住密码才允许自动登陆
        auth_opt->password        = !auth_opt->rember_password ? QString{} : auth_opt->password;
        auth_opt->token           = !auth_opt->rember_password ? QString{} : auth_opt->token   ;
        AuthOptRW::save(auth_opt);
        // 设置到界面
        m_action_rember_password->setChecked(auth_opt->rember_password);
        m_action_auto_login     ->setChecked(auth_opt->auto_login     );
    });
    connect(m_action_auto_login     .get(), &QAction::triggered, this, [&](){
        auto current_account = Accounts::instance().get_current_account();
        if (!current_account) { return; }
        // 创建副本，不要修改原先的数据，防止恢复为保存登陆信息时数据丢失
        std::shared_ptr<AuthOpt> auth_opt = std::make_shared<AuthOpt>(*Accounts::instance().get_auth_opt(current_account->baseInfo().accountId()).get());
        if (!auth_opt) { return; }
        // 保存数据
        auth_opt->rember_password = m_action_rember_password->isChecked() || m_action_auto_login->isChecked();  // 自动登陆必须记住密码
        auth_opt->auto_login      = m_action_auto_login->isChecked();
        auth_opt->password        = !auth_opt->rember_password ? QString{} : auth_opt->password;
        auth_opt->token           = !auth_opt->rember_password ? QString{} : auth_opt->token   ;
        AuthOptRW::save(auth_opt);
        // 设置到界面
        m_action_rember_password->setChecked(auth_opt->rember_password);
        m_action_auto_login     ->setChecked(auth_opt->auto_login     );
    });
    // 支持单击展开群组
    connect(m_ui->page_list_contact, &QTreeView::clicked, this, &ChatWindow::handle_contact_item_clicked);
    // // 使用 currentChanged 信号支持键盘上下键导航和鼠标点击  // 需要先 setModel 才能连接
    // connect(m_ui->page_list_contact->selectionModel(), &QItemSelectionModel::currentChanged, this, &ChatWindow::handle_contact_selection_changed);
    // 发送消息
    connect(m_contact_window.get(), &ContactWindow::sig_contact_message, this, &ChatWindow::slot_message_to);
    connect(m_contact_window.get(), &ContactWindow::sig_contact_voice  , this, &ChatWindow::slot_voice_to  );
    connect(m_contact_window.get(), &ContactWindow::sig_contact_video  , this, &ChatWindow::slot_video_to  );
    // 点击聊天室列表中的项目
    connect(m_ui->page_list_chat_room, &QTreeView::clicked, this, &ChatWindow::handle_chatroom_item_clicked);
    // // 使用 currentChanged 信号支持键盘上下键导航和鼠标点击  // 需要先 setModel 才能连接
    // connect(m_ui->page_list_chat_room->selectionModel(), &QItemSelectionModel::currentChanged, this, &ChatWindow::handle_chatroom_selection_changed);

    // 执行操作
    m_ui->chat_splitter->setStretchFactor(0, 0); // 编号 0 的窗口拉伸因子为 0 // 整个窗口调整大小时左侧宽度不调整
    m_ui->chat_splitter->setStretchFactor(1, 1);

    // 设置图标
    static Utils::FontIcons font_icon =  Utils::FontIcons().update();
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
    button_set(m_ui->navigate_button_chat_room, font_icon.icon.chat_room);
    button_set(m_ui->navigate_button_contact  , font_icon.icon.contacts );
    button_set(m_ui->button_settings          , font_icon.icon.settings );
    // tooltips
    m_ui->navigate_button_avatar->setIcon     (font_icon.icon.profile);
    m_ui->navigate_button_avatar->setIconSize (font_icon.size.avatar );
    m_ui->navigate_button_avatar->setFixedSize(font_icon.size.avatar );
    m_ui->navigate_button_avatar->setToolTip  (m_ui->navigate_button_avatar->text());
    // 控件居中对齐
    m_ui->navigate_layout        ->setAlignment(m_ui->navigate_button_avatar   , Qt::AlignHCenter);
    m_ui->navigate_layout_contact->setAlignment(m_ui->navigate_button_chat_room, Qt::AlignHCenter);
    m_ui->navigate_layout_contact->setAlignment(m_ui->navigate_button_contact  , Qt::AlignHCenter);
    m_ui->navigate_layout        ->setAlignment(m_ui->button_settings          , Qt::AlignHCenter);
    // window icon
    // 设置应用级别的图标后就不需要再手动设置窗口级别的图标了
    QApplication::setWindowIcon(font_icon.icon.profile);

    auto current_account = Accounts::instance().get_current_account();
    auto auth_opt = Accounts::instance().get_auth_opt(!current_account ? QString{} : current_account->baseInfo().accountId());

    // 设置菜单
    // m_ui->navigate_button_menu->setMenu(m_menu_settings);
    // m_ui->navigate_button_menu->setPopupMode(QToolButton::InstantPopup);        // 点击按钮立即弹出菜单  // 带下三角
    // // m_ui->navigate_button_menu->setPopupMode(QToolButton::MenuButtonPopup);  // 分隔符 + 下三角
    // // m_ui->navigate_button_menu->setPopupMode(QToolButton::DelayedPopup);     // 带下三角
    // 记住密码及自动登陆
    m_action_rember_password->setText     (tr("Rember password"));
    m_action_rember_password->setCheckable(true);
    m_action_rember_password->setChecked  (!auth_opt ? false : auth_opt->rember_password);
    m_action_auto_login     ->setText     (tr("Auto login"));
    m_action_auto_login     ->setCheckable(true);
    m_action_auto_login     ->setChecked  (!auth_opt ? false : auth_opt->auto_login);
    m_action_settings->setText     (tr("Settings"));
    m_action_settings->setDisabled (true); // TODO:功能未实现，暂时禁用
    m_action_signout ->setText     (tr("Signout"));  // 退出到登陆界面
    m_action_exit    ->setText     (tr("Exit"   ));  // 退出程序
    m_menu_settings  ->addAction   (m_action_settings.get());
    m_menu_settings  ->addSeparator();
    m_menu_settings  ->addAction   (m_action_rember_password.get());
    m_menu_settings  ->addAction   (m_action_auto_login     .get());
    m_menu_settings  ->addSeparator();
    m_menu_settings  ->addAction   (m_action_signout        .get());
    m_menu_settings  ->addSeparator();
    m_menu_settings  ->addAction   (m_action_exit           .get());
    // add
    m_action_add   ->setText   (tr("Add as a friend or join a group"));
    m_action_create->setText   (tr("Create a group"));
    m_menu_add     ->addAction (m_action_add   .get());
    m_menu_add     ->addAction (m_action_create.get());
    m_action_add   ->setEnabled(false); // TODO:功能未实现，暂时禁用
    m_action_create->setEnabled(false); // TODO:功能未实现，暂时禁用

    // 设置列表属性
    // m_ui->page_list_chat_room->setSpacing(0);
    // 设置按像素滚动列表内容
    m_ui->page_list_chat_room->setVerticalScrollMode(QListView::ScrollPerPixel);
    m_ui->page_list_contact  ->setVerticalScrollMode(QListView::ScrollPerPixel);
    // 禁用水平滚动条，防止由于局部测宽抖动引发滚动条乱闪
    m_ui->page_list_chat_room->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_ui->page_list_contact  ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // // 设置视图尺寸变更模式为实时重新包裹自适应大小
    // m_ui->page_list_chat_room->setResizeMode(QListView::Adjust);
    // m_ui->page_list_contact  ->setResizeMode(QListView::Adjust);
    // m_split_contact->setSizes(QList<int>{100, 100});
    m_ui->widget_contact_list->resize(300, m_ui->widget_contact_list->height());

    // TODO:设置最小宽度为 ItemChat 宽度 + 滚动条宽度
    // m_ui->stacked_contact_list->setMinimumWidth(ItemChat().minimumWidth() + m_ui->page_list_chat->verticalScrollBar()->sizeHint().width());

    // contact view/model/delegate
    m_ui->page_list_contact->setModel(m_contact_model.get());
    m_ui->page_list_contact->setItemDelegate(m_contact_delegate.get());
    m_ui->page_list_contact->setHeaderHidden(true); // 隐藏表头，看起来更像 QQ 列表
    m_ui->page_list_contact->setIndentation(0); // m_ui->page_list_contact->indentation() / 2);  // 设置分组缩进  // 必须
    m_ui->page_list_contact->setAnimated(false);  // 禁用展开/折叠动画，加速加载
    m_ui->page_list_contact->setExpandsOnDoubleClick(false);  // 改成单击展开
    // m_ui->page_list_contact->setUniformRowHeights(true);  // 统一行高，避免逐行计算高度
    // m_ui->page_list_contact->expandAll();  // 添加数据后才能展开
    // 设置最小折叠宽度
    m_ui->page_list_contact      ->setMinimumWidth(200);  // 有效  // 建议
    // m_ui->stacked_contact_list->setMinimumWidth(300);  // 有效
    // m_ui->widget_contact_list ->setMinimumWidth(400);  // 有效
    // // 使用 currentChanged 信号支持键盘上下键导航和鼠标点击  // 需要先 setModel 才能连接
    connect(m_ui->page_list_contact->selectionModel(), &QItemSelectionModel::currentChanged, this, &ChatWindow::handle_contact_selection_changed);

    // chatroom
    m_chatroom_proxy->setSourceModel(m_chatroom_model.get());  // 将原始 Model 设置给代理 Model
    // m_chatroom_proxy->sort(0, Qt::AscendingOrder); // 按第 0 列升序排序
    m_chatroom_proxy->sort(0, Qt::DescendingOrder); // 按第 0 列倒序排序
    m_chatroom_proxy->setDynamicSortFilter(true);  // 开启动态排序（当 Model 内部数据改变时，自动重新排序）
    m_ui->page_list_chat_room->setModel(m_chatroom_proxy.get());  // 这里传入 Proxy Model
    m_ui->page_list_chat_room->setItemDelegate(m_contact_delegate.get());
    m_ui->page_list_chat_room->setHeaderHidden(true); // 隐藏表头，看起来更像 QQ 列表
    m_ui->page_list_chat_room->setIndentation(0); // m_ui->page_list_contact->indentation() / 2);  // 设置分组缩进  // 必须
    m_ui->page_list_chat_room->setAnimated(false);  // 禁用展开/折叠动画，加速加载
    m_ui->page_list_chat_room->setUniformRowHeights(true);  // 统一行高，避免逐行计算高度
    // m_ui->page_list_chat_room->setExpandsOnDoubleClick(false);  // 改成单击展开
    // m_ui->page_list_chat_room->expandAll();  // 添加数据后才能展开
    // 设置消息显示列表最小折叠宽度，防止消息显示不全、错位
    // m_ui->page_list_detail->setMinimumWidth(200);
    m_ui->page_list_detail->setMinimumWidth(400);
    // 使用 currentChanged 信号支持键盘上下键导航和鼠标点击  // 需要先 setModel 才能连接
    connect(m_ui->page_list_chat_room->selectionModel(), &QItemSelectionModel::currentChanged, this, &ChatWindow::handle_chatroom_selection_changed);
    // // 绑定搜索框文本改变信号
    // connect(m_ui->edit_search, &QLineEdit::textChanged, this, [this](const QString &text) {
    //     // 设置过滤关键词（区分/不区分大小写）
    //     // m_chatroom_proxy->setFilterKeyColumn(0); // 仅过滤第 0 列
    //     // m_chatroom_proxy->setFilterRole(ENUMS::role_item_data()); // 或 Qt::UserRole 自定义数据角色
    //     m_chatroom_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    //     m_chatroom_proxy->setFilterFixedString(text);
    // });

    // 添加好友分组
    m_contact_model->add_group(ENUMS::ItemGroupType::REQUEST);
    m_contact_model->add_group(ENUMS::ItemGroupType::FRIENDS);
    // 联系人群组初始化时始终展开
    auto index_group = m_contact_model->index_group(ENUMS::ItemGroupType::FRIENDS);
    if (index_group.isValid() && index_group.model()->hasChildren(index_group)) {
        m_ui->page_list_contact->expand(index_group);
    }

    // TODO: network -> data -> notify -> gui

#ifdef _DEBUG
    if (0) {
        // m_contact_model->add_member(ENUMS::ItemGroupType::REQUEST, Accounts::instance().get_current_account());
        // m_contact_model->add_member(ENUMS::ItemGroupType::FRIENDS, Accounts::instance().get_current_account());
        for (int i = 0; i <= 300; ++i) {
            auto contact = std::make_shared<ContactInfo>();
            auto num = QString("%1").arg(i, 5, 10, '0');
            auto time = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
            contact->mutBaseInfo().setAccountId(QString("Account ID : %1 (%2)").arg(num).arg(time));
            contact->mutBaseInfo().setUserId   (QString("User ID    : %1 (%2)").arg(num).arg(time));
            contact->mutBaseInfo().setNickname (QString("Nickname   : %1 (%2)").arg(num).arg(time));
            contact->mutBaseInfo().setSignature(QString("Signature  : %1 (%2)").arg(num).arg(time));
            m_contact_model->add_member(ENUMS::ItemGroupType::REQUEST, contact);
        }
        // auto index_group = m_contact_model->index_group(ENUMS::ItemGroupType::REQUEST);
        // if (index_group.isValid() && index_group.model()->hasChildren(index_group)) {
        //     m_ui->page_list_contact->expand(index_group);
        // }
        for (auto group_type : std::vector<ENUMS::ItemGroupType>({ ENUMS::ItemGroupType::REQUEST, ENUMS::ItemGroupType::FRIENDS })) {
            auto index_group = m_contact_model->index_group(group_type);
            if (index_group.isValid() && index_group.model()->hasChildren(index_group)) {
                m_ui->page_list_contact->expand(index_group);
            }
        }
        // m_ui->page_list_contact->expandAll();  // 会展开无成员的节点
    }
#endif // __DEBUG

    //
    // m_search_window->setWindowFlag(Qt::Window);
    // m_search_window->setAttribute(Qt::WA_DeleteOnClose);  // 关闭窗口时自动销毁内存（防止内存泄漏）
    // m_search_window->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
    // m_search_window->setParent(this);

    // 点击编辑框时使用弹窗替代
    m_ui->edit_search->installEventFilter(this);
    m_ui->page_list_contact->installEventFilter(this);  // debug

    // 设置要初始化显示的页面
    m_ui->navigate_button_chat_room->click();  // 初始化状态

    // 更新个人信息显示  // 初始化赋值，需要创建副本，而不是共享同一片内存
    auto copy_contact_info = std::make_shared<ContactInfo>(*Accounts::instance().get_current_account().get());
    // set_cache_contact_info(copy_contact_info);
    update_contact_info(copy_contact_info);  // 更新信息为 copy_contact_info

    // 从网络获取当前用户的个人信息并更新显示
    updata_current_account_info();  // 从网络或更新

    // 消息同步
    network_status_sync();

    // TODO:未添加的功能
    m_ui->button_add->setVisible(false);
}

bool ChatWindow::updata_current_account_info() {
    // 从网络获取当前用户的个人信息并更新显示
    auto&user_id = Accounts::instance().get_current_account()->baseInfo().userId();
    search_user_params params_search_user = std::make_shared<SearchUserRequest>();
    params_search_user->setRequestUserAuth(*Accounts::instance().get_auth_request().get());
    params_search_user->setUserId(user_id);
    NetworkClient::instance().search_user(params_search_user, [this](const search_user_params&params_search_user, const search_user_reply&reply_search_user) {
        if (!reply_search_user || !reply_search_user->result().hasSuccessCode()) { return; }
        // update_contact_info(std::make_shared<ContactInfo>(*reply_search_user->contactInfo().get()));
        Accounts::instance().set_current_account(std::make_shared<ContactInfo>(reply_search_user->contactInfo()));
    });
    return false;
}

bool ChatWindow::network_status_sync() {
    auto get_local_last_id = [](SyncType type) {
        return 0;
    };
    // get last id
    get_last_id_params   params_get_last_id   = Accounts::instance().get_auth_request();
    get_last_id_callback callback_get_last_id = [&](const get_last_id_params&params_get_last_id, const get_last_id_reply&reply_get_last_id) {
        if (!reply_get_last_id) { return; }
        if (!reply_get_last_id->hasResult()) { return; }
        if (!reply_get_last_id->result().hasSuccessCode()) { return; }

        QList<TaoTieChat::SyncTag> sync_tags;  // 需要更新的消息
        for (auto&tag : reply_get_last_id->tags()) {
            auto server_last_id = tag.idEnd();
            auto local_last_id = get_local_last_id(tag.type());
            auto type_name = QMetaEnum::fromType<SyncType>().valueToKey(quint64(tag.type()));
            qDebug().noquote().nospace() << "get_last_id SyncType : " << type_name << ", server_last_id : " << server_last_id << ", local_last_id : " << local_last_id;
            if (server_last_id > local_last_id) {
                SyncTag sync_tag;
                sync_tag.setType   (tag.type());
                sync_tag.setIdStart(local_last_id );
                sync_tag.setIdEnd  (server_last_id);
                sync_tags.emplace_back(sync_tag);
            }
        }
        // 触发消息同步消息
        // 同步监听
        {
            struct SyncNotify{
                bool inited = false;
                bool error = false;
            };
            std::shared_ptr<SyncNotify> notify_sync = std::make_shared<SyncNotify>();
            //
            sync_params params_sync = std::make_shared<SyncRequest>();
            params_sync->setRequestUserAuth(*Accounts::instance().get_auth_request().get());
            params_sync->setSyncTag(sync_tags);
            //
            sync_callback callback_sync = [&, notify_sync](const sync_params&params_sync, const sync_reply&reply_sync) {
                // 鉴权结果
                if (!notify_sync->inited) {
                    if (!reply_sync->hasHeader() && !reply_sync->header().hasResult() && !reply_sync->header().result().hasSuccessCode()) {
                        notify_sync->error = true;
                        return;
                    }
                    notify_sync->inited = true;
                    return;
                }
                // 进行同步
                // qDebug().noquote().nospace() << "Get Sync Messages";
                if (!reply_sync->hasBody()) { return; }
                auto sync_reply_body = reply_sync->body();
                auto sync_tag        = sync_reply_body.tag();
                auto count           = sync_tag.count();

                qDebug().noquote().nospace() << "sync_reply : " << reply_sync;

                // 服务端数据异常或无更新时 `hasContacts() == false`
                if (sync_tag.type() == SyncType::SYNC_CONTACTS && sync_reply_body.hasContacts()) {
                    for (auto&sync_contact : sync_reply_body.contacts().contact()) {
                        // 更新联系人信息
                        // qDebug().nospace().noquote() << "Sync ContactInfo for AccountId : " << contact_info.baseInfo().accountId();
                        if (sync_contact.subType() == SyncSubTypeContact::SYNC_CONTACT_REUQEST) {
                            auto sync_contact_request = sync_contact.request();
                            auto request_contact = sync_contact_request.contactRequest();  // 申请者的信息
                            auto request_time    = sync_contact_request.time();
                            auto request_message = sync_contact_request.message();
                            qDebug().noquote().nospace() << "Get sync contact request. form : " << request_contact << ", time : " << request_time << ", message : " << request_message;
                            // qDebug().noquote().nospace() << "Contacts : " << Accounts::instance().get_contact_list()->contactInfos();
                            // 添加到联系人列表中，但标记为申请好友（放入申请分组）
                            add_contact(std::make_shared<ContactInfo>(request_contact), ENUMS::ItemGroupType::REQUEST);
                            // 获取头像，防止列表中头像显示异常
                            download_contact_avatar(std::make_shared<ContactInfo>(request_contact));
                            // qDebug().noquote().nospace() << "Contacts : " << Accounts::instance().get_contact_list()->contactInfos();
                        }
                        else if (sync_contact.subType() == SyncSubTypeContact::SYNC_CONTACT_REPLY) {
                            // 通过好友申请时无需获取好友申请结果，获取服务器直接推送的好友变更结果即可
                            if (sync_contact.hasShip() && sync_contact.ship().hasContact()) {
                                auto contact_info = sync_contact.ship().contact();
                                qDebug().noquote().nospace() << "Contact updata : " << contact_info;

                                // 更新好友列表  // 使用昵称及用户 ID 排序，不使用账号 ID 排序 // 联系人列表的排序不在这
                                auto contact_infos = Accounts::instance().get_contact_list()->contactInfos();
                                contact_infos.append(contact_info);
                                // TODO:排序函数未验证
                                auto func_contact_sort = [](const ContactInfo&l, const ContactInfo&r) {
                                    if (!l.hasBaseInfo() || !r.hasBaseInfo()) { return false; }
                                    auto lb = l.baseInfo(); auto ln = lb.nickname(); auto lu = lb.userId();
                                    auto rb = r.baseInfo(); auto rn = rb.nickname(); auto ru = rb.userId();
                                    if (ln != rn) { return ln < rn; }
                                    else if (ln == rn && lu != ru) { return lu < ru; }
                                    return false;
                                };
                                std::sort(contact_infos.begin(), contact_infos.end(), func_contact_sort);
                                auto cis = std::make_shared<ContactInfos>();
                                cis->setContactInfos(contact_infos);
                                Accounts::instance().set_contact_list(cis);  // notify. TODO:shared_ptr -> *

                                // 可以放到任务队列中触发的部分操作
                                // 添加到联系人列表中  // TODO:不兼容上面的排序函数
                                add_contact(std::make_shared<ContactInfo>(contact_info), ENUMS::ItemGroupType::FRIENDS);
                                // 获取头像，防止列表中头像显示异常
                                download_contact_avatar(std::make_shared<ContactInfo>(contact_info));
                            }
                        }
                    }
                }
                // 服务端只区分了 SyncType::MESSAGES，文件也是 SyncType::MESSAGES 的一个子类型，需要客户端手动测试
                else if (sync_tag.type() == SyncType::SYNC_MESSAGES && sync_reply_body.hasMessages()) {
                    for (auto&message : sync_reply_body.messages().message()) {
                        // 信息未必来自同一对话，暂不处理为批量添加
                        add_message(std::make_shared<SyncMessage>(message));
                    }
                    // auto add_messages = [&](const std::shared_ptr<SyncMessages> sync_messages){
                    // };
                    // add_messages(std::make_shared<SyncMessages>(sync_reply_body.messages()));
                    // if (sync_reply_body.hasMessages()) {
                    //     for (auto message : sync_reply_body.messages().messages()) {
                    //         // 更新消息
                    //         qDebug().nospace().noquote() << "Sync Message(message) for sender_account_id : " << message.senderAccountId();
                    //     }
                    // }
                    // // 服务端只区分了 SyncType::MESSAGES，文件也是 SyncType::MESSAGES 的一个子类型，需要客户端手动测试
                    // else if (sync_reply_body.hasFileEvents()) {
                    //     for (auto file_event : sync_reply_body.fileEvents().fileEvents()) {
                    //         // 更新文件信息
                    //         qDebug().nospace().noquote() << "Sync Message(file) for file_id : " << file_event.fileId();
                    //     }
                    // }
                }
            };
            NetworkClient::instance().sync(params_sync, callback_sync);
        }
    };
    NetworkClient::instance().get_last_id(params_get_last_id, callback_get_last_id);
    return false;
}

bool ChatWindow::download_contact_avatar(const std::shared_ptr<ContactInfo>&contact_info) {
    if (!contact_info || !contact_info->hasBaseInfo() || contact_info->baseInfo().avatarId().isEmpty()) { return false; }

    // 本地有头像时无需再次下载
    auto avatar_id = contact_info->baseInfo().avatarId();
    auto avatar_path = Database::instance().get_local_file_id_path(avatar_id);
    auto avatar_pixmap(avatar_path);
    if (!avatar_pixmap.isEmpty() && !avatar_pixmap.isNull()) { return true; }

    // 下载并显示头像
    network_file_io_notify_callback callback_download_file_notify = [this](const NetworkFileIONotify&notify) {
        // 忽略下载进度通知
        if (notify.ok && notify.code == int64_t(SuccessCode::SUCCESS) && notify.uploading) {
            return;
        }
        if (!notify.ok) {
            QString error_message = tr("Failed to download avatar.");
            error_message += tr("\n\ncode : 0x%1\nmessage : %2")
                .arg(notify.code, 8, 16, QChar('0'))  // 16 进制显示，宽度 8 位，不足补 0
                .arg(notify.message);
            // QMessageBox::information(this, tr("Error"), error_message, QMessageBox::StandardButton::Ok);
            return;
        }
    };
    qint64 downloaded_size = QFileInfo(avatar_path).size();
    NetworkClient::instance().download_file(avatar_id, qMax(qint64(0), downloaded_size), callback_download_file_notify);
    return false;
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

// void ChatWindow::on_chat_room_item_selected(const QModelIndex &index) {
//     qDebug() << "index : " << index;
// }

// void ChatWindow::on_chat_room_item_activated(const QModelIndex &index) {
//     qDebug() << "index : " << index;
// }

void ChatWindow::add_chat_room(const std::shared_ptr<ContactInfo>&contact_info) {
    // qDebug() << "contact : " << ContactManager(contact).get_nick_name();
    auto chatroom_data = std::make_shared<ChatroomData>();
    chatroom_data->contact = contact_info;
    m_chatroom_model->add_contact_chatroom_row(chatroom_data);
}

void ChatWindow::show_chat_room_messages(const std::shared_ptr<ContactInfo>&contact_info) {
    // qDebug() << "contact : " << ContactManager(contact).get_nick_name();
}

void ChatWindow::add_contact(const std::shared_ptr<ContactInfo>&contact_info, const ENUMS::ItemGroupType&group_type) {
    m_contact_model->add_member(group_type, contact_info);
}

// 按键移动导致所选行变化时触发激活动作
void ChatWindow::handle_contact_selection_changed(const QModelIndex &current, const QModelIndex &previous) {
    handle_contact_item_clicked(current);
    // // 索引值需要有效
    // if (!current.isValid()) { return; }
    // // 能够获取到有效的联系人信息
    // auto item_provider = m_contact_model->get(current.row());
    // if (item_provider->type() == "contact") {
    //     // 1. 智能指针要使用 dynamic_pointer_cast
    //     // 2. <...> 中不必写 std::shared_ptr
    //     auto item_provider_contact = std::dynamic_pointer_cast<ItemDelegateContact>(item_provider);
    //     if (!item_provider_contact) { return; }
    //     const std::shared_ptr<ContactInfo>&contact_info = item_provider_contact->get_contact_info();
    //     if (!contact_info) { return; }
    //     // 查找是否已经添加
    //     ContactWindow *contact_window_update = nullptr;  // 需要更新的窗口
    //     for (int idx = 0; idx < m_ui->stacked_contact_detail->count(); ++idx) {
    //         ContactWindow*contact_window_already = dynamic_cast<ContactWindow*>(m_ui->stacked_contact_detail->widget(idx));
    //         if (!contact_window_already) {
    //             break;
    //         }
    //         const std::shared_ptr<ContactInfo>&contact_info_already = contact_window_already->get_cache_contact_info();
    //         if (contact_info->baseInfo().accountId() == contact_info_already->baseInfo().accountId() &&
    //         contact_info->baseInfo().userId   () == contact_info_already->baseInfo().userId   ()) {
    //             contact_window_update = contact_window_already;  // 找到需要更新的窗口
    //             break;
    //         }
    //     }
    //     // 如果已经添加则更新信息
    //     if (contact_window_update) {
    //         contact_window_update->show_contact(contact_info);
    //     }
    //     // 如果没有则添加
    //     else {
    //         contact_window_update = new ContactWindow(this);
    //         contact_window_update->show_contact(contact_info);
    //         m_ui->stacked_contact_detail->addWidget(contact_window_update);
    //     }
    //     // 显示
    //     m_ui->stacked_contact_detail->setCurrentWidget(contact_window_update);
    // }
    // 展开折叠不在这个事件里面，在 editorEvent 中
    // else if (item_provider->type() == "spacer") {
    //     auto item_provider_spacer = std::dynamic_pointer_cast<ItemDelegateSpacer>(item_provider);
    //     if (!item_provider_spacer) { return; }
    //     item_provider_spacer->is_expand() ? item_provider_spacer->collapse() : item_provider_spacer->expand();
    //     m_ui->page_list_contact->update(current);
    // }
}

// 支持单击动作
void ChatWindow::handle_contact_item_clicked(const QModelIndex &index) {
    do {
        if (!index.isValid()) { break; }

        auto var_item_type = index.data(ENUMS::role_item_type());
        if (!var_item_type.isValid()) { break; }
        if (!var_item_type.canConvert<ENUMS::ItemDataType>()) { break; }

        auto item_type = var_item_type.value<ENUMS::ItemDataType>();
        auto var_item_data = index.data(ENUMS::role_item_data());

        if (item_type == ENUMS::ItemDataType::GROUP) {
            if (!var_item_data.canConvert<ENUMS::ItemGroupType>()) { break; }
            // debug info
            auto group_type = var_item_data.value<ENUMS::ItemGroupType>();
            qDebug().noquote().nospace() << "clicked group_type : " << Utils::Transform::to_string(group_type);
            // expanded
            const bool expanded = m_ui->page_list_contact->isExpanded(index);
            m_ui->page_list_contact->setExpanded(index, !expanded);
        }

        else if (item_type == ENUMS::ItemDataType::CONTACT) {
            if (!var_item_data.canConvert<std::shared_ptr<ContactInfo>>()) { break; }
            auto contact_info = var_item_data.value<std::shared_ptr<ContactInfo>>();
            if (!contact_info) { break; }
            // 获取节点分组
            ENUMS::ItemGroupType parent_group_type = ENUMS::ItemGroupType::UNSET;
            do {
                auto index_parent = index.parent();
                if (!index_parent.isValid()) { break; }

                // 判断数据类型
                auto var_parent_type = index_parent.data(ENUMS::role_item_type());
                if (!var_parent_type.isValid()) { break; }
                if (!var_parent_type.canConvert<ENUMS::ItemDataType>()) { break; }
                auto data_parent_type = var_parent_type.value<ENUMS::ItemDataType>();
                if (data_parent_type != ENUMS::ItemDataType::GROUP) { break; }

                // 获取数据
                auto var_parent_data = index_parent.data(ENUMS::role_item_data());
                if (!var_parent_data.canConvert<ENUMS::ItemGroupType>()) { break; }
                parent_group_type = var_parent_data.value<ENUMS::ItemGroupType>();
                qDebug().noquote().nospace() << "parent group_type : " << Utils::Transform::to_string(parent_group_type);
            } while (false);

            // debug info
            qDebug().noquote().nospace() << "clicked ContactInfo : ENUMS::ItemGroupType::" << *contact_info;
            // show contact
            if (0) {
                // 查找是否已经添加
                ContactWindow *contact_window_update = nullptr;  // 需要更新的窗口
                for (int idx = 0; idx < m_ui->stacked_contact_detail->count(); ++idx) {
                    ContactWindow*contact_window_already = dynamic_cast<ContactWindow*>(m_ui->stacked_contact_detail->widget(idx));
                    if (!contact_window_already) {
                        break;
                    }
                    const std::shared_ptr<ContactInfo>&contact_info_already = contact_window_already->get_cache_contact_info();
                    if (contact_info->baseInfo().accountId() == contact_info_already->baseInfo().accountId() &&
                        contact_info->baseInfo().userId   () == contact_info_already->baseInfo().userId   ()
                    ) {
                        contact_window_update = contact_window_already;  // 找到需要更新的窗口
                        break;
                    }
                }
                // 如果已经添加则更新信息
                if (contact_window_update) {
                    contact_window_update->show_contact(contact_info);
                }
                // 如果没有则添加
                else {
                    contact_window_update = new ContactWindow(this);
                    contact_window_update->show_contact(contact_info);
                    m_ui->stacked_contact_detail->addWidget(contact_window_update);
                }
                // 显示
                m_ui->stacked_contact_detail->setCurrentWidget(contact_window_update);
            }

            // 中心置顶显示
            else if (1) {
                class CenterFrame : public QScrollArea {
                public:
                    CenterFrame() : m_layout(new QGridLayout), m_container_widget(new QWidget()) {
                        m_layout->setParent(m_container_widget);
                        m_container_widget->setLayout(m_layout);
                        setWidgetResizable(true);
                        setWidget(m_container_widget);
                    }
                    void set_child_widget(QWidget*child_widget) {
                        // qDebug().noquote().nospace() << "child_widget size : " << child_widget->size();
                        if (!m_child_widget) {
                            m_child_widget = child_widget;
                            m_layout->addWidget(m_child_widget, 0, 0, Qt::AlignCenter | Qt::AlignTop);
                        }
                        else {
                            m_layout->replaceWidget(m_child_widget, child_widget);
                            if (m_child_widget) { delete m_child_widget; m_child_widget = nullptr; }
                            m_child_widget = child_widget;
                        }
                        // qDebug().noquote().nospace() << "child_widget size : " << child_widget->size();
                    }
                    QWidget*get_child_widget() {
                        return m_child_widget;
                    }
                private:
                    QGridLayout* m_layout           = nullptr;
                    QWidget    * m_container_widget = nullptr;
                    QWidget    * m_child_widget     = nullptr;
                };
                // 查找需要更新的窗口
                CenterFrame *m_center_frame = nullptr;
                for (int idx = 0; idx < m_ui->stacked_contact_detail->count(); ++idx) {
                    m_center_frame = dynamic_cast<CenterFrame*>(m_ui->stacked_contact_detail->widget(idx));
                    if (m_center_frame) {
                        break;
                    }
                }
                // 如果不存在则添加
                if (!m_center_frame) {
                    m_center_frame = new CenterFrame;
                    m_ui->stacked_contact_detail->addWidget(m_center_frame);
                }
                ContactWindow *contact_window = dynamic_cast<ContactWindow*>(m_center_frame->get_child_widget());
                if (!contact_window) {
                    contact_window = new ContactWindow;
                    m_center_frame->set_child_widget(contact_window);
                    // 发送消息
                    connect(contact_window, &ContactWindow::sig_contact_message, this, &ChatWindow::slot_message_to);
                    connect(contact_window, &ContactWindow::sig_contact_voice  , this, &ChatWindow::slot_voice_to  );
                    connect(contact_window, &ContactWindow::sig_contact_video  , this, &ChatWindow::slot_video_to  );
                }
                contact_window->show_contact(contact_info);  // 显示指定信息
                // 获取好友关系和分组信息
                auto show_mode = contact_window->get_show_mode();
                if (parent_group_type == ENUMS::ItemGroupType::REQUEST) { show_mode |= ContactWindow::ShowFlags(ContactWindow::ShowFlag::Request); }  // 是否显示同意添加按钮
                contact_window->set_show_mode(show_mode);  // 标明分组
                m_ui->stacked_contact_detail->setCurrentWidget(m_center_frame);
            }
        }
        return;
    } while (false);
    qDebug() << "canConvert error.";
}

// 按键移动导致所选行变化时触发激活动作
void ChatWindow::handle_chatroom_selection_changed(const QModelIndex &current, const QModelIndex &previous) {
    handle_chatroom_item_clicked(current);
}

// 单击聊天室
void ChatWindow::handle_chatroom_item_clicked(const QModelIndex &index) {
    do {
        if (!index.isValid()) { break; }

        auto var_item_type = index.data(ENUMS::role_item_type());
        if (!var_item_type.isValid()) { break; }
        if (!var_item_type.canConvert<ENUMS::ItemDataType>()) { break; }

        auto item_type = var_item_type.value<ENUMS::ItemDataType>();
        auto var_item_data = index.data(ENUMS::role_item_data());
        if (item_type != ENUMS::ItemDataType::CHATROOM) { break; }

        // 获取对话信息
        if (!var_item_data.canConvert<std::shared_ptr<ChatroomData>>()) { break; }
        auto chatroom_data = var_item_data.value<std::shared_ptr<ChatroomData>>();
        if (!chatroom_data) { break; }
        auto contact_show = chatroom_data->contact;
        if (!contact_show) { break; }
        if (!contact_show->hasBaseInfo()) { break; }
        auto account_id_show = contact_show->baseInfo().accountId();

        MessageWindow* target_message_window = get_message_window(account_id_show);
        m_ui->stacked_contact_detail->setCurrentWidget(target_message_window);
    } while (false);
}

// 添加聊天内容到对话内
void ChatWindow::add_message(const std::shared_ptr<SyncMessage>&sync_message) {
    do {
        // 无效数据
        if (!sync_message) { break; }
        if (!sync_message->hasText() || !sync_message->text().hasShortText()) { break; }
        if ( sync_message->serverMessageId  ().isEmpty()) { break; }
        if ( sync_message->clientMessageId  ().isEmpty()) { break; }
        if ( sync_message->senderAccountId  ().isEmpty()) { break; }
        if ( sync_message->receiverAccountId().isEmpty()) { break; }

        // 读取数据
        auto subtype = sync_message->subType();
        auto text = (sync_message->hasText() && sync_message->text().hasShortText()) ? sync_message->text().shortText() : "";
        auto server_message_id   = sync_message->serverMessageId  ();
        auto client_message_id   = sync_message->clientMessageId  ();
        auto sender_account_id   = sync_message->senderAccountId  ();
        auto receiver_account_id = sync_message->receiverAccountId();
        auto timestamp           = sync_message->timestamp        ();

        // receiver_account_id 为当前账号 ID 或对话 ID
        auto current_account_id = Accounts::instance().get_current_account()->baseInfo().accountId();
        QString account_id_show = receiver_account_id;
        // 如果收发都是自己
        ;    if (receiver_account_id == sender_account_id && current_account_id == receiver_account_id) { account_id_show = current_account_id; }
        // 如果发是自己
        else if (current_account_id == sender_account_id  ) { account_id_show = receiver_account_id; }
        // 如果收是自己
        else if (current_account_id == receiver_account_id) { account_id_show = sender_account_id  ; }
        // 收发都不是自己，那么收应该为对话 ID
        // else { ... }

        MessageWindow* target_message_window = get_message_window(account_id_show);
        // 不需要切换显示
        // m_ui->stacked_contact_detail->setCurrentWidget(target_message_window);
        target_message_window->add_message(sync_message);

        // 添加对话
        auto contact_show = Accounts::instance().get_contact(account_id_show);
        add_chat_room(contact_show);

        // TODO:如果显示末尾消息时，添加新消息后也需要滑动到底部
    } while (false);
}

// 账号当前用户信息被修改时触发
void ChatWindow::current_accounut_info_changed() {
    update_contact_info(Accounts::instance().get_current_account());
}

void ChatWindow::update_contact_info(const std::shared_ptr<ContactInfo>&contact_info) {

    // qDebug().noquote().nospace() << "Current account contactInfo: " << Utils::Transform::to_string(Accounts::instance().get_current_account());
    // qDebug().noquote().nospace() << "Cache           contactInfo: " << Utils::Transform::to_string(m_cache_self_info);
    // qDebug().noquote().nospace() << "To              contactInfo: " << Utils::Transform::to_string(contact_info);

    bool is_self = Accounts::instance().is_self(contact_info);
    if (is_self) {
        // 更新头像显示
        QString old_avatar_id = m_cache_self_info ? m_cache_self_info->baseInfo().avatarId() : QString();
        QString to_avatar_id  = contact_info->baseInfo().avatarId();
        if (old_avatar_id != to_avatar_id) {
            update_contact_avatar(to_avatar_id);
        }
        m_cache_self_info = std::make_shared<ContactInfo>(*contact_info.get());
    }
    else {
        // // 更新联系人列表中的信息
        // Accounts::instance().set_contact_list(Accounts::instance().get_contact_list());  // 触发联系人列表更新信号
    }
}

bool ChatWindow::eventFilter(QObject *obj, QEvent *event) {
    if (m_ui && obj && m_ui->edit_search && obj == m_ui->edit_search) {
        if (event->type() == QEvent::MouseButtonRelease) {
            // // 主动关闭后才应该重建窗口
            // if (m_search_window) {
            //     m_search_window.reset();
            // }
            if (!m_search_window) {
                m_search_window = std::make_shared<SearchWindow>();
                // 用于同步显示搜索词
                connect(m_search_window.get(), &SearchWindow::SearchEditTextChanged, m_ui->edit_search, &QLineEdit::setText);
            }

            Utils::widget_raise(m_search_window.get());
            return true;
        }
    }
    // else if (obj == m_ui->page_list_contact) {
    //     qDebug().nospace().noquote() << "Event type : " << QMetaEnum::fromType<QEvent::Type>().valueToKey(event->type());
    // }
    return QMainWindow::eventFilter(obj, event);
}

void ChatWindow::slot_button_navigate_avatar() {
    // 重建后再显示
    // if ( m_contact_window) { m_contact_window.reset(); }
    if (!m_contact_window) { m_contact_window = std::make_shared<ContactWindow>(this); }
    // 设置为弹窗窗口，否则有父窗口时会显示在父窗口内，发生与父窗口内容重叠的情况
    m_contact_window->setWindowFlags(m_contact_window->windowFlags() |  Qt::Dialog);
    m_contact_window->show_contact(Accounts::instance().get_current_account());
    // 只有点击主界面的头像时才允许修改自己的资料
    using ShowFlags = ContactWindow::ShowFlags;
    using ShowFlag  = ContactWindow::ShowFlag ;
    auto show_mode = m_contact_window->get_show_mode();
    show_mode |= ShowFlags(ShowFlag::Profile );
    show_mode |= ShowFlags(ShowFlag::Self    );
    m_contact_window->set_show_mode(show_mode);
    m_contact_window->show();

    // 先显示窗口再更新内容，避免网络请求较慢时窗口无响应
    auto&user_id = Accounts::instance().get_current_account()->baseInfo().userId();
    // 更新成功，刷新显示
    // update(params_update_profile->contact());
    search_user_params params_search_user = std::make_shared<TaoTieChat::SearchUserRequest>();
    params_search_user->setRequestUserAuth(*Accounts::instance().get_auth_request().get());
    params_search_user->setUserId(user_id);
    NetworkClient::instance().search_user(params_search_user, [this](const search_user_params&params_search_user, const search_user_reply&reply_search_user) {
        if (!reply_search_user || !reply_search_user->result().hasSuccessCode()) { return; }
        m_contact_window->show_contact(std::make_shared<ContactInfo>(reply_search_user->contactInfo()));
        // m_contact_window->setWindowTitle(tr("Profile"));
    });
}

void ChatWindow::slot_button_navigate_message() {
    m_ui->stacked_contact_list     ->setCurrentWidget(m_ui->stacked_page_chat_room);
    m_ui->navigate_button_chat_room->setEnabled(false);
    m_ui->navigate_button_contact  ->setEnabled(true );
}

void ChatWindow::slot_button_navigate_contact() {
    m_ui->stacked_contact_list     ->setCurrentWidget(m_ui->stacked_page_contact);
    m_ui->navigate_button_chat_room->setEnabled(true );
    m_ui->navigate_button_contact  ->setEnabled(false);
}

void ChatWindow::slot_message_to(const std::shared_ptr<ContactInfo>&contact_show) {
    // 先查找或添加对话
    slot_button_navigate_message();
    auto chatroom_data = std::make_shared<ChatroomData>();
    chatroom_data->contact = contact_show;
    int row = m_chatroom_model->add_contact_chatroom_row(chatroom_data);
    // 显示对话
    if (!contact_show) { return; }
    if (!contact_show->hasBaseInfo()) { return; }
    auto account_id_show = contact_show->baseInfo().accountId();

    MessageWindow* target_message_window = get_message_window(account_id_show);
    m_ui->stacked_contact_detail->setCurrentWidget(target_message_window);
}

void ChatWindow::slot_voice_to(const std::shared_ptr<ContactInfo>&contact_info) {
    // 先查找或添加对话
}

void ChatWindow::slot_video_to(const std::shared_ptr<ContactInfo>&contact_info) {
    // 先查找或添加对话
}

void ChatWindow::set_cache_contact_info(const std::shared_ptr<ContactInfo>&contact_info) {
    // m_cache_self_info = m_cache_self_info == contact_info ? std::make_shared<ContactInfo>(*contact_info.get()) : contact_info;
    m_cache_self_info = std::make_shared<ContactInfo>(*contact_info.get());  // 必须直接创建副本
}

void ChatWindow::show_avatar(const std::shared_ptr<QPixmap>&avatar_pixmap) {
    // 设置头像按钮
    QIcon icon_avatar(avatar_pixmap->scaled(m_ui->navigate_button_avatar->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_ui->navigate_button_avatar->setIcon(icon_avatar);
    // 设置窗口图标
    QIcon icon_window(avatar_pixmap->scaled(QSize(128, 128), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QApplication::setWindowIcon(icon_window);
    // 保存
    m_cache_self_avatar = avatar_pixmap;
}

// 获取聊天记录窗口
MessageWindow* ChatWindow::get_message_window(const QString&account_id) {
    // 查找窗口提取数据
    MessageWindow* target_message_window = nullptr;
    for (int idx = 0; idx < m_ui->stacked_contact_detail->count(); ++idx) {
        // 获取窗口
        MessageWindow* message_window = dynamic_cast<MessageWindow*>(m_ui->stacked_contact_detail->widget(idx));
        if (!message_window) { continue; }
        // 获取信息
        auto contact_widget = message_window->get_contact();
        if (!contact_widget) { continue; }
        if (!contact_widget->hasBaseInfo()) { continue; }
        auto account_id_widget = contact_widget->baseInfo().accountId();
        // 比较存储
        if (account_id != account_id_widget) { continue; }
        target_message_window = message_window;
        break;
    }
    // 找不到窗口时进行添加
    if (!target_message_window) {
        target_message_window = new MessageWindow;
        target_message_window->set_contact(Accounts::instance().get_contact(account_id));
        m_ui->stacked_contact_detail->addWidget(target_message_window);
        // 连接更新信号
        connect(target_message_window, &MessageWindow::signal_new_message, this, [this, account_id](const std::shared_ptr<MessageContext>&last_message){
            update_last_message(account_id, last_message);
        });
    }
    return target_message_window;
}

// 更新列表中显示的最后一条消息
void ChatWindow::update_last_message(const QString&chatroom_id, const std::shared_ptr<MessageContext>&last_message) {
    // 更新列表中显示的内容
    if (!m_chatroom_model) { return; }
    auto short_text = last_message->hasShortText() ? last_message->shortText() : "";
    auto datatime = last_message->hasTimestamp() && last_message->timestamp().hasTimestamp() ? last_message->timestamp().timestamp().toDateTime() : QDateTime{};
    m_chatroom_model->update_last_message(chatroom_id, short_text, datatime);  // 界面更新应由model更新数据触发
    // // 对聊天列表进行排序
    // m_chatroom_model->sort(0);
}

void ChatWindow::update_contact_avatar(const QString&avatar_id, bool local_only) {
    // m_cache_self_avatar = m_cache_self_avatar == avatar_pixmap ? std::make_shared<QPixmap>(avatar_pixmap) : avatar_pixmap;
    // 必须创建副本
    // 本地有则直接加载
    // 先尝试加载缩略图，失败再用原图兜底
    auto avatar_pixmap = std::make_shared<QPixmap>(Database::instance().get_local_file_id_path(avatar_id + ";size:128*128"));
    if (avatar_pixmap->isNull()) {
        avatar_pixmap = std::make_shared<QPixmap>(Database::instance().get_local_file_id_path(avatar_id));
    }
    if (!avatar_pixmap->isNull()) {
        show_avatar(avatar_pixmap);
        // QIcon icon_avatar(m_cache_self_avatar->scaled(m_ui->navigate_button_avatar->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        // m_ui->navigate_button_avatar->setIcon(icon_avatar);
        // QIcon icon_avatar(m_cache_self_avatar->scaled(m_ui->navigate_button_avatar->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        // QApplication::setWindowIcon(icon_avatar);
        return;
    }
    if (local_only) { return; }
    // 本地没有则需要从网络下载
    network_file_io_notify_callback callback_download_file_notify = [this, avatar_id](const NetworkFileIONotify&notify) {
        // 忽略下载进度通知
        if (notify.ok && notify.code == int64_t(SuccessCode::SUCCESS) && notify.uploading) {
            return;
        }
        if (!notify.ok) {
            // 忽略错误弹窗 // 防止退出再登陆时弹窗，实际后续会更新头像
            // QString error_message = tr("Failed to download avatar.");
            // error_message += tr("\n\ncode : 0x%1\nmessage : %2")
            //     .arg(notify.code, 8, 16, QChar('0'))  // 16 进制显示，宽度 8 位，不足补 0
            //     .arg(notify.message);
            // QMessageBox::information(this, tr("Error"), error_message, QMessageBox::StandardButton::Ok);
            return;
        }
        // 下载成功，文件已保存到本地，可以直接显示了
        auto avatar_pixmap = std::make_shared<QPixmap>(Database::instance().get_local_file_id_path(avatar_id + ";size:128*128"));
        // m_cache_self_avatar = avatar_pixmap;
        // m_ui->navigate_button_avatar->setIcon(QIcon(m_cache_self_avatar->scaled(m_ui->navigate_button_avatar->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
        show_avatar(avatar_pixmap);
    };
    qint64 downloaded_size = QFileInfo(Database::instance().get_local_file_id_path(avatar_id)).size();
    NetworkClient::instance().download_file(avatar_id, qMax(qint64(0), downloaded_size), callback_download_file_notify);
}

std::shared_ptr<ContactInfo> ChatWindow::get_cache_contact_info  () const {
    return m_cache_self_info;
}

std::shared_ptr<QPixmap> ChatWindow::get_cache_contact_avatar() const {
    return m_cache_self_avatar;
}

void ChatWindow::slot_file_id_downloaded(const QString&file_id) {
    if (file_id.isEmpty()) { return; }
    // 更新头像
    QString cache_avatar_id = !get_cache_contact_info() ? QString{} : get_cache_contact_info()->baseInfo().avatarId();
    if (cache_avatar_id == file_id) {
        update_contact_avatar(file_id);
    }
}
