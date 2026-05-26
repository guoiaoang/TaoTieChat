#include "ContactWindow.h"
#include "ui_ContactWindow.h"
#include "NetworkClient.h"
#include "ChatWindow.h"

ContactWindow::ContactWindow(QWidget *parent) : QWidget(parent), m_ui(std::make_shared<Ui::ContactWindow>()) {
    m_ui->setupUi(this);
    // new
    // 防止未初始化
    if(!m_cache_contact_info) { m_cache_contact_info = std::make_shared<ContactInfo>(); }
    if(!m_menu_manager      ) { m_menu_manager       = std::make_shared<QMenu      >(); }
    if(!m_action_edit       ) { m_action_edit        = std::make_shared<QAction    >(); }
    if(!m_action_remove     ) { m_action_remove      = std::make_shared<QAction    >(); }

    // connect
    connect(m_ui->button_manager , &QToolButton::clicked  , this, &ContactWindow::on_button_manager );
    connect(m_ui->button_avatar  , &QToolButton::clicked  , this, &ContactWindow::on_button_avatar  );
    connect(m_ui->button_modify  , &QToolButton::clicked  , this, &ContactWindow::on_button_save    );
    connect(m_ui->button_cancel  , &QToolButton::clicked  , this, &ContactWindow::on_button_cancel  );
    connect(m_ui->button_add     , &QToolButton::clicked  , this, &ContactWindow::on_button_add     );
    connect(m_ui->button_messages, &QToolButton::clicked  , this, &ContactWindow::on_button_messages);
    connect(m_ui->button_voice   , &QToolButton::clicked  , this, &ContactWindow::on_button_voice   );
    connect(m_ui->button_video   , &QToolButton::clicked  , this, &ContactWindow::on_button_video   );
    connect(m_action_edit  .get(), &QAction    ::triggered, this, &ContactWindow::on_action_edit    );
    connect(m_action_remove.get(), &QAction    ::triggered, this, &ContactWindow::on_action_remove  );

    // set
    // 设置动态头像大小，与右侧内容高度一致
    QSize size_right = m_ui->verticalLayout->sizeHint();
    int avatar_pixmap_size = qMin(size_right.width(), size_right.height());
    m_ui->button_avatar->setFixedSize      (avatar_pixmap_size, avatar_pixmap_size);
    // m_ui->button_avatar->setFixedSize      (64, 64);
    m_ui->button_avatar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_ui->button_avatar->setIconSize       (QSize(m_ui->button_avatar->size()));
    m_action_edit  ->setText  (tr("edit"  ));
    m_action_remove->setText  (tr("remove"));
    m_menu_manager ->addAction(m_action_edit  .get());
    m_menu_manager ->addAction(m_action_remove.get());

    //
    Database::instance().set_config_dir(Utils::user_home_path());
}

ContactWindow::~ContactWindow() {
    if(m_menu_manager      ) { m_menu_manager       .reset(); }
    if(m_action_edit       ) { m_action_edit        .reset(); }
    if(m_action_remove     ) { m_action_remove      .reset(); }
    if(m_cache_contact_info) { m_cache_contact_info .reset(); }
    if(m_ui                ) { m_ui                 .reset(); }
}

const std::shared_ptr<ContactInfo>&ContactWindow::get_cache_contact_info() const {
    // qDebug().noquote().nospace() << "Cache contactInfo: " << Utils::Transform::to_string(m_cache_contact_info);
    return m_cache_contact_info;
}

const std::shared_ptr<QPixmap>&ContactWindow::get_cache_avatar_pixmap() const {
    return m_cache_avatar;
}

void ContactWindow::set_cache_contact_info(const std::shared_ptr<ContactInfo>&contact_info) {
    // qDebug().noquote().nospace() << "To contactInfo: " << Utils::Transform::to_string(contact_info);
    if(m_cache_contact_info == contact_info) { return; }
    m_cache_contact_info = contact_info;
    bool is_self = Accounts::instance().is_self(m_cache_contact_info);
    if(is_self && !m_cache_contact_info->hasSelfInfo()) {
        // 确保自己信息存在，避免后续访问 selfInfo() 时崩溃
        ContactInfoSelf self_info;
        self_info.setModifySelfInfo(true);
        m_cache_contact_info->setSelfInfo(self_info);
    }
}

void ContactWindow::set_cache_avatar_pixmap(const std::shared_ptr<QPixmap    >&avatar_pixmap) {
    m_cache_avatar = avatar_pixmap;
}

// 更新显示当前页面中联系人的信息
void ContactWindow::show_contact(const std::shared_ptr<ContactInfo>&contact_info, const std::shared_ptr<QPixmap>&avatar_pixmap) {
    // 缓存信息，用于取消编辑时回复显示
    set_cache_contact_info(contact_info);

    // 更新显示模式  // 初始化时没有账号信息，会导致显示模式错误
    set_edit_mode(m_edit_mode);

    // 文本信息
    auto base_info   = contact_info->baseInfo();
    auto friend_info = contact_info->hasFriendInfo() ? contact_info->friendInfo() : ContactInfoFriend{};
    auto self_info   = contact_info->hasSelfInfo  () ? contact_info->selfInfo  () : ContactInfoSelf  {};
    m_ui->edit_nickname_big->setText(base_info  .nickname());
    m_ui->edit_nickname    ->setText(base_info  .nickname());
    m_ui->edit_userid      ->setText(base_info  .userId  ());
    m_ui->edit_region      ->setText(base_info  .region  ());
    m_ui->edit_notes       ->setText(friend_info.notes   ());
    m_ui->edit_phones      ->setText(base_info  .phones  ().join(";"));
    m_ui->edit_emails      ->setText(base_info  .emails  ().join(";"));
    m_ui->edit_tags        ->setText(friend_info.tags    ().join(";"));
    m_ui->edit_desc        ->setText(base_info  .desc    ());
    m_ui->edit_sign        ->setText(base_info  .sign    ());

    // // 设置头像  // 临时显示
    // if(avatar_pixmap) {
    //     m_ui->button_avatar->setIcon(QIcon(avatar_pixmap->scaled(m_ui->button_avatar->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    //     set_cache_avatar_pixmap(avatar_pixmap);
    // }

    // 无论如何都更新头像
    show_avatar_id(base_info.avatarId());
}

// 显示编辑模式，edit=true 编辑模式，edit=false 显示模式
bool ContactWindow::set_edit_mode(bool edit) {
    m_edit_mode = edit;
    // 未设计的功能
    m_ui->button_gender ->hide();
    m_ui->button_starred->hide();
    // 设置默认状态
    m_ui->button_modify ->hide();
    m_ui->button_cancel ->hide();

    // 1. 非联系人不允许编辑
    // 2. 非自己时编辑内容受限较大

    auto& contact_info = get_cache_contact_info();
    if(!contact_info) { return false; }
    bool is_self    = Accounts::instance().is_self   (contact_info);
    bool is_contact = Accounts::instance().is_contact(contact_info);
    // 模式错误，暂时全部显示并禁用
    if(!is_self && !is_contact) {
        m_menu_manager         ->setDisabled(false); m_menu_manager         ->setVisible(true);
        m_action_edit          ->setDisabled(true ); m_action_edit          ->setVisible(true);
        m_action_remove        ->setDisabled(true ); m_action_remove        ->setVisible(true);
        m_ui->label_desc       ->setDisabled(true ); m_ui->label_desc       ->setVisible(true);
        m_ui->label_emails     ->setDisabled(true ); m_ui->label_emails     ->setVisible(true);
        m_ui->label_nickname   ->setDisabled(true ); m_ui->label_nickname   ->setVisible(true);
        m_ui->label_notes      ->setDisabled(true ); m_ui->label_notes      ->setVisible(true);
        m_ui->label_phones     ->setDisabled(true ); m_ui->label_phones     ->setVisible(true);
        m_ui->label_region     ->setDisabled(true ); m_ui->label_region     ->setVisible(true);
        m_ui->label_sign       ->setDisabled(true ); m_ui->label_sign       ->setVisible(true);
        m_ui->label_tags       ->setDisabled(true ); m_ui->label_tags       ->setVisible(true);
        m_ui->label_userid     ->setDisabled(true ); m_ui->label_userid     ->setVisible(true);
        m_ui->edit_desc        ->setDisabled(true ); m_ui->edit_desc        ->setVisible(true);
        m_ui->edit_emails      ->setDisabled(true ); m_ui->edit_emails      ->setVisible(true);
        m_ui->edit_nickname_big->setDisabled(true ); m_ui->edit_nickname_big->setVisible(true);
        m_ui->edit_nickname    ->setDisabled(true ); m_ui->edit_nickname    ->setVisible(true);
        m_ui->edit_notes       ->setDisabled(true ); m_ui->edit_notes       ->setVisible(true);
        m_ui->edit_phones      ->setDisabled(true ); m_ui->edit_phones      ->setVisible(true);
        m_ui->edit_region      ->setDisabled(true ); m_ui->edit_region      ->setVisible(true);
        m_ui->edit_sign        ->setDisabled(true ); m_ui->edit_sign        ->setVisible(true);
        m_ui->edit_tags        ->setDisabled(true ); m_ui->edit_tags        ->setVisible(true);
        m_ui->edit_userid      ->setDisabled(true ); m_ui->edit_userid      ->setVisible(true);
        m_ui->button_add       ->setDisabled(true ); m_ui->button_add       ->setVisible(true);
        m_ui->button_avatar    ->setDisabled(true ); m_ui->button_avatar    ->setVisible(true);
        m_ui->button_cancel    ->setDisabled(true ); m_ui->button_cancel    ->setVisible(true);
        m_ui->button_gender    ->setDisabled(true ); m_ui->button_gender    ->setVisible(true);
        m_ui->button_manager   ->setDisabled(false); m_ui->button_manager   ->setVisible(true);
        m_ui->button_messages  ->setDisabled(true ); m_ui->button_messages  ->setVisible(true);
        m_ui->button_modify    ->setDisabled(true ); m_ui->button_modify    ->setVisible(true);
        m_ui->button_starred   ->setDisabled(true ); m_ui->button_starred   ->setVisible(true);
        m_ui->button_video     ->setDisabled(true ); m_ui->button_video     ->setVisible(true);
        m_ui->button_voice     ->setDisabled(true ); m_ui->button_voice     ->setVisible(true);
        return false;
    }

    auto set_editable = [](QLineEdit*edit_field, bool edit) {
        // 编辑模式下使用默认的 QLineEdit 样式，非编辑模式下使用透明背景和无边框的样式，使其看起来像标签一样
        const QString style_sheet_view    = "QLineEdit { background-color: transparent; border: none; }";
        edit_field->setReadOnly  (!edit);  // 只有在编辑模式下才可编辑
        edit_field->setStyleSheet( edit ? QString() : style_sheet_view);
    };
    auto set_visiable = [](QLabel*label, QLineEdit*edit_field, bool visiable) {
        label     ->setVisible(visiable);
        edit_field->setVisible(visiable);
    };

    // 关联组件
    set_visiable(m_ui->label_notes, m_ui->edit_notes, is_contact);  // 好友显示备注
    set_visiable(m_ui->label_tags , m_ui->edit_tags , is_contact);  // 好友显示标签
    set_visiable(m_ui->label_desc , m_ui->edit_desc , is_contact);  // 好友显示描述

    // 按钮显示
    m_action_edit        ->setVisible(               !edit);  // 编辑模式下隐藏编辑按钮
    m_ui->button_modify  ->setVisible(                edit);  // 编辑模式下显示修改按钮（保存按钮）
    m_ui->button_cancel  ->setVisible(                edit);  // 编辑模式下显示取消按钮
    m_ui->button_add     ->setVisible(!is_contact && !edit);  // 非好友，非编辑模式才显示添加好友按钮
    m_ui->button_messages->setVisible( is_contact && !edit);  // 好友且非编辑模式才显示发送消息按钮
    m_ui->button_voice   ->setVisible( is_contact && !edit);  // 好友且非编辑模式才显示语音通话按钮
    m_ui->button_video   ->setVisible( is_contact && !edit);  // 好友且非编辑模式才显示视频通话按钮
    m_action_remove      ->setVisible( is_contact && !edit);  // 好友且非编辑模式才显示删除好友按钮
    // 输入组件
    set_editable(m_ui->edit_notes   , is_contact && edit);  // 好友才允许修改备注
    set_editable(m_ui->edit_tags    , is_contact && edit);  // 好友才允许修改标签
    set_editable(m_ui->edit_desc    , is_contact && edit);  // 好友才允许修改个人说明
    set_editable(m_ui->edit_nickname, is_self    && edit);  // 自己才允许修改昵称
    set_editable(m_ui->edit_userid  , is_self    && edit);  // 自己才允许修改用户 ID
    set_editable(m_ui->edit_region  , is_self    && edit);  // 自己才允许修改地区
    set_editable(m_ui->edit_phones  , is_self    && edit);  // 自己才允许修改电话
    set_editable(m_ui->edit_emails  , is_self    && edit);  // 自己才允许修改邮箱
    set_editable(m_ui->edit_sign    , is_self    && edit);  // 自己才允许修改签名

    // if(is_self) {
    //     m_ui->edit_notes ->setVisible(false); m_ui->label_notes->setVisible(false);
    //     m_ui->edit_tags  ->setVisible(false); m_ui->label_tags ->setVisible(false);
    //     m_ui->edit_desc  ->setVisible(false); m_ui->label_desc ->setVisible(false);
    // }
    return true;
}

void ContactWindow::on_button_manager() {
    // 在按钮下方左侧弹出  // 右下角往左下角弹窗
    m_menu_manager->adjustSize(); // 内存调整为实际大小
    QPoint pos = m_ui->button_manager->mapToGlobal(m_ui->button_manager->rect().bottomRight());
    int width = m_menu_manager->size().width();
    m_menu_manager->popup(QPoint(pos.x() - width, pos.y()));
}

void ContactWindow::on_button_avatar() {
    // 编辑模式为修改头像
    auto& contact_info = get_cache_contact_info();
    if(!contact_info) { return; }
    // 非自己资料的编辑页面不允许修改头像，点击头像直接查看头像
    if(!m_ui->button_modify->isVisible() || !Accounts::instance().is_self(contact_info)) {
        view_avatar(contact_info);
        return;
    }
    // 个人资料页面的编辑模式允许点击头像进行个人资料中头像的修改
    QString avatar_file_path = dialog_select_avatar_file();
    if(avatar_file_path.isEmpty()) { return; }

    // 先显示头像再更新资料
    show_avatar(std::make_shared<QPixmap>(avatar_file_path));
    update_profile_avatar(avatar_file_path);
}

void ContactWindow::on_button_save() {
    using namespace Utils::Wrapper;
    set_edit_mode(false);

    // 不要修改原先的内容，使用副本进行操作
    auto copy_contact_info = std::make_shared<ContactInfo>(*get_cache_contact_info().get());
    if(!copy_contact_info) { return; }
    bool is_self    = Accounts::instance().is_self   (copy_contact_info);
    bool is_contact = Accounts::instance().is_contact(copy_contact_info);
    // 既不是自己也不是好友
    if(!is_self && !is_contact) { return; }

    // 填充参数

    // ContactInfoFriend 和 ContactInfoSelf 只能填充一种。
    // 修改自己资料时使用 ContactInfoSelf，修改好友资料时使用 ContactInfoFriend。
    // ContactInfoSelf 目前仅作为修改自己资料的标志，无实际字段。

    // 公共字段，自己和好友都需要填充。
    // 修改自己的资料时，base_info 的账号 ID 和用户 ID 也需要填充，
    // 因为服务器需要验证用户身份，确保修改的是自己的资料而不是别人的资料。
    ContactInfoBase base_info = copy_contact_info->baseInfo();
    // base_info.setAccountId(copy_contact_info->baseInfo().accountId());
    base_info.setUserId   (m_ui->edit_userid->text());
    copy_contact_info->setBaseInfo(base_info);

    // 好友特有字段，只有在是好友时才需要填充
    if(is_contact) {
        // friend_info 的备注和标签只有在是好友时才需要填充
        ContactInfoFriend friend_info = copy_contact_info->friendInfo();
        friend_info.setNotes(m_ui->edit_notes->text());
        friend_info.setTags (m_ui->edit_tags ->text().split(";", Qt::SkipEmptyParts));
        copy_contact_info->setFriendInfo(friend_info);
    }
    // 自己特有字段，只有在是自己的资料时才需要填充
    else if(is_self) {
        // 修改自己资料时包括可修改自己的基础资料
        base_info.setNickname(m_ui->edit_nickname->text());
        base_info.setRegion  (m_ui->edit_region  ->text());
        base_info.setDesc    (m_ui->edit_desc    ->text());
        base_info.setSign    (m_ui->edit_sign    ->text());
        base_info.setPhones  (m_ui->edit_phones  ->text().split(";", Qt::SkipEmptyParts));
        base_info.setEmails  (m_ui->edit_emails  ->text().split(";", Qt::SkipEmptyParts));
        copy_contact_info->setBaseInfo(base_info);

        // 修改自己资料时还需要一个标志，告诉服务器这是修改自己的资料，而不是修改别人的资料。
        ContactInfoSelf self_info_proto = copy_contact_info->selfInfo();
        self_info_proto.setModifySelfInfo(true);  // 必须，标识这是修改自己的资料
        copy_contact_info->setSelfInfo(self_info_proto);
    }
    else {
        // 理论上不应该出现修改按钮，但这里做个保险，防止出现修改好友资料的界面但实际上不是好友的情况。
        return;
    }

    // qDebug().noquote().nospace() << "Current account contactInfo: " << Utils::Transform::to_string(Accounts::instance().get_current_account());
    // qDebug().noquote().nospace() << "To      account contactInfo: " << Utils::Transform::to_string(copy_contact_info);

    auto update_avatar_text_callback = [this](const update_profile_params&params_update_profile, const update_profile_reply&reply_update_profile) {
        // qDebug().noquote().nospace() << "Current account contactInfo: " << Utils::Transform::to_string(Accounts::instance().get_current_account());
        if(!reply_update_profile || !reply_update_profile->result().hasSuccessCode()) {
            QString error_message = tr("Failed to update profile.");
            if(reply_update_profile && reply_update_profile->result().hasErrorCode()) {
                error_message += tr("\n\ncode : 0x%1\nmessage : %2")
                    .arg(int64_t(reply_update_profile->result().errorCode()), 8, 16, QChar('0'))  // 16 进制显示，宽度 8 位，不足补 0
                    .arg(reply_update_profile->result().message());
            }
            QMessageBox::information(this, tr("Error"), error_message, QMessageBox::StandardButton::Ok);
            return;
        }

        std::shared_ptr<ContactInfo> contact_info = std::make_shared<ContactInfo>(reply_update_profile->request().contactInfo());
        if(contact_info) { contact_info = std::make_shared<ContactInfo>(params_update_profile->contactInfo()); }

        // qDebug().noquote().nospace() << "Current account contactInfo: " << Utils::Transform::to_string(Accounts::instance().get_current_account());
        // qDebug().noquote().nospace() << "Profile avatar updated successfully";
        // qDebug().noquote().nospace() << "To              contactInfo: " << Utils::Transform::to_string(params_update_profile->contactInfo());

        show_contact(contact_info);
        // set_cache_contact_info(contact_info);

        if(Accounts::instance().is_self(contact_info)) {
            Accounts::instance().set_current_account(contact_info);
        }
    };
    update_profile_text(*copy_contact_info.get(), update_avatar_text_callback);
}

void ContactWindow::on_button_cancel() {
    set_edit_mode(false);  // 关闭编辑模式
    show_contact(get_cache_contact_info()); // 重置显示的内容，防止页面对内容进行了修改
}

void ContactWindow::on_button_add() {
    // TODO: 发送好友请求，等待对方同意后才能添加到好友列表中
    emit contact_add(get_cache_contact_info());
}

void ContactWindow::on_button_messages() {
    emit contact_message(get_cache_contact_info());
}

void ContactWindow::on_button_voice() {
    // TODO: 发起语音通话
    emit contact_voice(get_cache_contact_info());
}

void ContactWindow::on_button_video() {
    // TODO: 发起视频通话
    emit contact_video(get_cache_contact_info());
}

void ContactWindow::on_action_edit() {
    set_edit_mode(true);
}

void ContactWindow::on_action_remove() {
    QMessageBox message_box;
    message_box.setWindowTitle(tr("Remove Contact"));
    message_box.setText(tr("Remove Contact"));
    message_box.setInformativeText(tr("Are you sure?"));
    QPushButton*button_remove = message_box.addButton(tr("remove"), QMessageBox::ApplyRole);
    QPushButton*button_cancel = message_box.addButton(tr("cancel"), QMessageBox::RejectRole);
    message_box.setDefaultButton(button_cancel);
    message_box.exec();
    QAbstractButton *button_clicked = message_box.clickedButton();
    if(dynamic_cast<QPushButton*>(button_clicked) == button_remove) {
        emit contact_remove(get_cache_contact_info());
    }
}

// 根据文件 ID 在本地查找文件并加载为头像图片
std::shared_ptr<QPixmap> ContactWindow::get_local_avatar_id_pixmap(const QString&avatar_id) {
    return std::make_shared<QPixmap>(Database::instance().get_local_file_id_path(avatar_id));
}

// 在弹窗中查看头像
void ContactWindow::view_avatar(const std::shared_ptr<ContactInfo>&contact_info) {
    // 获取显示内容
    std::vector<std::shared_ptr<MediaInfo>> play_list = {
        std::make_shared<MediaInfo>(MediaInfo{
            .media_path   = "",
            .media_pixmap = *get_cache_avatar_pixmap().get(),
            .media_name   = get_cache_contact_info()->baseInfo().nickname()
        })
    };
    MediaWindow*media_window = &MediaWindow::instance();
    media_window->set_play_list(play_list);
    Utils::widget_raise(media_window);
}

// 更新当前页面上的头像显示，无网络及校验功能
void ContactWindow::show_avatar(const std::shared_ptr<QPixmap>&avatar_pixmap) {
    if(avatar_pixmap == m_cache_avatar) { return; }
    set_cache_avatar_pixmap(avatar_pixmap);
    m_ui->button_avatar->setIcon(get_cache_avatar_pixmap() == nullptr ?  QIcon() : QIcon(get_cache_avatar_pixmap()->scaled(m_ui->button_avatar->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
}

// 传入头像 ID，自动从本地或网络加载头像
void ContactWindow::show_avatar_id(const QString&avatar_id) {
    // 先尝试从本地加载头像
    std::shared_ptr<QPixmap> avatar_pixmap = get_local_avatar_id_pixmap(avatar_id);
    if((avatar_pixmap && !avatar_pixmap->isNull()) || avatar_id.isEmpty()) {
        show_avatar(avatar_pixmap);
        return;
    }

    if(avatar_pixmap && !avatar_pixmap->isNull()) {
        return;
    }

    // 下载并显示头像
    network_file_io_notify_callback callback_download_file_notify = [this](const NetworkFileIONotify&notify) {
        // 忽略下载进度通知
        if(notify.ok && notify.code == int64_t(SuccessCode::SUCCESS) && notify.uploading) {
            return;
        }
        if(!notify.ok) {
            QString error_message = tr("Failed to download avatar.");
            error_message += tr("\n\ncode : 0x%1\nmessage : %2")
                .arg(notify.code, 8, 16, QChar('0'))  // 16 进制显示，宽度 8 位，不足补 0
                .arg(notify.message);
            QMessageBox::information(this, tr("Error"), error_message, QMessageBox::StandardButton::Ok);
            return;
        }
        // 下载成功，文件已保存到本地，可以直接显示了
        show_avatar(get_local_avatar_id_pixmap(notify.id));
    };
    qint64 downloaded_size = QFileInfo(Database::instance().get_local_file_id_path(avatar_id)).size();
    NetworkClient::instance().download_file(avatar_id, qMax(qint64(0), downloaded_size), callback_download_file_notify);
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
    if(file_dialog.exec() == QDialog::Accepted) {
        avatar_file_path = file_dialog.selectedFiles().first();
    }
    return avatar_file_path;
}

// 更新个人资料中的文本信息
void ContactWindow::update_profile_text(const ContactInfo&contact_info, const update_profile_callback&callback_update_profile) {
    // qDebug().noquote().nospace() << "Current account contactInfo: " << Utils::Transform::to_string(Accounts::instance().get_current_account());

    update_profile_params params = std::make_shared<TaoTieChat::UpdateProfileRequest>();
    // 身份验证参数，必填
    auto auth_request = Accounts::instance().get_auth_request();
    params->setRequestUserAuth(*auth_request.get());
    // 联系人信息参数
    params->setContactInfo(contact_info);
    NetworkClient::instance().update_profile(params, callback_update_profile);
}

// 更新个人资料中的头像，上传头像文件并更新服务器上的头像 ID，最后刷新显示
void ContactWindow::update_profile_avatar(const QString&avatar_path) {
    // qDebug().noquote().nospace() << "Current account contactInfo: " << Utils::Transform::to_string(Accounts::instance().get_current_account());
    // qDebug().noquote().nospace() << "Cache           contactInfo: " << Utils::Transform::to_string(m_cache_contact_info);

    // 上传头像
    network_file_io_notify_callback callback_upload_file_notify = [this](const NetworkFileIONotify&notify) {
        if(!notify.ok || notify.status != FileStatusCode::FILE_STATUS_UPLOADED) {
            // 16 进制显示，宽度 8 位，不足补 0
            QString error_message = tr("Failed to upload avatar. \n\ncode : 0x%1\nmessage : %2").arg(notify.code, 8, 16, QChar('0')).arg(notify.message);
            QMessageBox::information(this, tr("Error"), error_message, QMessageBox::StandardButton::Ok);
            return;
        }

        // qDebug().noquote().nospace() << "Current account contactInfo: " << Utils::Transform::to_string(Accounts::instance().get_current_account());

        // 构造参数
        QString file_id = notify.id;
        // 更新头像 ID 并保存到服务器
        // auto contact_info = get_cache_contact_info(); // 智能指针会修改缓存数据，实际上不要修改缓存的内容
        std::shared_ptr<ContactInfo> contact_info = std::make_shared<ContactInfo>(*get_cache_contact_info().get());
        auto base_info = contact_info->baseInfo();
        base_info.setAvatarId(file_id);
        contact_info->setBaseInfo(base_info);
        // 只允许修改自己的头像，如果有问题就补充
        if(!contact_info->hasSelfInfo()) {
            ContactInfoSelf self_info;
            self_info.setModifySelfInfo(true);
            contact_info->setSelfInfo(self_info);
        }
        // qDebug().noquote().nospace() << "Current account contactInfo: " << Utils::Transform::to_string(Accounts::instance().get_current_account());
        auto update_avatar_callback = [this, notify = notify](const update_profile_params&params_update_profile, const update_profile_reply&reply_update_profile) {
            // qDebug().noquote().nospace() << "Current account contactInfo: " << Utils::Transform::to_string(Accounts::instance().get_current_account());
            if(!reply_update_profile || !reply_update_profile->result().hasSuccessCode()) {
                QString error_message = tr("Failed to update profile.");
                if(reply_update_profile && reply_update_profile->result().hasErrorCode()) {
                    error_message += tr("\n\ncode : 0x%1\nmessage : %2")
                        .arg(int64_t(reply_update_profile->result().errorCode()), 8, 16, QChar('0'))  // 16 进制显示，宽度 8 位，不足补 0
                        .arg(reply_update_profile->result().message());
                }
                QMessageBox::information(this, tr("Error"), error_message, QMessageBox::StandardButton::Ok);
                return;
            }

            // qDebug().noquote().nospace() << "Current account contactInfo: " << Utils::Transform::to_string(Accounts::instance().get_current_account());
            // qDebug().noquote().nospace() << "Profile avatar updated successfully";
            // qDebug().noquote().nospace() << "Request         contactInfo: " << Utils::Transform::to_string(params_update_profile->contactInfo());

            std::shared_ptr<ContactInfo> contact_info = std::make_shared<ContactInfo>(reply_update_profile->request().contactInfo());
            if(contact_info) { contact_info = std::make_shared<ContactInfo>(params_update_profile->contactInfo()); }
            set_cache_contact_info(contact_info);

            show_avatar_id(notify.id);
            // qDebug().noquote().nospace() << "Updated profile avatar, old avatar ID: " << Accounts::instance().get_current_account()->baseInfo().avatarId();
            // qDebug().noquote().nospace() << "Updated profile avatar, new avatar ID: " << contact_info->baseInfo().avatarId();

            Accounts::instance().set_current_account(contact_info);
            // ChatWindow *chat_window = dynamic_cast<ChatWindow*>(parent());
            // if(chat_window) {
            //     chat_window->update_contact_info(contact_info);
            // }
        };
        update_profile_text(*contact_info, update_avatar_callback);
    };
    NetworkClient::instance().upload_file(avatar_path, callback_upload_file_notify);
}
