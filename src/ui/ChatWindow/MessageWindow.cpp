#include "MessageWindow.h"
#include "ui_MessageWindow.h"

#include <QApplication>
#include <QClipboard>

#include "Accounts.h"
#include "ContactWindow.h"
#include "Database.h"
#include "NetworkClient.h"
#include "ItemModel.h"
#include "ItemDelegate.h"
#include "MessageDelegate.h"

MessageWindow::MessageWindow(QWidget *parent)
    : QWidget(parent), m_ui(std::make_shared<Ui::MessageWindow>()) {
    // base
    m_ui->setupUi(this);

    // new
    m_message_model = std::make_shared<MessageModel>();
    m_message_delegate = std::make_shared<MessageDelegate>();

    // connect
    // QAction *action_show_contact = new QAction;
    // m_ui->button_title_avatar->setDefaultAction(action_show_contact);
    connect(m_ui->button_title_avatar, &QToolButton::clicked, this, &MessageWindow::slot_button_title_avatar);  // 点击了头像按钮
    connect(m_ui->button_detail      , &QToolButton::clicked, this, &MessageWindow::slot_button_detail      );  // 点击了详情按钮
    connect(m_ui->button_emoji       , &QToolButton::clicked, this, &MessageWindow::slot_button_emoji       );  // 点击了表情按钮
    connect(m_ui->button_file        , &QToolButton::clicked, this, &MessageWindow::slot_button_file        );  // 点击了文件按钮
    connect(m_ui->button_screenshot  , &QToolButton::clicked, this, &MessageWindow::slot_button_screenshot  );  // 点击了截图按钮
    connect(m_ui->button_voice       , &QToolButton::clicked, this, &MessageWindow::slot_button_voice       );  // 点击了语音按钮
    connect(m_ui->button_video       , &QToolButton::clicked, this, &MessageWindow::slot_button_video       );  // 点击了视频按钮
    connect(m_ui->button_send        , &QToolButton::clicked, this, &MessageWindow::slot_button_send        );  // 点击了发送按钮
    // 链接点击 / 图片与文件点击
    connect(m_message_delegate.get() , &MessageDelegate::linkActivated , this, &MessageWindow::slot_open_link);
    connect(m_message_delegate.get() , &MessageDelegate::fileActivated , this, &MessageWindow::slot_open_folder);
    connect(m_message_delegate.get() , &MessageDelegate::imageActivated, this, &MessageWindow::slot_openn_image);
    // 右键菜单
    connect(m_ui->list_messages, &QListView::customContextMenuRequested, this, &MessageWindow::slot_show_context_menu);

    // set
    m_ui->list_messages->setSpacing(0);
    // 按像素滚动，而不是按条目 (会发生跳变)
    m_ui->list_messages->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    // 设置拉伸因子 // 拉伸主窗口时窗口调整大小的策略
    m_ui->splitter->setStretchFactor(0, 1);
    m_ui->splitter->setStretchFactor(1, 0);
    m_ui->splitter->setChildrenCollapsible(false);
    // 设置是否可以被折叠
    // m_ui->splitter->setCollapsible(0, false);
    // m_ui->splitter->setCollapsible(1, false);

    // 修改初始化时的显示比例
    QTimer::singleShot(0, [this](){
        QList<int> list = m_ui->splitter->sizes();
        int height_view = list.at(0);
        int height_ctrl = list.at(1);
        m_ui->splitter->setSizes({ height_view + height_ctrl - 200, 200});
    });

    m_ui->textEdit->setHtml(QString{});
    m_ui->textEdit->setPlainText(QString{});
    // 设置编辑区支持的输入模式
    m_ui->textEdit->setAcceptRichText(false);
    m_ui->textEdit->setAcceptDrops(false);
    // 设置任意位置换行
    m_ui->textEdit->setWordWrapMode(QTextOption::WrapAnywhere);
    // // 1. 确保 LineWrapMode 是 WidgetWidth（在控件边缘换行）
    // m_ui->textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    // // 2. 获取文档的默认 QTextOption 对象，强制设置换行模式为 WrapAnywhere
    // QTextOption option = m_ui->textEdit->document()->defaultTextOption();
    // option.setWrapMode(QTextOption::WrapAnywhere);
    // // 3. 将修改后的 Option 重新应用回 QTextDocument
    // m_ui->textEdit->document()->setDefaultTextOption(option);
    
    // 输入框提示文字
    const auto placeholderText = m_ui->textEdit->placeholderText()
        .arg( enable_return_send ? "Enter" : "Ctrl + Enter")
        .arg(!enable_return_send ? "Enter" : "Ctrl + Enter");
    m_ui->textEdit->setPlaceholderText(placeholderText);

    // model/delegate
    m_ui->list_messages->setModel(m_message_model.get());
    m_ui->list_messages->setItemDelegate(m_message_delegate.get());
    m_ui->list_messages->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // 像素级滚动更顺滑
    m_ui->list_messages->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_ui->list_messages->setResizeMode(QListView::Adjust);  // resize 后自动重新布局（重新调用 sizeHint）
    m_ui->list_messages->setMovement(QListView::Static);
    m_ui->list_messages->setSelectionMode(QAbstractItemView::NoSelection);    // 选择由委托自己实现（富文本级别）
    m_ui->list_messages->setEditTriggers(QAbstractItemView::NoEditTriggers);  //
    m_ui->list_messages->setSpacing(0);                                       // 行距在 delegate 内部处理
    m_ui->list_messages->setMouseTracking(true);                              // 悬停光标（手型/文本光标）需要鼠标追踪
    m_ui->list_messages->setContextMenuPolicy(Qt::CustomContextMenu);         // 右键菜单（视图的 contextMenuEvent 不会进委托）

    // Ctrl+C 复制选中文字 / Esc 清除选区 / 点空白处清除选区
    m_ui->list_messages->installEventFilter(this);
    m_ui->list_messages->viewport()->installEventFilter(this);
    m_ui->textEdit->installEventFilter(this);

    // TODO:未添加的功能
    m_ui->button_detail    ->setVisible(false);
    m_ui->button_emoji     ->setVisible(false);
    m_ui->button_file      ->setVisible(false);
    m_ui->button_screenshot->setVisible(false);
    m_ui->button_voice     ->setVisible(false);
    m_ui->button_video     ->setVisible(false);
    m_ui->layout_widget_input->removeItem(m_ui->layout_button_ctrls);
}

MessageWindow::~MessageWindow() {
}

void MessageWindow::set_contact(const std::shared_ptr<ContactInfo>& contact_info) {
    m_contact_info = contact_info;
    QString name_show;
    QString avatar_id;
    do {
        if (!contact_info) { break; }
        // 暂不启用备注
        // if (contact_info->hasFriendInfo()) {
        //     name_show = contact_info->friendInfo().notes();
        // }
        if (name_show.isEmpty() && contact_info->hasBaseInfo()) {
            name_show = contact_info->baseInfo().nickname();
            avatar_id = contact_info->baseInfo().avatarId();
        }

#ifdef _DEBUG
        if (name_show.isEmpty() && contact_info->hasBaseInfo()) {
            name_show = contact_info->baseInfo().accountId();
        }
#endif // _DEBUG

    } while (false);
    m_ui->label_title->setText(name_show);
    show_avatar_id(avatar_id);
}

std::shared_ptr<ContactInfo> MessageWindow::get_contact() const {
    return m_contact_info;
}

// 根据文件 ID 在本地查找文件并加载为头像图片
std::shared_ptr<QPixmap> MessageWindow::get_local_avatar_id_pixmap(const QString&avatar_id) {
    return std::make_shared<QPixmap>(Database::instance().get_local_file_id_path(avatar_id));
}

// 更新当前页面上的头像显示，无网络及校验功能
void MessageWindow::show_avatar(const std::shared_ptr<QPixmap>&avatar_pixmap) {
    m_ui->button_title_avatar->resize(32, 32);
    m_ui->button_title_avatar->setIcon(QIcon(avatar_pixmap->scaled(m_ui->button_title_avatar->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
}

// 传入头像 ID，自动从本地或网络加载头像
void MessageWindow::show_avatar_id(const QString&avatar_id) {
    // 先尝试从本地加载头像
    std::shared_ptr<QPixmap> avatar_pixmap = get_local_avatar_id_pixmap(avatar_id + ";size:128*128");
    if ((avatar_pixmap && !avatar_pixmap->isNull()) || avatar_id.isEmpty()) {
        show_avatar(avatar_pixmap);
        return;
    }

    if (avatar_pixmap && !avatar_pixmap->isNull()) {
        return;
    }

    // 下载并显示头像
    network_file_io_notify_callback callback_download_file_notify = [this](const NetworkFileIONotify&notify) {
        // 忽略下载进度通知
        if (notify.ok && notify.code == int64_t(SuccessCode::SUCCESS) && notify.uploading) {
            return;
        }
        if (!notify.ok) {
            // QString error_message = tr("Failed to download avatar.");
            // error_message += tr("\n\ncode : 0x%1\nmessage : %2")
            //     .arg(notify.code, 8, 16, QChar('0'))  // 16 进制显示，宽度 8 位，不足补 0
            //     .arg(notify.message);
            // QMessageBox::information(this, tr("Error"), error_message, QMessageBox::StandardButton::Ok);
            return;
        }
        // 下载成功，文件已保存到本地，可以直接显示了
        show_avatar(get_local_avatar_id_pixmap(notify.id + ";size:128*128"));
    };
    qint64 downloaded_size = QFileInfo(Database::instance().get_local_file_id_path(avatar_id)).size();
    NetworkClient::instance().download_file(avatar_id, qMax(qint64(0), downloaded_size), callback_download_file_notify);
}

bool MessageWindow::eventFilter(QObject *watched, QEvent *event) {
    // 如果是按键消息，那么先获取按键
    QKeyEvent  * key_event   = event->type() == QEvent::KeyPress         ? static_cast<QKeyEvent  *>(event) : nullptr;
    QMouseEvent* mouse_event = event->type() == QEvent::MouseButtonPress ? static_cast<QMouseEvent*>(event) : nullptr;
    bool is_key_press    = event->type() == QEvent::KeyPress        ;  // 键盘按键
    bool is_mouse_press  = event->type() == QEvent::MouseButtonPress;  // 鼠标左键
    bool is_event_resize = event->type() == QEvent::Resize          ;  // 窗口大小调整
    // 没有修饰键  // 小键盘会被添加 KeypadModifier 标志
    bool is_no_ctrls     = is_key_press ? (key_event->modifiers() == Qt::NoModifier || key_event->modifiers() == Qt::KeypadModifier) : false;
    // Windows 上回车键是 Return 消息，而不是 Enter 消息
    bool is_key_return   = is_key_press ? (key_event->key      () == Qt::Key_Enter  || key_event->key      () == Qt::Key_Return    ) : false;
    bool is_key_ctrl     = is_key_press ? (key_event->modifiers() &  Qt::ControlModifier) : false;  // Ctrl 按键
    bool is_key_esc      = is_key_press ? key_event->key() == Qt::Key_Escape : false;  // Esc 按键
    bool is_key_c        = is_key_press ? key_event->key() == Qt::Key_C      : false;  // C 按键
    bool is_key_v        = is_key_press ? key_event->key() == Qt::Key_V      : false;  // V 按键
    bool is_obj_edit     = watched == m_ui->textEdit;
    bool is_obj_listview = watched == m_ui->list_messages;
    bool is_obj_viewport = watched == m_ui->list_messages->viewport();

#ifdef _DEBUG
    if (0) {
        if (is_key_press) {
            qDebug().noquote().nospace()
                << "Key Press : " << QMetaEnum::fromType<Qt::KeyboardModifiers>().valueToKeys(key_event->modifiers())
                << " + " << QMetaEnum::fromType<Qt::Key>().valueToKeys(key_event->key())
                << ", obj : " << watched->objectName();
        }
    }
#endif // _DEBUG

    // 按下 Ctrl + C 进行选中文本消息的复制
    if (is_key_press && is_key_ctrl && is_key_c) {
        // Ctrl+C：复制委托内富文本级别的选中文字
        const QString sel = m_message_delegate->selectedText();
        if (!sel.isEmpty()) {
            QApplication::clipboard()->setText(sel);
            return true;  // 已经处理，不要再传递给控件
        }
    }
    // 输入框 Ctrl+V：剪贴板有图片 → 直接发送图片消息（吃掉按键，不粘贴路径/无内容）
    else if (is_key_press && is_key_ctrl && is_key_v && is_obj_edit) {
        if (QApplication::clipboard()->mimeData()->hasImage()) {
            const QImage img = QApplication::clipboard()->image();
            if (!img.isNull()) {
                // sendImage(img);
                return true;  // 已经处理，不要再传递给控件
            }
        }
    }
    // Enter 发送消息，需要额外修改按键消息  // 小键盘 Return 也处理
    else if (is_key_press && is_key_return && is_obj_edit) {
        // 只处理仅 Return 和 Ctrl + Return 的消息
        if (   ( enable_return_send && is_no_ctrls)
            || (!enable_return_send && is_key_ctrl)) {
            m_ui->button_send->animateClick();
            return true;  // 已经处理，不要再传递给控件
        }
        // 只处理仅 Return 和 Ctrl + Return 的消息
        // 1.启用 Return        换行时只处理   Ctrl + Return 按键消息，不处理 Shift/... + Return 等消息
        // 2.启用 Ctrl + Return 换行时只处理仅        Return 按键消息
        else if (( enable_return_send && is_key_ctrl)
            ||   (!enable_return_send && is_no_ctrls)
        ) {
            // // 1.修改后再次投递时需要做标记，防止被再次捕获
            // static bool is_internal_sending = false;
            // if (is_internal_sending) { return QWidget::eventFilter(watched, event); }
            // QKeyEvent modified_event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, "\n");
            // QCoreApplication::sendEvent(m_ui->textEdit, &modified_event);

            // // 2. 先移除事件过滤器后再次安装
            // m_ui->textEdit->removeEventFilter(this);                      // 1. 临时移除
            // QKeyEvent modified_event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, "\n");
            // QCoreApplication::sendEvent(m_ui->textEdit, &modified_event); // 2. 发送事件
            // m_ui->textEdit->installEventFilter(this);                     // 3. 重新安装

            // 3. 直接在 QTextEdit 光标位置插入新行。插入新行后光标不会自动更新位置且会消失。
            m_ui->textEdit->insertPlainText("\n");  // 1. 插入换行符
            m_ui->textEdit->setFocus();             // 2. 确保 QTextEdit 获得焦点
            m_ui->textEdit->ensureCursorVisible();  // 3. 强制视图自动滚动，保持当前光标可见
            return true; // 拦截原始的 Ctrl + Enter
        }
    }
    // 按 Esc 键清除选择的文字
    else if (is_key_press && is_key_esc) {
        m_message_delegate->resetSelection();
        m_ui->list_messages->viewport()->update();
        return true;  // 已经处理，不要再传递给控件
    }
    // 点击空白处（无有效条目）清除选区
    else if (is_mouse_press && is_obj_viewport) {
        if (!m_ui->list_messages->indexAt(mouse_event->pos()).isValid()) {
            m_message_delegate->resetSelection();
            m_ui->list_messages->viewport()->update();
            return true;  // 已经处理，不要再传递给控件
        }
    }
    // 窗口大小改变时刷新消息列表的显示
    else if (is_obj_listview && is_event_resize) {
        // 1. 告知布局重算并更新滚动条
        // m_ui->list_messages->updateGeometries();     // 1.私有，不可访问
        // m_ui->list_messages->doItemsLayout();        // 2. 刷新布局
        m_ui->list_messages->model()->layoutChanged();  // 3. 刷新视图
        // 2. 刷新绘制
        m_ui->list_messages->viewport()->update();
    }
    // 没有处理的消息交给系统默认处理程序(继续由对应控件处理)
    return QWidget::eventFilter(watched, event);
}

void MessageWindow::slot_button_title_avatar() {
    // return;  // TODO:编辑功能有问题
    // 重建后再显示
    // if ( m_contact_window) { m_contact_window.reset(); }
    if (!m_contact_window) { m_contact_window = std::make_shared<ContactWindow>(this); }
    // 设置为弹窗窗口，否则有父窗口时会显示在父窗口内，发生与父窗口内容重叠的情况
    m_contact_window->setWindowFlags(m_contact_window->windowFlags() |  Qt::Dialog);
    m_contact_window->show_contact(m_contact_info);
    using ShowFlags = ContactWindow::ShowFlags;
    using ShowFlag  = ContactWindow::ShowFlag ;
    auto show_mode = m_contact_window->get_show_mode();
    show_mode |= ShowFlags(ShowFlag::Profile );
    show_mode |= ShowFlags(ShowFlag::ViewOnly);
    m_contact_window->set_show_mode(show_mode);
    m_contact_window->show();

    // 先显示窗口再更新内容，避免网络请求较慢时窗口无响应
    auto&user_id = m_contact_info->baseInfo().userId();
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

void MessageWindow::slot_button_detail() {
}

void MessageWindow::slot_button_emoji() {
}

void MessageWindow::slot_button_file() {
}

void MessageWindow::slot_button_screenshot() {
}

void MessageWindow::slot_button_video() {
}

void MessageWindow::slot_button_voice() {
}

void MessageWindow::slot_button_send() {
    // auto current_char_format = m_ui->textEdit->currentCharFormat();
    // QString text_html        = m_ui->textEdit->toHtml();
    // QString text_markdown    = m_ui->textEdit->toMarkdown();
    QString text_plain          = m_ui->textEdit->toPlainText();

    if (text_plain.isEmpty()) { return; }

    // 1. 创建内部的 google::protobuf::Timestamp 对象
    google::protobuf::Timestamp pb_timestamp;
    // 2. 获取当前毫秒级时间戳
    qint64 msecs = QDateTime::currentMSecsSinceEpoch();
    // 3. 填充秒和纳秒
    pb_timestamp.setSeconds(msecs / 1000);
    pb_timestamp.setNanos((msecs % 1000) * 1000000);
    // 4. 创建包装的 TaoTieChat::Timestamp 并设置内部的 google::protobuf::Timestamp
    TaoTieChat::Timestamp timestamp;
    timestamp.setTimestamp(pb_timestamp);

    auto message_context = std::make_shared<MessageContext>();
    message_context->setTimestamp(timestamp);
    message_context->setClientMessageId(Utils::get_new_uuid_string());
    message_context->setUserId(Accounts::instance().get_current_account()->baseInfo().accountId());
    message_context->setShortText(text_plain);
    if (m_message_model->add_message(message_context)) {
        m_ui->textEdit->clear();
        m_ui->list_messages->scrollToBottom();
    }
}

void MessageWindow::slot_open_link(const QString &href) {
    const QUrl url = QUrl::fromUserInput(href);
    if (url.isValid()) {
        QDesktopServices::openUrl(url);
    }
}

void MessageWindow::slot_open_folder(const QString &filePath) {
    if (filePath.isEmpty() || !QFileInfo::exists(filePath)) { return; }
#ifdef Q_OS_WIN
    // 资源管理器中定位并选中该文件
    QProcess::startDetached(QStringLiteral("explorer.exe"), { QStringLiteral("/select,") + QDir::toNativeSeparators(filePath) });
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(filePath).absolutePath()));
#endif
}

void MessageWindow::slot_openn_image(const QString &filePath) {
    if (filePath.isEmpty() || !QFileInfo::exists(filePath)) { return; }
    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath)); // 系统默认看图软件
}

void MessageWindow::slot_show_context_menu(const QPoint &viewportPos) {
    const QModelIndex index = m_ui->list_messages->indexAt(viewportPos);
    if (!index.isValid()) { return; }
    m_message_delegate->showContextMenu(m_ui->list_messages, index, m_ui->list_messages->viewport()->mapToGlobal(viewportPos), viewportPos);
}

// // 消息较多时应该差异化更新
// void MessageWindow::update_messages(Messages messages) {
//     m_messages = messages;
// }

// void MessageWindow::append_message (Message message ) {
//     //
//     ui->list_messages->scrollToBottom(); // 滚动到最后
// }

