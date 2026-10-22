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
#include "MediaWindow.h"
#include "SignalInner.h"

MessageWindow::MessageWindow(QWidget *parent)
    : QWidget(parent), m_ui(std::make_shared<Ui::MessageWindow>()) {
    // base
    m_ui->setupUi(this);

    // new
    m_message_model    = std::make_shared<MessageModel   >();
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
    // connect(m_message_delegate.get() , &MessageDelegate::signal_open_link  , this, &MessageWindow::slot_open_link  );
    // connect(m_message_delegate.get() , &MessageDelegate::signal_open_file  , this, &MessageWindow::slot_open_file  );
    // connect(m_message_delegate.get() , &MessageDelegate::signal_open_folder, this, &MessageWindow::slot_open_folder);
    // connect(m_message_delegate.get() , &MessageDelegate::signal_open_data  , this, &MessageWindow::slot_open_data  );
    // connect(m_message_delegate.get() , &MessageDelegate::signal_copy_data  , this, &MessageWindow::slot_copy_data  );
    // connect(m_message_delegate.get() , &MessageDelegate::signal_copy_file  , this, &MessageWindow::slot_copy_file  );
    // connect(m_message_delegate.get() , &MessageDelegate::signal_copy_path  , this, &MessageWindow::slot_copy_path  );
    // connect(m_message_delegate.get() , &MessageDelegate::signal_save_to    , this, &MessageWindow::slot_save_to    );
    connect(SignalInner::instance(), &SignalInner::signal_open_link  , this, &MessageWindow::slot_open_link  );
    connect(SignalInner::instance(), &SignalInner::signal_open_file  , this, &MessageWindow::slot_open_file  );
    connect(SignalInner::instance(), &SignalInner::signal_open_folder, this, &MessageWindow::slot_open_folder);
    connect(SignalInner::instance(), &SignalInner::signal_open_data  , this, &MessageWindow::slot_open_data  );
    connect(SignalInner::instance(), &SignalInner::signal_copy_data  , this, &MessageWindow::slot_copy_data  );
    connect(SignalInner::instance(), &SignalInner::signal_copy_file  , this, &MessageWindow::slot_copy_file  );
    connect(SignalInner::instance(), &SignalInner::signal_copy_path  , this, &MessageWindow::slot_copy_path  );
    connect(SignalInner::instance(), &SignalInner::signal_save_to    , this, &MessageWindow::slot_save_to    );
    // 右键菜单
    connect(m_ui->list_messages, &QListView::customContextMenuRequested, this, &MessageWindow::slot_show_context_menu);
    // 限制消息长度
    connect(m_ui->textEdit, &QTextEdit::textChanged, this, [this](){
        auto plain_text = m_ui->textEdit->toPlainText();
        auto current_text_length = plain_text.length();
        auto avaliable_text_length = SHORT_MESSAGE_MAX_LENGTH - 1;
        // 允许发送的字符长度：%1
        // 当前输入的字符长度：%2
        auto tooltips = tr("Allowed character length: %1\nCurrent input character length: %2").arg(current_text_length).arg(avaliable_text_length);
        m_ui->textEdit->setToolTip(tooltips);
        m_ui->button_send->setToolTip(tooltips);
        // 基于服务器对此的要求，超长文本不允许发送
        m_ui->button_send->setEnabled(current_text_length < avaliable_text_length);
    });
    // connect(Accounts::instance(), &Accounts::signal_contact_info_changed, this, &MessageWindow::slot_update_contact_info);
    connect(SignalInner::instance(), &SignalInner::signal_contact_info_changed, this, &MessageWindow::slot_update_contact_info);

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
    m_ui->list_messages->verticalScrollBar()->setSingleStep(20);              // 调整鼠标一次滚动的像素数

    // Ctrl+C 复制选中文字 / Esc 清除选区 / 点空白处清除选区
    m_ui->list_messages->installEventFilter(this);
    m_ui->list_messages->viewport()->installEventFilter(this);
    m_ui->textEdit->installEventFilter(this);
    m_ui->label_title->installEventFilter(this);

    // TODO:未添加的功能
    m_ui->button_detail    ->setVisible(false);
    m_ui->button_emoji     ->setVisible(false);
    m_ui->button_file      ->setVisible(false);  // 待测试
    m_ui->button_screenshot->setVisible(false);
    m_ui->button_voice     ->setVisible(false);
    m_ui->button_video     ->setVisible(false);
    // m_ui->layout_widget_input->removeItem(m_ui->layout_button_ctrls);
}

MessageWindow::~MessageWindow() {
}

void MessageWindow::set_contact(std::shared_ptr<ContactData::ItemData> contact_data) {
    m_contact_data = contact_data;
    m_ui->label_title->setText(contact_data->display_name);
    show_avatar_id(contact_data->avatar_id);
    // TODO:更新聊天记录中的账号信息
}

std::shared_ptr<ContactData::ItemData> MessageWindow::get_contact() const {
    return m_contact_data;
}

// 根据文件 ID 在本地查找文件并加载为头像图片
std::shared_ptr<QPixmap> MessageWindow::get_local_avatar_id_pixmap(const QString&avatar_id) {
    return std::make_shared<QPixmap>(Database::instance()->get_local_file_id_path(avatar_id));
}

// 更新当前页面上的头像显示，无网络及校验功能
void MessageWindow::show_avatar(std::shared_ptr<QPixmap> avatar_pixmap) {
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
    QString notify_id = QString("notify MessageWindow::show_avatar_id %1").arg(avatar_id);
    NetworkClient::instance()->download_file(avatar_id, notify_id, callback_download_file_notify);
}

// 判断是否添加一条时间消息及滚动到底部
bool MessageWindow::add_message(std::shared_ptr<MessageData::ItemData> message_data) {
    do {
        if (!message_data) { break; }

        // 判断滚动条是否在底部，在底部时新增消息后依然置底
        // 1. 获取 Model 中最后一条消息的索引
        int last_row = m_message_model->rowCount() - 1;
        bool should_auto_scroll = true;
        if (last_row >= 0) {
            QModelIndex last_index = m_message_model->index(last_row, 0);
            // 2. 获取最后一条消息在 View 内的视觉矩形 (Visual Rect)
            QRect item_rect = m_ui->list_messages->visualRect(last_index);
            // 3. 获取 View 当前的可视区域矩形
            QRect viewport_rect = m_ui->list_messages->viewport()->rect();
            // 4. 判断最后一条消息的矩形是否与可视区域重合（即可视区域内能看到最后一条消息）
            should_auto_scroll = item_rect.intersects(viewport_rect);
        }

        // 读取数据
        // 获取上一条消息，如果消息间隔大于 2 分钟则添加一条时间提示
        bool need_timestamp = false;
        // 没有任何消息时需要在顶部添加时间提示
        if (m_message_model->row_count() <= 0) { need_timestamp = true; }
        // 有历史消息时进行时间戳比较
        else {
            do {
                auto last_message = m_message_model->get_last_message();
                if (!last_message) { break; }
                if (!last_message->message_time.isValid()) { break; }
                // 如果时间间隔在 2 分钟及以上
                if (message_data->message_time - last_message->message_time >= std::chrono::minutes(2)) {
                    need_timestamp = true;
                }
            } while(false);
        }
        if (need_timestamp) {
            // 基于副本进行修改
            // 系统消息也要生成消息 ID 以便过滤
            auto message_timestamp = std::make_shared<MessageData::ItemData>(*message_data.get());
            message_timestamp->sender_type = MessageData::SenderType::SYSTEM;
            message_timestamp->media_type  = MessageData::MediaType::TIMESTAMP;
            message_timestamp->message_id  = "system::timestamp:" + message_data->message_time.toLocalTime().toString(Qt::ISODate);
            m_message_model->add_message(message_timestamp);
        }

        // 消息内容
        if (!m_message_model->add_message(message_data)) { break; }
        // 通知列表更新消息
        emit SignalInner::instance()->signal_new_message(message_data);

        // 6. 如果刚才最后一条消息在视口内，强制精准滚动到最新消息的【底部】
        if (should_auto_scroll) {
            // 必须用 QTimer::singleShot 延迟到下一帧，等待 Delegate 重新计算完新消息的 Height 并更新 ScrollBar 范围
            QTimer::singleShot(0, this, [this]() {
                QScrollBar *v_bar = m_ui->list_messages->verticalScrollBar();
                v_bar->setValue(v_bar->maximum());
            });
        }
        return true;
    } while (false);
    return false;
}

// 获取最后一条消息以便列表显示
std::shared_ptr<MessageData::ItemData> MessageWindow::get_last_message() {
    if (!m_message_model) { return nullptr; }
    return m_message_model->get_last_message();
}

// 通知视图更新
void MessageWindow::update_message(std::shared_ptr<MessageData::ItemData> message_data) {
    m_message_model->update_message(message_data);
}

std::shared_ptr<MessageData::ItemData> MessageWindow::get_message(const QString&message_id) const {
    return m_message_model->get_message(message_id);
}

bool MessageWindow::eventFilter(QObject *watched, QEvent *event) {
    // 如果是按键消息，那么先获取按键
    QKeyEvent   * key_event    = event->type() == QEvent::KeyPress         ? static_cast<QKeyEvent   *>(event) : nullptr;
    QMouseEvent * mouse_event  = event->type() == QEvent::MouseButtonPress ? static_cast<QMouseEvent *>(event) : nullptr;
    QResizeEvent* reszie_event = event->type() == QEvent::Resize           ? static_cast<QResizeEvent*>(event) : nullptr;
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

    // 显示裁剪文本
    if (watched == m_ui->label_title && reszie_event) {
        if (m_contact_data) {
            const QFontMetrics font_metrics(m_ui->label_title->font());
            auto label_width = m_ui->label_title->contentsRect().width();
            auto elided_text = font_metrics.elidedText(m_contact_data->display_name, Qt::ElideRight, label_width);
            m_ui->label_title->setText(elided_text);
        }
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
    m_contact_window->set_contact_data(m_contact_data, ContactWindow::SourceFlag::Contact);
    m_contact_window->show();

    // 先显示窗口再更新内容，避免网络请求较慢时窗口无响应
    auto user_id = m_contact_data->user_id;
    // 更新成功，刷新显示
    // update(params_update_profile->contact());
    search_user_params params_search_user = std::make_shared<TaoTieChat::SearchUserRequest>();
    params_search_user->setRequestUserAuth(*Accounts::instance()->get_auth_request().get());
    params_search_user->setUserId(user_id);
    NetworkClient::instance()->search_user(params_search_user, [this](search_user_params params_search_user, search_user_reply reply_search_user) {
        if (!reply_search_user || !reply_search_user->result().hasSuccessCode()) { return; }
        auto contact_data = Utils::Transform::to_contact_data(std::make_shared<ContactInfo>(reply_search_user->contactInfo()));
        m_contact_window->set_contact_data(contact_data, ContactWindow::SourceFlag::Contact);
        // m_contact_window->setWindowTitle(tr("Profile"));
    });
}

void MessageWindow::slot_button_detail() {
}

void MessageWindow::slot_button_emoji() {
}

void MessageWindow::slot_button_file() {
    const QString path = QFileDialog::getOpenFileName(this, tr("Select file to send"), QString(), tr("All files (*.*)"));
    if (path.isEmpty()) { return; }

    // QPixmap 主要为显示设计，相当于显示优化专用
    // QImage 支持像素级操作与图像处理，相当于原始图像
    // QBitmap 只有 1 位的色彩深度
    // QPicture 不存储像素数据，而是记录 QPainter 的绘制命令
    MessageData::MediaType media_type = MessageData::MediaType::FILE;
    std::shared_ptr<MessageData::TextData > text_data;
    std::shared_ptr<MessageData::ImageData> image_data;
    std::shared_ptr<MessageData::FileData > file_data;
    if (0) {
        media_type = MessageData::MediaType::TEXT;
        // ...
    }
    else if ( QFileInfo::exists(path)) {
        QFileInfo file_info(path);
        file_data = std::make_shared<MessageData::FileData>();
        file_data->file_id    = "";
        file_data->file_path  = file_info.absoluteFilePath();
        file_data->file_name  = file_info.fileName();
        file_data->human_size = Utils::Transform::human_size(file_info.size());
        file_data->file_size  = file_info.size();
        file_data->file_icon  = qApp->style()->standardIcon(QStyle::SP_FileIcon);
        if (!QImageReader::imageFormat(path).isEmpty()) {
            media_type = MessageData::MediaType::IMAGE;
            image_data = std::make_shared<MessageData::ImageData>();
            // TODO:这里仅需缓存预览图以便节省空间，需要按比例缩放。视图中会拉伸窗口，因此此图像不宜太小。
            image_data->pixmap = QPixmap(path);;
            // QSize size_scaled  = QSize(512, 512);
            // image_data->pixmap = pixmap.scaled(size_scaled, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }
    else {
        return;  // 不支持的消息类型
    }

    auto sender_data   = Accounts::instance()->get_current_account();
    auto account_id    = sender_data->account_id;
    auto avatar_id     = sender_data->account_id;
    auto message_id    = Utils::get_new_uuid_string();
    auto message_time  = QDateTime::currentDateTimeUtc();
    auto avatar_pixamp = QPixmap(Database::instance()->get_local_file_id_path(avatar_id + ";size:128*128"));

    // UI 所需数据
    auto message_data  = std::make_shared<MessageData::ItemData>();
    message_data->sender_type           = MessageData::SenderType::USER;
    message_data->media_type            = media_type;
    message_data->is_me                 = true;
    message_data->message_id            = message_id;
    message_data->message_time          = message_time;
    message_data->sender                = std::make_shared<MessageData::SenderData>();
    message_data->sender->account_id    = account_id;
    message_data->sender->display_name  = Utils::get_display_name(sender_data);
    message_data->sender->avatar_id     = sender_data->avatar_id;
    message_data->sender->avatar_pixmap = avatar_pixamp;
    ;    if (media_type == MessageData::MediaType::TEXT ) { message_data->text  = text_data ; }
    else if (media_type == MessageData::MediaType::IMAGE) { message_data->image = image_data; }
    else if (media_type == MessageData::MediaType::FILE ) { message_data->file  = file_data ; }
    if (!add_message(message_data)) { return; }
    m_ui->textEdit->clear();
    m_ui->list_messages->scrollToBottom();

    // return;

    // qDebug().noquote().nospace() << "Current account contactInfo: " << Utils::Transform::to_string(Accounts::instance()->get_current_account());
    // qDebug().noquote().nospace() << "Cache           contactInfo: " << Utils::Transform::to_string(m_cache_contact_info);

    // 先本地界面添加上传记录，待文件上传完成后，该条消息才有意义
    struct MessageSendFileData {
        QString   file_path;
        QString   account_id;
        QString   message_id;
        QDateTime message_time;
    };
    std::shared_ptr<MessageSendFileData> message_send_file_data = std::make_shared<MessageSendFileData>();
    message_send_file_data->file_path    = path;
    message_send_file_data->account_id   = account_id;
    message_send_file_data->message_id   = message_id;
    message_send_file_data->message_time = message_time;
    auto send_file_message = [this, message_send_file_data](const QString&file_id) {
        if (!message_send_file_data) { return; }
        // 1. 通知服务器有文件上传消息
        auto message_context = std::make_shared<MessageContext>();
        message_context->setRequestUserAuth(*Accounts::instance()->get_auth_request().get());
        message_context->setSenderAccountId(message_send_file_data->account_id);
        message_context->setClientMessageId(message_send_file_data->message_id);
        message_context->setTimestamp(Utils::Transform::to_timestamp(message_send_file_data->message_time));
        message_context->setAccountId(get_contact()->account_id);
        // message_context->setShortText(text_plain);
        message_context->setFileId(file_id);
        // qDebug().noquote().nospace() << "addr : " << (void *)&message_data << ", context : " << *message_data.get();
        send_message_callback callback_send_message = [this](send_message_params params_send_message, send_message_reply reply_send_message) {
            // 消息发送失败
            if (!reply_send_message || !reply_send_message->hasResult() || !reply_send_message->result().hasSuccessCode()) {
                QString error_message = tr("Failed to send chat message.");
                if (reply_send_message && reply_send_message->hasResult()) {
                    error_message += tr("\n\ncode : 0x%1\nmessage : %2")
                                        .arg(int64_t(reply_send_message->result().errorCode()), 8, 16, QChar('0'))  // 16 进制显示，宽度 8 位，不足补 0
                                        .arg(reply_send_message->result().message());
                }
                qDebug().noquote().nospace() << "error message : " << error_message;
                // QMessageBox::information(this, tr("Error"), error_message, QMessageBox::StandardButton::Ok);
                // TODO:显示重新发送按钮
                return;
            }
        };
        NetworkClient::instance()->send_message(message_context, callback_send_message);
    };

    // 2. 上传文件
    network_file_io_notify_callback callback_upload_file_notify = [this, send_file_message](const NetworkFileIONotify&notify) {
        if (!notify.ok || notify.status != FileStatusCode::FILE_STATUS_UPLOADED) {
            // 16 进制显示，宽度 8 位，不足补 0
            QString error_message = tr("Failed to upload avatar.\n\ncode : 0x%1\nmessage : %2").arg(notify.code, 8, 16, QChar('0')).arg(notify.message);
            // QMessageBox::information(this, tr("Error"), error_message, QMessageBox::StandardButton::Ok);
            qDebug().noquote().nospace() << "network_file_io_notify_callback error_message : " << error_message;
            return;
        }
        // 上传完成后再通知服务器写入消息
        do {
            // 验证有效
            if (!notify.ok) { break; }
            if ( notify.code   != int(SuccessCode::SUCCESS)            ) { break; }
            if ( notify.status != FileStatusCode::FILE_STATUS_UPLOADED ) { break; }
            if ( notify.id .isEmpty()) { break; }
            if ( notify.md5.isEmpty()) { break; }
            if ( notify.size < 0) { break; }
            //
            const QString&file_id = notify.id;
            if (file_id.isEmpty()) { return; }
            send_file_message(file_id);
        } while (false);
    };
    NetworkClient::instance()->upload_file(path, callback_upload_file_notify);
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

    auto sender_data   = Accounts::instance()->get_current_account();
    auto account_id    = sender_data->account_id;
    auto avatar_id     = sender_data->avatar_id;
    auto message_id    = Utils::get_new_uuid_string();
    auto message_time  = QDateTime::currentDateTimeUtc();
    auto avatar_pixamp = QPixmap(Database::instance()->get_local_file_id_path(avatar_id + ";size:128*128"));

    // UI 所需数据
    auto message_data  = std::make_shared<MessageData::ItemData>();
    message_data->chatroom_id           = get_contact()->account_id;
    message_data->sender_type           = MessageData::SenderType::USER;
    message_data->media_type            = MessageData::MediaType::TEXT;
    message_data->is_me                 = true;
    message_data->message_id            = message_id;
    message_data->message_time          = message_time;
    message_data->sender                = std::make_shared<MessageData::SenderData>();
    message_data->sender->account_id    = account_id;
    message_data->sender->display_name  = Utils::get_display_name(sender_data);
    message_data->sender->avatar_id     = sender_data->avatar_id;
    message_data->sender->avatar_pixmap = avatar_pixamp;
    message_data->text                  = std::make_shared<MessageData::TextData>();
    message_data->text->text            = text_plain;
    if (!add_message(message_data)) { return; }
    m_ui->textEdit->clear();
    m_ui->list_messages->scrollToBottom();

    // 通知服务器
    // 服务器所需数据
    auto message_context = std::make_shared<MessageContext>();
    message_context->setRequestUserAuth(*Accounts::instance()->get_auth_request().get());
    message_context->setSenderAccountId(account_id);
    message_context->setClientMessageId(message_id);
    message_context->setTimestamp(Utils::Transform::to_timestamp(message_time));
    message_context->setAccountId(get_contact()->account_id);
    message_context->setShortText(text_plain);
    // qDebug().noquote().nospace() << "addr : " << (void *)&message_data << ", context : " << *message_data.get();
    send_message_callback callback_send_message = [this](send_message_params params_send_message, send_message_reply reply_send_message) {
        // 消息发送失败
        if (!reply_send_message || !reply_send_message->hasResult() || !reply_send_message->result().hasSuccessCode()) {
            QString error_message = tr("Failed to send chat message.");
            if (reply_send_message && reply_send_message->hasResult()) {
                error_message += tr("\n\ncode : 0x%1\nmessage : %2")
                                     .arg(int64_t(reply_send_message->result().errorCode()), 8, 16, QChar('0'))  // 16 进制显示，宽度 8 位，不足补 0
                                     .arg(reply_send_message->result().message());
            }
            qDebug().noquote().nospace() << "error message : " << error_message;
            // QMessageBox::information(this, tr("Error"), error_message, QMessageBox::StandardButton::Ok);
            // TODO:显示重新发送按钮
            return;
        }
    };
    NetworkClient::instance()->send_message(message_context, callback_send_message);
}

void MessageWindow::slot_show_context_menu(const QPoint &viewportPos) {
    const QModelIndex index = m_ui->list_messages->indexAt(viewportPos);
    if (!index.isValid()) { return; }
    m_message_delegate->showContextMenu(m_ui->list_messages, index, m_ui->list_messages->viewport()->mapToGlobal(viewportPos), viewportPos);
}

void MessageWindow::slot_open_link(const QString &href) {
    const QUrl url = QUrl::fromUserInput(href);
    if (url.isValid()) {
        QDesktopServices::openUrl(url);
    }
}

// 使用内部插件打开
void MessageWindow::slot_open_data(std::shared_ptr<MessageData::ItemData> message_data) {
    do {
        if (!message_data) { break; }
        if (!message_data->file) { break; }
        if ( message_data->file->file_path.isEmpty()) { break; }
        const QString &file_path = message_data->file->file_path;
        bool is_file_exists = QFileInfo::exists(file_path);
        if (file_path.isEmpty() || !is_file_exists) { break; }
        //
        bool is_system = message_data->sender_type == MessageData::SenderType::SYSTEM;
        bool is_user   = message_data->sender_type == MessageData::SenderType::USER  ;
        bool is_text   = message_data->media_type  == MessageData::MediaType ::TEXT  ;
        bool is_image  = message_data->media_type  == MessageData::MediaType ::IMAGE ;
        bool is_file   = message_data->media_type  == MessageData::MediaType ::FILE  ;
        bool is_me     = message_data->is_me;
        //
        if (is_image && is_file_exists) {
            QFileInfo file_info(file_path);
            //
            std::vector<std::shared_ptr<MediaInfo>> play_list;
            std::shared_ptr<MediaInfo> media_info = std::make_shared<MediaInfo>();
            media_info->media_name  = file_info.fileName();
            media_info->media_path  = file_path;
            media_info->media_image = QImage();
            play_list.push_back(media_info);
            //
            MediaWindow*media_window = &MediaWindow::instance();
            media_window->set_play_list(play_list);
            Utils::widget_raise(media_window);
        }
    } while(false);
}

// 复制文件内容到剪切板，比如图像内容
void MessageWindow::slot_copy_data(std::shared_ptr<MessageData::ItemData> message_data) {
    do {
        if (!message_data) { break; }
        if (!message_data->file) { break; }
        if ( message_data->file->file_path.isEmpty()) { break; }
        const QString &file_path = message_data->file->file_path;
        bool is_file_exists = QFileInfo::exists(file_path);
        if (file_path.isEmpty() || !is_file_exists) { break; }
        //
        bool is_system = message_data->sender_type == MessageData::SenderType::SYSTEM;
        bool is_user   = message_data->sender_type == MessageData::SenderType::USER  ;
        bool is_text   = message_data->media_type  == MessageData::MediaType ::TEXT  ;
        bool is_image  = message_data->media_type  == MessageData::MediaType ::IMAGE ;
        bool is_file   = message_data->media_type  == MessageData::MediaType ::FILE  ;
        bool is_me     = message_data->is_me;
        //
        if (is_image && is_file_exists) {
            QImage img(file_path);
            if (!img.isNull()) {
                QApplication::clipboard()->setImage(img);
            }
        }
    } while(false);
}

// 使用系统默认的应用程序打开文件
void MessageWindow::slot_open_file(std::shared_ptr<MessageData::ItemData> message_data) {
    do {
        if (!message_data) { break; }
        if (!message_data->file) { break; }
        if ( message_data->file->file_path.isEmpty()) { break; }
        const QString &file_path = message_data->file->file_path;
        bool is_file_exists = QFileInfo::exists(file_path);
        if (file_path.isEmpty() || !is_file_exists) { break; }
        //
        bool is_system = message_data->sender_type == MessageData::SenderType::SYSTEM;
        bool is_user   = message_data->sender_type == MessageData::SenderType::USER  ;
        bool is_text   = message_data->media_type  == MessageData::MediaType ::TEXT  ;
        bool is_image  = message_data->media_type  == MessageData::MediaType ::IMAGE ;
        bool is_file   = message_data->media_type  == MessageData::MediaType ::FILE  ;
        bool is_me     = message_data->is_me;
        //
        QDesktopServices::openUrl(QUrl::fromLocalFile(file_path)); // 系统默认的打开程序
    } while(false);
}

// 打开文件所在文件夹
// QStringLiteral 类似于 constexpr，tr用于翻译
void MessageWindow::slot_open_folder(std::shared_ptr<MessageData::ItemData> message_data) {
    do {
        if (!message_data) { break; }
        if (!message_data->file) { break; }
        if ( message_data->file->file_path.isEmpty()) { break; }
        const QString &file_path = message_data->file->file_path;
        bool is_file_exists = QFileInfo::exists(file_path);
        if (file_path.isEmpty() || !is_file_exists) { break; }
        //
        bool is_system = message_data->sender_type == MessageData::SenderType::SYSTEM;
        bool is_user   = message_data->sender_type == MessageData::SenderType::USER  ;
        bool is_text   = message_data->media_type  == MessageData::MediaType ::TEXT  ;
        bool is_image  = message_data->media_type  == MessageData::MediaType ::IMAGE ;
        bool is_file   = message_data->media_type  == MessageData::MediaType ::FILE  ;
        bool is_me     = message_data->is_me;
        //
        if (is_file_exists) {
#if defined(Q_OS_WIN)
// #ifdef Q_OS_WIN
            // 资源管理器中定位并选中该文件
            // Windows 下唤起 explorer 并高亮选中文件
            QStringList args;
            args << QStringLiteral("/select,") << QDir::toNativeSeparators(file_path);
            QProcess::startDetached(QStringLiteral("explorer.exe"), args);
#else
            // 其他平台打开文件所在目录
            QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(file_path).absolutePath()));
#endif
        }
    } while(false);
}

// 文件复制到指定目录
void MessageWindow::slot_save_to(std::shared_ptr<MessageData::ItemData> message_data) {
    do {
        if (!message_data) { break; }
        if (!message_data->file) { break; }
        if ( message_data->file->file_path.isEmpty()) { break; }
        const QString &file_path = message_data->file->file_path;
        bool is_file_exists = QFileInfo::exists(file_path);
        if (file_path.isEmpty() || !is_file_exists) { break; }
        //
        bool is_system = message_data->sender_type == MessageData::SenderType::SYSTEM;
        bool is_user   = message_data->sender_type == MessageData::SenderType::USER  ;
        bool is_text   = message_data->media_type  == MessageData::MediaType ::TEXT  ;
        bool is_image  = message_data->media_type  == MessageData::MediaType ::IMAGE ;
        bool is_file   = message_data->media_type  == MessageData::MediaType ::FILE  ;
        bool is_me     = message_data->is_me;
        //
        if (is_file_exists) {
            QString default_name = QFileInfo(file_path).fileName();
            QString save_path = QFileDialog::getSaveFileName(this, tr("Save to..."), default_name);
            if (!save_path.isEmpty()) {
                QFile::copy(file_path, save_path);
            }
        }
    } while(false);
}

// 复制文件到剪切板，可在资源管理器中粘贴
void MessageWindow::slot_copy_file(std::shared_ptr<MessageData::ItemData> message_data) {
    do {
        if (!message_data) { break; }
        if (!message_data->file) { break; }
        if ( message_data->file->file_path.isEmpty()) { break; }
        const QString &file_path = message_data->file->file_path;
        bool is_file_exists = QFileInfo::exists(file_path);
        if (file_path.isEmpty() || !is_file_exists) { break; }
        //
        bool is_system = message_data->sender_type == MessageData::SenderType::SYSTEM;
        bool is_user   = message_data->sender_type == MessageData::SenderType::USER  ;
        bool is_text   = message_data->media_type  == MessageData::MediaType ::TEXT  ;
        bool is_image  = message_data->media_type  == MessageData::MediaType ::IMAGE ;
        bool is_file   = message_data->media_type  == MessageData::MediaType ::FILE  ;
        bool is_me     = message_data->is_me;
        //
        if (is_file_exists) {
            // 标准 Qt 复制文件到剪贴板，可在资源管理器/桌面中 Ctrl+V 粘贴
            auto *mimeData = new QMimeData();
            mimeData->setUrls({ QUrl::fromLocalFile(file_path) });
            QApplication::clipboard()->setMimeData(mimeData);
        }
    } while(false);
}

// 复制文件路径到剪切板
void MessageWindow::slot_copy_path(std::shared_ptr<MessageData::ItemData> message_data) {
    do {
        if (!message_data) { break; }
        if (!message_data->file) { break; }
        if ( message_data->file->file_path.isEmpty()) { break; }
        const QString &file_path = message_data->file->file_path;
        bool is_file_exists = QFileInfo::exists(file_path);
        if (file_path.isEmpty() || !is_file_exists) { break; }
        //
        bool is_system = message_data->sender_type == MessageData::SenderType::SYSTEM;
        bool is_user   = message_data->sender_type == MessageData::SenderType::USER  ;
        bool is_text   = message_data->media_type  == MessageData::MediaType ::TEXT  ;
        bool is_image  = message_data->media_type  == MessageData::MediaType ::IMAGE ;
        bool is_file   = message_data->media_type  == MessageData::MediaType ::FILE  ;
        bool is_me     = message_data->is_me;
        //
        if (!file_path.isEmpty()) {
            QApplication::clipboard()->setText(QDir::toNativeSeparators(file_path));
        }
    } while(false);
}

// to_contact_data 为不定账号，可能不是当前账号，必须校验账号 ID
void MessageWindow::slot_update_contact_info(std::shared_ptr<ContactData::ItemData> to_contact_data) {
    do {
        if (!to_contact_data || to_contact_data->account_id.isEmpty()) { break; }
        auto from_contact_data = get_contact();
        auto from_account_id   = from_contact_data ? from_contact_data->account_id : "";
        auto to_account_id     = to_contact_data->account_id;
        auto to_avatar_id      = to_contact_data->avatar_id;
        if (from_account_id != to_account_id) { break; }
        // 设置  // 传入副本，防止意外修改原先的内容
        auto update_contact_info = std::make_shared<ContactData::ItemData>(*to_contact_data);
        set_contact(update_contact_info);
    } while (false);
}

// // 消息较多时应该差异化更新
// void MessageWindow::update_messages(Messages messages) {
//     m_messages = messages;
// }

// void MessageWindow::append_message (Message message ) {
//     //
//     ui->list_messages->scrollToBottom(); // 滚动到最后
// }

