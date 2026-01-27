#include "ChatWindow.h"
#include "ui_ChatWindow.h"

ChatWindow::ChatWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::ChatWindow) {
    ui->setupUi(this);
    connect(ui->button_chat, &QToolButton::clicked, this, &ChatWindow::accept);
}

ChatWindow::~ChatWindow() {
    delete ui;
    ui = nullptr;
}
