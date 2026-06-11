#pragma once

#include "headers.h"
#include "Utils.h"

class DemoContactDelegateItem;
class ListDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit ListDelegate(QObject *parent = nullptr);
    ~ListDelegate();

public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    // 无法在绘图函数中传出参数，需要预先计算高度
    void set_height(int height) { m_height = height; }
    int get_height() const { return m_height; }

private:
    int m_height = 80;
    // 缓存动态的数据内容  // TODO:缓存内容的更新使用专属的接口
    struct UserData{
        bool need_update = true; // 是否需要更新数据
        std::shared_ptr<ContactInfo> contact_info;
        QString account_id;
        QString user_id;
        QString avatar_id;
        QString nickname;
        QString signature;
        QString message;
        std::shared_ptr<QPixmap> pixmap_avatar;
    };
    // 加上 mutable，允许在 const 函数（如 paint）中修改缓存
    mutable std::map<QString, std::shared_ptr<UserData>> m_cache_account_data;
    std::shared_ptr<DemoContactDelegateItem> m_demo_contact_delegate_item;
};

// 无特殊设置时一个 QAbstractListModel 只会创建一个 ContactDelegate，再轮流调用 paint 对每一行数据进行绘制
// 但也可以：1.为不同的行/列设置了不同的 Delegate; 2.多栏视图（如 QTreeView 或 QTableView）为不同的“列（Column）”设置不同的 Delegate
