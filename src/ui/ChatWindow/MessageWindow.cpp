#include "MessageWindow.h"
#include "ui_MessageWindow.h"

MessageWindow::MessageWindow(QWidget *parent)
    : MessageWindow(ChatroomMessages(), parent) {
}

MessageWindow::MessageWindow(const ChatroomMessages&chatroom_messages, QWidget *parent)
    : QWidget(parent), ui(new Ui::MessageWindow) {
    ui->setupUi(this);
    m_splitter = new QSplitter;
    ui->list_messages->setSpacing(0);
    ui->list_messages->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // 替换
    m_splitter->setOrientation(Qt::Vertical);
    ui->layout_message_window->removeWidget(ui->list_messages);
    m_splitter->addWidget(ui->list_messages);
    ui->layout_message_window->replaceWidget(ui->widget_input, m_splitter);
    m_splitter->addWidget(ui->widget_input);
    // ui->widget_input->resize(200, ui->widget_input->width()); // 为生效??
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 0);
    update_ui();
}

MessageWindow::~MessageWindow() {
    if(ui) {
        delete ui;
        ui = nullptr;
    }
    if(m_splitter) {
        delete m_splitter;
        m_splitter = nullptr;
    }
}

void MessageWindow::update_info(const ChatroomInfo&chatroom_info) {
    m_chatroom_info = chatroom_info;
}

// 消息较多时应该差异化更新
void MessageWindow::update_data(const ChatroomMessages &chatroom_messages) {
    // if(chatroom_messages == m_chatroom_messages) {
    //     return;
    // }
    m_chatroom_messages = chatroom_messages;
    update_ui();
}

void MessageWindow::update_ui() {
    // 清空当前消息列表
    ui->list_messages->clear();
    for (const ItemMessageData &item_message_data : m_chatroom_messages) {
        QListWidgetItem *list_widget_item = new QListWidgetItem(ui->list_messages);
        ItemMessage     *item_message     = new ItemMessage(item_message_data, ui->list_messages);
        list_widget_item ->setSizeHint(item_message->sizeHint());
        ui->list_messages->setItemWidget(list_widget_item, item_message);
    }
    // 更新标题名称
    ui->label_title->setText(m_chatroom_info.title);
    // 更新头像
    if (!m_chatroom_info.avatar.isEmpty() && !QIcon(m_chatroom_info.avatar).isNull()) {
        QIcon ico(m_chatroom_info.avatar);
        // avatar->setPixmap(pixmap.scaled(48, 48, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        ui->button_title_avatar->setIcon(ico);
    } else {
        // 显示昵称首字母
        if (m_chatroom_info.title.length() > 0) {
            QString initial = m_chatroom_info.title.left(1).toUpper();
            ui->button_title_avatar->setText(initial);
            // avatar->setAlignment(Qt::AlignCenter);
            QFont avatarFont;
            avatarFont.setPointSize(20);
            avatarFont.setBold(true);
            ui->button_title_avatar->setFont(avatarFont);
        }
    }
}

void MessageWindow::append_message(const ItemMessageData&item_message_data) {
    m_chatroom_messages.append(item_message_data);
    QListWidgetItem *list_widget_item = new QListWidgetItem(ui->list_messages);
    ItemMessage     *item_message     = new ItemMessage(item_message_data, ui->list_messages);
    list_widget_item ->setSizeHint(item_message->sizeHint());
    ui->list_messages->setItemWidget(list_widget_item, item_message);
    ui->list_messages->scrollToBottom(); // 滚动到最后
}
