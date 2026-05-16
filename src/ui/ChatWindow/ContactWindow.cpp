#include "ContactWindow.h"
#include "ui_ContactWindow.h"
#include "NetworkClient.h"

// ContactWindow::ContactWindow(QWidget *parent)
//     : ContactWindow(std::make_shared<ContactInfo>(), parent) {
// }

ContactWindow::ContactWindow(const std::shared_ptr<ContactInfo>&contact_info, QWidget *parent)
    : QWidget(parent), m_ui(std::make_shared<Ui::ContactWindow>()), m_contact_info(contact_info) {
    m_ui->setupUi(this);
    // new
    // 防止未初始化
    if(!m_contact_info ) { m_contact_info  = std::make_shared<ContactInfo>(); }
    if(!m_menu_manager ) { m_menu_manager  = std::make_shared<QMenu      >(); }
    if(!m_action_edit  ) { m_action_edit   = std::make_shared<QAction    >(); }
    if(!m_action_remove) { m_action_remove = std::make_shared<QAction    >(); }

    // connect
    connect(m_ui->button_manager , &QToolButton  ::clicked    , this, &ContactWindow::on_button_manager );
    connect(m_ui->button_avatar  , &QToolButton  ::clicked    , this, &ContactWindow::on_button_avatar  );
    connect(m_ui->button_modify  , &QToolButton  ::clicked    , this, &ContactWindow::on_button_modify  );
    connect(m_ui->button_cancel  , &QToolButton  ::clicked    , this, &ContactWindow::on_button_cancel  );
    connect(m_ui->button_add     , &QToolButton  ::clicked    , this, &ContactWindow::on_button_add     );
    connect(m_ui->button_messages, &QToolButton  ::clicked    , this, &ContactWindow::on_button_messages);
    connect(m_ui->button_voice   , &QToolButton  ::clicked    , this, &ContactWindow::on_button_voice   );
    connect(m_ui->button_video   , &QToolButton  ::clicked    , this, &ContactWindow::on_button_video   );
    connect(m_action_edit  .get(), &QAction      ::triggered  , this, &ContactWindow::on_action_edit    );
    connect(m_action_remove.get(), &QAction      ::triggered  , this, &ContactWindow::on_action_remove  );
    connect(this                 , &ContactWindow::show_avatar, this, [this](const std::shared_ptr<ContactInfo>&contact_info) {
        // 获取显示内容
        QString nickname      = contact_info->baseInfo().nickname();  // 昵称
        QString user_id       = contact_info->baseInfo().userId  ();  // 用户 ID
        QString notes         = contact_info->hasFriendInfo() ? contact_info->friendInfo().notes() : QString();  // 备注
        QPixmap avatar_pixmap;  // = contact_info->get_avatar_pixmap();  // 头像图像
        QString avatar_path  ;  // = contact_info->get_avatar_path  ();  // 头像路径
        QString str_name = !nickname     .isEmpty() ? nickname      : user_id;
        QPixmap avatar   = !avatar_pixmap.isNull () ? avatar_pixmap : QPixmap(avatar_path);
        avatar = !avatar.isNull() ? avatar : Utils::to_pixmap(str_name);
        //
        MediaWindow*media_window = &MediaWindow::instance();
        media_window->load(avatar);
        media_window->set_media_name(str_name);
        Utils::widget_raise(media_window);
    });

    // set
    m_ui->button_avatar->setFixedSize(128, 128);
    m_ui->button_avatar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_ui->button_avatar->setIconSize(QSize(m_ui->button_avatar->size()));
    m_action_edit  ->setText(tr("edit"  ));
    m_action_remove->setText(tr("remove"));
    m_menu_manager->addAction(m_action_edit  .get());
    m_menu_manager->addAction(m_action_remove.get());
    // 未设计的功能
    m_ui->button_gender ->hide();
    m_ui->button_starred->hide();
    // 设置默认状态
    m_ui->button_modify ->hide();
    m_ui->button_cancel ->hide();

    // update
    update(contact_info);
}

ContactWindow::~ContactWindow() {
    if(m_menu_manager ) { m_menu_manager .reset(); }
    if(m_action_edit  ) { m_action_edit  .reset(); }
    if(m_action_remove) { m_action_remove.reset(); }
    if(m_ui           ) { m_ui           .reset(); }
}

void ContactWindow::update(const std::shared_ptr<ContactInfo>&contact_info) {
    m_contact_info.reset();
    m_contact_info = contact_info;
    if(!m_contact_info) {
        m_contact_info = std::make_shared<ContactInfo>();
    }

    Utils::ContactInfoWrapper ciw(contact_info);
    QString str_name = !ciw.nickname.isEmpty() ? ciw.nickname : ciw.user_id;
    QPixmap avatar = Utils::to_pixmap(str_name);
    // show
    m_ui->edit_nickname_big->setText(ciw.nickname        );
    m_ui->edit_nickname    ->setText(ciw.nickname        );
    m_ui->edit_userid      ->setText(ciw.user_id         );
    m_ui->edit_region      ->setText(ciw.region          );
    m_ui->edit_notes       ->setText(ciw.notes           );
    m_ui->edit_phones      ->setText(ciw.phones.join(";"));
    m_ui->edit_emails      ->setText(ciw.emails.join(";"));
    m_ui->edit_tags        ->setText(ciw.tags  .join(";"));
    m_ui->edit_desc        ->setText(ciw.desc            );
    m_ui->edit_sign        ->setText(ciw.sign            );

    m_ui->button_avatar->setIcon(QIcon(avatar.scaled(m_ui->button_avatar->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));

    // 判断显示模式
    bool is_contact = Accounts::instance().is_contact(m_contact_info);
    bool is_self    = Accounts::instance().is_self   (m_contact_info);
    ;    if(is_contact) { show_contact_mode (); }
    else if(is_self   ) { show_stranger_mode(); }
}

bool ContactWindow::show_contact_mode() {
    m_ui->label_notes->setVisible(true);
    m_ui->edit_notes ->setVisible(true);
    m_ui->label_tags ->setVisible(true);
    m_ui->edit_tags  ->setVisible(true);
    m_ui->button_add     ->hide();
    m_ui->button_messages->show();
    m_ui->button_voice   ->show();
    m_ui->button_video   ->show();
    m_action_edit  ->setVisible(!true );
    m_action_remove->setVisible(!false);
    return false;
}

bool ContactWindow::show_stranger_mode() {
    m_ui->label_notes->setVisible(false);
    m_ui->edit_notes ->setVisible(false);
    m_ui->label_tags ->setVisible(false);
    m_ui->edit_tags  ->setVisible(false);
    m_ui->button_add     ->show();
    m_ui->button_messages->hide();
    m_ui->button_voice   ->hide();
    m_ui->button_video   ->hide();
    m_action_edit  ->setVisible(!false);
    m_action_remove->setVisible(!true );
    return false;
}

const std::shared_ptr<ContactInfo>&ContactWindow::get_contact() const {
    return m_contact_info;
}

void ContactWindow::on_button_manager () {
    // 在按钮下方左侧弹出  // 右下角往左下角弹窗
    m_menu_manager->adjustSize(); // 内存调整为实际大小
    QPoint pos = m_ui->button_manager->mapToGlobal(m_ui->button_manager->rect().bottomRight());
    int w = m_menu_manager->size().width();
    m_menu_manager->popup(QPoint(pos.x() - w, pos.y()));
}

void ContactWindow::on_button_avatar() {
    // 编辑模式为修改头像
    if(m_ui->button_modify->isVisible() && m_contact_info && Accounts::instance().is_self(m_contact_info)) {
        QString pictures_location = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

        // 获取 Qt 支持的所有图片后缀
        QList<QByteArray> formats = QImageReader::supportedImageFormats();
        QString allExtensions = tr("Image Files (*.png;*.jpg;*.bmp)");   // 默认的图片格式过滤器
        allExtensions.replace("(*.png;*.jpg;*.bmp)", "(*." + formats.join(";*.") + ")");  // 输出支持的图片格式列表

        QFileDialog file_dialog(this, tr("Select Avatar"), pictures_location, allExtensions);
        file_dialog.setFileMode(QFileDialog::ExistingFile);
        if(file_dialog.exec() == QDialog::Accepted) {
            QString file_path = file_dialog.selectedFiles().first();
            QPixmap avatar_pixmap(file_path);
            if(avatar_pixmap.isNull()) {
                QMessageBox::warning(this, tr("Invalid Image"), tr("The selected file is not a valid image."));
                return;
            }
            // 更新显示
            m_ui->button_avatar->setIcon(QIcon(avatar_pixmap.scaled(m_ui->button_avatar->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
            // // 保存回去
            // ContactManager cm(m_contact);
            // cm.set_avatar_pixmap(avatar_pixmap);
            // cm.set_avatar_path(file_path);
        }
        return;
    }

    // 非编辑模式查看头像
    emit show_avatar(m_contact_info);
}

void ContactWindow::on_button_modify() {
    show_edit_mode(false);
    // TODO：通知服务器进行修改
    update_profile_params params = std::make_shared<TaoTieChat::UpdateProfileRequest>();

    bool is_self    = Accounts::instance().is_self   (m_contact_info);
    bool is_contact = Accounts::instance().is_contact(m_contact_info);
    if(!is_self && !is_contact) {
        // 既不是自己也不是好友，理论上不应该出现修改按钮，但这里做个保险，防止出现修改好友资料的界面但实际上不是好友的情况。
        return;
    }

    // 填充参数

    // 身份验证参数，必填
    auto auth_request = Accounts::instance().get_auth_request();
    params->setRequestUserAuth(*auth_request.get());

    TaoTieChat::ContactInfo contact_info_proto;

    // ContactInfoFriend 和 ContactInfoSelf 只能填充一种。
    // 修改自己资料时使用 ContactInfoSelf，修改好友资料时使用 ContactInfoFriend。
    // ContactInfoSelf 目前仅作为修改自己资料的标志，无实际字段。

    // 公共字段，自己和好友都需要填充。
    // 修改自己的资料时，base_info 的账号 ID 和用户 ID 也需要填充，
    // 因为服务器需要验证用户身份，确保修改的是自己的资料而不是别人的资料。
    TaoTieChat::ContactInfoBase base_info_proto;
    base_info_proto.setAccountId(m_contact_info->baseInfo().accountId());
    base_info_proto.setUserId   (m_ui->edit_userid->text());
    contact_info_proto.setBaseInfo(base_info_proto);

    // 好友特有字段，只有在是好友时才需要填充
    if(is_contact) {
        // friend_info 的备注和标签只有在是好友时才需要填充
        TaoTieChat::ContactInfoFriend friend_info_proto;
        friend_info_proto.setNotes(m_ui->edit_notes->text());
        friend_info_proto.setTags (m_ui->edit_tags ->text().split(";", Qt::SkipEmptyParts));
        contact_info_proto.setFriendInfo(friend_info_proto);
    }
    // 自己特有字段，只有在是自己的资料时才需要填充
    else if(is_self) {
        // 修改自己资料时包括可修改自己的基础资料
        base_info_proto.setNickname(m_ui->edit_nickname->text());
        base_info_proto.setRegion  (m_ui->edit_region  ->text());
        base_info_proto.setDesc    (m_ui->edit_desc    ->text());
        base_info_proto.setSign    (m_ui->edit_sign    ->text());
        base_info_proto.setPhones  (m_ui->edit_phones  ->text().split(";", Qt::SkipEmptyParts));
        base_info_proto.setEmails  (m_ui->edit_emails  ->text().split(";", Qt::SkipEmptyParts));
        contact_info_proto.setBaseInfo  (base_info_proto  );

        // 修改自己资料时还需要一个标志，告诉服务器这是修改自己的资料，而不是修改别人的资料。
        TaoTieChat::ContactInfoSelf self_info_proto;
        self_info_proto.setModifySelfInfo(true);  // 标志这是修改自己的资料
        contact_info_proto.setSelfInfo(self_info_proto);
    }
    else {
        // 理论上不应该出现修改按钮，但这里做个保险，防止出现修改好友资料的界面但实际上不是好友的情况。
        return;
    }

    params->setContactInfo(contact_info_proto);

    NetworkClient::instance().update_profile(params, [this](const update_profile_params&params_update_profile, const update_profile_reply&reply_update_profile) {
        if(!reply_update_profile || !reply_update_profile->result().hasSuccessCode()) {
            QMessageBox::information(this, tr("Error"), tr("Failed to update profile."), QMessageBox::StandardButton::Ok);  // TODO:弹窗无法设置为中文
            return;
        }
        // 更新成功，刷新显示
        // TODO: 实际上用户 ID 可能被修改，但账号 ID 不允许修改，需要实现 search_account_id() 接口，通过 account_id 进行更新
        auto&user_id = params_update_profile->contactInfo().baseInfo().userId();
        search_user_params params_search_user = std::make_shared<TaoTieChat::SearchUserRequest>();
        params_search_user->setRequestUserAuth(*Accounts::instance().get_auth_request().get());
        params_search_user->setUserId(user_id);
        NetworkClient::instance().search_user(params_search_user, [this](const search_user_params&params_search_user, const search_user_reply&reply_search_user) {
            if(!reply_search_user || !reply_search_user->result().hasSuccessCode()) {
                QMessageBox::information(this, tr("Error"), tr("Failed to get updated profile."), QMessageBox::StandardButton::Ok);  // TODO:弹窗无法设置为中文
                return;
            }
            update(std::make_shared<ContactInfo>(reply_search_user->contactInfo()));
        });
    });
}

void ContactWindow::on_button_cancel() {
    show_edit_mode(false);
    update(m_contact_info); // 重置显示的内容，防止页面对内容进行了修改
}

void ContactWindow::on_button_add() {
    // TODO: 发送好友请求，等待对方同意后才能添加到好友列表中
}

void ContactWindow::on_button_messages() {
    emit show_message_window(m_contact_info);
}

void ContactWindow::on_button_voice() {
    // TODO: 发起语音通话
}

void ContactWindow::on_button_video() {
    // TODO: 发起视频通话
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
        emit remove_contact(m_contact_info);
    }
}

void ContactWindow::on_action_edit() {
    show_edit_mode(true);
}

bool ContactWindow::show_edit_mode(bool edit) {
    // 1. 非联系人不允许编辑
    // 2. 非自己时编辑内容受限较大

    if(!m_contact_info) {
        return false;
    }
    if(!Accounts::instance().is_self(m_contact_info) && !Accounts::instance().is_contact(m_contact_info)) {
        return false;
    }

    m_ui->button_modify->setVisible(edit);
    m_ui->button_cancel->setVisible(edit);
    m_action_edit    ->setVisible(!edit);
    if(Accounts::instance().is_contact(m_contact_info)) {
        m_ui->button_messages->setVisible(!edit);
        m_ui->button_voice   ->setVisible(!edit);
        m_ui->button_video   ->setVisible(!edit);
        m_action_remove      ->setVisible(!edit);
    }

    // add 按钮：编辑模式不显示，非编辑模式且非联系人时才显示
    if(edit) {
        m_ui->button_add->setVisible(false);
    }
    else if(!Accounts::instance().is_contact(m_contact_info)) {
        m_ui->button_add->setVisible(true);
    }

    const QString style_sheet_view    = "QLineEdit { background-color: transparent; border: none; }";

    //
    m_ui->edit_notes->setReadOnly(!edit);
    m_ui->edit_tags ->setReadOnly(!edit);
    m_ui->edit_desc ->setReadOnly(!edit);
    m_ui->edit_notes->setStyleSheet(edit ? QString() : style_sheet_view );
    m_ui->edit_tags ->setStyleSheet(edit ? QString() : style_sheet_view );
    m_ui->edit_desc ->setStyleSheet(edit ? QString() : style_sheet_view );
    if(Accounts::instance().is_self(m_contact_info)) {
        // m_ui->edit_nickname_big->setReadOnly(!edit);
        m_ui->edit_nickname->setReadOnly(!edit);
        m_ui->edit_userid  ->setReadOnly(!edit);
        m_ui->edit_region  ->setReadOnly(!edit);
        m_ui->edit_phones  ->setReadOnly(!edit);
        m_ui->edit_emails  ->setReadOnly(!edit);
        m_ui->edit_sign    ->setReadOnly(!edit);
        m_ui->edit_nickname->setStyleSheet(edit ? QString() : style_sheet_view );
        m_ui->edit_userid  ->setStyleSheet(edit ? QString() : style_sheet_view );
        m_ui->edit_region  ->setStyleSheet(edit ? QString() : style_sheet_view );
        m_ui->edit_phones  ->setStyleSheet(edit ? QString() : style_sheet_view );
        m_ui->edit_emails  ->setStyleSheet(edit ? QString() : style_sheet_view );
        m_ui->edit_sign    ->setStyleSheet(edit ? QString() : style_sheet_view );
    }
    return false;
}
