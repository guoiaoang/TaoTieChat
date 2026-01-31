#include "ItemMessage.h"
#include "ui_ItemMessage.h"

ItemMessage::ItemMessage(QWidget *parent)
    : ItemMessage(ItemMessageData(), parent) {
}

ItemMessage::ItemMessage(const ItemMessageData&data, QWidget *parent)
    : QWidget(parent), ui(new Ui::ItemMessage), m_data(data) {
    ui->setupUi(this);
    update_ui();
}

ItemMessage::~ItemMessage() {
    delete ui;
}

void ItemMessage::update_data(const ItemMessageData &data) {
    m_data = data;
    update_ui();
}

void ItemMessage::update_ui() {
    if(m_data.type == ItemMessageData::Type::TIME) {
        // // 设置时间
        // QFont timeFont;
        // timeFont.setPointSize(8);
        // ui->time_button_text->setFont(timeFont);
        // ui->time_button_text->setStyleSheet("QLabel { color: #999; }");
        // ui->time_button_text->setAlignment(Qt::AlignCenter);
        //
        ui->time_button_text->setText(m_data.time);
        ui->stacked_item_message->setCurrentWidget(ui->time_page);
        return;
    }

    QToolButton *avatar   = nullptr;
    QLineEdit   *nickname = nullptr;
    QTextEdit   *content  = nullptr;
    if(m_data.type == ItemMessageData::Type::FRIEND) {
        avatar   = ui->friend_button_avatar;
        nickname = ui->friend_edit_nickname;
        content  = ui->friend_edit_content;
        ui->stacked_item_message->setCurrentWidget(ui->friend_page);
        // content->setStyleSheet(
        //     "QTextEdit {"
        //     "    background-color: white;"
        //     "    border: 1px solid #ddd;"
        //     "    border-radius: 8px;"
        //     "    padding: 8px 12px;"
        //     "}"
        //     );
    }
    else if(m_data.type == ItemMessageData::Type::SELF) {
        avatar   = ui->self_button_avatar;
        nickname = ui->self_edit_nickname;
        content  = ui->self_edit_content;
        ui->stacked_item_message->setCurrentWidget(ui->self_page);
        // content->setStyleSheet(
        //     "QTextEdit {"
        //     "    background-color: #95ec69;"
        //     "    border: 1px solid #95ec69;"
        //     "    border-radius: 8px;"
        //     "    padding: 8px 12px;"
        //     "}"
        //     );
    }

    // 设置头像
    avatar->setFixedSize(40, 40);
    // avatar->setScaledContents(true);
    avatar->setStyleSheet(
        "QToolButton {"
        "    border-radius: 20px;"
        "    background-color: #e0e0e0;"
        "    border: 1px solid #ccc;"
        "}"
        );

    // 设置昵称
    QFont nicknameFont;
    nicknameFont.setPointSize(9);
    nickname->setFont(nicknameFont);
    nickname->setStyleSheet("QLabel { color: #666; }");

    // 设置消息内容
    // content->setWordWrap(true);
    // content->setTextFormat(Qt::PlainText);
    // content->setStyleSheet(
    //     "QTextEdit {"
    //     "    background-color: white;"
    //     "    border: 1px solid #ddd;"
    //     "    border-radius: 8px;"
    //     "    padding: 8px 12px;"
    //     "}"
    //     );
    // content->setMaximumWidth(400);

    // 主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 5, 10, 5);

    this->setMinimumHeight(60);


    // update

    // 更新头像
    if (!m_data.avatar.isEmpty() && !QIcon(m_data.avatar).isNull()) {
        QIcon ico(m_data.avatar);
        // avatar->setPixmap(pixmap.scaled(48, 48, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        avatar->setIcon(ico);
    } else {
        // 显示昵称首字母
        if (m_data.nickname.length() > 0) {
            QString initial = m_data.nickname.left(1).toUpper();
            avatar->setText(initial);
            // avatar->setAlignment(Qt::AlignCenter);
            QFont avatarFont;
            avatarFont.setPointSize(20);
            avatarFont.setBold(true);
            avatar->setFont(avatarFont);
        }
    }

    // 更新昵称
    nickname->setText(m_data.nickname);

    // 更新消息内容
    content->setText(m_data.content);
}
