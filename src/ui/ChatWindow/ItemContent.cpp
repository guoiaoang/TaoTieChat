#include "ItemContent.h"
#include "ui_ItemContent.h"

ItemContent::ItemContent(QWidget *parent)
    : ItemContent(ItemContentData(), parent) {
}

ItemContent::ItemContent(const ItemContentData data, QWidget *parent)
    : QWidget(parent), ui(new Ui::ItemContent), m_data(data) {
    ui->setupUi(this);
    update_ui();
}

ItemContent::~ItemContent() {
    delete ui;
    ui = nullptr;
}

void ItemContent::update_data(const ItemContentData &data) {
    m_data = data;
    update_ui();
}

void ItemContent::update_ui() {
    // 更新头像
    if (!m_data.avatar.isEmpty() && !QPixmap(m_data.avatar).isNull()) {
        QPixmap pixmap(m_data.avatar);
        ui->label_avatar->setPixmap(pixmap.scaled(48, 48, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    } else {
        // 显示昵称首字母
        if (m_data.nickname.length() > 0) {
            QString initial = m_data.nickname.left(1).toUpper();
            ui->label_avatar->setText(initial);
            ui->label_avatar->setAlignment(Qt::AlignCenter);
            QFont avatarFont;
            avatarFont.setPointSize(20);
            avatarFont.setBold(true);
            ui->label_avatar->setFont(avatarFont);
        }
    }

    // 更新昵称
    ui->label_nickname->setText(m_data.nickname);
}
