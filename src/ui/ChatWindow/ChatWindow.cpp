#include "ChatWindow.h"
#include "ui_ChatWindow.h"

#include "NetworkClient.h"
#include "Accounts.h"
#include "Database.h"
#include "ContactWindow.h"
#include "SearchWindow.h"
#include "ItemModel.h"
#include "ItemDelegate.h"

ChatWindow::ChatWindow(QWidget *parent) : QMainWindow(parent) {}

ChatWindow::~ChatWindow() {
    release_resources();
}

void ChatWindow::reinit() {
    release_resources();
    init();
}

void ChatWindow::release_resources() {
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
    // model/view
    m_contact_model    = std::make_shared<ContactModel>();
    m_contact_delegate = std::make_shared<ItemDelegate>();

    // 连接消息
    connect(m_ui->navigate_button_avatar   , &QToolButton::clicked, this, &ChatWindow::slot_button_navigate_avatar ); // 点击了头像按钮
    connect(m_ui->navigate_button_chat_room, &QToolButton::clicked, this, &ChatWindow::slot_button_navigate_message); // 点击了聊天室按钮
    connect(m_ui->navigate_button_contact  , &QToolButton::clicked, this, &ChatWindow::slot_button_navigate_contact); // 点击了联系人按钮
    connect(m_ui->button_settings          , &QToolButton::clicked, this, [&](){ this->on_show_settings_menu(); }); // 点击了设置按钮
    connect(m_ui->button_add               , &QToolButton::clicked, this, [&](){ this->on_show_add_menu     (); }); // 点击了添加按钮
    // 各种菜单事件
    connect(m_action_signout .get(), &QAction::triggered, this, [&](){
        emit signout();
        // 单客户端单账号：退出登录时释放管道，允许下一次登录
        Utils::OS::try_grab_account_pipe(Accounts::instance().get_current_account()->baseInfo().userId(), false);
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
        if(!current_account) { return; }
        // 创建副本，不要修改原先的数据，防止恢复为保存登陆信息时数据丢失
        std::shared_ptr<AuthOpt> auth_opt = std::make_shared<AuthOpt>(*Accounts::instance().get_auth_opt(current_account->baseInfo().accountId()).get());
        if(!auth_opt) { return; }
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
        if(!current_account) { return; }
        // 创建副本，不要修改原先的数据，防止恢复为保存登陆信息时数据丢失
        std::shared_ptr<AuthOpt> auth_opt = std::make_shared<AuthOpt>(*Accounts::instance().get_auth_opt(current_account->baseInfo().accountId()).get());
        if(!auth_opt) { return; }
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
    m_ui->page_list_contact->setIndentation(m_ui->page_list_contact->indentation() / 2);  // 设置分组缩进
    // m_ui->page_list_contact->setHeaderHidden(true); // 隐藏表头，看起来更像 QQ 列表
    // m_ui->page_list_contact->setItemDelegate(m_contact_delegate.get());
    m_ui->page_list_contact->expandAll();
    // 使用 currentChanged 信号支持键盘上下键导航和鼠标点击
    connect(m_ui->page_list_contact->selectionModel(), &QItemSelectionModel::currentChanged, this, &ChatWindow::handle_contact_selection_changed);
    // 设置最小折叠宽度
    m_ui->page_list_contact      ->setMinimumWidth(200);  // 有效  // 建议
    // m_ui->stacked_contact_list->setMinimumWidth(300);  // 有效
    // m_ui->widget_contact_list ->setMinimumWidth(400);  // 有效

//     // 添加好友分组
//     m_contact_model->add(std::make_shared<ItemDelegateSpacer>(tr("New Friends")));
//     m_contact_model->add(std::make_shared<ItemDelegateSpacer>(tr("Contacts")));
//
// #ifdef _DEBUG
//     if(0) {
//         for(int i = 0; i <= 10000; ++i) {
//             // m_contact_model->add(std::make_shared<ItemDelegateSpacer>(QString("Contacts_%1").arg(i, 5, 10, '0')));
//             std::shared_ptr<ItemDelegateSpacer> item_spacer_contacts = std::make_shared<ItemDelegateSpacer>();
//             item_spacer_contacts->data().group_name    = tr("Contacts_%1").arg(i, 5, 10, '0');
//             item_spacer_contacts->data().members_count = i;
//             m_contact_model->add(item_spacer_contacts);
//         }
//     }
// #endif // __DEBUG

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
    update_contact_info(copy_contact_info);

    // 从网络获取当前用户的个人信息并更新显示
    {
        auto&user_id = Accounts::instance().get_current_account()->baseInfo().userId();
        search_user_params params_search_user = std::make_shared<SearchUserRequest>();
        params_search_user->setRequestUserAuth(*Accounts::instance().get_auth_request().get());
        params_search_user->setUserId(user_id);
        NetworkClient::instance().search_user(params_search_user, [this](const search_user_params&params_search_user, const search_user_reply&reply_search_user) {
            if(!reply_search_user || !reply_search_user->result().hasSuccessCode()) { return; }
            // update_contact_info(std::make_shared<ContactInfo>(*reply_search_user->contactInfo().get()));
            Accounts::instance().set_current_account(std::make_shared<ContactInfo>(reply_search_user->contactInfo()));
        });
    }

    // 消息同步
    {
        auto get_local_last_id = [](SyncType type) {
            return 0;
        };
        // get last id
        get_last_id_params   params_get_last_id   = Accounts::instance().get_auth_request();
        get_last_id_callback callback_get_last_id = [&](const get_last_id_params&params_get_last_id, const get_last_id_reply&reply_get_last_id) {
            if(!reply_get_last_id) { return; }
            if(!reply_get_last_id->hasResult()) { return; }
            if(!reply_get_last_id->result().hasSuccessCode()) { return; }

            QList<TaoTieChat::SyncTag> sync_tags;  // 需要更新的消息
            for(auto&tag : reply_get_last_id->tags()) {
                auto server_last_id = tag.idEnd();
                auto local_last_id = get_local_last_id(tag.type());
                auto type_name = QMetaEnum::fromType<SyncType>().valueToKey(quint64(tag.type()));
                qDebug().noquote().nospace() << "get_last_id SyncType : " << type_name << ", server_last_id : " << server_last_id << ", local_last_id : " << local_last_id;
                if(server_last_id > local_last_id) {
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
                    if(!notify_sync->inited) {
                        if(!reply_sync->hasHeader() && !reply_sync->header().hasResult() && !reply_sync->header().result().hasSuccessCode()) {
                            notify_sync->error = true;
                            return;
                        }
                        notify_sync->inited = true;
                        return;
                    }
                    // 进行同步
                    // qDebug().noquote().nospace() << "Get Sync Messages";
                    if(!reply_sync->hasBody()) { return; }
                    auto sync_reply_body = reply_sync->body();
                    auto sync_tag        = sync_reply_body.tag();
                    auto count           = sync_tag.count();

                    qDebug().noquote().nospace() << "sync_reply : " << reply_sync;

                    // 服务端数据异常或无更新时 `hasContacts() == false`
                    if(sync_tag.type() == SyncType::SYNC_CONTACTS && sync_reply_body.hasContacts()) {
                        for(auto sync_contact : sync_reply_body.contacts().contact()) {
                            // 更新联系人信息
                            // qDebug().nospace().noquote() << "Sync ContactInfo for AccountId : " << contact_info.baseInfo().accountId();
                            if(sync_contact.subType() == SyncSubTypeContact::SYNC_CONTACT_REUQEST) {
                                auto sync_contact_request = sync_contact.request();
                                auto request_contact = sync_contact_request.contactRequest();  // 申请者的信息
                                auto request_time    = sync_contact_request.time();
                                auto request_message = sync_contact_request.message();
                                qDebug().noquote().nospace() << "Get sync contact request. form : " << request_contact << ", time : " << request_time << ", message : " << request_message;
                                // 添加到联系人列表中，但标记为申请好友（放入申请分组）
                                add_contact(std::make_shared<ContactInfo>(request_contact));
                                // m_contact_model->add_contact_request(std::make_shared<ContactInfo>(request_contact));
                            }
                            else if(sync_contact.subType() == SyncSubTypeContact::SYNC_CONTACT_REPLY) {
                                auto sync_contact_reply = sync_contact.reply();
                                auto request_contact = sync_contact_reply.contactReply();  // 被申请者的信息
                                auto request_time    = sync_contact_reply.time();
                                auto request_message = sync_contact_reply.message();
                                qDebug().noquote().nospace() << "Get sync contact reply. form : " << request_contact << ", time : " << request_time << ", message : " << request_message;
                            }
                        }
                    }
                    // 服务端只区分了 SyncType::MESSAGES，文件也是 SyncType::MESSAGES 的一个子类型，需要客户端手动测试
                    else if(sync_tag.type() == SyncType::SYNC_MESSAGES) {
                        // if(sync_reply_body.hasMessages()) {
                        //     for(auto message : sync_reply_body.messages().messages()) {
                        //         // 更新消息
                        //         qDebug().nospace().noquote() << "Sync Message(message) for sender_account_id : " << message.senderAccountId();
                        //     }
                        // }
                        // // 服务端只区分了 SyncType::MESSAGES，文件也是 SyncType::MESSAGES 的一个子类型，需要客户端手动测试
                        // else if(sync_reply_body.hasFileEvents()) {
                        //     for(auto file_event : sync_reply_body.fileEvents().fileEvents()) {
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
    }
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

void ChatWindow::add_contact(const std::shared_ptr<ContactInfo>&contact_info, const ENUMS::ItemGroupType&group_type) {
    // m_contact_model->add(std::make_shared<ItemDelegateContact>(contact_info, group_type));
    // TODO:判断群组
    // for(int row = 0; row < m_contact_model->rowCount(); ++row) {
    //     auto provider = m_contact_model->get(row);
    //     if(auto provider_contact = dynamic_pointer_cast<ItemDelegateContact>(provider)) {
    //         // TODO:群组与账号 ID 一致即认为是同一条目
    //         // if(provider_contact->is_member(group_type) && provider_contact.)
    //     }
    // }
}

void ChatWindow::handle_contact_selection_changed(const QModelIndex &current, const QModelIndex &previous) {
    // 索引值需要有效
    if (!current.isValid()) { return; }
    // // 能够获取到有效的联系人信息
    // auto item_provider = m_contact_model->get(current.row());
    // if(item_provider->type() == "contact") {
    //     // 1. 智能指针要使用 dynamic_pointer_cast
    //     // 2. <...> 中不必写 std::shared_ptr
    //     auto item_provider_contact = std::dynamic_pointer_cast<ItemDelegateContact>(item_provider);
    //     if(!item_provider_contact) { return; }
    //     const std::shared_ptr<ContactInfo>&contact_info = item_provider_contact->get_contact_info();
    //     if (!contact_info) { return; }
    //     // 查找是否已经添加
    //     ContactWindow *contact_window_update = nullptr;  // 需要更新的窗口
    //     for(int idx = 0; idx < m_ui->stacked_contact_detail->count(); ++idx) {
    //         ContactWindow*contact_window_already = dynamic_cast<ContactWindow*>(m_ui->stacked_contact_detail->widget(idx));
    //         if(!contact_window_already) {
    //             break;
    //         }
    //         const std::shared_ptr<ContactInfo>&contact_info_already = contact_window_already->get_cache_contact_info();
    //         if(contact_info->baseInfo().accountId() == contact_info_already->baseInfo().accountId() &&
    //         contact_info->baseInfo().userId   () == contact_info_already->baseInfo().userId   ()) {
    //             contact_window_update = contact_window_already;  // 找到需要更新的窗口
    //             break;
    //         }
    //     }
    //     // 如果已经添加则更新信息
    //     if(contact_window_update) {
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
    // else if(item_provider->type() == "spacer") {
    //     auto item_provider_spacer = std::dynamic_pointer_cast<ItemDelegateSpacer>(item_provider);
    //     if(!item_provider_spacer) { return; }
    //     item_provider_spacer->is_expand() ? item_provider_spacer->collapse() : item_provider_spacer->expand();
    //     m_ui->page_list_contact->update(current);
    // }
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
    if(is_self) {
        // 更新头像显示
        QString old_avatar_id = m_cache_self_info ? m_cache_self_info->baseInfo().avatarId() : QString();
        QString to_avatar_id  = contact_info->baseInfo().avatarId();
        if(old_avatar_id != to_avatar_id) {
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
    if(obj == m_ui->edit_search) {
        if(event->type() == QEvent::MouseButtonRelease) {
            // // 主动关闭后才应该重建窗口
            // if(m_search_window) {
            //     m_search_window.reset();
            // }
            if(!m_search_window) {
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
    // if( m_contact_window) { m_contact_window.reset(); }
    if(!m_contact_window) { m_contact_window = std::make_shared<ContactWindow>(this); }
    // 设置为弹窗窗口，否则有父窗口时会显示在父窗口内，发生与父窗口内容重叠的情况
    m_contact_window->setWindowFlags(m_contact_window->windowFlags() |  Qt::Dialog);
    m_contact_window->show_contact(Accounts::instance().get_current_account());
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
        m_contact_window->show_contact(std::make_shared<ContactInfo>(reply_search_user->contactInfo()));
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

void ChatWindow::update_contact_avatar(const QString&avatar_id, bool local_only) {
    // m_cache_self_avatar = m_cache_self_avatar == avatar_pixmap ? std::make_shared<QPixmap>(avatar_pixmap) : avatar_pixmap;
    // 必须创建副本
    // 本地有则直接加载
    // 先尝试加载缩略图，失败再用原图兜底
    auto avatar_pixmap = std::make_shared<QPixmap>(Database::instance().get_local_file_id_path(avatar_id + ";size:128*128"));
    if(avatar_pixmap->isNull()) {
        avatar_pixmap = std::make_shared<QPixmap>(Database::instance().get_local_file_id_path(avatar_id));
    }
    if(!avatar_pixmap->isNull()) {
        show_avatar(avatar_pixmap);
        // QIcon icon_avatar(m_cache_self_avatar->scaled(m_ui->navigate_button_avatar->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        // m_ui->navigate_button_avatar->setIcon(icon_avatar);
        // QIcon icon_avatar(m_cache_self_avatar->scaled(m_ui->navigate_button_avatar->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        // QApplication::setWindowIcon(icon_avatar);
        return;
    }
    if(local_only) { return; }
    // 本地没有则需要从网络下载
    network_file_io_notify_callback callback_download_file_notify = [this, avatar_id](const NetworkFileIONotify&notify) {
        // 忽略下载进度通知
        if(notify.ok && notify.code == int64_t(SuccessCode::SUCCESS) && notify.uploading) {
            return;
        }
        if(!notify.ok) {
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
    if(file_id.isEmpty()) { return; }
    // 更新头像
    QString cache_avatar_id = !get_cache_contact_info() ? QString{} : get_cache_contact_info()->baseInfo().avatarId();
    if(cache_avatar_id == file_id) {
        update_contact_avatar(file_id);
    }
}
