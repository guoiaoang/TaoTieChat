#pragma once

#include "Headers.h"
#include "Utils.h"
#include "MediaWindow.h"
#include "Accounts.h"

namespace Ui {
class ContactWindow;
}

class ContactWindow : public QWidget {
    Q_OBJECT

public:
    // explicit ContactWindow(QWidget *parent = nullptr);
    explicit ContactWindow(const std::shared_ptr<ContactInfo>&contact_info = std::make_shared<ContactInfo>(), QWidget *parent = nullptr);
    ~ContactWindow();

public:
    const std::shared_ptr<ContactInfo>& get_contact() const;
    void update(const std::shared_ptr<ContactInfo>&contact_info);

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
    void remove_contact     (const std::shared_ptr<ContactInfo>&contact_info);
    void show_message_window(const std::shared_ptr<ContactInfo>&contact_info);
    void show_avatar        (const std::shared_ptr<ContactInfo>&contact_info);

private:
    std::shared_ptr<Ui::ContactWindow> m_ui           ;
    std::shared_ptr<ContactInfo      > m_contact_info ;
    std::shared_ptr<QMenu            > m_menu_manager ;
    std::shared_ptr<QAction          > m_action_edit  ;
    std::shared_ptr<QAction          > m_action_remove;
};
