#pragma once

#include "Headers.h"
#include "data.h"
#include "Utils.h"
#include "MediaWindow.h"
#include "Accounts.h"

namespace Ui {
class ContactWindow;
}

class ContactWindow : public QWidget {
    Q_OBJECT

public:
    explicit ContactWindow(                 QWidget *parent = nullptr);
    explicit ContactWindow(Contact contact, QWidget *parent = nullptr);
    ~ContactWindow();

public:
    Contact contact(               ) const;
    void    update (Contact contact)      ;

private:
    void on_button_manager ();
    void on_button_avatar  ();
    void on_button_modify  ();
    void on_button_cancel  ();
    void on_button_add     ();
    void on_button_messages();
    void on_button_voice   ();
    void on_button_video   ();
    void on_action_edit    ();
    void on_action_remove  ();
    bool show_contact_mode ();
    bool show_stranger_mode();
    bool show_edit_mode(bool edit = true);

Q_SIGNALS:
    void remove_contact     (Contact contact);
    void show_message_window(Contact contact);
    void show_avatar        (Contact contact);

private:
    Ui::ContactWindow *ui              = nullptr;
    Contact            m_contact       = nullptr;
    QMenu             *m_menu_manager  = nullptr;
    QAction           *m_action_edit   = nullptr;
    QAction           *m_action_remove = nullptr;
};
