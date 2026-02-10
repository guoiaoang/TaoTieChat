#include "ContactWindow.h"
#include "ui_ContactWindow.h"

ContactWindow::ContactWindow(QWidget *parent) : ContactWindow(nullptr, parent) {}

ContactWindow::ContactWindow(Contact contact, QWidget *parent)
    : QWidget(parent), ui(new Ui::ContactWindow), m_contact(contact) {
    ui->setupUi(this);
    // new
    // 防止未初始化
    if(m_contact       == nullptr) { m_contact       = QSharedPointer<ContactData>(new ContactData); }
    if(m_menu_manager  == nullptr) { m_menu_manager  = new QMenu  ; }
    if(m_action_remove == nullptr) { m_action_remove = new QAction; }

    // connect
    connect(ui->button_avatar  , &QToolButton::clicked, this, &ContactWindow::on_button_avatar  );
    connect(ui->button_messages, &QToolButton::clicked, this, &ContactWindow::on_button_messages);
    connect(ui->button_manager , &QToolButton::clicked, this, &ContactWindow::on_button_manager );
    connect(m_action_remove    , &QAction::triggered  , this, &ContactWindow::on_action_remove  );

    // set
    ui->button_avatar->setFixedSize(128, 128);
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
    if(m_contact       == nullptr) { m_contact       = QSharedPointer<ContactData>(new ContactData); }
    // show
    ui->edit_nickname->setText(ContactManager(contact).nickname());
    QPixmap pixmap = QPixmap(ContactManager(contact).avatar());
    if(!pixmap.isNull()) {
        // ui->button_avatar->setLayout();
        ui->button_avatar->setText(QString());
        ui->button_avatar->setIconSize(QSize(ui->button_avatar->size()));
        ui->button_avatar->setIcon(QIcon(pixmap.scaled(ui->button_avatar->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
        // ui->button_avatar->setIconSize(QSzie(128, 128));
        // ui->button_avatar->setIcon(QIcon(pixmap));
    }
    else {
        // 直接根据按钮高度估算一个值，比如高度的 50% - 60%
        // setPointSize 与 setPixelSize 均可
        QFont font = ui->button_avatar->font();
        font.setPointSize(ui->button_avatar->height() * 0.5);
        // font.setPixelSize(ui->button_avatar->height() * 0.5);
        font.setBold(true);
        ui->button_avatar->setFont(font);
        ui->button_avatar->setIcon(QIcon()); // 清除之前的图标
        ui->button_avatar->setText(ContactManager(contact).avatar().left(2));
    }
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
