#include "ItemChat.h"
#include "ui_ItemChat.h"

ItemChat::ItemChat(QWidget *parent)
    : ItemChat(ItemChatData(), parent) {
}

ItemChat::ItemChat(const ItemChatData data, QWidget *parent)
    : QWidget(parent), ui(new Ui::ItemChat), m_data(data) {
    ui->setupUi(this);
    //
    // 设置未读消息数属性  // 圆形红色背景
    ui->label_unread->setFixedSize(20, 20);
    ui->label_unread->setAlignment(Qt::AlignCenter);
    ui->label_unread->setStyleSheet(
        "QLabel {"
        "    background-color: #ff5722;"
        "    color: white;"
        "    border-radius: 10px;"
        "    font-size: 10px;"
        "    font-weight: bold;"
        "}"
        );
    ui->label_unread->hide(); // 默认隐藏
    update_ui();
}

ItemChat::~ItemChat() {
    delete ui;
    ui = nullptr;
}

void ItemChat::update_data(const ItemChatData &data) {
    m_data = data;
    update_ui();
}

void ItemChat::update_ui() {
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

    // 更新消息内容 (限制长度)
    QString message = m_data.lastMessage;
    if (message.length() > 30) {
        message = message.left(30) + "...";
    }
    ui->label_message->setText(message);

    // 更新时间
    ui->label_time->setText(m_data.timestamp);

    // 更新未读消息数
    if (m_data.unreadCount > 0) {
        ui->label_unread->setText(QString::number(m_data.unreadCount > 99 ? 99 : m_data.unreadCount));
        ui->label_unread->show();
    } else {
        ui->label_unread->hide();
    }
}
