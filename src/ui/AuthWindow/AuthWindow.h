#pragma once

#include <QDialog>

namespace Ui {
class AuthWindow;
}

class AuthWindow : public QDialog{
    Q_OBJECT

public:
    explicit AuthWindow(QWidget *parent = nullptr);
    ~AuthWindow();

private:
    Ui::AuthWindow *ui = nullptr;
};
