#pragma once

#include "headers.h"
#include "Utils.h"
#include "Defines.h"

class ItemDelegate;

// Q_OBJECT 不支持在模板中使用
// MVC 中 model 可用来存数据，view 用于显示模式，delegate 用来显示具体的项目
class ItemModel : public QAbstractListModel {
    Q_OBJECT

public:
    using QAbstractListModel::QAbstractListModel;

public:
    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::UserRole + 1) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    // ====================================================================== //
    // 添加数据
    void add(const std::shared_ptr<ItemDelegate>&item);
    // 移除数据
    void remove(const std::shared_ptr<ItemDelegate>&item);
    // 清空或替换
    void reset(const std::vector<std::shared_ptr<ItemDelegate>>&items = std::vector<std::shared_ptr<ItemDelegate>>{});
    // 获取数据组
    const std::vector<std::shared_ptr<ItemDelegate>>& get() const;
    // 获取位置所在数据
    std::shared_ptr<ItemDelegate> get(int pos) const;
    // 替换指定位置的元素
    bool replace(int pos, std::shared_ptr<ItemDelegate>&provider);
    // 获取数据所在位置
    int get(const std::shared_ptr<ItemDelegate>&item_providers) const;
private:
    // 获取数据组  // get_ref();
    std::vector<std::shared_ptr<ItemDelegate>>& get();
private:
    // 修改数据频繁，建议使用 vector 而不是 list
    std::vector<std::shared_ptr<ItemDelegate>> m_items;
};
