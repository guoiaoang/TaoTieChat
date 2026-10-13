#pragma once

#include "headers.h"
#include "Utils.h"
#include "Defines.h"

class ContactWindow;
class MessageModel;
class ItemDelegate;
class MessageDelegate;

namespace Ui {
class MessageWindow;
}

// 单个聊天对话框
class MessageWindow : public QWidget {
    Q_OBJECT

public:
    explicit MessageWindow(QWidget *parent = nullptr);
    // explicit MessageWindow(const std::shared_ptr<ContactData::ItemData>&contact_data, Messages messages, QWidget *parent = nullptr);
    ~MessageWindow();

    void set_contact(const std::shared_ptr<ContactData::ItemData>& contact_data);
    std::shared_ptr<ContactData::ItemData> get_contact() const;

    // get_local_avatar_id_pixmap 根据文件 ID 在本地查找文件并加载为头像图片
    std::shared_ptr<QPixmap> get_local_avatar_id_pixmap(const QString&avatar_id);

    // show_avatar 当前窗口更新头像显示，无网络及校验功能
    void show_avatar(const std::shared_ptr<QPixmap>&avatar_pixmap);
    // 传入头像 ID，自动从本地或网络加载头像
    void show_avatar_id(const QString&avatar_id);

    // 添加聊天内容到对话内
    bool add_message(const std::shared_ptr<MessageData::ItemData>&message_data);
    // 获取最后一条消息以便列表显示
    const std::shared_ptr<MessageData::ItemData> get_last_message();
    void update_message(const std::shared_ptr<MessageData::ItemData>&message_data);
    std::shared_ptr<MessageData::ItemData> get_message(const QString&message_id) const;

    // void update_messages(Messages messages);
    // void append_message (Message  message );
    // void update_ui();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

public Q_SLOTS:
    void slot_button_title_avatar();
    void slot_button_detail      ();
    void slot_button_emoji       ();
    void slot_button_file        ();
    void slot_button_screenshot  ();
    void slot_button_voice       ();
    void slot_button_video       ();
    void slot_button_send        ();
private slots:
    void slot_show_context_menu(const QPoint &viewportPos);
    void slot_open_link  (const QString &href);
    void slot_open_file  (const std::shared_ptr<MessageData::ItemData> &message_data);
    void slot_open_folder(const std::shared_ptr<MessageData::ItemData> &message_data);
    void slot_open_data  (const std::shared_ptr<MessageData::ItemData> &message_data);
    void slot_copy_data  (const std::shared_ptr<MessageData::ItemData> &message_data);
    void slot_copy_file  (const std::shared_ptr<MessageData::ItemData> &message_data);
    void slot_copy_path  (const std::shared_ptr<MessageData::ItemData> &message_data);
    void slot_save_to    (const std::shared_ptr<MessageData::ItemData> &message_data);
private slots:
    void slot_update_contact_info(const std::shared_ptr<ContactData::ItemData>& to_contact_data);

signals:
    void signal_new_message(const std::shared_ptr<MessageData::ItemData>&message);

private:
    std::shared_ptr<Ui::MessageWindow    > m_ui;
    std::shared_ptr<ContactData::ItemData> m_contact_data;
    std::shared_ptr<ContactWindow        > m_contact_window;  // 当前显示的联系人详情窗口
    std::shared_ptr<MessageModel         > m_message_model;
    std::shared_ptr<MessageDelegate      > m_message_delegate;
    bool enable_return_send = true;  // 按回车键发送消息
};
