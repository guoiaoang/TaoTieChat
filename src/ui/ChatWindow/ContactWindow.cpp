#include "ContactWindow.h"
#include "ui_ContactWindow.h"

#include "NetworkClient.h"
#include "ChatWindow.h"
#include "Database.h"
#include "MediaWindow.h"
#include "Accounts.h"
#include "Database.h"

ContactWindow::ContactWindow(QWidget *parent) : QWidget(parent), m_ui(std::make_shared<Ui::ContactWindow>()) {
    m_ui->setupUi(this);
    // new
    // 防止未初始化
    if (!m_contact_data ) { m_contact_data = std::make_shared<ContactData::ItemData>(); }
    if (!m_menu_manager ) { m_menu_manager = std::make_shared<QMenu  >(); }
    if (!m_action_edit  ) { m_action_edit  = std::make_shared<QAction>(); }
    if (!m_action_remove) { m_action_remove= std::make_shared<QAction>(); }

    // connect
    connect(m_ui->button_manager , &QToolButton::clicked  , this, &ContactWindow::slot_button_manager );
    connect(m_ui->button_avatar  , &QToolButton::clicked  , this, &ContactWindow::slot_button_avatar  );
    connect(m_ui->button_modify  , &QToolButton::clicked  , this, &ContactWindow::slot_button_save    );
    connect(m_ui->button_cancel  , &QToolButton::clicked  , this, &ContactWindow::slot_button_cancel  );
    connect(m_ui->button_add     , &QToolButton::clicked  , this, &ContactWindow::slot_button_add     );
    connect(m_ui->button_accept  , &QToolButton::clicked  , this, &ContactWindow::slot_button_accept  );
    connect(m_ui->button_ignore  , &QToolButton::clicked  , this, &ContactWindow::slot_button_ignore  );
    connect(m_ui->button_messages, &QToolButton::clicked  , this, &ContactWindow::slot_button_messages);
    connect(m_ui->button_voice   , &QToolButton::clicked  , this, &ContactWindow::slot_button_voice   );
    connect(m_ui->button_video   , &QToolButton::clicked  , this, &ContactWindow::slot_button_video   );
    connect(m_action_edit  .get(), &QAction    ::triggered, this, &ContactWindow::slot_action_edit    );
    connect(m_action_remove.get(), &QAction    ::triggered, this, &ContactWindow::slot_action_remove  );
    // 新添加好友时刷新好友关系状态
    connect(&Accounts::instance(), &Accounts::signal_contact_list_changed, this, &ContactWindow::slot_contact_status_changed);

    // set
    // 设置动态头像大小，与右侧内容高度一致
    QSize size_right = m_ui->verticalLayout->sizeHint();
    int avatar_pixmap_size = qMin(size_right.width(), size_right.height());
    m_ui->button_avatar->setFixedSize      (avatar_pixmap_size, avatar_pixmap_size);
    // m_ui->button_avatar->setFixedSize   (64, 64);
    m_ui->button_avatar->setText           ("");  // 清空文字，防止意外显示
    m_ui->button_avatar->setWindowIconText ("");  // 清空文字，防止意外显示
    m_ui->button_avatar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_ui->button_avatar->setIconSize       (QSize(m_ui->button_avatar->size()));
    m_action_edit  ->setText  (tr("edit"  ));
    m_action_remove->setText  (tr("remove"));
    m_menu_manager ->addAction(m_action_edit  .get());
    m_menu_manager ->addAction(m_action_remove.get());

    // 添加样式
    m_edit_tool.add_long_text_tip_button(m_ui->edit_description );
    m_edit_tool.add_long_text_tip_button(m_ui->edit_emails      );
    m_edit_tool.add_long_text_tip_button(m_ui->edit_nickname_big);
    m_edit_tool.add_long_text_tip_button(m_ui->edit_nickname    );
    m_edit_tool.add_long_text_tip_button(m_ui->edit_notes       );
    m_edit_tool.add_long_text_tip_button(m_ui->edit_phones      );
    m_edit_tool.add_long_text_tip_button(m_ui->edit_region      );
    m_edit_tool.add_long_text_tip_button(m_ui->edit_signature   );
    m_edit_tool.add_long_text_tip_button(m_ui->edit_tags        );
    m_edit_tool.add_long_text_tip_button(m_ui->edit_userid      );
}

ContactWindow::~ContactWindow() {
}

const std::shared_ptr<ContactData::ItemData>&ContactWindow::get_contact_data() const {
    return m_contact_data;
}

// TODO：添加仅修改头像时导致的页面信息修改 (更新contact_data->avatar_id)
void ContactWindow::set_contact_data(const std::shared_ptr<ContactData::ItemData>&contact_data, const SourceFlag&source_flag) {
    auto set_exit_text = [](QLineEdit*edit, const QString&text) {
        if (!edit) { return; }
        edit->setText(text);
        edit->setToolTip(text);
        edit->setCursorPosition(0); // 光标回到文本开头
    };

    if (!contact_data) { return; }

    // 缓存信息，用于取消编辑时恢复显示
    // if (m_contact_data == contact_data) { return; }
    if (m_contact_data != contact_data) {
        // 先清除
        m_contact_data.reset();
        // 复制
        m_contact_data = std::make_shared<ContactData::ItemData>(*contact_data);
    }

    // 文本信息
    set_exit_text(m_ui->edit_nickname_big, contact_data->display_name         );
    set_exit_text(m_ui->edit_nickname    , contact_data->nickname             );
    set_exit_text(m_ui->edit_userid      , contact_data->user_id              );
    set_exit_text(m_ui->edit_region      , contact_data->region               );
    set_exit_text(m_ui->edit_notes       , contact_data->notes                );
    set_exit_text(m_ui->edit_phones      , contact_data->phones     .join(";"));
    set_exit_text(m_ui->edit_emails      , contact_data->emails     .join(";"));
    set_exit_text(m_ui->edit_tags        , contact_data->tags       .join(";"));
    set_exit_text(m_ui->edit_description , contact_data->description          );
    set_exit_text(m_ui->edit_signature   , contact_data->signature            );

    if (!contact_data->avatar_image.isNull()) {
        const auto&avatar_pixamp = QPixmap::fromImage(contact_data->avatar_image)
            .scaled(m_ui->button_avatar->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_ui->button_avatar->setIcon(QIcon(avatar_pixamp));
    }
    else {
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
            const auto& contact_data = get_contact_data();
            // 下载成功
            if (notify.ok &&
                notify.code    == int64_t(SuccessCode::SUCCESS) &&
                notify.status  == FileStatusCode::FILE_STATUS_UPLOADED &&
                !notify.uploading &&
                contact_data &&  // 与缓存比较 // 缓存不能为空
                contact_data->avatar_id == notify.id
            ) {
                // 头像下载成功时仅需修改头像相关的信息
                contact_data->avatar_path  = Database::instance().get_local_file_id_path(contact_data->avatar_id);
                contact_data->avatar_image = QImage(contact_data->avatar_path);
                if (!contact_data->avatar_image.isNull()) {
                    const auto&avatar_pixamp = QPixmap::fromImage(contact_data->avatar_image)
                        .scaled(m_ui->button_avatar->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    m_ui->button_avatar->setIcon(QIcon(avatar_pixamp));
                }
            }
        };
        QString notify_id = QString("notify ContactWindow::set_contact_data, account_id:%1, avatar_id:%2")
            .arg(contact_data->account_id).arg(contact_data->avatar_id);
        NetworkClient::instance().download_file(contact_data->avatar_id, notify_id, callback_download_file_notify);
    }

    ViewFlag  view_flag   = ViewFlag::View;
    ShipFlags ship_flags;
    if ( contact_data->is_friend                        ) { ship_flags |= ShipFlag::Friend  ; }
    if (                             contact_data->is_me) { ship_flags |= ShipFlag::Self    ; }
    if (!contact_data->is_friend && !contact_data->is_me) { ship_flags |= ShipFlag::Stranger; }
    updata_show(source_flag, view_flag, ship_flags);
}

void ContactWindow::updata_show(const SourceFlag&source_flag, const ViewFlag&view_flag, const ShipFlags&ship_flags) {
    // NOTES:服务端逻辑为一次只允许修改自己或好友信息，不可同时修改
    //       因此，is_profile 时才允许修改个人资料

    auto set_widget_visiable = [](QWidget*widget, bool visiable) {
        widget->setDisabled(!visiable);
        widget->setVisible ( visiable);
    };
    auto set_edit_editable = [](QLineEdit*edit, bool editable) {
        const QString style_sheet_view = "QLineEdit { background-color: transparent; border: none; }";
        edit->setReadOnly(!editable);
        edit->setStyleSheet(editable ? QString() : style_sheet_view);
    };
    auto set_action_enable = [](std::shared_ptr<QAction>& action, bool enable) {
        action->setDisabled(!enable);
    };

    // 存储
    m_source_flag = source_flag;
    m_view_flag   = view_flag  ;
    m_ship_flags  = ship_flags ;

    // 获取标志
    // auto source_flag = get_source_flag();
    // auto view_flag   = get_view_flag  ();
    // auto ship_flags  = get_ship_flags ();
    //
    bool is_profile  = source_flag == SourceFlag::Profile ;
    bool is_search   = source_flag == SourceFlag::Search  ;
    bool is_request  = source_flag == SourceFlag::Request ;
    bool is_contact  = source_flag == SourceFlag::Contact ;
    //
    bool is_view     = view_flag == ViewFlag::View;
    bool is_viewonly = view_flag == ViewFlag::ViewOnly;
    bool is_edit     = view_flag == ViewFlag::Edit    ;
    //
    bool is_stranger = ship_flags.testFlag(ShipFlag::Stranger);
    bool is_friend   = ship_flags.testFlag(ShipFlag::Friend  );
    bool is_self     = ship_flags.testFlag(ShipFlag::Self    );

    // label_notes/label_description/label_tags,edit_notes/edit_description/edit_tags
    // 非自己页面的好友时显示(自己可以添加自己为好友)
    // bool widget_contact_visiable = is_friend && !is_profile;
    bool widget_contact_visiable = is_friend && !is_profile && !is_search;  // 搜索页面暂未适配同时显示好友资料
    set_widget_visiable(m_ui->label_notes      , widget_contact_visiable);
    set_widget_visiable(m_ui->label_description, widget_contact_visiable);
    set_widget_visiable(m_ui->label_tags       , widget_contact_visiable);
    set_widget_visiable(m_ui->edit_notes       , widget_contact_visiable);
    set_widget_visiable(m_ui->edit_description , widget_contact_visiable);
    set_widget_visiable(m_ui->edit_tags        , widget_contact_visiable);

    // edit_nickname/edit_userid/edit_region/edit_signature/edit_phones/edit_emails
    // 自己页面才允许编辑基础资料
    bool edit_self_editable = is_edit && is_self && is_profile;
    set_edit_editable(m_ui->edit_nickname , edit_self_editable);
    set_edit_editable(m_ui->edit_userid   , edit_self_editable);
    set_edit_editable(m_ui->edit_region   , edit_self_editable);
    set_edit_editable(m_ui->edit_signature, edit_self_editable);
    set_edit_editable(m_ui->edit_phones   , edit_self_editable);
    set_edit_editable(m_ui->edit_emails   , edit_self_editable);
    // edit_notes/edit_description/edit_tags
    // 非自己页面的好友时才允许编辑好友信息(自己可以添加自己为好友)
    bool edit_contact_editable = is_edit && is_friend && !is_profile;
    set_edit_editable(m_ui->edit_notes      , edit_contact_editable);
    set_edit_editable(m_ui->edit_description, edit_contact_editable);
    set_edit_editable(m_ui->edit_tags       , edit_contact_editable);

    // button_add，添加好友按钮
    // 非编辑模式，非个人页面，非好友关系  // TODO:对方发送好友申请后应当显示为通过
    bool button_add_visiable    = !is_edit && !is_profile && !is_friend;
    set_widget_visiable(m_ui->button_add, button_add_visiable);
    // button_accept/button_ignore，接受/忽略好友申请按钮
    // TODO:非编辑模式，非个人页面，非好友关系，对方发送好友申请后显示
    bool button_accept_visiable = !is_edit && !is_profile && !is_friend &&  is_request;
    set_widget_visiable(m_ui->button_accept, button_accept_visiable);
    set_widget_visiable(m_ui->button_ignore, button_accept_visiable);
    // button_modify/button_cancel，修改/取消修改个人(自己/好友)资料按钮
    // 编辑模式，自己或好友资料时显示
    bool button_save_visiable = is_edit && (is_self || is_friend);
    set_widget_visiable(m_ui->button_modify  , button_save_visiable);
    set_widget_visiable(m_ui->button_cancel  , button_save_visiable);
    // button_messages/button_voice/button_video，发送文本消息/语音通话/视频通话按钮
    // 非编辑模式，非个人页面，好友时显示
    bool button_message_visiable = !is_edit && !is_profile &&  is_friend;
    set_widget_visiable(m_ui->button_messages, button_message_visiable);
    set_widget_visiable(m_ui->button_voice   , button_message_visiable);
    set_widget_visiable(m_ui->button_video   , button_message_visiable);

    // m_action_edit/m_action_remove，编辑资料/删除好友按钮
    // 非编辑模式，是自己页面或有好友关系时
    // bool action_edit_enable = !is_edit && (is_profile || is_friend);
    bool action_edit_enable = !is_edit && (is_profile || is_friend) && !is_search;  // 搜索页面暂未适配同时显示好友资料
    set_action_enable(m_action_edit, action_edit_enable);
    // 非编辑模式，非个人页面，是好友时才允许删除。自己为好友时在查看自身资料页面不显示该动作。
    // bool action_remove_enable = !is_edit && !is_profile && is_friend;
    bool action_remove_enable = !is_edit && !is_profile && is_friend && !is_search;  // 搜索页面暂未适配同时显示好友资料
    set_action_enable(m_action_remove, action_remove_enable);

    // 设置窗口标题
    ;    if (is_request) { setWindowTitle(tr("Contact request")); }
    else if (is_search ) { setWindowTitle(tr("Search result"  )); }
    // else if (is_profile) { setWindowTitle(tr("Profile"     )); }
    else                 { setWindowTitle(tr("Profile"        )); }

    if (is_viewonly) {
        set_action_enable(m_action_edit  , false);
        set_action_enable(m_action_remove, false);
    }

    // 暂时禁用修改用户 ID
    set_edit_editable(m_ui->edit_userid, false);

    // TODO:分类错误，暂时禁用
    set_widget_visiable(m_ui->label_description, false);
    set_widget_visiable(m_ui->edit_description , false);

    // TODO:未添加的功能
    set_widget_visiable(m_ui->button_gender , false);
    set_widget_visiable(m_ui->button_starred, false);
}

void ContactWindow::slot_button_manager() {
    // 在按钮下方左侧弹出  // 右下角往左下角弹窗
    m_menu_manager->adjustSize(); // 内存调整为实际大小
    QPoint pos = m_ui->button_manager->mapToGlobal(m_ui->button_manager->rect().bottomRight());
    int width = m_menu_manager->size().width();
    m_menu_manager->popup(QPoint(pos.x() - width, pos.y()));
}

void ContactWindow::slot_button_avatar() {
    const auto& contact_data = get_contact_data();
    if (!contact_data) { return; }

    const auto&is_edit = get_view_flag() == ViewFlag::Edit;

    // 编辑模式且为自己时，编辑头像
    if (is_edit && contact_data->is_me) {
        // 个人资料页面的编辑模式允许点击头像进行个人资料中头像的修改
        QString avatar_file_path = dialog_select_avatar_file();
        if (avatar_file_path.isEmpty()) { return; }
        update_profile_avatar(avatar_file_path);
    }
    // 否则非编辑模式，查看头像
    else {
        view_avatar();
    }
}

void ContactWindow::slot_button_save() {
    using namespace Utils::Wrapper;

    set_view_flag(ViewFlag::View);

    // 不要修改原先的内容，使用副本进行操作，以便失败时还原
    const auto& contact_data = get_contact_data();
    if (!contact_data) { return; }
    auto copy_contact_data = std::make_shared<ContactData::ItemData>(*contact_data);

    // 获取标志
    auto source_flag = get_source_flag();
    auto view_flag   = get_view_flag  ();
    auto ship_flags  = get_ship_flags ();
    //
    bool is_profile  = source_flag == SourceFlag::Profile ;
    bool is_search   = source_flag == SourceFlag::Search  ;
    bool is_request  = source_flag == SourceFlag::Request ;
    bool is_contact  = source_flag == SourceFlag::Contact ;
    //
    bool is_view     = view_flag == ViewFlag::View;
    bool is_viewonly = view_flag == ViewFlag::ViewOnly;
    bool is_edit     = view_flag == ViewFlag::Edit    ;
    //
    bool is_stranger = ship_flags.testFlag(ShipFlag::Stranger);
    bool is_friend   = ship_flags.testFlag(ShipFlag::Friend  );
    bool is_self     = ship_flags.testFlag(ShipFlag::Self    );

    // 既不是自己也不是好友时，不允许修改资料
    if (!is_self && !is_contact) { return; }

    // 服务器端限制一次只允许修改自己或好友的资料

    // 从个人页面查看自己资料时才允许修改自己的资料
    if (is_self && is_profile) {
        copy_contact_data->nickname   = m_ui->edit_nickname ->text();
        // copy_contact_data->user_id = m_ui->edit_userid   ->text();  // 暂不允许修改
        copy_contact_data->region     = m_ui->edit_region   ->text();
        copy_contact_data->signature  = m_ui->edit_signature->text();
        copy_contact_data->phones     = m_ui->edit_phones   ->text().split(";", Qt::SkipEmptyParts);
        copy_contact_data->emails     = m_ui->edit_emails   ->text().split(";", Qt::SkipEmptyParts);
    }
    // ~~自己可以是自己的好友~~，不允许同时修改个人资料与好友资料
    else if (is_contact) {
        copy_contact_data->notes       = m_ui->edit_notes      ->text();
        // copy_contact_data->description = m_ui->edit_description->text();  // 分类错误，暂时禁用
        copy_contact_data->tags        = m_ui->edit_tags       ->text().split(";", Qt::SkipEmptyParts);
    }

    update_profile_text(copy_contact_data);
}

void ContactWindow::slot_button_cancel() {
    set_view_flag(ViewFlag::View);
    set_contact_data(get_contact_data(), get_source_flag()); // 重置显示的内容，防止页面对内容进行了修改
}

void ContactWindow::slot_button_add() {
    // TODO:添加好友请求支持附带留言

    const auto& contact_data = get_contact_data();
    if (!contact_data) { return; }

    emit sig_contact_add(contact_data);

    contact_request_params params_contact_request = std::make_shared<ContactRequestRequest>();
    params_contact_request->setRequestUserAuth(*Accounts::instance().get_auth_request().get());
    params_contact_request->setTargetUserId(contact_data->user_id);  // TODO: 改为添加账号 ID
    auto callback_contact_request = [this](const contact_request_params&params_contact_request, const contact_request_reply&reply_contact_request) {
        // 消息发送失败
        if (!reply_contact_request || !reply_contact_request->hasResult() || !reply_contact_request->result().hasSuccessCode()) {
            QString error_message = tr("Failed to send contact request.");
            if (reply_contact_request && reply_contact_request->hasResult()) {
                error_message += tr("\n\ncode : 0x%1\nmessage : %2")
                                     .arg(int64_t(reply_contact_request->result().errorCode()), 8, 16, QChar('0'))  // 16 进制显示，宽度 8 位，不足补 0
                                     .arg(reply_contact_request->result().message());
            }
            QMessageBox::information(this, tr("Error"), error_message, QMessageBox::StandardButton::Ok);
            return;
        }
        // 提示消息发送成功
        QMessageBox::information(this, tr("Success"), tr("Send contact request success."), QMessageBox::StandardButton::Ok);
    };
    NetworkClient::instance().contact_request(params_contact_request, callback_contact_request);
}

void ContactWindow::slot_button_accept() {
    const auto& contact_data = get_contact_data();
    if (!contact_data) { return; }
    contact_reply_params params_contact_reply = std::make_shared<ContactReplyRequest>();
    params_contact_reply->setRequestUserAuth(*Accounts::instance().get_auth_request().get());
    params_contact_reply->setTargetUserId(contact_data->user_id);  // TODO: 改为添加账号 ID
    params_contact_reply->setReply(ContactReplyCode::CONTACT_REPLY_ACCEPT);
    auto callback_contact_reply = [this](const contact_reply_params&params_contact_reply, const contact_reply_reply&reply_contact_reply) {
        // 消息发送失败
        if (!reply_contact_reply || !reply_contact_reply->hasResult() || !reply_contact_reply->result().hasSuccessCode()) {
            QString error_message = tr("Failed to send contact reply.");
            if (reply_contact_reply && reply_contact_reply->hasResult()) {
                error_message += tr("\n\ncode : 0x%1\nmessage : %2")
                                     .arg(int64_t(reply_contact_reply->result().errorCode()), 8, 16, QChar('0'))  // 16 进制显示，宽度 8 位，不足补 0
                                     .arg(reply_contact_reply->result().message());
            }
            QMessageBox::information(this, tr("Error"), error_message, QMessageBox::StandardButton::Ok);
            return;
        }
        // 提示消息发送成功  // 发送成功不必提示
        // QMessageBox::information(this, tr("Success"), tr("Send contact request success."), QMessageBox::StandardButton::Ok);
    };
    NetworkClient::instance().contact_reply(params_contact_reply, callback_contact_reply);
}

void ContactWindow::slot_button_ignore() {
    const auto& contact_data = get_contact_data();
    contact_reply_params params_contact_reply = std::make_shared<ContactReplyRequest>();
    params_contact_reply->setRequestUserAuth(*Accounts::instance().get_auth_request().get());
    params_contact_reply->setTargetUserId(contact_data->user_id);  // TODO: 改为添加账号 ID
    params_contact_reply->setReply(ContactReplyCode::CONTACT_REPLY_IGNORE);
    auto callback_contact_reply = [this](const contact_reply_params&params_contact_reply, const contact_reply_reply&reply_contact_reply) {
        // 消息发送失败
        if (!reply_contact_reply || !reply_contact_reply->hasResult() || !reply_contact_reply->result().hasSuccessCode()) {
            QString error_message = tr("Failed to send contact reply.");
            if (reply_contact_reply && reply_contact_reply->hasResult()) {
                error_message += tr("\n\ncode : 0x%1\nmessage : %2")
                                     .arg(int64_t(reply_contact_reply->result().errorCode()), 8, 16, QChar('0'))  // 16 进制显示，宽度 8 位，不足补 0
                                     .arg(reply_contact_reply->result().message());
            }
            QMessageBox::information(this, tr("Error"), error_message, QMessageBox::StandardButton::Ok);
            return;
        }
        // 提示消息发送成功  // 发送成功不必提示
        // QMessageBox::information(this, tr("Success"), tr("Send contact request success."), QMessageBox::StandardButton::Ok);
    };
    NetworkClient::instance().contact_reply(params_contact_reply, callback_contact_reply);
}

void ContactWindow::slot_button_messages() {
    const auto& contact_data = get_contact_data();
    emit sig_contact_message(contact_data);
}

void ContactWindow::slot_button_voice() {
    // TODO: 发起语音通话
    const auto& contact_data = get_contact_data();
    emit sig_contact_voice(contact_data);
}

void ContactWindow::slot_button_video() {
    // TODO: 发起视频通话
    const auto& contact_data = get_contact_data();
    emit sig_contact_video(contact_data);
}

void ContactWindow::slot_action_edit() {
    set_view_flag(ViewFlag::Edit);
}

void ContactWindow::slot_action_remove() {
    QMessageBox message_box;
    message_box.setWindowTitle(tr("Remove Contact"));
    message_box.setText(tr("Remove Contact"));
    message_box.setInformativeText(tr("Are you sure?"));
    QPushButton*button_remove = message_box.addButton(tr("remove"), QMessageBox::ApplyRole);
    QPushButton*button_cancel = message_box.addButton(tr("cancel"), QMessageBox::RejectRole);
    message_box.setDefaultButton(button_cancel);
    message_box.exec();
    QAbstractButton *button_clicked = message_box.clickedButton();
    if (dynamic_cast<QPushButton*>(button_clicked) == button_remove) {
        const auto& contact_data = get_contact_data();
        emit sig_contact_remove(contact_data);
    }
}

void ContactWindow::slot_contact_status_changed(const QVector<std::shared_ptr<ContactData::ItemData>>& to_contact_datas) {
    for (auto&contact_data : to_contact_datas) {
        if (contact_data->account_id.isEmpty()) { continue; }
        const auto& account_id = contact_data->account_id;
        if (account_id != m_contact_data->account_id) { continue; }
        // image??
        set_contact_data(contact_data, get_source_flag());
        break;
    }
}

// 在弹窗中查看头像
void ContactWindow::view_avatar() {
    const auto& contact_data = get_contact_data();
    // 获取显示内容
    std::vector<std::shared_ptr<MediaInfo>> play_list = {
        std::make_shared<MediaInfo>(MediaInfo{
            .media_path  = contact_data->avatar_path,
            .media_image = contact_data->avatar_image,  // 使用原始的图片
            .media_name  = contact_data->nickname
        })
    };
    MediaWindow*media_window = &MediaWindow::instance();
    media_window->set_play_list(play_list);
    Utils::widget_raise(media_window);
}

// 弹窗选择新的头像图片
QString ContactWindow::dialog_select_avatar_file() {
    QString avatar_file_path;
    QString pictures_location = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    // 获取 Qt 支持的所有图片后缀
    QList<QByteArray> formats = QImageReader::supportedImageFormats();
    QString allExtensions = tr("Image Files (*.png;*.jpg;*.bmp)");   // 默认的图片格式过滤器
    allExtensions.replace("(*.png;*.jpg;*.bmp)", "(*." + formats.join(";*.") + ")");  // 输出支持的图片格式列表

    QFileDialog file_dialog(this, tr("Select Avatar"), pictures_location, allExtensions);
    file_dialog.setFileMode(QFileDialog::ExistingFile);
    if (file_dialog.exec() == QDialog::Accepted) {
        avatar_file_path = file_dialog.selectedFiles().first();
    }
    return avatar_file_path;
}

// 更新个人资料中的文本信息
void ContactWindow::update_profile_text(const std::shared_ptr<ContactData::ItemData>&contact_data, const update_profile_callback&callback_update_profile) {
    // qDebug().noquote().nospace() << "Current account contactInfo: " << Utils::Transform::to_string(Accounts::instance().get_current_account());

    // 联系人信息参数
    const auto& contact_info = Utils::Transform::to_contact_info(contact_data);
    // 身份验证参数，必填
    const auto& auth_request = Accounts::instance().get_auth_request();

    update_profile_params params = std::make_shared<TaoTieChat::UpdateProfileRequest>();
    params->setRequestUserAuth(*auth_request.get());
    params->setContactInfo    (*contact_info.get());

    auto update_avatar_text_callback = [this](const update_profile_params&params_update_profile, const update_profile_reply&reply_update_profile) {
        // qDebug().noquote().nospace() << "Current account contactInfo: " << Utils::Transform::to_string(Accounts::instance().get_current_account());
        if (!reply_update_profile || !reply_update_profile->result().hasSuccessCode() || !reply_update_profile->request().hasContactInfo()) {
            QString error_message = tr("Failed to update profile.");
            if (reply_update_profile && reply_update_profile->result().hasErrorCode()) {
                error_message += tr("\n\ncode : 0x%1\nmessage : %2")
                    .arg(int64_t(reply_update_profile->result().errorCode()), 8, 16, QChar('0'))  // 16 进制显示，宽度 8 位，不足补 0
                    .arg(reply_update_profile->result().message());
            }
            QMessageBox::information(this, tr("Error"), error_message, QMessageBox::StandardButton::Ok);
            return;
        }

        std::shared_ptr<ContactInfo> contact_info = std::make_shared<ContactInfo>(reply_update_profile->request().contactInfo());
        auto contact_data = Utils::Transform::to_contact_data(contact_info);
        set_contact_data(contact_data, get_source_flag());

        // 发送更新通知
        if (contact_data->is_me) {
            Accounts::instance().set_current_account(contact_data);
        }
    };
    NetworkClient::instance().update_profile(params, callback_update_profile);
}

// 更新个人资料中的头像，上传头像文件并更新服务器上的头像 ID，最后刷新显示
void ContactWindow::update_profile_avatar(const QString&avatar_path) {
    // 上传头像
    network_file_io_notify_callback callback_upload_file_notify = [this, avatar_path](const NetworkFileIONotify&notify) {
        // 忽略上传进度通知
        if (notify.ok && notify.code == int64_t(SuccessCode::SUCCESS) && notify.uploading) {
            return;
        }
        if (!notify.ok || notify.status != FileStatusCode::FILE_STATUS_UPLOADED) {
            // 16 进制显示，宽度 8 位，不足补 0
            QString error_message = tr("Failed to upload avatar.\n\ncode : 0x%1\nmessage : %2").arg(notify.code, 8, 16, QChar('0')).arg(notify.message);
            QMessageBox::information(this, tr("Error"), error_message, QMessageBox::StandardButton::Ok);
            return;
        }
        const auto& contact_data = get_contact_data();
        // 上传成功
        if (notify.ok &&
            notify.code    == int64_t(SuccessCode::SUCCESS) &&
            notify.status  == FileStatusCode::FILE_STATUS_UPLOADED &&
            !notify.uploading &&
            contact_data &&  // 与缓存比较 // 缓存不能为空
            contact_data->avatar_id == notify.id
        ) {
            // // 这里是临时的内容，仅用来更新当前界面中的头像显示
            // auto copy_contact_data = std::make_shared<ContactData::ItemData>(*contact_data);
            // copy_contact_data->avatar_id    = notify.id;
            // copy_contact_data->avatar_path  = avatar_path;
            // copy_contact_data->avatar_image = QImage(avatar_path);
            // // 可能用户编辑了其他资料，此时重置不合理
            // // update_profile_text(copy_contact_data);
            // update_profile_avatar_only(copy_contact_data);

            contact_data->avatar_id    = notify.id;
            contact_data->avatar_path  = avatar_path;
            contact_data->avatar_image = QImage(avatar_path);
            if (!contact_data->avatar_image.isNull()) {
                const auto&avatar_pixamp = QPixmap::fromImage(contact_data->avatar_image)
                    .scaled(m_ui->button_avatar->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                m_ui->button_avatar->setIcon(QIcon(avatar_pixamp));
            }

        }
    };
    NetworkClient::instance().upload_file(avatar_path, callback_upload_file_notify);
}

// // 用户仅修改头像时不应该完全重置资料
// void ContactWindow::update_profile_avatar_only(const std::shared_ptr<ContactData::ItemData>&avatar_data) {
//     if (!avatar_data) { return; }
//     const auto& contact_data = get_contact_data();
//     if (!contact_data) { return; }
//     contact_data->avatar_id    = avatar_data->avatar_id;
//     contact_data->avatar_path  = avatar_data->avatar_path;
//     contact_data->avatar_image = avatar_data->avatar_image;
// 
//     if (!contact_data->avatar_image.isNull()) {
//         const auto&avatar_pixamp = QPixmap::fromImage(contact_data->avatar_image)
//             .scaled(m_ui->button_avatar->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
//         m_ui->button_avatar->setIcon(QIcon(avatar_pixamp));
//     }
// }
