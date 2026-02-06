#include "MessageWindow.h"
#include "ui_MessageWindow.h"

MessageWindow::MessageWindow(QWidget *parent)
    : MessageWindow(nullptr, nullptr, parent) {
}

MessageWindow::MessageWindow(Content content, Messages messages, QWidget *parent)
    : QWidget(parent), ui(new Ui::MessageWindow), m_content(content), m_messages(messages) {
    ui->setupUi(this);
    m_splitter = new QSplitter;
    if(m_content == nullptr) {
        m_content = QSharedPointer<TypeContent>(new TypeContent);
    }
    if(m_messages == nullptr) {
        m_messages = QSharedPointer<TypeMessages>(new TypeMessages);
    }

    ui->list_messages->setSpacing(0);
    // 按像素滚动，而不是按条目 (会发生跳变)
    ui->list_messages->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // 替换
    m_splitter->setOrientation(Qt::Vertical);
    ui->layout_message_window->removeWidget(ui->list_messages);
    m_splitter->addWidget(ui->list_messages);
    ui->layout_message_window->replaceWidget(ui->widget_input, m_splitter);
    m_splitter->addWidget(ui->widget_input);

    // ui->widget_input->resize(200, ui->widget_input->width()); // 未生效？?
    // 设置拉伸因子 // 拉伸主窗口时窗口调整大小的策略
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 0);

    // update_ui();
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

void MessageWindow::update_contents(Content content ) {
    m_content = content;
}

// 消息较多时应该差异化更新
void MessageWindow::update_messages(Messages messages) {
    m_messages = messages;
}

void MessageWindow::append_message (Message message ) {
    //
    ui->list_messages->scrollToBottom(); // 滚动到最后
}

