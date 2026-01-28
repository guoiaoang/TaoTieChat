#include "AuthWindow.h"
#include "ui_AuthWindow.h"

AuthWindow::AuthWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::AuthWindow) {
    ui->setupUi(this);
    connect(ui->signin_button_signin, &QToolButton::clicked, this, &AuthWindow::accepted);
    connect(ui->signin_button_signup, &QToolButton::clicked, this, [&](){ ui->auth_stacked->setCurrentWidget(ui->page_signup); });
    connect(ui->signup_button_signin, &QToolButton::clicked, this, [&](){ ui->auth_stacked->setCurrentWidget(ui->page_signin); });
}

AuthWindow::~AuthWindow() {
    delete ui;
    ui = nullptr;
}
