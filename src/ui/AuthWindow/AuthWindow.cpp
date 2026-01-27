#include "AuthWindow.h"
#include "ui_AuthWindow.h"

AuthWindow::AuthWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::AuthWindow) {
    ui->setupUi(this);
    connect(ui->button_login, &QToolButton::clicked, this, &AuthWindow::accepted);
}

AuthWindow::~AuthWindow() {
    delete ui;
    ui = nullptr;
}
