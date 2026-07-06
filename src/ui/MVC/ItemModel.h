#pragma once

#include "headers.h"
#include "Utils.h"
#include "Defines.h"

class ItemDelegate;

// 节点数据
class TreeItemData{
public:
    // 默认构造函数
    // 可用于设置数据列数
    TreeItemData(int column_count = 0) {
        if(column_count > 0) {
            set_data_columns(std::vector<std::map<int, QVariant>>(column_count));
        }
    }

    // 单列表，每单元格可保存多种角色的数据
    TreeItemData(const std::map<int, QVariant>& data_column) {
        set_data_columns(std::vector<std::map<int, QVariant>>{{data_column}});
    }

    // 单列表，每单元格默认保存显示角色的数据
    TreeItemData(const QVariant& data_column) {
        TreeItemData(std::map<int, QVariant>{{Qt::DisplayRole, data_column}});  // 添加一列数据
    }

    // 析构函数：递归释放子节点内存，防止内存泄漏
    ~TreeItemData() {
        auto&item_childs = rw_item_childs();
        // 子节点的父节点就是当前节点，因此不必写父节点清理代码
        for (auto item_child : item_childs) {
            // 释放子节点的资源
            delete item_child;
            item_child = nullptr;
        }
    }

    // 获取当前节点在父节点中的索引位置
    int row_index() const {
        // 查找符合条件的节点函数
        auto cmp = [this](const TreeItemData* item) {
            return item == this;
        };
        // 有父节点，索引才有效
        auto item_parent = get_item_parent();
        if (!item_parent) { return 0; }
        auto& item_childs = item_parent->get_item_childs();
        auto item_find = std::find_if(item_childs.begin(), item_childs.end(), cmp);
        return (item_find != item_childs.end()) ? std::distance(item_childs.begin(), item_find) : 0;
    }

    // 将传入节点添加为子节点  // 允许重复
    TreeItemData* add_child(TreeItemData* child) {
        if(!child) { return nullptr; }
        child->set_item_parent(this);
        this->rw_item_childs().emplace_back(child);
        // 如果有子节点，那么至少需要显示一列数据，否则树无法显示  // 仅在当前节点为根节点时设置
        if (!get_item_parent() && get_data_columns().size() <= 0) { set_data_columns(std::vector<std::map<int, QVariant>>(1)); }
        return child;
    }

    // 将字符串数组添加为多个子节点  // 允许重复
    bool add_childs(const QStringList& strings) {
        for(auto&str : strings) {
            add_child(new TreeItemData(str));
        }
        return true;
    }

    // 设置指定单元格的数据，可同时设置多种角色的数据  // 不新增列，单元格不存在则失败
    bool set_column_data(int column, const std::map<int, QVariant>& colum_data) {
        if(column < 0 || column > get_data_columns().size()) { return false; }
        rw_data_columns().at(column) = colum_data;
        return true;
    }

    // 设置指定单元格的数据，一次仅设置一种角色的数据  // 不新增列，单元格不存在则失败
    bool set_column_data(int column, int role, const QVariant&data) {
        std::map<int, QVariant> column_data;
        column_data[role] = data;
        return set_column_data(column, column_data);
    }

    // 删除指定值的元素，并返回删除的数量  // 不释放子节点的资源
    int remove_child(const TreeItemData*item_child) {
        if (!item_child) { return 0; }
        // 删除指定值的元素
        return static_cast<int>(std::erase(rw_item_childs(), item_child));
        // 删除指定条件的元素
        return static_cast<int>(std::erase_if(rw_item_childs(), [item_child](const TreeItemData* item_ite) { return item_ite == item_child; }));
    }

public:
    auto get_item_parent () const -> const             TreeItemData*                          { return m_item_parent ; }
    auto get_item_childs () const -> const std::vector<TreeItemData*          >&              { return m_item_childs ; }
    auto get_data_columns() const -> const std::vector<std::map<int, QVariant>>&              { return m_data_columns; }
    void set_item_parent (           const             TreeItemData*            item_parent ) { m_item_parent  = const_cast<TreeItemData*>(item_parent) ; }
    void set_item_childs (           const std::vector<TreeItemData*          > item_childs ) { m_item_childs  = item_childs ; }
    void set_data_columns(           const std::vector<std::map<int, QVariant>> data_columns) { m_data_columns = data_columns; }
private:
    auto rw_item_parent () ->             TreeItemData*             { return m_item_parent ; }
    auto rw_item_childs () -> std::vector<TreeItemData*          >& { return m_item_childs ; }
    auto rw_data_columns() -> std::vector<std::map<int, QVariant>>& { return m_data_columns; }
private:
    TreeItemData*                        m_item_parent  = nullptr;  // 父节点
    std::vector<TreeItemData*          > m_item_childs ;  // 子节点
    std::vector<std::map<int, QVariant>> m_data_columns;  // 子列数据 // vec{role,var}
};

// Q_OBJECT 不支持在模板中使用
// MVC 中 model 可用来存数据，view 用于显示模式，delegate 用来显示具体的项目
class TreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    using QAbstractItemModel::QAbstractItemModel;
    ~TreeModel() {
        if(m_data_root) {
            delete m_data_root;
            m_data_root = nullptr;
        }
    }

protected:
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent     (const QModelIndex &child) const override;
    virtual int         rowCount   (const QModelIndex &parent = QModelIndex()) const override;
    virtual int         columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant    data       (const QModelIndex &index, int role = Qt::DisplayRole) const override;

public:
    const TreeItemData* data_root() const { return m_data_root; }
protected:
    TreeItemData* data_root() { return m_data_root; }
private:
    TreeItemData* m_data_root = new TreeItemData;
};

class ItemModel : public TreeModel {
public:
    using TreeModel::TreeModel;

public:
#ifdef _DEBUG
    ItemModel() {
        test();
    }
    void test() {
        {
            auto item_group = data_root()->add_child(new TreeItemData({{Qt::DisplayRole, QVariant("hello world! group!")}, {Qt::EditRole, QVariant("how are you? group!")}}));
            auto item_member = item_group->add_child(new TreeItemData({{Qt::DisplayRole, QVariant("hello world! member!")}, {Qt::EditRole, QVariant("how are you? member!")}}));
            item_member->add_child(new TreeItemData({{Qt::DisplayRole, QVariant("hello world! infos!")}, {Qt::EditRole, QVariant("how are you? infos!")}}));
        }
        {
            auto item_group = data_root()->add_child(new TreeItemData({{Qt::DisplayRole, QVariant("hello world! group!")}, {Qt::EditRole, QVariant("how are you? group!")}}));
            auto item_member = item_group->add_child(new TreeItemData({{Qt::DisplayRole, QVariant("hello world! member!")}, {Qt::EditRole, QVariant("how are you? member!")}}));
            item_member->add_child(new TreeItemData({{Qt::DisplayRole, QVariant("hello world! infos!")}, {Qt::EditRole, QVariant("how are you? infos!")}}));
        }
    }
#endif // _DEBUG

// public:
//     void add_group() {
//         ;
//     }
//     void add_member() {
//         ;
//     }
//     // ====================================================================== //
//     // 添加数据
//     void add(const std::shared_ptr<ItemDelegate>&item);
//     // 移除数据
//     void remove(const std::shared_ptr<ItemDelegate>&item);
//     // 清空或替换
//     void reset(const std::vector<std::shared_ptr<ItemDelegate>>&items = std::vector<std::shared_ptr<ItemDelegate>>{});
//     // 获取数据组
//     const std::vector<std::shared_ptr<ItemDelegate>>& get() const;
//     // 获取位置所在数据
//     std::shared_ptr<ItemDelegate> get(int pos) const;
//     // 替换指定位置的元素
//     bool replace(int pos, std::shared_ptr<ItemDelegate>&provider);
//     // 获取数据所在位置
//     int get(const std::shared_ptr<ItemDelegate>&item_providers) const;
// private:
//     // 获取数据组  // get_ref();
//     std::vector<std::shared_ptr<ItemDelegate>>& get();
// private:
//     // 修改数据频繁，建议使用 vector 而不是 list
//     std::vector<std::shared_ptr<ItemDelegate>> m_items;
};
