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
#include "SignalInner.h"

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
    //
    m_action_clean_search_box.reset();
    // add
    m_menu_add     .reset();
    m_action_add   .reset();
    m_action_create.reset();
    // model/view
    m_contact_model   .reset();
    m_contact_delegate.reset();
    m_contact_proxy    .reset();
    m_chatroom_model   .reset();
    m_chatroom_proxy   .reset();
    // 退出时需要关闭及重置的页面
    m_search_window .reset();  // 搜素页面
    m_contact_window.reset();  // 点击头像时显示的个人信息页面
    //
    m_cache_self_data.reset();

    // 清除好友关系等
    Accounts::instance()->set_contact_list(QVector<std::shared_ptr<ContactData::ItemData>>());

    // 单客户端单账号：退出登录时释放管道，允许下一次登录
    Utils::OS::try_grab_account_pipe(Accounts::instance()->get_current_account()->user_id, false);

    // 断开实时同步的连接网络
    // NetworkClient::instance()->reset_connection();
    NetworkClient::instance()->disconnect_sync(Accounts::instance()->get_current_account()->account_id);
}

void ChatWindow::init() {
    // 重新初始化界面及数据
    m_ui = std::make_shared<Ui::ChatWindow>();
    m_ui->setupUi(this);
    // 创建对象
    m_menu_settings          = std::make_shared<QMenu  >();
    m_action_settings        = std::make_shared<QAction>();
    m_action_rember_password = std::make_shared<QAction>();
    m_action_auto_login      = std::make_shared<QAction>();
    m_action_signout         = std::make_shared<QAction>();
    m_action_exit            = std::make_shared<QAction>();
    //
    m_action_clean_search_box = std::make_shared<QAction>();
    // add
    m_menu_add         = std::make_shared<QMenu  >();
    m_action_add       = std::make_shared<QAction>();
    m_action_create    = std::make_shared<QAction>();
    // model/view/proxy
    m_contact_model    = std::make_shared<ContactModel >();
    m_contact_delegate = std::make_shared<ItemDelegate >();
    m_contact_proxy    = std::make_shared<ContactProxy >();
    m_chatroom_model   = std::make_shared<ChatroomModel>();
    m_chatroom_proxy   = std::make_shared<ChatroomProxy>();

    //
    m_search_window    = std::make_shared<SearchWindow>();
    m_search_window->hide();

    // 连接消息
    connect(m_ui->navigate_button_avatar   , &QToolButton::clicked, this, &ChatWindow::slot_button_navigate_avatar ); // 点击了头像按钮
    connect(m_ui->navigate_button_chat_room, &QToolButton::clicked, this, &ChatWindow::slot_button_navigate_message); // 点击了聊天室按钮
    connect(m_ui->navigate_button_contact  , &QToolButton::clicked, this, &ChatWindow::slot_button_navigate_contact); // 点击了联系人按钮
    connect(m_ui->button_settings          , &QToolButton::clicked, this, &ChatWindow::slot_show_settings_menu     ); // 点击了设置按钮
    connect(m_ui->button_add               , &QToolButton::clicked, this, &ChatWindow::slot_show_add_menu          ); // 点击了添加按钮
    // 各种菜单事件
    connect(m_action_signout .get(), &QAction::triggered, this, [&](){
        release_resources();
        emit SignalInner::instance()->signal_signout();
    });
    //
    connect(m_action_clean_search_box.get(), &QAction::triggered, this, [this](bool checked = false){ m_ui->edit_search->clear(); });
    connect(m_ui->edit_search, &QLineEdit::textChanged, this, [this]() {
        m_action_clean_search_box->setVisible(!m_ui->edit_search->text().isEmpty());
        if (m_search_window && m_search_window->get_keyword() != m_ui->edit_search->text()) {
            m_search_window->set_keyword(m_ui->edit_search->text());
        }
    });
    //
    connect(m_action_exit    .get(), &QAction::triggered, this, [&](){ QApplication::exit(0); });
    connect(m_action_settings.get(), &QAction::triggered, this, [&](){ qDebug() << "m_action_settings"; });
    connect(m_action_add     .get(), &QAction::triggered, this, [&](){ qDebug() << "m_action_add"     ; });
    connect(m_action_create  .get(), &QAction::triggered, this, [&](){ qDebug() << "m_action_create"  ; });

    // 更新联系人信息
    connect(SignalInner::instance(), &SignalInner::signal_account_changed     , this, &ChatWindow::slot_update_contact_data      );  // 更改当前账号信息时触发
    connect(SignalInner::instance(), &SignalInner::signal_contact_info_changed, this, &ChatWindow::slot_update_contact_list_info );
    connect(SignalInner::instance(), &SignalInner::signal_contact_info_changed, this, &ChatWindow::slot_update_chatroom_list_info);
    // 发送消息
    connect(SignalInner::instance(), &SignalInner::sig_contact_message, this, &ChatWindow::slot_message_to);
    connect(SignalInner::instance(), &SignalInner::sig_contact_voice  , this, &ChatWindow::slot_voice_to  );
    connect(SignalInner::instance(), &SignalInner::sig_contact_video  , this, &ChatWindow::slot_video_to  );
    // 文件更新消息
    connect(SignalInner::instance(), &SignalInner::signal_file_downloaded, this, &ChatWindow::slot_update_avatar_file  );
    connect(SignalInner::instance(), &SignalInner::signal_file_downloaded, this, &ChatWindow::slot_update_chatroom_file);
    connect(SignalInner::instance(), &SignalInner::signal_file_downloaded, this, &ChatWindow::slot_update_contact_file );
    // chatroom 更新最后一条消息的提示
    connect(SignalInner::instance(), &SignalInner::signal_new_message , this, &ChatWindow::slot_update_last_message  );
    connect(SignalInner::instance(), &SignalInner::signal_unread_count, this, &ChatWindow::slot_update_message_unread);

    connect(m_action_rember_password.get(), &QAction::triggered, this, [&](){
        auto current_account = Accounts::instance()->get_current_account();
        if (!current_account) { return; }
        // 创建副本，不要修改原先的数据，防止恢复为保存登陆信息时数据丢失
        std::shared_ptr<AuthOpt> auth_opt = std::make_shared<AuthOpt>(*Accounts::instance()->get_auth_opt(current_account->account_id).get());
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
        auto current_account = Accounts::instance()->get_current_account();
        if (!current_account) { return; }
        // 创建副本，不要修改原先的数据，防止恢复为保存登陆信息时数据丢失
        std::shared_ptr<AuthOpt> auth_opt = std::make_shared<AuthOpt>(*Accounts::instance()->get_auth_opt(current_account->account_id).get());
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
    connect(m_ui->page_list_contact, &QTreeView::clicked, this, &ChatWindow::slot_contact_item_clicked);
    // // 使用 currentChanged 信号支持键盘上下键导航和鼠标点击  // 需要先 setModel 才能连接
    // connect(m_ui->page_list_contact->selectionModel(), &QItemSelectionModel::currentChanged, this, &ChatWindow::slot_contact_selection_changed);
    // 用于同步显示搜索词
    connect(SignalInner::instance(), &SignalInner::signal_search_edit_text_changed, this, [this](const QString &keyword){
        m_action_clean_search_box->setVisible(!keyword.isEmpty());
        if (m_ui->edit_search->text() != keyword) {
            m_ui->edit_search->setText(keyword);
        }
    });
    // 搜索框中的发送消息按钮
    // 点击聊天室列表中的项目
    connect(m_ui->page_list_chat_room, &QTreeView::clicked, this, &ChatWindow::slot_chatroom_item_clicked);
    // // 使用 currentChanged 信号支持键盘上下键导航和鼠标点击  // 需要先 setModel 才能连接
    // connect(m_ui->page_list_chat_room->selectionModel(), &QItemSelectionModel::currentChanged, this, &ChatWindow::slot_chatroom_selection_changed);
    // 连接聊天列表点击事件
    // connect(m_ui->page_list_chat_room, &QListView::itemClicked, this, &ChatWindow::slot_chat_room_item_clicked);

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

    auto current_account = Accounts::instance()->get_current_account();
    auto auth_opt = Accounts::instance()->get_auth_opt(!current_account ? QString{} : current_account->account_id);
    //
    m_action_clean_search_box->setIcon(style()->standardIcon(QStyle::SP_LineEditClearButton));
    m_ui->edit_search->addAction(m_action_clean_search_box.get(), QLineEdit::TrailingPosition);
    m_action_clean_search_box->setVisible(!m_ui->edit_search->text().isEmpty());  // 需要添加后再设置隐藏？？
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

    // contact view/model/delegate/proxy
    m_contact_proxy->setSourceModel(m_contact_model.get());  // 将原始 Model 设置给代理 Model
    m_contact_proxy->sort(0, Qt::AscendingOrder); // 按第 0 列正序排序
    m_contact_proxy->setDynamicSortFilter(true);  // 开启动态排序（当 Model 内部数据改变时，自动重新排序）
    m_ui->page_list_contact->setModel(m_contact_proxy.get());
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
    connect(m_ui->page_list_contact->selectionModel(), &QItemSelectionModel::currentChanged, this, &ChatWindow::slot_contact_selection_changed);

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
    connect(m_ui->page_list_chat_room->selectionModel(), &QItemSelectionModel::currentChanged, this, &ChatWindow::slot_chatroom_selection_changed);
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
        // m_contact_model->add_member(ENUMS::ItemGroupType::REQUEST, Accounts::instance()->get_current_account());
        // m_contact_model->add_member(ENUMS::ItemGroupType::FRIENDS, Accounts::instance()->get_current_account());
        for (int i = 0; i <= 300; ++i) {
            auto contact_data = std::make_shared<ContactData::ItemData>();
            auto num          = QString("%1").arg(i, 5, 10, '0');
            auto time         = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
            contact_data->account_id = QString("Account ID : %1 (%2)").arg(num).arg(time);
            contact_data->user_id    = QString("User ID    : %1 (%2)").arg(num).arg(time);
            contact_data->nickname   = QString("Nickname   : %1 (%2)").arg(num).arg(time);
            contact_data->signature  = QString("Signature  : %1 (%2)").arg(num).arg(time);
            m_contact_model->add_member(ENUMS::ItemGroupType::REQUEST, contact_data);
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

    // // 更新个人信息显示  // 初始化赋值，需要创建副本，而不是共享同一片内存
    // slot_update_current_account_data();  // 触发更新  // 本地更新

    // 消息同步
    network_status_sync();

    // TODO:未添加的功能
    m_ui->button_add->setVisible(false);
}

// QImageReader::supportedImageFormats()：返回当前 Qt 环境中可用的图像格式列表，
//   例如 png、jpeg、bmp。它不检查某个具体文件是否存在、是否损坏。
// QImageReader::imageFormat(fileName)：尝试从指定文件中识别图像格式。
//   返回格式名称（如 png）；如果无法识别，通常返回空的 QByteArray。
// supportedImageFormats() 查询可用格式，而 imageFormat(fileName) 查询指定文件的格式。
bool ChatWindow::is_supported_image_formats(const QString& file_name) {
    static const QSet<QString> supported_image_formats = []() {
        QSet<QString> formats;
        const auto image_formats = QImageReader::supportedImageFormats();
        for (const auto& fmt : image_formats) {
            formats.insert(QString::fromUtf8(fmt).toLower());
        }
        return formats;
    }();

    QString suffix = QFileInfo(file_name).suffix().toLower();
    if (suffix.isEmpty()) {
        return false;
    }

    return supported_image_formats.contains(suffix);
};

// 服务端消息同步消息
bool ChatWindow::network_status_sync() {
    auto get_local_last_id = [](SyncType type) {
        return 0;
    };
    // get last id
    get_last_id_params   params_get_last_id   = Accounts::instance()->get_auth_request();
    get_last_id_callback callback_get_last_id = [&](get_last_id_params params_get_last_id, get_last_id_reply reply_get_last_id) {
        if (!reply_get_last_id) { return; }
        if (!reply_get_last_id->hasResult()) { return; }
        if (!reply_get_last_id->result().hasSuccessCode()) { return; }

        // 需要更新的消息
        QList<TaoTieChat::SyncTag> sync_tags;
        for (const auto &tag : reply_get_last_id->tags()) {
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
            params_sync->setRequestUserAuth(*Accounts::instance()->get_auth_request().get());
            params_sync->setSyncTag(sync_tags);
            //
            sync_callback callback_sync = [&, notify_sync](sync_params params_sync, sync_reply reply_sync) {
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
                if (!reply_sync->hasBody()) { return; }
                auto sync_reply_body = reply_sync->body();
                auto sync_tag        = sync_reply_body.tag();
                auto count           = sync_tag.count();

                qDebug().noquote().nospace() << "sync_reply : " << reply_sync;

                // 服务端数据异常或无更新时 `hasContacts() == false`
                if (sync_tag.type() == SyncType::SYNC_CONTACTS && sync_reply_body.hasContacts()) {
                    for (auto&sync_contact : sync_reply_body.contacts().contact()) {
                        // 更新联系人信息
                        if (sync_contact.subType() == SyncSubTypeContact::SYNC_CONTACT_REUQEST) {
                            auto sync_contact_request = sync_contact.request();
                            auto request_contact      = sync_contact_request.contactRequest();  // 申请者的信息
                            auto request_message_id   = sync_contact_request.messageId();
                            auto request_time         = sync_contact_request.timestamp();
                            auto request_message_text = sync_contact_request.messageText();
                            auto contact_data         = Utils::Transform::to_contact_data(std::make_shared<ContactInfo>(request_contact));
                            do {
                                // messages
                                auto current_account = Accounts::instance()->get_current_account();
                                if (!current_account) { break; }
                                if ( current_account->account_id.isEmpty()) { break; }
                                if ( contact_data   ->account_id.isEmpty()) { break; }
                                contact_data->request_messages = std::make_shared<ContactRequestData::ItemData>();
                                contact_data->request_messages->receiver_account_id = current_account->account_id;
                                contact_data->request_messages->sender_account_id   = contact_data   ->account_id;
                                ContactRequestData::RequestMessage request_message_data;
                                request_message_data.account_id   = contact_data->account_id;
                                request_message_data.message_id   = request_message_id;
                                request_message_data.datetime     = request_time.hasTimestamp() ? request_time.timestamp().toDateTime().toLocalTime() : QDateTime{};
                                request_message_data.message_text = request_message_text;
                                contact_data->request_messages->messages.append(request_message_data);
                            } while (false);
                            qDebug().noquote().nospace() << "Get sync contact request. form : " << request_contact << ", request_time : " << request_time << ", request_message_text : " << request_message_text;
                            // 添加到联系人列表中，但标记为申请好友（放入申请分组）
                            add_contact(contact_data, ENUMS::ItemGroupType::REQUEST);
                            // Accounts::instance()->set_request_messages();
                            continue;
                        }
                        // 好友申请通过
                        else if (sync_contact.subType() == SyncSubTypeContact::SYNC_CONTACT_REPLY) {
                            // 通过好友申请时无需获取好友申请结果，获取服务器直接推送的好友变更结果即可
                            if (!sync_contact.hasShip() || !sync_contact.ship().hasContactShip()) { continue; }
                            auto contact_info = sync_contact.ship().contactShip();
                            auto contact_data = Utils::Transform::to_contact_data(std::make_shared<ContactInfo>(contact_info));
                            qDebug().noquote().nospace() << "Contact updata : " << contact_info;
                            // 可以放到任务队列中触发的部分操作
                            // 添加到联系人列表中
                            add_contact(contact_data, ENUMS::ItemGroupType::FRIENDS);

                            // 更新好友列表  // 使用昵称及用户 ID 排序，不使用账号 ID 排序  // 联系人列表的排序不在这
                            auto contact_datas = Accounts::instance()->get_contact_list();
                            contact_datas.append(contact_data);
                            // TODO:排序函数未验证
                            auto func_contact_sort = [](std::shared_ptr<ContactData::ItemData> left, std::shared_ptr<ContactData::ItemData> right) {
                                if (!left || ! right) { return false; }
                                ;    if (left->display_name != right->display_name) { return left->display_name < right->display_name; }
                                else if (left->user_id      != right->user_id     ) { return left->user_id      < right->user_id     ; }
                                return false;
                            };
                            std::sort(contact_datas.begin(), contact_datas.end(), func_contact_sort);
                            Accounts::instance()->set_contact_list(contact_datas);
                            // Accounts::instance()->set_contact(contact_data);
                            continue;
                        }
                        // 好友资料更新
                        else if (sync_contact.subType() == SyncSubTypeContact::SYNC_CONTACT_INFO) {
                            // 通过好友申请时无需获取好友申请结果，获取服务器直接推送的好友变更结果即可
                            if (!sync_contact.hasUpdate() || !sync_contact.update().hasContactUpdate()) { continue; }
                            auto contact_info = sync_contact.update().contactUpdate();
                            auto contact_data = Utils::Transform::to_contact_data(std::make_shared<ContactInfo>(contact_info));
                            qDebug().noquote().nospace() << "Contact updata : " << contact_info;
                            // 新账号登陆后获取账号信息也是通过该同步消息初始化获取，因此该窗口之前可能没有账号信息
                            Accounts::instance()->set_contact(contact_data);
                            continue;
                        }
                    }
                }
                // 服务端只区分了 SyncType::MESSAGES，文件也是 SyncType::MESSAGES 的一个子类型，需要客户端手动测试
                else if (sync_tag.type() == SyncType::SYNC_MESSAGES && sync_reply_body.hasMessages()) {
                    for (auto& message : sync_reply_body.messages().message()) {
                        // qDebug().noquote().nospace() << "trace message file download."
                        //     << " ChatWindow::add_message(...)."
                        //     << "serverMessageId : " << message.serverMessageId()
                        //     << ", senderAccountId : " << message.senderAccountId();
                        // 信息未必来自同一对话，暂不处理为批量添加
                        add_message(std::make_shared<SyncMessage>(message));
                    }
                }
            };
            NetworkClient::instance()->sync(params_sync, callback_sync);
        }
    };
    NetworkClient::instance()->get_last_id(params_get_last_id, callback_get_last_id);
    return true;
}

// 显示设置菜单
void ChatWindow::slot_show_settings_menu() {
    // 在按钮上方弹出
    m_menu_settings->adjustSize(); // 内存调整为实际大小
    QPoint pos = m_ui->button_settings->mapToGlobal(m_ui->button_settings->rect().bottomRight());
    int h = m_menu_settings->size().height();
    m_menu_settings->popup(QPoint(pos.x(), pos.y() - h));
}

// 点击了添加按钮弹出菜单
void ChatWindow::slot_show_add_menu() {
    // 在按钮下方左侧弹出
    m_menu_add->adjustSize(); // 内存调整为实际大小
    QPoint pos = m_ui->button_add->mapToGlobal(m_ui->button_add->rect().bottomRight());
    int w = m_menu_add->size().width();
    m_menu_add->popup(QPoint(pos.x() - w, pos.y()));
}

// 使用示例：将 64x64 的 SVG 放大渲染保存为 1024x1024 的 PNG
// svg_to_image("icon.svg", "icon_1024.png", QSize(1024, 1024));
std::shared_ptr<QImage> svg_to_image(const QString &inputSvgPath, const QSize &newSize = QSize(128, 128)) {
    // 1. 加载 SVG 渲染器
    QSvgRenderer renderer(inputSvgPath);
    if (!renderer.isValid()) {
        return std::make_shared<QImage>(); // SVG 文件解析失败
    }

    // 2. 创建指定大小的透明 QImage（支持 Alpha 通道）
    auto image = std::make_shared<QImage>(newSize, QImage::Format_ARGB32);
    image->fill(Qt::transparent); // 背景透明

    // 3. 使用 QPainter 将 SVG 绘制到 QImage 上
    QPainter painter(&(*image.get()));
    painter.setRenderHint(QPainter::Antialiasing);           // 抗锯齿
    painter.setRenderHint(QPainter::SmoothPixmapTransform);  // 顺滑缩放

    // 绘制到指定的区域大小
    renderer.render(&painter, QRectF(0, 0, newSize.width(), newSize.height()));
    painter.end();

    // 4. 保存为目标文件（根据扩展名自动识别 PNG/JPG 等）
    return image;
}

// 聊天列表添加一条记录
void ChatWindow::add_chat_room(const QString &chatroom_id) {
    // qDebug() << "contact : " << ContactManager(contact).get_nick_name();
    do {
        if ( chatroom_id.isEmpty()) { break; }
        if ( m_chatroom_model->get_chatroom_data(chatroom_id)) { break; }

        auto contact_data = Accounts::instance()->get_contact(chatroom_id);
        if (!contact_data) { break; }
        if ( contact_data->account_id.isEmpty()) { break; }

        // 添加
        auto chatroom_data = std::make_shared<ChatroomData::ItemData>();
        // auto datetime   = QDateTime::currentDateTimeUtc();
        auto avatar_id     = contact_data->avatar_id;
        auto avatar_path   = Database::instance()->get_local_file_id_path(avatar_id + ";size:128*128");
        auto avatar_image  = std::make_shared<QImage>(avatar_path);
        if (avatar_image->isNull()) { avatar_image = svg_to_image(avatar_path); }
        // if (avatar_image.isNull()) { avatar_image = Utils::FontIcons().update().icon.contacts; }
        //
        chatroom_data->chatroom_id           = contact_data->account_id;
        chatroom_data->last_message_text     = "";
        chatroom_data->last_message_datetime = QDateTime(); // datetime;
        chatroom_data->display_datetime      = ""; // Utils::format_smart_time(datetime);
        chatroom_data->display_name          = Utils::get_display_name(contact_data);
        chatroom_data->avatar_id             = avatar_id;
        chatroom_data->avatar_path           = avatar_path;
        chatroom_data->avatar_image          = avatar_image;
        m_chatroom_model->add_chatroom_data(chatroom_data);

        qDebug().noquote().nospace() << "add_chat_room"
            << ", chatroom_id:" << chatroom_id
            << ", account_id:" << contact_data->account_id
            << ", user_id:" << contact_data->user_id
            << ", display_name:" << contact_data->display_name;

        // 下载并显示头像
        if (!avatar_id.isEmpty() && avatar_image->isNull()) {
            NetworkClient::instance()->download_file_notify_signal(avatar_id);
        }
    } while(false);
}

// 显示聊天室聊天消息记录窗口
void ChatWindow::show_chat_room_messages(std::shared_ptr<ContactData::ItemData> contact_data) {
    // qDebug() << "contact : " << ContactManager(contact).get_nick_name();
}

void ChatWindow::add_contact(std::shared_ptr<ContactData::ItemData> contact_data, const ENUMS::ItemGroupType&group_type) {
    if (!contact_data || contact_data->account_id.isEmpty()) { return; }

    // 如果没有图片则试图加载
    if (!contact_data->avatar_id.isEmpty() && contact_data->avatar_image->isNull()) {
        contact_data->avatar_path  = Database::instance()->get_local_file_id_path(contact_data->avatar_id);
        contact_data->avatar_image = std::make_shared<QImage>(contact_data->avatar_path);
    }

    // 添加到好友列表时要更新好友关系
    if (group_type == ENUMS::ItemGroupType::FRIENDS) {
        contact_data->is_friend = true;
    }
    // // 添加到申请者列表时要否定好友关系
    // else if (group_type == ENUMS::ItemGroupType::REQUEST) {
    //     contact_data->is_friend = false;
    // }

    // 是否已经添加相同账号 ID 的信息
    auto old_contact_data = m_contact_model->get_contact_data(group_type, contact_data->account_id);
    // 填充时间和消息
    if (group_type == ENUMS::ItemGroupType::FRIENDS) {
        contact_data->info_text = contact_data->signature;
        contact_data->info_time = QString{}; // 好友列表不需要时间
    }
    else if (group_type == ENUMS::ItemGroupType::REQUEST) {
        if (contact_data->request_messages && !contact_data->request_messages->messages.isEmpty()) {
            auto messages = contact_data->request_messages->messages;
            auto last_message = messages.at(messages.size() - 1);
            contact_data->info_text = last_message.message_text;
            contact_data->info_time = Utils::format_smart_time(last_message.datetime);
        }
    }

    // 接收到的是更新消息时，有则改之无则加勉
    if ( old_contact_data) {
        // m_contact_model->update_contact(group_type, contact_data);
        slot_update_contact_list_info(contact_data);
    }
    else {
        m_contact_model->add_member(group_type, contact_data);
    }

    // 如果成功加载了图片则不需要再下载
    if (!contact_data->avatar_image->isNull()) { return; }

    // 没有设置头像则不必更新
    if ( contact_data->avatar_id.isEmpty()) { return; }

    // 下载并显示头像
    NetworkClient::instance()->download_file_notify_signal(contact_data->avatar_id);
}

// 按键移动导致所选行变化时触发激活动作
void ChatWindow::slot_contact_selection_changed(const QModelIndex &current, const QModelIndex &previous) {
    slot_contact_item_clicked(current);
}

// 支持单击动作
void ChatWindow::slot_contact_item_clicked(const QModelIndex &index) {
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
            if (!var_item_data.canConvert<std::shared_ptr<ContactData::ItemData>>()) { break; }
            auto contact_data = var_item_data.value<std::shared_ptr<ContactData::ItemData>>();
            if (!contact_data) { break; }
            contact_data->is_friend = Accounts::instance()->is_me    (contact_data->account_id);
            contact_data->is_friend = Accounts::instance()->is_friend(contact_data->account_id);
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
                // qDebug().noquote().nospace() << "parent group_type : " << Utils::Transform::to_string(parent_group_type);
            } while (false);

            // debug info
            // qDebug().noquote().nospace() << "clicked ContactData::ItemData : ENUMS::ItemGroupType::" << *contact_data;
            // show contact
            class CenterFrame : public QScrollArea {
            public:
                CenterFrame(QWidget* parent = nullptr)
                    : QScrollArea(parent)
                    , m_layout(new QGridLayout)
                    , m_container_widget(new QWidget())
                {
                    m_layout->setContentsMargins(0, 0, 0, 0);
                    m_layout->setSpacing(0);

                    m_container_widget->setLayout(m_layout);

                    setWidgetResizable(true);
                    setWidget(m_container_widget);

                    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
                    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
                }
                void set_child_widget(QWidget*child_widget) {
                    if (!child_widget) { return; }
                    if (m_child_widget == child_widget) { return; }

                    if (m_child_widget) {
                        m_layout->removeWidget(m_child_widget);
                        delete m_child_widget;
                    }

                    m_child_widget = child_widget;

                    // 不指定 AlignCenter，让布局自动分配可用空间
                    m_layout->addWidget(m_child_widget, 0, 0);

                    m_layout->setRowStretch(0, 1);
                    m_layout->setColumnStretch(0, 1);
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
            }
            auto source_flag = parent_group_type == ENUMS::ItemGroupType::REQUEST ? ContactWindow::SourceFlag::Request : ContactWindow::SourceFlag::Contact;
            contact_window->set_contact_data(contact_data, source_flag);  // 显示指定信息
            m_ui->stacked_contact_detail->setCurrentWidget(m_center_frame);
            Utils::in_memory_layout_expander(contact_window);
        }
        return;
    } while (false);
    qDebug() << "canConvert error.";
}

// 按键移动导致所选行变化时触发激活动作
void ChatWindow::slot_chatroom_selection_changed(const QModelIndex &current, const QModelIndex &previous) {
    slot_chatroom_item_clicked(current);
}

// 单击聊天室
void ChatWindow::slot_chatroom_item_clicked(const QModelIndex &index) {
    do {
        if (!index.isValid()) { break; }

        auto var_item_type = index.data(ENUMS::role_item_type());
        if (!var_item_type.isValid()) { break; }
        if (!var_item_type.canConvert<ENUMS::ItemDataType>()) { break; }

        auto item_type = var_item_type.value<ENUMS::ItemDataType>();
        auto var_item_data = index.data(ENUMS::role_item_data());
        if (item_type != ENUMS::ItemDataType::CHATROOM) { break; }

        // 获取对话信息
        if (!var_item_data.canConvert<std::shared_ptr<ChatroomData::ItemData>>()) { break; }
        auto chatroom_data = var_item_data.value<std::shared_ptr<ChatroomData::ItemData>>();
        if (!chatroom_data) { break; }

        MessageWindow* target_message_window = get_message_window(chatroom_data->chatroom_id);
        m_ui->stacked_contact_detail->setCurrentWidget(target_message_window);
    } while (false);
}

// 添加服务器聊天消息更新
void ChatWindow::add_message(std::shared_ptr<SyncMessage> sync_message) {

    auto is_file_message = [this](std::shared_ptr<SyncMessage> sync_message) {
        if (!sync_message || !sync_message->hasFile()) { return false; }
        auto file_info = sync_message->file();
        if (file_info.fileName().isEmpty()) { return false; }
        if (file_info.fileId  ().isEmpty()) { return false; }
        if (file_info.fileMd5 ().isEmpty()) { return false; }
        if (file_info.fileSize() < 0      ) { return false; }
        return true;
    };

    do {
        // 无效数据
        if (!sync_message) { break; }
        // if ( sync_message->chatroomId    ().isEmpty()) { break; }
        if ( sync_message->serverMessageId  ().isEmpty()) { break; }
        if ( sync_message->clientMessageId  ().isEmpty()) { break; }
        if ( sync_message->senderAccountId  ().isEmpty()) { break; }
        if ( sync_message->receiverAccountId().isEmpty()) { break; }
        if (!sync_message->hasTimestamp() && !sync_message->timestamp().hasTimestamp()) { break; }

        // 读取数据
        auto subtype             = sync_message->subType          ();
        // auto chatroom_id      = sync_message->chatroomId       ();
        auto server_message_id   = sync_message->serverMessageId  ();
        auto client_message_id   = sync_message->clientMessageId  ();
        auto sender_account_id   = sync_message->senderAccountId  ();
        auto receiver_account_id = sync_message->receiverAccountId();
        auto timestamp           = sync_message->timestamp        ();
        auto message_time        = sync_message->timestamp().timestamp().toDateTime();
        //
        auto sender_data         = Accounts::instance()->get_contact(sender_account_id);
        auto receiver_data       = Accounts::instance()->get_contact(receiver_account_id);
        auto current_data        = Accounts::instance()->get_current_account();
        auto sender_avatar_id    = sender_data->avatar_id;
        auto receiver_avatar_id  = receiver_data->avatar_id;
        auto current_account_id  = current_data->account_id;
        bool is_me               = sender_account_id == current_account_id;

        // 验证是否是当前账号的消息
        if (sender_account_id != current_account_id && receiver_account_id != current_account_id) {
            qDebug().noquote().nospace() << "not current account message, server_message_id:" << server_message_id;
            break;
        }

        // 聊天对象不是发送方就是接收方
        auto contact_data = sender_data->account_id != current_account_id ? sender_data : receiver_data;

        // 可能耗时的操作验证后再执行
        auto sender_avatar_path  = Database::instance()->get_local_file_id_path(sender_avatar_id);
        auto sender_avatar_image = std::make_shared<QImage>(Database::instance()->get_local_file_id_path(sender_avatar_id + ";size:128*128"));

        // =================================================================== //

        // 添加对话
        QString chatroom_id = contact_data->account_id;
        add_chat_room(chatroom_id);

        // UI 所需数据
        auto message_data  = std::make_shared<MessageData::ItemData>();
        message_data->chatroom_id           = chatroom_id;
        message_data->sender_type           = MessageData::SenderType::USER;
        message_data->media_type            = MessageData::MediaType::TEXT;
        message_data->is_me                 = is_me;
        message_data->message_id            = client_message_id;  // server_message_id;
        message_data->message_time          = message_time;
        if (!message_data->sender) { message_data->sender = std::make_shared<MessageData::SenderData>(); }
        message_data->sender->account_id    = sender_account_id;
        message_data->sender->display_name  = Utils::get_display_name(sender_data);
        message_data->sender->avatar_id     = sender_avatar_id;
        message_data->sender->avatar_path   = sender_avatar_path;
        message_data->sender->avatar_image  = sender_avatar_image;
        // 消息内容
        if (sync_message->hasText()) {
            message_data->media_type = MessageData::MediaType::TEXT;
            if (!message_data->text) { message_data->text = std::make_shared<MessageData::TextData>(); }
            message_data->text->text = sync_message->text().shortText();
        }
        else if(is_file_message(sync_message)) {
            // 必定为文件
            message_data->media_type = MessageData::MediaType::FILE;
            if (!message_data->file) { message_data->file = std::make_shared<MessageData::FileData>(); }
            auto file = message_data->file;
            file->file_size  = sync_message->file().fileSize();
            file->file_id    = sync_message->file().fileId();
            file->file_path  = Database::instance()->get_local_file_id_path(file->file_id);
            file->file_name  = sync_message->file().fileName();
            file->file_hash  = sync_message->file().fileMd5();
            file->human_size = Utils::Transform::human_size(file->file_size);
            file->file_icon  = qApp->style()->standardIcon(QStyle::SP_FileIcon);
            // // 如果是其他格式则再补充内容
            // if ( is_supported_image_formats(file->file_name) && !QImageReader::imageFormat(file->file_path).isEmpty()) {
            QImageReader reader(file->file_path);
            auto image = std::make_shared<QImage>(reader.read());
            if (!image->isNull()) {
                message_data->media_type = MessageData::MediaType::IMAGE;
                if (!message_data->image) { message_data->image = std::make_shared<MessageData::ImageData>(); }
                message_data->image->image = image;
                // if (message_data->image->pixmap.isNull()) { message_data->image->pixmap = svg_to_image(file->file_path); }
            }
        }

        // =================================================================== //
        // 添加消息
        MessageWindow* target_message_window = get_message_window(chatroom_id);
        if (!target_message_window->get_chatroom_info()) {
            auto chatroom_data = m_chatroom_model->get_chatroom_data(chatroom_id);
            target_message_window->set_chatroom_info(chatroom_data);
        }
        if (!target_message_window->get_contact_info()) {
            target_message_window->set_contact_info(contact_data);
        }

        // qDebug().noquote().nospace() << "trace message file download."
        //     << " target_message_window->add_message(...)."
        //     << "chatroom_id : " << message_data->chatroom_id
        //     << ", message_id : " << message_data->message_id
        //     << ", sender->account_id : " << message_data->sender->account_id
        //     << ", sender->avatar_id : " << message_data->sender->avatar_id
        //     << ", file_id : " << ((message_data && message_data->file) ? message_data->file->file_id : "[no fileid]");

        target_message_window->add_message(message_data);

        // 不需要切换显示
        // m_ui->stacked_contact_detail->setCurrentWidget(target_message_window);

        // 添加新消息时会发送通知，从通知中触发
        // // 下载头像
        // if (message_data->sender->avatar_image->isNull()) {
        //     NetworkClient::instance()->download_file_notify_signal(message_data->sender->avatar_id);
        // }
        // // 下载消息文件  // 下载时会校验  // 图片与普通文件都要下载
        // if (is_file_message(sync_message)) {
        //     NetworkClient::instance()->download_file_notify_signal(message_data->file->file_id);
        // }
    } while (false);
}

// // 账号当前用户信息被修改时触发
// void ChatWindow::slot_update_current_account_data() {
//     slot_update_contact_data(Accounts::instance()->get_current_account());
// }

void ChatWindow::slot_update_contact_data(std::shared_ptr<ContactData::ItemData> contact_data) {
    if (!contact_data) { return; }
    if (!contact_data->is_me) { return; }
    // 先复制再重新初始化/补充数据
    set_cache_contact_data(contact_data);
    // 更新头像显示  // 有数据就要重新读取
    auto cache_contact_data = get_cache_contact_data();
    // avatar_image 在服务器数据转换为本地数据 contact_data 时已经加载
    // if (cache_contact_data && !cache_contact_data->avatar_id.isEmpty() && cache_contact_data->avatar_image.isNull()) {
    if (cache_contact_data && cache_contact_data->avatar_image && !cache_contact_data->avatar_image->isNull()) {
        show_avatar(cache_contact_data->avatar_image);  // 本地加载
    }
    else if (cache_contact_data && !cache_contact_data->avatar_id.isEmpty()) {
        update_contact_avatar(cache_contact_data->avatar_id, true);  // 网络下载
    }
}

bool ChatWindow::eventFilter(QObject *obj, QEvent *event) {
    if (m_ui && obj && m_ui->edit_search && obj == m_ui->edit_search) {
        if (event->type() == QEvent::MouseButtonRelease) {
            // // 主动关闭后才应该重建窗口
            // if (m_search_window) {
            //     m_search_window.reset();
            // }
            // if (!m_search_window) {
            //     m_search_window = std::make_shared<SearchWindow>();
            //     // 用于同步显示搜索词
            //     connect(m_search_window.get(), &SearchWindow::SearchEditTextChanged, this, [this](const QString &keyword){
            //         m_action_clean_search_box->setVisible(!keyword.isEmpty());
            //         if (m_ui->edit_search->text() != keyword) {
            //             m_ui->edit_search->setText(keyword);
            //         }
            //     });
            // }

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
    auto current_account = Accounts::instance()->get_current_account();
    if (!current_account) { return; }
    current_account->is_me = true;  // 标记为查看自身资料
    current_account->display_name = current_account->nickname;  // 自身资料显示为昵称，不能为备注

    // 重建后再显示
    // if ( m_contact_window) { m_contact_window.reset(); }
    if (!m_contact_window) { m_contact_window = std::make_shared<ContactWindow>(this); }
    // 设置为弹窗窗口，否则有父窗口时会显示在父窗口内，发生与父窗口内容重叠的情况
    m_contact_window->setWindowFlags(m_contact_window->windowFlags() |  Qt::Dialog);
    // 只有点击主界面的头像时才允许修改自己的资料
    m_contact_window->set_contact_data(current_account, ContactWindow::SourceFlag::Profile);
    m_contact_window->show();

    // // 先显示窗口再更新内容，避免网络请求较慢时窗口无响应
    // auto &user_id = current_account->user_id;
    // // 更新成功，刷新显示
    // // update(params_update_profile->contact());
    // search_user_params params_search_user = std::make_shared<TaoTieChat::SearchUserRequest>();
    // params_search_user->setRequestUserAuth(*Accounts::instance()->get_auth_request().get());
    // params_search_user->setUserId(user_id);
    // NetworkClient::instance()->search_user(params_search_user, [this](search_user_params params_search_user, search_user_reply reply_search_user) {
    //     if (!reply_search_user || !reply_search_user->result().hasSuccessCode()) { return; }
    //     auto contact_info = Utils::Transform::to_contact_data(std::make_shared<ContactInfo>(reply_search_user->contactInfo()));
    //     contact_info->is_me = true;  // 标记为查看自身资料
    //     contact_info->display_name = contact_info->nickname;  // 自身资料显示为昵称，不能为备注
    //     m_contact_window->set_contact_data(contact_info, ContactWindow::SourceFlag::Profile);
    // });
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

void ChatWindow::slot_message_to(std::shared_ptr<ContactData::ItemData> contact_data) {
    if (!contact_data) { return; }
    if ( contact_data->account_id.isEmpty()) { return; }

    // 先查找或添加对话
    auto old_chatroom_data = m_chatroom_model->get_chatroom_data(contact_data->account_id);
    auto new_chatroom_data = std::make_shared<ChatroomData::ItemData>();
    // copy data // 保留最后消息及时间显示
    new_chatroom_data->chatroom_id           = contact_data->account_id;
    new_chatroom_data->last_message_text     = old_chatroom_data ? old_chatroom_data->last_message_text     : QString{};
    new_chatroom_data->last_message_datetime = old_chatroom_data ? old_chatroom_data->last_message_datetime : QDateTime{};
    // cache
    new_chatroom_data->display_datetime = Utils::format_smart_time(new_chatroom_data->last_message_datetime);
    new_chatroom_data->display_name     = Utils::get_display_name(contact_data);
    new_chatroom_data->avatar_id        = contact_data->avatar_id;
    new_chatroom_data->avatar_path      = contact_data->avatar_path;
    new_chatroom_data->avatar_image     = contact_data->avatar_image;
    if (!old_chatroom_data) {
        m_chatroom_model->add_chatroom_data(new_chatroom_data);
    }
    else {
        m_chatroom_model->updata_chatroom_data(new_chatroom_data);
    }

    // qDebug().noquote().nospace() << "new_chatroom_data addr : " << (void*)&new_chatroom_data
    //     << ", display_name : " << new_chatroom_data->display_name
    //     << ", avatar_image : " << !contact_data->avatar_image.isNull();

    // 1. 显示聊天列表
    slot_button_navigate_message();
    // 2. 高亮当前对话
    highlight_chatroom(contact_data->account_id);
    // 3. 显示对话
    MessageWindow* target_message_window = get_message_window(new_chatroom_data->chatroom_id);
    if (!target_message_window->get_chatroom_info()) {
        auto chatroom_data = m_chatroom_model->get_chatroom_data(new_chatroom_data->chatroom_id);
        target_message_window->set_chatroom_info(chatroom_data);
    }
    if (!target_message_window->get_contact_info()) {
        target_message_window->set_contact_info(contact_data);
    }
    m_ui->stacked_contact_detail->setCurrentWidget(target_message_window);
}

void ChatWindow::slot_voice_to(std::shared_ptr<ContactData::ItemData> contact_data) {
    // 先查找或添加对话
}

void ChatWindow::slot_video_to(std::shared_ptr<ContactData::ItemData> contact_data) {
    // 先查找或添加对话
}

// 联系人列表与好友申请
void ChatWindow::slot_update_contact_list_info(std::shared_ptr<ContactData::ItemData> to_contact_data) {
    // 对多个分组都需要操作

    // ENUMS::ItemGroupType::REQUEST
    do {
        if (!to_contact_data || to_contact_data->account_id.isEmpty()) { break; }
        // 更新好友申请列表与联系人列表  // 无则忽略有则修改

        // 好友申请列表要显示消息与时间
        auto old_contact_info = m_contact_model->get_contact_data(ENUMS::ItemGroupType::REQUEST, to_contact_data->account_id);
        if (!old_contact_info) { break; }  // 没有该信息时不需要更新

        auto new_contact_info = std::make_shared<ContactData::ItemData>(*to_contact_data);
        // 重新获取
        new_contact_info->is_me        = Accounts::instance()->is_me(old_contact_info->account_id);
        new_contact_info->is_friend    = Accounts::instance()->is_friend(old_contact_info->account_id);
        new_contact_info->display_name = Utils::get_display_name(new_contact_info);
        new_contact_info->avatar_path  = Database::instance()->get_local_file_id_path(new_contact_info->avatar_id);
        new_contact_info->avatar_image = std::make_shared<QImage>(new_contact_info->avatar_path);

        // if (!new_contact_info->request_messages->messages.isEmpty()) {
        //     auto&messages = new_contact_info->request_messages->messages;
        //     qDebug().noquote().nospace() << "new_contact_info messages size : " << messages.size();
        //     for(auto&msg:messages) {
        //         qDebug().noquote().nospace() << "new_contact_info messages id : " << msg.message_id << ", datetime : " << Utils::format_smart_time(msg.datetime);
        //     }
        // }

        // 复制
        // 消息仅追加
        if (   new_contact_info                                        && old_contact_info
            && new_contact_info->request_messages                      && old_contact_info->request_messages
            && new_contact_info->request_messages->sender_account_id   == old_contact_info->request_messages->sender_account_id
            && new_contact_info->request_messages->receiver_account_id == old_contact_info->request_messages->receiver_account_id
        ) {
            new_contact_info->request_messages->messages.append(old_contact_info->request_messages->messages);
        }
        else {
            new_contact_info->request_messages = old_contact_info->request_messages;
        }
        // 排序去重
        auto&messages = new_contact_info->request_messages->messages;
        auto func_sort_by_time = [](const auto &a, const auto &b) { return a.datetime < b.datetime; };
        std::sort(messages.begin(), messages.end(), func_sort_by_time);
        auto func_unique_by_message_id = [](const auto &a, const auto &b) { return a.message_id == b.message_id; };
        messages.erase(std::unique(messages.begin(), messages.end(), func_unique_by_message_id), messages.end());
        if (new_contact_info->request_messages && !new_contact_info->request_messages->messages.isEmpty()) {
            auto messages = new_contact_info->request_messages->messages;
            auto last_message = messages.at(messages.size() - 1);
            new_contact_info->info_text = last_message.message_text;
            new_contact_info->info_time = Utils::format_smart_time(last_message.datetime);  // 同时可刷新时间显示
        }
        m_contact_model->update_contact(ENUMS::ItemGroupType::REQUEST, new_contact_info);

        // // 联系人列表要显示签名
        // auto friend_contact_info = std::make_shared<ContactData::ItemData>(*new_contact_info);  // 复制，减少重复获取
        // friend_contact_info->info_text = friend_contact_info->signature;
        // friend_contact_info->info_time = QString{}; // 好友列表不需要时间
        // m_contact_model->update_contact(ENUMS::ItemGroupType::FRIENDS, friend_contact_info);
    } while (false);

    // ENUMS::ItemGroupType::FRIENDS
    do {
        if (!to_contact_data || to_contact_data->account_id.isEmpty()) { break; }
        // 更新好友申请列表与联系人列表  // 无则忽略有则修改

        // 好友申请列表要显示消息与时间
        auto old_contact_info = m_contact_model->get_contact_data(ENUMS::ItemGroupType::FRIENDS, to_contact_data->account_id);
        if (!old_contact_info) { break; }  // 没有该信息时不需要更新

        // 复制并刷新
        auto new_contact_info          = std::make_shared<ContactData::ItemData>(*to_contact_data);
        new_contact_info->is_me        = Accounts::instance()->is_me(old_contact_info->account_id);
        new_contact_info->is_friend    = Accounts::instance()->is_friend(old_contact_info->account_id);
        new_contact_info->display_name = Utils::get_display_name(new_contact_info);
        new_contact_info->avatar_path  = Database::instance()->get_local_file_id_path(new_contact_info->avatar_id);
        new_contact_info->avatar_image = std::make_shared<QImage>(new_contact_info->avatar_path);
        new_contact_info->info_text    = new_contact_info->signature;
        new_contact_info->info_time    = QString{}; // 好友列表不需要时间
        m_contact_model->update_contact(ENUMS::ItemGroupType::FRIENDS, new_contact_info);
    } while (false);
}

// 对话列表
void ChatWindow::slot_update_chatroom_list_info(std::shared_ptr<ContactData::ItemData> to_contact_data) {
    do {
        if (!to_contact_data || to_contact_data->account_id.isEmpty()) { break; }
        auto chatroom_id = to_contact_data->account_id;

        // 修改
        auto chatroom_data = m_chatroom_model->get_chatroom_data(chatroom_id);
        if (!chatroom_data) { break; }  // 没有该对话则忽略

        // 获取
        // auto display_name = Utils::get_display_name(to_contact_data);
        auto display_name  = to_contact_data->display_name;
        auto avatar_id     = to_contact_data->avatar_id;
        auto avatar_path   = Database::instance()->get_local_file_id_path(avatar_id + ";size:128*128");
        auto avatar_image  = std::make_shared<QImage>(avatar_path);
        if (avatar_image->isNull()) { avatar_image = svg_to_image(avatar_path); }

        // 同步
        chatroom_data->display_name = display_name;
        chatroom_data->avatar_id    = avatar_id   ;
        chatroom_data->avatar_path  = avatar_path ;
        chatroom_data->avatar_image = avatar_image;

        // 更新
        m_chatroom_model->updata_chatroom_data(chatroom_data);
    } while (false);
}

// 缓存副本
void ChatWindow::set_cache_contact_data(std::shared_ptr<ContactData::ItemData> contact_data) {
    // 防止空指针
    if (!contact_data) {
        m_cache_self_data = std::make_shared<ContactData::ItemData>();
        return;
    }
    m_cache_self_data = std::make_shared<ContactData::ItemData>(*contact_data.get());  // 必须直接创建副本
}

// 不保存到 cache
void ChatWindow::show_avatar(std::shared_ptr<QImage> avatar_image) {
    if (!avatar_image || avatar_image->isNull()) { return; }
    // 设置头像按钮
    QIcon icon_avatar(QPixmap::fromImage(avatar_image->scaled(m_ui->navigate_button_avatar->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    m_ui->navigate_button_avatar->setIcon(icon_avatar);
    // 设置窗口图标
    QIcon icon_window(QPixmap::fromImage(avatar_image->scaled(QSize(128, 128), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    QApplication::setWindowIcon(icon_window);
}

// 获取聊天记录窗口
MessageWindow* ChatWindow::get_message_window(const QString& chatroom_id) {
    // 查找窗口提取数据
    MessageWindow* target_message_window = nullptr;
    for (int idx = 0; idx < m_ui->stacked_contact_detail->count(); ++idx) {
        // 获取窗口
        MessageWindow* message_window = dynamic_cast<MessageWindow*>(m_ui->stacked_contact_detail->widget(idx));
        if (!message_window) { continue; }
        // 获取信息
        auto chatroom_info = message_window->get_chatroom_info();
        if (!chatroom_info) { continue; }
        // 比较存储
        if (chatroom_id != chatroom_info->chatroom_id) { continue; }
        target_message_window = message_window;
        break;
    }
    // 找不到窗口时进行添加
    if (!target_message_window) {
        target_message_window = new MessageWindow;
        m_ui->stacked_contact_detail->addWidget(target_message_window);
    }
    return target_message_window;
}

// 更新列表中显示的最后一条消息
void ChatWindow::slot_update_last_message(const QPointer<MessageModel> signal_sender, std::shared_ptr<MessageData::ItemData> last_message) {
    do {
        // 更新列表中显示的内容
        if (!last_message) { break; }
        if (!m_chatroom_model) { break; }
        if ( last_message->chatroom_id.isEmpty()) { break; }
        auto chatroom_data = m_chatroom_model->get_chatroom_data(last_message->chatroom_id);
        if (!chatroom_data) { break; }

        // 生成要显示的文字
        QString last_message_text;
        if (last_message->text) {
            last_message_text = last_message->text->text.trimmed().replace("\n", "");
        }
        else if(last_message->file) {
            QFileInfo fileinfo(last_message->file->file_name);
            last_message_text = tr("[File]%1").arg(fileinfo.fileName());
        }

        // 保存文字和时间
        chatroom_data->last_message_text     = last_message_text;
        chatroom_data->last_message_datetime = last_message->message_time;
        chatroom_data->display_datetime      = Utils::format_smart_time(chatroom_data->last_message_datetime);

        // 通知界面更新
        m_chatroom_model->updata_chatroom_data(chatroom_data);  // 界面更新应由model更新数据触发

        // // 对聊天列表进行排序
        // m_chatroom_model->sort(0);
    } while(false);
}

void ChatWindow::slot_update_message_unread(const QString &chatroom_id, int unread_count) {
    if ( chatroom_id.isEmpty()) { return; }
    auto chatroom_data = m_chatroom_model->get_chatroom_data(chatroom_id);
    if (!chatroom_data) { return; }
    chatroom_data->message_unread_count = unread_count;
    m_chatroom_model->updata_chatroom_data(chatroom_data);
}

void ChatWindow::update_contact_avatar(const QString& avatar_id, bool online_download) {
    // 必须创建副本
    // 本地有则直接加载
    // 先尝试加载缩略图，失败再用原图兜底
    QString avatar_path = Database::instance()->get_local_file_id_path(avatar_id + ";size:128*128");
    auto avatar_image = std::make_shared<QImage>(avatar_path);
    if (avatar_image->isNull()) {
        avatar_path  = Database::instance()->get_local_file_id_path(avatar_id);
        avatar_image = std::make_shared<QImage>(avatar_path);
    }

    if (!avatar_image->isNull()) {
        if (!m_cache_self_data) {
            m_cache_self_data = std::make_shared<ContactData::ItemData>();
        }
        m_cache_self_data->avatar_id    = avatar_id;
        m_cache_self_data->avatar_path  = avatar_path;
        m_cache_self_data->avatar_image = avatar_image;
        show_avatar(avatar_image);
    }

    // 没有文件时刷新
    if ( online_download && avatar_image->isNull()) {
        // 本地没有则需要从网络下载
        NetworkClient::instance()->download_file_notify_signal(avatar_id);
    }
}

std::shared_ptr<ContactData::ItemData> ChatWindow::get_cache_contact_data() const {
    return m_cache_self_data;
}

// 左上角主界面的图标
void ChatWindow::slot_update_avatar_file(const Network::FileDownloadNotify notify) {
    const QString& file_id = notify.id;
    if (file_id.isEmpty()) { return; }
    if (!notify.finished || !notify.ok || notify.status != FileStatusCode::FILE_STATUS_UPLOADED) { return; }
    // 更新头像  // 有数据，id 相同，但实际未加载到图片
    auto cache_contact_data = get_cache_contact_data();
    if (cache_contact_data && cache_contact_data->avatar_id == file_id && cache_contact_data->avatar_image->isNull()) {
        update_contact_avatar(file_id, false);
    }
}

// 聊天室列表中的头像
void ChatWindow::slot_update_chatroom_file(const Network::FileDownloadNotify notify) {
    const QString& file_id = notify.id;
    if (file_id.isEmpty()) { return; }
    if (!notify.finished || !notify.ok || notify.status != FileStatusCode::FILE_STATUS_UPLOADED) { return; }
    for(auto& chatroom_data : m_chatroom_model->get_members()) {
        if (chatroom_data->avatar_id == notify.id) {
            // 文件下载仅修改相关参数即可
            auto avatar_id    = chatroom_data->avatar_id;
            auto avatar_path  = Database::instance()->get_local_file_id_path(avatar_id + ";size:128*128");
            auto avatar_image = std::make_shared<QImage>(avatar_path);
            if (avatar_image->isNull()) { avatar_image = svg_to_image(avatar_path); }
            chatroom_data->avatar_path  = avatar_path;
            chatroom_data->avatar_image = avatar_image;
            m_chatroom_model->updata_chatroom_data(chatroom_data);
        }
    }
}

// 联系人列表中的头像
void ChatWindow::slot_update_contact_file(const Network::FileDownloadNotify notify) {
    const QString& file_id = notify.id;
    if (file_id.isEmpty()) { return; }
    if (!notify.finished || !notify.ok || notify.status != FileStatusCode::FILE_STATUS_UPLOADED) { return; }
    const auto& group_datas = m_contact_model->get_members();  // 获取到的是副本
    for(auto& group_pair : group_datas) {
        auto& group_type    = group_pair.first;
        auto& contact_datas = group_pair.second;
        for(auto& contact_data : contact_datas) {
            if (contact_data->avatar_id == notify.id) {
                // 头像下载成功时仅需修改头像相关的信息
                contact_data->avatar_path  = Database::instance()->get_local_file_id_path(contact_data->avatar_id);
                contact_data->avatar_image = std::make_shared<QImage>(contact_data->avatar_path);
                m_contact_model->update_contact(group_type, contact_data);
            }
        }
    }
}

// 选中并高亮指定的聊天列表
bool ChatWindow::highlight_chatroom(const QString&chatroom_id) {
    QModelIndex index_chatroom;
    auto&view = m_ui->page_list_chat_room;
    QAbstractItemModel * model = view->model();
    int row_count = model->rowCount();
    for (int row = 0; row < row_count; ++row) {
        QModelIndex index = model->index(row, 0);
        QVariant var = model->data(index, ENUMS::role_item_data());
        if (var.canConvert<std::shared_ptr<ChatroomData::ItemData>>()) {
            auto item = var.value<std::shared_ptr<ChatroomData::ItemData>>();
            if (item && item->chatroom_id == chatroom_id) {
                index_chatroom = index;
                break;
            }
        }
    }
    if (!index_chatroom.isValid()) { return false; }
    view->selectionModel()->select(index_chatroom, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    view->setCurrentIndex(index_chatroom);
    view->scrollTo(index_chatroom, QAbstractItemView::PositionAtCenter);
    return true;
}
