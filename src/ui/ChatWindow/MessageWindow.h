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
    ~MessageWindow();

    void set_contact(std::shared_ptr<ContactData::ItemData> contact_data);
    std::shared_ptr<ContactData::ItemData> get_contact() const;

    // 传入头像 ID，自动从本地或网络加载头像
    void show_avatar_id(const QString& avatar_id, bool online_download);
    // show_avatar 当前窗口更新头像显示，无网络及校验功能
    void show_avatar_image(std::shared_ptr<QImage> avatar_image);
    // void update_self_avatar_messages(const QString& file_id);

    // 添加聊天内容到对话内
    bool add_message(std::shared_ptr<MessageData::ItemData> message_data);
    std::shared_ptr<MessageData::ItemData> get_message(const QString& message_id) const;
    // 获取最后一条消息以便列表显示
    std::shared_ptr<MessageData::ItemData> get_last_message();

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
    void slot_open_file  (std::shared_ptr<MessageData::ItemData> message_data);
    void slot_open_folder(std::shared_ptr<MessageData::ItemData> message_data);
    void slot_open_data  (std::shared_ptr<MessageData::ItemData> message_data);
    void slot_copy_data  (std::shared_ptr<MessageData::ItemData> message_data);
    void slot_copy_file  (std::shared_ptr<MessageData::ItemData> message_data);
    void slot_copy_path  (std::shared_ptr<MessageData::ItemData> message_data);
    void slot_save_to    (std::shared_ptr<MessageData::ItemData> message_data);
private slots:
    // 修改个人资料或头像是需要同时修改消息中的个人信息及头像(好友或自己均可)
    void slot_update_message_contact   (std::shared_ptr<ContactData::ItemData> contact_data);
    // 更新好友信息
    void slot_update_contact_info      (std::shared_ptr<ContactData::ItemData> to_contact_data);
    void slot_update_button_avatar_file(const Network::FileDownloadNotify notify);  // 更新窗口中的头像按钮
    void slot_update_message_file      (const Network::FileDownloadNotify notify);  // 更新文件消息/头像

private:
    std::shared_ptr<Ui::MessageWindow    > m_ui;
    std::shared_ptr<ContactData::ItemData> m_contact_data;
    std::shared_ptr<ContactWindow        > m_contact_window;  // 当前显示的联系人详情窗口
    std::shared_ptr<MessageModel         > m_message_model;
    std::shared_ptr<MessageDelegate      > m_message_delegate;
    bool enable_return_send = true;  // 按回车键发送消息
};
