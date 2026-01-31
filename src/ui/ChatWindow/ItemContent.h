#pragma once

#include <QWidget>

// 联系人列表项数据结构
struct ItemContentData {
    QString avatar;    // 头像路径或标识
    QString nickname;  // 昵称
};

namespace Ui {
class ItemContent;
}

class ItemContent : public QWidget {
    Q_OBJECT

public:
    explicit ItemContent(QWidget *parent = nullptr);
    explicit ItemContent(const ItemContentData data, QWidget *parent = nullptr);
    ~ItemContent();

private:
    void update_data(const ItemContentData &data);
    void update_ui();

private:
    Ui::ItemContent *ui;
    ItemContentData m_data;
};
