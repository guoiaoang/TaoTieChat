#pragma once

#include "headers.h"
#include "Utils.h"
#include "Defines.h"

namespace Ui {
class ContactWindow;
}

class ContactWindow : public QWidget {
    Q_OBJECT

public:
    explicit ContactWindow(QWidget *parent = nullptr);
    ~ContactWindow();

public:
    const std::shared_ptr<ContactInfo>&get_cache_contact_info () const;
    const std::shared_ptr<QPixmap    >&get_cache_avatar_pixmap() const;
    void set_cache_contact_info (const std::shared_ptr<ContactInfo>&contact_info );
    void set_cache_avatar_pixmap(const std::shared_ptr<QPixmap    >&avatar_pixmap);
    void show_contact(const std::shared_ptr<ContactInfo>&contact_info, const std::shared_ptr<QPixmap>&avatar_pixmap = nullptr);

private:
    // 显示模式
    bool set_edit_mode(bool edit = true);
    // 按钮和菜单的槽函数
    void on_button_manager ();
    void on_button_avatar  ();
    void on_button_save    ();
    void on_button_cancel  ();
    void on_button_add     ();
    void on_button_messages();
    void on_button_voice   ();
    void on_button_video   ();
    void on_action_edit    ();
    void on_action_remove  ();

    // get_local_avatar_id_pixmap 根据文件 ID 在本地查找文件并加载为头像图片
    std::shared_ptr<QPixmap> get_local_avatar_id_pixmap(const QString&avatar_id);
    // view_avatar 在媒体窗口查看当前头像
    void view_avatar(const std::shared_ptr<ContactInfo>&contact_info);
    // show_avatar 当前窗口更新头像显示，无网络及校验功能
    void show_avatar(const std::shared_ptr<QPixmap>&avatar_pixmap);
    // 传入头像 ID，自动从本地或网络加载头像
    void show_avatar_id(const QString&avatar_id);
    // dialog_select_avatar_file 选择头像文件的对话框
    QString dialog_select_avatar_file();

    // network
    void update_profile_text(const ContactInfo&contact_info, const update_profile_callback&callback_update_profile = nullptr);
    void update_profile_avatar(const QString&avatar_path);

Q_SIGNALS:
    void contact_add    (const std::shared_ptr<ContactInfo>&contact_info);
    void contact_remove (const std::shared_ptr<ContactInfo>&contact_info);
    void contact_message(const std::shared_ptr<ContactInfo>&contact_info);
    void contact_voice  (const std::shared_ptr<ContactInfo>&contact_info);
    void contact_video  (const std::shared_ptr<ContactInfo>&contact_info);

private:
    std::shared_ptr<Ui::ContactWindow> m_ui                 ;
    std::shared_ptr<QMenu            > m_menu_manager       ;
    std::shared_ptr<QAction          > m_action_edit        ;
    std::shared_ptr<QAction          > m_action_remove      ;
    std::shared_ptr<ContactInfo      > m_cache_contact_info ;
    std::shared_ptr<QPixmap          > m_cache_avatar       ;
    bool m_edit_mode = false;
    Utils::WidgetTools::LineEdit m_edit_tool;
};
