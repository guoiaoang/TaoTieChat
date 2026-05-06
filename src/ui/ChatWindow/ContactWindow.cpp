#include "ContactWindow.h"
#include "ui_ContactWindow.h"

ContactWindow::ContactWindow(QWidget *parent)
    : ContactWindow(nullptr, parent) {
}

ContactWindow::ContactWindow(Contact contact, QWidget *parent)
    : QWidget(parent), ui(new Ui::ContactWindow), m_contact(contact) {
    ui->setupUi(this);
    // new
    // 防止未初始化
    if(m_contact       == nullptr) { m_contact       = ContactManager::create(); }
    if(m_menu_manager  == nullptr) { m_menu_manager  = new QMenu  ; }
    if(m_action_edit   == nullptr) { m_action_edit   = new QAction; }
    if(m_action_remove == nullptr) { m_action_remove = new QAction; }

    // connect
    connect(ui->button_manager , &QToolButton  ::clicked    , this, &ContactWindow::on_button_manager );
    connect(ui->button_avatar  , &QToolButton  ::clicked    , this, &ContactWindow::on_button_avatar  );
    connect(ui->button_modify  , &QToolButton  ::clicked    , this, &ContactWindow::on_button_modify  );
    connect(ui->button_cancel  , &QToolButton  ::clicked    , this, &ContactWindow::on_button_cancel  );
    connect(ui->button_add     , &QToolButton  ::clicked    , this, &ContactWindow::on_button_add     );
    connect(ui->button_messages, &QToolButton  ::clicked    , this, &ContactWindow::on_button_messages);
    connect(ui->button_voice   , &QToolButton  ::clicked    , this, &ContactWindow::on_button_voice   );
    connect(ui->button_video   , &QToolButton  ::clicked    , this, &ContactWindow::on_button_video   );
    connect(m_action_edit      , &QAction      ::triggered  , this, &ContactWindow::on_action_edit    );
    connect(m_action_remove    , &QAction      ::triggered  , this, &ContactWindow::on_action_remove  );
    connect(this               , &ContactWindow::show_avatar, this, [this](Contact contact) {
        // 获取显示内容
        ContactManager cm(contact);
        QString nickname      = cm.get_nick_name    ();  // 昵称
        QString user_id       = cm.get_user_id      ();  // 用户 ID
        QString notes         = cm.get_notes        ();  // 备注
        QPixmap avatar_pixmap = cm.get_avatar_pixmap();  // 头像图像
        QString avatar_path   = cm.get_avatar_path  ();  // 头像路径
        QString str_name = !nickname     .isEmpty() ? nickname      : user_id;
        QPixmap avatar   = !avatar_pixmap.isNull () ? avatar_pixmap : QPixmap(avatar_path);
        avatar = !avatar.isNull() ? avatar : Utils::to_pixamp(str_name);
        //
        MediaWindow*media_window = &MediaWindow::instance();
        // media_window->load(cm.get_avatar_path());
        media_window->load(avatar);
        media_window->set_media_name(str_name);
        Utils::widget_raise(media_window);
    });

    // set
    ui->button_avatar->setFixedSize(128, 128);
    ui->button_avatar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    ui->button_avatar->setIconSize(QSize(ui->button_avatar->size()));
    m_action_edit  ->setText(tr("edit"  ));
    m_action_remove->setText(tr("remove"));
    m_menu_manager->addAction(m_action_edit  );
    m_menu_manager->addAction(m_action_remove);
    // 未设计的功能
    ui->button_gender ->hide();
    ui->button_starred->hide();
    // 设置默认状态
    ui->button_modify ->hide();
    ui->button_cancel ->hide();

    // update
    update(contact);
}

ContactWindow::~ContactWindow() {
    if(ui             ) { delete ui             ; ui              = nullptr; }
    if(m_menu_manager ) { delete m_menu_manager ; m_menu_manager  = nullptr; }
    if(m_action_edit  ) { delete m_action_edit  ; m_action_edit   = nullptr; }
    if(m_action_remove) { delete m_action_remove; m_action_remove = nullptr; }
}

void ContactWindow::update(Contact contact) {
    m_contact = contact;
    if(m_contact == nullptr) { m_contact = ContactManager::create(); }
    // 获取显示内容
    ContactManager cm(contact);
    QString nickname      = cm.get_nick_name    ();  // 昵称
    QString user_id       = cm.get_user_id      ();  // 用户 ID
    QString notes         = cm.get_notes        ();  // 备注
    QPixmap avatar_pixmap = cm.get_avatar_pixmap();  // 头像图像
    QString avatar_path   = cm.get_avatar_path  ();  // 头像路径
    QString str_name = !nickname     .isEmpty() ? nickname      : user_id;
    QPixmap avatar   = !avatar_pixmap.isNull () ? avatar_pixmap : QPixmap(avatar_path);
    avatar = !avatar.isNull() ? avatar : Utils::to_pixamp(str_name);
    // if(nickname     .isEmpty()) { cm.set_nick_name    (str_name); }  // 保存回去？？
    // if(avatar_pixmap.isNull ()) { cm.set_avatar_pixmap(avatar  ); }  // 保存回去？？
    // show
    ui->edit_nickname_big->setText(str_name);
    ui->edit_nickname    ->setText(str_name);
    ui->edit_userid      ->setText(user_id );
    ui->edit_notes       ->setText(notes   );
    ui->edit_region      ->setText(cm.get_region().join(","));
    ui->edit_phones      ->setText(cm.get_phones().join(","));
    ui->edit_emails      ->setText(cm.get_emails().join(","));
    ui->edit_tags        ->setText(cm.get_tags  ().join(","));
    ui->edit_desc        ->setText(cm.get_description());
    ui->edit_sign        ->setText(cm.get_signature  ());

    ui->button_avatar->setIcon(QIcon(avatar.scaled(ui->button_avatar->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    if(Accounts::instance().is_contact(m_contact)) {
        show_contact_mode();
    }
    else {
        show_stranger_mode();
    }
}

bool ContactWindow::show_contact_mode() {
    ui->button_add     ->hide();
    ui->button_messages->show();
    ui->button_voice   ->show();
    ui->button_video   ->show();
    m_action_edit  ->setVisible(!true );
    m_action_remove->setVisible(!false);
    return false;
}

bool ContactWindow::show_stranger_mode() {
    ui->button_add     ->show();
    ui->button_messages->hide();
    ui->button_voice   ->hide();
    ui->button_video   ->hide();
    m_action_edit  ->setVisible(!false);
    m_action_remove->setVisible(!true );
    return false;
}

Contact ContactWindow::contact() const {
    return m_contact;
}

void ContactWindow::on_button_manager () {
    // 在按钮下方左侧弹出  // 右下角往左下角弹窗
    m_menu_manager->adjustSize(); // 内存调整为实际大小
    QPoint pos = ui->button_manager->mapToGlobal(ui->button_manager->rect().bottomRight());
    int w = m_menu_manager->size().width();
    m_menu_manager->popup(QPoint(pos.x() - w, pos.y()));
}
#include <QFileDialog>
#include <QMimeDatabase>
#include <QMimeType>
#include <QImageReader>

void ContactWindow::on_button_avatar() {
    if(ui->button_modify->isVisible() && Accounts::instance().is_self(m_contact)) {
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
            ui->button_avatar->setIcon(QIcon(avatar_pixmap.scaled(ui->button_avatar->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
            // // 保存回去
            // ContactManager cm(m_contact);
            // cm.set_avatar_pixmap(avatar_pixmap);
            // cm.set_avatar_path(file_path);
        }
        return;
    }
    emit show_avatar(contact());
}

void ContactWindow::on_button_modify() {
    show_edit_mode(false);
    // TODO：通知服务器进行修改
}

void ContactWindow::on_button_cancel() {
    show_edit_mode(false);
    update(m_contact); // 重置显示的内容，防止页面对内容进行了修改
}

void ContactWindow::on_button_add() {
    // TODO: 发送好友请求，等待对方同意后才能添加到好友列表中
}

void ContactWindow::on_button_messages() {
    emit show_message_window(contact());
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
        emit remove_contact(contact());
    }
}

void ContactWindow::on_action_edit() {
    show_edit_mode(true);
}

bool ContactWindow::show_edit_mode(bool edit) {
    // 1. 非联系人不允许编辑
    // 2. 非自己时编辑内容受限较大

    if(!Accounts::instance().is_self(m_contact) && !Accounts::instance().is_contact(m_contact)) {
        return false;
    }

    ui->button_modify->setVisible(edit);
    ui->button_cancel->setVisible(edit);
    m_action_edit    ->setVisible(!edit);
    if(Accounts::instance().is_contact(m_contact)) {
        ui->button_messages->setVisible(!edit);
        ui->button_voice   ->setVisible(!edit);
        ui->button_video   ->setVisible(!edit);
        m_action_remove    ->setVisible(!edit);
    }

    // add 按钮：编辑模式不显示，非编辑模式且非联系人时才显示
    if(edit) {
        ui->button_add->setVisible(false);
    }
    else if(!Accounts::instance().is_contact(m_contact)) {
        ui->button_add->setVisible(true);
    }

    const QString style_sheet_edit    = "QLineEdit { background-color: white; }";
    const QString style_sheet_view    = "QLineEdit { background-color: transparent; border: none; }";
    const QString style_sheet_enable  = "QLineEdit { background-color: transparent; }";
    const QString style_sheet_disable = "QLineEdit { background-color: transparent; border: none; }";

    //
    ui->edit_notes->setReadOnly(!edit);
    ui->edit_tags ->setReadOnly(!edit);
    ui->edit_desc ->setReadOnly(!edit);
    ui->edit_notes->setStyleSheet(edit ? QString() : style_sheet_view );
    ui->edit_tags ->setStyleSheet(edit ? QString() : style_sheet_view );
    ui->edit_desc ->setStyleSheet(edit ? QString() : style_sheet_view );
    if(Accounts::instance().is_self(m_contact)) {
        // ui->edit_nickname_big->setReadOnly(!edit);
        ui->edit_nickname->setReadOnly(!edit);
        ui->edit_userid  ->setReadOnly(!edit);
        ui->edit_region  ->setReadOnly(!edit);
        ui->edit_phones  ->setReadOnly(!edit);
        ui->edit_emails  ->setReadOnly(!edit);
        ui->edit_sign    ->setReadOnly(!edit);
        ui->edit_nickname->setStyleSheet(edit ? QString() : style_sheet_view );
        ui->edit_userid  ->setStyleSheet(edit ? QString() : style_sheet_view );
        ui->edit_region  ->setStyleSheet(edit ? QString() : style_sheet_view );
        ui->edit_phones  ->setStyleSheet(edit ? QString() : style_sheet_view );
        ui->edit_emails  ->setStyleSheet(edit ? QString() : style_sheet_view );
        ui->edit_sign    ->setStyleSheet(edit ? QString() : style_sheet_view );
    }
    return false;
}
