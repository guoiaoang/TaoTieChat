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
    if(m_contact       == nullptr) { m_contact       = QSharedPointer<ContactData>(new ContactData); }
    if(m_menu_manager  == nullptr) { m_menu_manager  = new QMenu  ; }
    if(m_action_remove == nullptr) { m_action_remove = new QAction; }

    // connect
    connect(ui->button_avatar  , &QToolButton  ::clicked    , this, &ContactWindow::on_button_avatar  );
    connect(ui->button_messages, &QToolButton  ::clicked    , this, &ContactWindow::on_button_messages);
    connect(ui->button_manager , &QToolButton  ::clicked    , this, &ContactWindow::on_button_manager );
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
    m_action_remove->setText(tr("remove"));
    m_menu_manager->addAction(m_action_remove);
    // update
    update(contact);
}

ContactWindow::~ContactWindow() {
    if(ui             ) { delete ui             ; ui              = nullptr; }
    if(m_menu_manager ) { delete m_menu_manager ; m_menu_manager  = nullptr; }
    if(m_action_remove) { delete m_action_remove; m_action_remove = nullptr; }
}

void ContactWindow::update(Contact contact) {
    m_contact = contact;
    if(m_contact == nullptr) { m_contact = QSharedPointer<ContactData>(new ContactData); }
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
    ui->edit_nickname->setText(str_name);
    ui->button_avatar->setIcon(QIcon(avatar.scaled(ui->button_avatar->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    // 判断是否是好友
    Contact account = Accounts::instance().get_current_account();
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

void ContactWindow::on_button_avatar() {
    emit show_avatar(contact());
}

void ContactWindow::on_button_messages() {
    emit show_message_window(contact());
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
