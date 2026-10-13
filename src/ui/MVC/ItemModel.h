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
        if (column_count > 0) {
            set_data_columns(std::vector<std::map<int, QVariant>>(column_count));
        }
    }

    // 单列表，每单元格默认保存显示角色的数据
    TreeItemData(const QVariant& data_column) {
        TreeItemData(std::map<int, QVariant>{{Qt::DisplayRole, data_column}});  // 添加一列数据
    }

    // 单列表，每单元格可保存多种角色的数据
    TreeItemData(const std::map<int, QVariant>& data_column) {
        set_data_columns(std::vector<std::map<int, QVariant>>{{data_column}});
    }

    // 单列表，每单元格默认保存显示角色的数据
    TreeItemData(int column, const std::map<int, QVariant>& colum_data) {
        set_column_data(column, colum_data);
    }

    // 单列表，每单元格默认保存显示角色的数据
    TreeItemData(int column, int role, const QVariant&data) {
        set_column_data(column, role, data);
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

    // 将传入节点添加为子节点  // 允许重复  // 插入到末尾
    TreeItemData* add_child(TreeItemData* child) {
        if (!child) { return nullptr; }
        child->set_item_parent(this);
        this->rw_item_childs().emplace_back(child);
        // 如果有子节点，那么至少需要显示一列数据，否则树无法显示  // 仅在当前节点为根节点时设置
        if (!get_item_parent() && get_data_columns().size() <= 0) { set_data_columns(std::vector<std::map<int, QVariant>>(1)); }
        return child;
    }

    // 将传入节点添加为子节点  // 允许重复  // 插入到指定位置
    TreeItemData* insert_child(int pos, TreeItemData* child) {
        auto&rw = this->rw_item_childs();
        if (pos < 0 || pos > rw.size()) { return nullptr; }
        if (!child) { return nullptr; }
        child->set_item_parent(this);
        rw.insert(rw.begin() + pos, child);
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

    std::map<int, QVariant> get_column_data(int column) {
        if (column >= get_data_columns().size()) { return std::map<int, QVariant>{}; }
        return get_data_columns()[column];
    }

    // 设置指定单元格的数据，可同时设置多种角色的数据  // 会重置 colum_data
    bool set_column_data(int column, const std::map<int, QVariant>& colum_data) {
        // if (column < 0 || column > get_data_columns().size()) { return false; }
        if (column > get_data_columns().size() + 1) {
            rw_data_columns().reserve(column + 1);
        }
        for(size_t idx = get_data_columns().size(); idx <= column; ++idx) {
            rw_data_columns().emplace_back(std::map<int, QVariant>());
        }
        rw_data_columns().at(column) = colum_data;
        return true;
    }

    // 设置指定单元格的数据，一次仅设置一种角色的数据  // 仅重置 role 的 data
    bool set_column_data(int column, int role, const QVariant&data) {
        if (column > get_data_columns().size() + 1) {
            rw_data_columns().reserve(column + 1);
        }
        for(size_t idx = get_data_columns().size(); idx <= column; ++idx) {
            rw_data_columns().emplace_back(std::map<int, QVariant>());
        }
        // 先读取已经存在的记录再修改
        std::map<int, QVariant> column_data = get_column_data(column);
        column_data[role] = data;
        return set_column_data(column, column_data);
    }

    // 删除指定值的元素，并返回删除的数量  // 不释放子节点的资源
    int remove_child(const TreeItemData*item_child) {
        if (!item_child) { return 0; }
        auto it = std::find(rw_item_childs().begin(), rw_item_childs().end(), item_child);
        if (it == rw_item_childs().end()) { return 0; }
        // 父节点要置空，否则父节点删除但节点指针未更新
        const_cast<TreeItemData*>(item_child)->set_item_parent(nullptr);
        rw_item_childs().erase(it);
        return 1;
        // // 删除指定值的元素
        // return static_cast<int>(std::erase(rw_item_childs(), item_child));
        // // 删除指定条件的元素
        // return static_cast<int>(std::erase_if (rw_item_childs(), [item_child](const TreeItemData* item_ite) { return item_ite == item_child; }));
    }

public:
    auto row_count() { return m_item_childs.size(); }
    auto row_data(int row) { return m_item_childs.at(row); }

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
        if (m_data_root) {
            delete m_data_root;
            m_data_root = nullptr;
        }
    }

// protected:
public:
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent     (const QModelIndex &child) const override;
    virtual int         rowCount   (const QModelIndex &parent = QModelIndex()) const override;
    virtual int         columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant    data       (const QModelIndex &index, int role = Qt::DisplayRole) const override;

public:
    const TreeItemData* data_root() const { return m_data_root; }
    void clean() { delete m_data_root; m_data_root = new TreeItemData; }
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

struct IndexData {
    QModelIndex index;
    TreeItemData* data = nullptr;
    std::shared_ptr<ContactData::ItemData> contact = nullptr;
    int row = -1;
};

// 联系人列表
class ContactModel : public TreeModel {
public:
    using TreeModel::TreeModel;
// #ifdef _DEBUG
//     ContactModel() {
//         for(int i = 0; i <= 300; ++i) {
//             auto contact = std::make_shared<ContactData::ItemData>();
//             auto num = QString("%1").arg(i, 5, 10, '0');
//             auto time = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
//             contact->mutBaseInfo().setAccountId(QString("Account ID : %1 (%2)").arg(num).arg(time));
//             contact->mutBaseInfo().setUserId   (QString("User ID    : %1 (%2)").arg(num).arg(time));
//             contact->mutBaseInfo().setNickname (QString("Nickname   : %1 (%2)").arg(num).arg(time));
//             contact->mutBaseInfo().setSignature(QString("Signature  : %1 (%2)").arg(num).arg(time));
//             add_member(ENUMS::ItemGroupType::REQUEST, contact);
//         }
//     }
// #endif // __DEBUG

public:

    // 返回添加的分组节点  // 分组不重复
    auto add_group(ENUMS::ItemGroupType group_type) -> TreeItemData* {
        // return data_root()->add_child(new TreeItemData(0, ENUMS::role_item_data(), QVariant::fromValue<ENUMS::ItemGroupType>(group_type)));
        if (0) {
            auto item_group = new TreeItemData;
            // std::vector<std::map<int, QVariant>> data_column;
            std::map<int, QVariant> data_item;
            data_item[ENUMS::role_item_data()] = QVariant::fromValue<ENUMS::ItemGroupType>(group_type);
            data_item[Qt::DisplayRole ] = QVariant("hello world! group DisplayRole!");
            data_item[Qt::EditRole    ] = QVariant("hello world! group EditRole!");
            // data_column.push_back(data_item);
            item_group->set_column_data(0, data_item);
            data_root()->add_child(item_group);
            // return item_group;
        }

        auto text_display = QMetaEnum::fromType<ENUMS::ItemGroupType>().valueToKey(static_cast<int>(group_type));
        std::map<int, QVariant> var_item;
        var_item[ENUMS::role_item_type()] = QVariant::fromValue<ENUMS::ItemDataType>(ENUMS::ItemDataType::GROUP);
        var_item[ENUMS::role_item_data()] = QVariant::fromValue<ENUMS::ItemGroupType>(group_type);
        // 设置一些默认显示的数据，防止未设置 delegete 显示为空
        var_item[Qt::DisplayRole   ] = text_display;
        var_item[Qt::DecorationRole] = icon_init(); // 设置默认图标样式
        var_item[Qt::EditRole      ] = text_display;
        // var_item[Qt::ToolTipRole  ] = text_display;
        // var_item[Qt::StatusTipRole] = text_display;  // 效果不好
        var_item[Qt::WhatsThisRole ] = text_display;
        // 为了展示不同类型，给群组设置背景颜色
        // var_item[Qt::BackgroundRole] = QVariant::fromValue<QColor>(Qt::darkCyan);
        // var_item[Qt::ForegroundRole] = QVariant::fromValue<QColor>(Qt::darkMagenta);

        auto row = static_cast<int>(data_root()->get_item_childs().size());
        beginInsertRows(QModelIndex(), row, row);
        auto item_group = data_root()->add_child(new TreeItemData(get_data_column(), var_item));
        endInsertRows();
        return item_group;
    }

    // 返回添加的成员节点  // 1.分组不重复；2.contact 必须有 account_id 且同样不允许重复
    auto add_member(ENUMS::ItemGroupType group_type, const std::shared_ptr<ContactData::ItemData>&contact_data) -> TreeItemData* {
        // ============================ 过滤重复 ============================ //

        if (!contact_data) { return nullptr; }
        if ( contact_data->account_id.isEmpty()) { return nullptr; }

        // 能找到说明已经添加
        auto index_data = get_contact_index_data(group_type, contact_data->account_id);
        if (index_data->contact) { return nullptr; }

        // ============================ 添加成员 ============================ //

        // 获取分组节点
        auto row = row_group(group_type);
        TreeItemData*item_group = row == -1 ? nullptr : data_root()->get_item_childs().at(row);
        // 没有该分组时添加该分组节点
        if (!item_group) {
            item_group = add_group(group_type);
        }

        // 分组节点添加成员  // 未校验重复
        auto text_display = contact_data->account_id;  // for DEBUG
        // Qt::DecorationRole1The : data to be rendered as a decoration in the form of an icon. (QColor, QIcon or QPixmap)
        QPixmap pixmap = contact_data->avatar_image.isNull() ? icon_init().pixmap(128) : QPixmap::fromImage(contact_data->avatar_image);
        std::map<int, QVariant> var_item;
        var_item[ENUMS::role_item_type()] = QVariant::fromValue<ENUMS::ItemDataType>(ENUMS::ItemDataType::CONTACT);
        var_item[ENUMS::role_item_data()] = QVariant::fromValue<std::shared_ptr<ContactData::ItemData>>(contact_data);
        // 设置一些默认显示的数据，防止未设置 delegete 显示为空
        var_item[Qt::DisplayRole   ] = text_display;
        var_item[Qt::DecorationRole] = pixmap; // 设置默认图标样式，实际不使用
        var_item[Qt::EditRole      ] = text_display;
        // var_item[Qt::ToolTipRole  ] = text_display;
        // var_item[Qt::StatusTipRole] = text_display;  // 效果不好
        var_item[Qt::WhatsThisRole ] = text_display;

        // 查找视图中的插入位置  // 判断条件是先判断昵称，昵称相同再判断用户 ID
        auto func_find_pos = [](const std::vector<TreeItemData*>&childs, const std::shared_ptr<ContactData::ItemData>&contact_data) -> int {
            int pos = -1;  // -1：不插入，0：置顶插入
            do {
                // 无效值不插入
                if (!contact_data) {
                    pos = -1;
                    break;
                }

                // 没有可比较的数据时
                if (contact_data->display_name.isEmpty() && contact_data->user_id.isEmpty()) {
                    pos = 0;  // 插入到顶部
                    break;
                }

                // 空值前置  // 插入位置在 ite 前
                auto ite = childs.begin();
                for (; ite != childs.end(); ++ite) {
                    // 跳过已有的无效值
                    const TreeItemData*tree_item_data = *ite;
                    if (!tree_item_data) { continue; }

                    for (auto&data_column : tree_item_data->get_data_columns()) {
                        using T = std::shared_ptr<ContactData::ItemData>;
                        // 不是目标数据
                        if (!data_column.contains(ENUMS::role_item_data())) { continue; }
                        // 获取目标数据容器
                        const QVariant&var_column = data_column.at(ENUMS::role_item_data());
                        if (!var_column.isValid()) { continue; }
                        if (!var_column.canConvert<T>()) { continue; }
                        // 对比目标数据容器内数据
                        T val = var_column.value<T>();
                        // if (var_column.value<T>() == group_type) { return true; }
                        if (!val) { continue; }
                        // 使用 Qt 的 localeAwareCompare 实现支持中文拼音/本地化排序的对比升序排序
                        // localeAwareCompare(left, right), left > right -> 1
                        int rdn  = QString::localeAwareCompare(contact_data->display_name, val->display_name);
                        int ruid = QString::localeAwareCompare(contact_data->user_id     , val->user_id     );
                        int raid = QString::localeAwareCompare(contact_data->account_id  , val->account_id  );
                        // 昵称一样时才比较用户 ID  // 插入到 ite 前面
                        if (rdn < 0 || (rdn == 0 && ruid < 0) || (rdn == 0 && ruid == 0 && raid < 0)) {
                            pos = static_cast<int>(std::distance(childs.begin(), ite));
                            break;
                        }
                    }
                    if (pos != -1) {
                        break;  // 找到插入位置时跳出循环
                    }
                }
                // 循环结束时未找到可插入节点前的位置时，则排序时应该插入到列表尾部
                if (pos == -1) {
                    // pos = childs.size() - 1;
                    pos = static_cast<int>(std::distance(childs.begin(), ite));
                }
            } while (false);
            return pos;
        };

        int pos_insert = static_cast<int>(item_group->get_item_childs().size()); // 默认不排序
        // 判断分组，查找插入位置
        // 好友群组需要排序，好友申请列表不需要排序
        if (group_type == ENUMS::ItemGroupType::FRIENDS) {
            pos_insert = func_find_pos(item_group->get_item_childs(), contact_data);
            qDebug().noquote().nospace() << "insert pos : " << pos_insert << ", contact display_name : " << contact_data->display_name;
        }
        if (pos_insert < 0) { return nullptr; }

        auto row_item = static_cast<int>(pos_insert);
        const QModelIndex parent_index = index_group(group_type);
        beginInsertRows(parent_index, row_item, row_item);
        // 需要支持按位置插入
        auto item_member = item_group->insert_child(pos_insert, new TreeItemData(get_data_column(), var_item));
        endInsertRows();
        return item_member;
    }

    QModelIndex index_group(ENUMS::ItemGroupType group_type) {
        auto row = row_group(group_type);
        return row == -1 ? QModelIndex() : createIndex(row, 0, data_root()->get_item_childs().at(row));
        return QModelIndex();
    }

    QModelIndex index_contact(ENUMS::ItemGroupType group_type, const QString&account_id) {
        auto index_data = get_contact_index_data(group_type, account_id);
        return index_data->index;
    }

    // 用于支持群组节点无子节点时展开
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override {
        if (!parent.isValid()) {
            return rowCount(parent) > 0;
        }
        QVariant var_type = parent.data(ENUMS::role_item_type());
        if (var_type.isValid() && var_type.canConvert<ENUMS::ItemDataType>()) {
            if (var_type.value<ENUMS::ItemDataType>() == ENUMS::ItemDataType::GROUP) {
                return true;
            }
        }
        return rowCount(parent) > 0;
    }

    int row_group(ENUMS::ItemGroupType group_type) {
        // ========================== 查找群组节点 ========================== //

        // 1.C++ 11 基础写法
        auto get_item_group_cxx11 = [this](ENUMS::ItemGroupType group_type){
            TreeItemData*item_group = nullptr;
            for(auto*item_ite : data_root()->get_item_childs()) {
                // 无效节点
                if (!item_ite) { continue; }
                // 无数据节点
                auto data_columns = item_ite->get_data_columns();
                if (data_columns.size() <= 0) { continue; }
                //
                for(auto&data_column : data_columns) {
                    // 不是目标数据
                    if (!data_column.contains(ENUMS::role_item_data())) { continue; }
                    // 获取目标数据容器
                    const QVariant&var_column = data_column.at(ENUMS::role_item_data());
                    if (!var_column.canConvert<ENUMS::ItemGroupType>()) { continue; }
                    // 对比目标数据容器内数据
                    if (var_column.value<ENUMS::ItemGroupType>() == group_type) {
                        item_group = item_ite;
                        break;
                    }
                }
                if (item_group) { break; }
            }
            // // 添加分组
            // if (!item_group) {
            //     item_group = add_group(group_type);
            // }
            return item_group;
            // return item_group->add_child(new TreeItemData(0, ENUMS::role_item_data(), QVariant::fromValue<std::shared_ptr<ContactData::ItemData>>(contact)));
        };
        Q_UNUSED(get_item_group_cxx11);

        // 2.C++ 20 std::views::filter 写法
        auto func_get_item_check_by_ranger = [group_type](const std::map<int, QVariant>&data_column) {
            using T = ENUMS::ItemGroupType;
            // 不是目标数据
            if (!data_column.contains(ENUMS::role_item_data())) { return false; }
            // 获取目标数据容器
            const QVariant&var_column = data_column.at(ENUMS::role_item_data());
            if (!var_column.canConvert<T>()) { return false; }
            // 对比目标数据容器内数据
            T val = var_column.value<T>();
            if (val == group_type) { return true; }
            return false;
        };
        auto func_get_item_cmp_by_ranger = [func_get_item_check_by_ranger](const TreeItemData* item_ite) -> bool {
            // 无效节点
            if (!item_ite) { return false; }
            // 无数据节点
            auto data_columns = item_ite->get_data_columns();
            if (data_columns.empty()) { return false; }
            // 验证数据
            auto vec_filter = data_columns | std::views::filter(func_get_item_check_by_ranger);

            // size() 可能需要完整遍历
            // if (vec_filter.size() > 0) { return true; }

            // empty() 具备隐式短路特性：找到第一个匹配项就会停下。
            return !vec_filter.empty();
        };
        auto func_get_item_by_ranger = [this, func_get_item_cmp_by_ranger](ENUMS::ItemGroupType group_type) {
            Q_UNUSED(group_type);
            auto vec_item_groups = data_root()->get_item_childs() | std::views::filter(func_get_item_cmp_by_ranger);
            return vec_item_groups.empty() ? nullptr : vec_item_groups.front();
        };
        Q_UNUSED(func_get_item_by_ranger);

        // 3.std::ranges::find_if + std::views::filter 写法
        auto func_get_item_check_by_findif = [group_type](const std::map<int, QVariant>&data_column) {
            return data_column.contains(ENUMS::role_item_data()) && data_column.at(ENUMS::role_item_data()).value<ENUMS::ItemGroupType>() == group_type;
        };
        auto func_get_item_cmp_by_findif = [func_get_item_check_by_findif](const TreeItemData* item_ite) {
            if (!item_ite) { return false; }
            auto vec_filter = item_ite->get_data_columns() | std::views::filter(func_get_item_check_by_findif);
            return !vec_filter.empty();
        };
        auto func_get_row_by_findif = [this, func_get_item_cmp_by_findif](ENUMS::ItemGroupType group_type) -> int {
            Q_UNUSED(group_type);
            auto& childs = data_root()->get_item_childs();
            auto it = std::ranges::find_if(childs, func_get_item_cmp_by_findif);
            return (it == childs.end()) ? -1 : std::distance(childs.begin(), it);  // 返回 row
        };
        auto row = func_get_row_by_findif (group_type);
        return row;
    }

    bool has_group(ENUMS::ItemGroupType group_type) {
        return row_group(group_type) != -1;
    }

    // 更新信息
    bool update_contact(ENUMS::ItemGroupType group_type, const std::shared_ptr<ContactData::ItemData>&contact_data) {
        if (!contact_data) { return false; }
        if ( contact_data->account_id.isEmpty()) { return false; }
        QModelIndex contact_index = index_contact(group_type, contact_data->account_id);
        if (!contact_index.isValid()) { return false; }
        // 获取节点内部的 TreeItemData* 指针并更新存储数据
        TreeItemData* item_data = static_cast<TreeItemData*>(contact_index.internalPointer());
        if (item_data) {
            QString text_display = contact_data->display_name; // Utils::get_display_name(contact_data);
            QPixmap pixmap = contact_data->avatar_image.isNull() ? icon_init().pixmap(128) : QPixmap::fromImage(contact_data->avatar_image);
            std::map<int, QVariant> var_item;
            var_item[ENUMS::role_item_type()] = QVariant::fromValue<ENUMS::ItemDataType>(ENUMS::ItemDataType::CONTACT);
            var_item[ENUMS::role_item_data()] = QVariant::fromValue<std::shared_ptr<ContactData::ItemData>>(contact_data);
            // 设置一些默认显示的数据，防止未设置 delegete 显示为空
            var_item[Qt::DisplayRole   ] = text_display;
            var_item[Qt::DecorationRole] = pixmap; // 设置默认图标样式，实际不使用
            var_item[Qt::EditRole      ] = text_display;
            // var_item[Qt::ToolTipRole  ] = text_display;
            // var_item[Qt::StatusTipRole] = text_display;  // 效果不好
            var_item[Qt::WhatsThisRole ] = text_display;
            // 设置回节点数据列中
            item_data->set_column_data(0, var_item);
        }
        emit dataChanged(contact_index, contact_index);
        return true;
    }

    // 这里的返回值不能使用引用，引用无法延长临时变量的生命周期，会导致被调用时必然崩溃
    std::shared_ptr<ContactData::ItemData> get_contact_data(ENUMS::ItemGroupType group_type, const QString&account_id) {
        auto index_data = get_contact_index_data(group_type, account_id);
        return index_data->contact;
    }
    // 生成一个占位图标，否则获取图标大小会异常
    QIcon icon_init() const {
        static auto icons = Utils::FontIcons().update();
        return icons.icon.contacts;
    }
private:
    std::shared_ptr<IndexData> get_contact_index_data(ENUMS::ItemGroupType group_type, const QString&account_id) {
        auto index_data = std::make_shared<IndexData>();
        do {
            if ( account_id.isEmpty()) { break; }
            int group_row = row_group(group_type);
            if (group_row < 0) { break; }
            const auto&group_index = index_group(group_type);  // 群组节点
            if (!group_index.isValid()) { break; }
            auto group_item = data_root()->get_item_childs().at(group_row);  // 群组数据
            if (!group_item) { break; }
            const auto& childs = group_item->get_item_childs();  // 群组子节点列表
            for (size_t row = 0; row < childs.size(); ++row) {
                index_data->data = childs.at(row);
                if (!index_data->data) { continue; }
                for (const auto& data_column : index_data->data->get_data_columns()) {
                    using T = std::shared_ptr<ContactData::ItemData>;
                    // 不是目标数据
                    if (!data_column.contains(ENUMS::role_item_data())) { continue; }
                    // 获取目标数据容器
                    const QVariant&var_column = data_column.at(ENUMS::role_item_data());
                    if (!var_column.isValid()) { continue; }
                    if (!var_column.canConvert<T>()) { continue; }
                    // 对比目标数据容器内数据
                    index_data->contact = var_column.value<T>();
                    if (!index_data->contact) { continue; }
                    // 匹配账号 ID
                    if (account_id == index_data->contact->account_id) {
                        index_data->row = static_cast<int>(row);
                        break;
                    }
                }
                if (index_data->row != -1) { break; }
            }
            // 获取子节点在父节点中的 index
            if (index_data->row != -1) { index_data->index = this->index(index_data->row, 0, group_index); }
        } while (false);
        return index_data;
    }
};

// 搜索结果
class SearchModel : public TreeModel {
public:
    using TreeModel::TreeModel;
// #ifdef _DEBUG
//     SearchModel() {
//         for(int i = 0; i <= 300; ++i) {
//             auto contact = std::make_shared<ContactData::ItemData>();
//             auto num = QString("%1").arg(i, 5, 10, '0');
//             auto time = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
//             contact->mutBaseInfo().setAccountId(QString("Account ID : %1 (%2)").arg(num).arg(time));
//             contact->mutBaseInfo().setUserId   (QString("User ID    : %1 (%2)").arg(num).arg(time));
//             contact->mutBaseInfo().setNickname (QString("Nickname   : %1 (%2)").arg(num).arg(time));
//             contact->mutBaseInfo().setSignature(QString("Signature  : %1 (%2)").arg(num).arg(time));
//             add_member(contact);
//         }
//     }
// #endif // __DEBUG

public:

    // 返回添加的成员节点  // 1.分组不重复；2.contact 必须有 account_id 且同样不允许重复
    auto add_member(const std::shared_ptr<ContactData::ItemData> contact_data) -> TreeItemData* {
        // ============================ 过滤重复 ============================ //
        if (!contact_data) { return nullptr; }

        // 获取账号 ID 作为重复判断条件
        const auto& account_id = contact_data->account_id;
        if (account_id.isEmpty()) {
            qWarning() << "Contact account id is EMPTY!";
            return nullptr;
        }

        auto func_check_dupe = [account_id](const std::map<int, QVariant>&data_column) {
            using T = std::shared_ptr<ContactData::ItemData>;
            // 不是目标数据
            if (!data_column.contains(ENUMS::role_item_data())) { return false; }
            // 获取目标数据容器
            const QVariant&var_column = data_column.at(ENUMS::role_item_data());
            if (!var_column.canConvert<T>()) { return false; }
            // 对比目标数据容器内数据
            T val = var_column.value<T>();
            if (account_id == val->account_id) { return true; }
            return false;
        };
        auto func_cmp_dupe = [func_check_dupe](const TreeItemData* item_ite) {
            if (!item_ite) { return false; }
            auto vec_filter = item_ite->get_data_columns() | std::views::filter(func_check_dupe);
            return !vec_filter.empty();
        };
        auto func_has_dupe = [this, func_cmp_dupe]() {
            auto& childs = data_root()->get_item_childs();
            auto it = std::ranges::find_if(childs, func_cmp_dupe);
            return it != childs.end();
        };
        Q_UNUSED(func_has_dupe);

        if (func_has_dupe()) { return nullptr; }

        // ============================ 添加成员 ============================ //

        // 获取分组节点
        TreeItemData*item_group = data_root();

        // 分组节点添加成员  // 未校验重复
        auto text_display = account_id;
        std::map<int, QVariant> var_item;
        var_item[ENUMS::role_item_type()] = QVariant::fromValue<ENUMS::ItemDataType>(ENUMS::ItemDataType::CONTACT);
        var_item[ENUMS::role_item_data()] = QVariant::fromValue<std::shared_ptr<ContactData::ItemData>>(contact_data);
        // 设置一些默认显示的数据，防止未设置 delegete 显示为空
        var_item[Qt::DisplayRole   ] = text_display;
        var_item[Qt::DecorationRole] = icon_init(); // 设置默认图标样式，实际不使用
        var_item[Qt::EditRole      ] = text_display;
        // var_item[Qt::ToolTipRole  ] = text_display;
        // var_item[Qt::StatusTipRole] = text_display;  // 效果不好
        var_item[Qt::WhatsThisRole ] = text_display;

        auto row = static_cast<int>(item_group->get_item_childs().size());
        beginInsertRows(QModelIndex(), row, row);
        auto item_member = item_group->add_child(new TreeItemData(get_data_column(), var_item));
        endInsertRows();
        return item_member;
    }

    QModelIndex index_contact(const QString&account_id) {
        auto index_data = get_contact_index_data(account_id);
        return index_data->index;
    }

    // 这里的返回值不能使用引用，引用无法延长临时变量的生命周期，会导致被调用时必然崩溃
    std::shared_ptr<ContactData::ItemData> get_contact_data(const QString&account_id) {
        auto index_data = get_contact_index_data(account_id);
        return index_data->contact;
    }

    bool update_contact(const std::shared_ptr<ContactData::ItemData>&contact_data) {
        if (!contact_data) { return false; }
        if ( contact_data->account_id.isEmpty()) { return false; }
        QModelIndex contact_index = index_contact(contact_data->account_id);
        if (!contact_index.isValid()) { return false; }
        // 获取节点内部的 TreeItemData* 指针并更新存储数据
        TreeItemData* item_data = static_cast<TreeItemData*>(contact_index.internalPointer());
        if (item_data) {
            QString text_display = contact_data->display_name; // Utils::get_display_name(contact_data);
            QPixmap pixmap = contact_data->avatar_image.isNull() ? icon_init().pixmap(128) : QPixmap::fromImage(contact_data->avatar_image);
            std::map<int, QVariant> var_item;
            var_item[ENUMS::role_item_type()] = QVariant::fromValue<ENUMS::ItemDataType>(ENUMS::ItemDataType::CONTACT);
            var_item[ENUMS::role_item_data()] = QVariant::fromValue<std::shared_ptr<ContactData::ItemData>>(contact_data);
            // 设置一些默认显示的数据，防止未设置 delegete 显示为空
            var_item[Qt::DisplayRole   ] = text_display;
            var_item[Qt::DecorationRole] = pixmap; // 设置默认图标样式，实际不使用
            var_item[Qt::EditRole      ] = text_display;
            // var_item[Qt::ToolTipRole  ] = text_display;
            // var_item[Qt::StatusTipRole] = text_display;  // 效果不好
            var_item[Qt::WhatsThisRole ] = text_display;
            // 设置回节点数据列中
            item_data->set_column_data(0, var_item);
        }
        emit dataChanged(contact_index, contact_index);
        return true;
    }

    // 生成一个占位图标，否则获取图标大小会异常
    QIcon icon_init() const {
        static auto icons = Utils::FontIcons().update();
        return icons.icon.contacts;
    }

private:
    std::shared_ptr<IndexData> get_contact_index_data(const QString&account_id) {
        auto index_data = std::make_shared<IndexData>();
        do {
            if ( account_id.isEmpty()) { break; }
            const auto& childs = data_root()->get_item_childs();  // 群组子节点列表
            for (size_t row = 0; row < childs.size(); ++row) {
                index_data->data = childs.at(row);
                if (!index_data->data) { continue; }
                for (const auto& data_column : index_data->data->get_data_columns()) {
                    using T = std::shared_ptr<ContactData::ItemData>;
                    // 不是目标数据
                    if (!data_column.contains(ENUMS::role_item_data())) { continue; }
                    // 获取目标数据容器
                    const QVariant&var_column = data_column.at(ENUMS::role_item_data());
                    if (!var_column.isValid()) { continue; }
                    if (!var_column.canConvert<T>()) { continue; }
                    // 对比目标数据容器内数据
                    index_data->contact = var_column.value<T>();
                    if (!index_data->contact) { continue; }
                    // 匹配账号 ID
                    if (account_id == index_data->contact->account_id) {
                        index_data->row = static_cast<int>(row);
                        break;
                    }
                }
                if (index_data->row != -1) { break; }
            }
            // 获取子节点在父节点中的 index
            if (index_data->row != -1) { index_data->index = this->index(index_data->row, 0, QModelIndex()); }
        } while (false);
        return index_data;
    }
};

class ChatroomModel : public TreeModel {
public:
    using TreeModel::TreeModel;

private:

    // 添加对话
    TreeItemData* add_chatroom_item(const std::shared_ptr<ChatroomData::ItemData>&chatroom_data) {
        TreeItemData* chatroom = nullptr;
        if (!chatroom_data) { return chatroom; }
        chatroom = get_chatroom_item(chatroom_data);
        if (chatroom) { return chatroom; }

        // 填充数据
        auto text_display = chatroom_data->display_name;
        std::map<int, QVariant> var_item;
        var_item[ENUMS::role_item_type()] = QVariant::fromValue<ENUMS::ItemDataType>(ENUMS::ItemDataType::CHATROOM);
        var_item[ENUMS::role_item_data()] = QVariant::fromValue<std::shared_ptr<ChatroomData::ItemData>>(chatroom_data);
        // 设置一些默认显示的数据，防止未设置 delegete 显示为空
        var_item[Qt::DisplayRole   ] = text_display;
        var_item[Qt::DecorationRole] = chatroom_data->avatar_image;
        var_item[Qt::EditRole      ] = text_display;
        // var_item[Qt::ToolTipRole  ] = text_display;
        // var_item[Qt::StatusTipRole] = text_display;  // 效果不好
        var_item[Qt::WhatsThisRole ] = text_display;

        // 插入到第一行即可，更新时间时再进行排序
        auto row_item = 0;
        const QModelIndex parent_index = get_item_chatroom_index(chatroom);
        beginInsertRows(parent_index, row_item, row_item);
        // 需要支持按位置插入
        chatroom = data_root()->insert_child(row_item, new TreeItemData(get_data_column(), var_item));
        endInsertRows();
        return chatroom;
    }

    // 获取数据
    TreeItemData* get_chatroom_item(const std::shared_ptr<ChatroomData::ItemData>&chatroom_data) const {
        TreeItemData* chatroom = nullptr;
        if (!chatroom_data) { return chatroom; }
        return get_chatroom_item(chatroom_data->chatroom_id);
    }

    // 获取数据
    TreeItemData* get_chatroom_item(const QString&chatroom_id) const {
        // 从列表中查找相同的账号 ID  // 聊天室没有分组信息
        TreeItemData * last_chatroom = nullptr;
        const std::vector<TreeItemData*>& item_chatrooms = data_root()->get_item_childs();
        for (TreeItemData * const & item_chatroom : item_chatrooms) {
            if (!item_chatroom) { continue; }
            for (const std::map<int, QVariant>&item_data_column : item_chatroom->get_data_columns()) {
                // 获取数据所在列
                if (!item_data_column.contains(ENUMS::role_item_data())) { continue; }
                QVariant item_var_data = item_data_column.at(ENUMS::role_item_data());
                // 获取数据
                if (!item_var_data.canConvert<std::shared_ptr<ChatroomData::ItemData>>()) { continue; }
                auto chatroom_data = item_var_data.value<std::shared_ptr<ChatroomData::ItemData>>();
                if (chatroom_id == chatroom_data->chatroom_id) {
                    last_chatroom = item_chatroom;
                    break;
                }
            }
            if (last_chatroom) { break; }
        }
        return last_chatroom;
    }

    int get_item_chatroom_row(const TreeItemData*chatroom) const {
        if (!chatroom) { return -1; }
        const std::vector<TreeItemData*>& item_chatrooms = data_root()->get_item_childs();
        auto ite = item_chatrooms.begin();
        for (; ite != item_chatrooms.end(); ++ite) {
            if (*ite == chatroom) { break; }
        }
        if (ite != item_chatrooms.end()) { return std::distance(item_chatrooms.begin(), ite); }
        return -1;
    }

    QModelIndex get_item_chatroom_index(const TreeItemData*chatroom) const {
        if (!chatroom) { return QModelIndex(); }
        auto row = get_item_chatroom_row(chatroom);
        return row == -1 ? QModelIndex() : createIndex(row, 0, data_root()->get_item_childs().at(row));
    }

    // 生成一个占位图标，否则获取图标大小会异常
    QIcon icon_init() const {
        static auto icons = Utils::FontIcons().update();
        return icons.icon.contacts;
    }

public:
    // 外部只需要知道行号即可
    int add_chatroom_row(const std::shared_ptr<ChatroomData::ItemData>&chatroom_data) {
        return get_item_chatroom_row(add_chatroom_item(chatroom_data));
    }
    int get_chatroom_row(const std::shared_ptr<ChatroomData::ItemData>&chatroom_data) const {
        return get_item_chatroom_row(get_chatroom_item(chatroom_data));
    }
    bool has_chatroom(const QString&chatroom_id) {
        // if ( chatroom_id.isEmpty()) { return false; }
        if ( get_chatroom_item(chatroom_id)) { return true; }
        return false;
    }
    std::shared_ptr<ChatroomData::ItemData> get_chatroom_data(const QString&chatroom_id) const {
        std::shared_ptr<ChatroomData::ItemData> chatroom_data;
        do {
            auto item_chatroom = get_chatroom_item(chatroom_id);
            if (!item_chatroom) { break; }
            for (const std::map<int, QVariant>&item_data_column : item_chatroom->get_data_columns()) {
                // 获取数据所在列
                if (!item_data_column.contains(ENUMS::role_item_data())) { continue; }
                QVariant item_var_data = item_data_column.at(ENUMS::role_item_data());
                // 获取数据
                if (!item_var_data.canConvert<std::shared_ptr<ChatroomData::ItemData>>()) { continue; }
                auto item_chatroom_data = item_var_data.value<std::shared_ptr<ChatroomData::ItemData>>();
                if (chatroom_id == item_chatroom_data->chatroom_id) {
                    chatroom_data = item_chatroom_data;
                    break;
                }
            }
        } while (false);
        return chatroom_data;
    }
    bool updata_chatroom(const std::shared_ptr<ChatroomData::ItemData>&chatroom_data) {
        do {
            auto index = get_item_chatroom_index(get_chatroom_item(chatroom_data));
            if (!index.isValid()) { break; }
            int row = index.row();
            if (row < 0) { break; }  // 没有找到时
            emit dataChanged(index, index);
            return true;
        } while (false);
        return false;
    }
};

class MessageModel : public TreeModel {
public:
    using TreeModel::TreeModel;
public:
    TreeItemData* add_message(const std::shared_ptr<MessageData::ItemData>&message_data) {
        // 先获取消息列表，再进行排序，查找插入位置，与上一条消息间隔时间过长时添加一条时间提示
        TreeItemData* item_data = nullptr;
        do {
            if (!message_data) { break; }

            auto item_childs = data_root()->get_item_childs();
            if (m_cache_message_id_index.contains(message_data->message_id)) { break; }

            // 获取子节点列表
            // // 查找插入位置
            // auto ite = item_childs.begin();
            // for ( ; ite != item_childs.end(); ++ite) {
            //     // TODO:可以插入非末尾位置
            // }

            // 填充数据
            auto text_display = message_data->sender->display_name;
            std::map<int, QVariant> var_item;
            var_item[ENUMS::role_item_type()] = QVariant::fromValue<ENUMS::ItemDataType>(ENUMS::ItemDataType::MESSAGE);
            var_item[ENUMS::role_item_data()] = QVariant::fromValue<std::shared_ptr<MessageData::ItemData>>(message_data);
            // 设置一些默认显示的数据，防止未设置 delegete 显示为空
            var_item[Qt::DisplayRole   ] = text_display;
            var_item[Qt::DecorationRole] = icon_init(); // 设置默认图标样式，实际不使用
            var_item[Qt::EditRole      ] = text_display;
            // var_item[Qt::ToolTipRole  ] = text_display;
            // var_item[Qt::StatusTipRole] = text_display;  // 效果不好
            var_item[Qt::WhatsThisRole ] = text_display;

            // 插入
            // auto row_item = std::distance(item_childs.begin(), ite);
            auto row_item = (int)item_childs.size();
            const QModelIndex parent_index = QModelIndex(); // get_item_message_index(chatroom);
            beginInsertRows(parent_index, row_item, row_item);
            // 需要支持按位置插入
            TreeItemData* item_message = data_root()->insert_child(row_item, new TreeItemData(get_data_column(), var_item));
            endInsertRows();

            // 添加缓存
            if (message_data && !message_data->message_id.isEmpty()) {
                m_cache_message_id_index[message_data->message_id] = createIndex(row_item, 0, item_message);
            }
            return item_message;
        } while(false);
        return nullptr;
    }

    // 获取最后一条消息以便列表显示
    const std::shared_ptr<MessageData::ItemData> get_last_message() {
        // 获取行号
        auto last_row_index = (int)data_root()->row_count() - 1;
        if ( last_row_index < 0) { return nullptr; }
        // 获取节点内容
        auto last_row_data = data_root()->row_data(last_row_index);
        if (!last_row_data) { return nullptr; }
        // 获取列中的数据
        auto column_data = last_row_data->get_column_data(get_data_column());
        if ( column_data.empty()) { return nullptr; }
        // 解析列中的数据
        auto column_variant = column_data.at(ENUMS::role_item_data());
        if (!column_variant.canConvert<std::shared_ptr<MessageData::ItemData>>()) { return nullptr; }
        return column_variant.value<std::shared_ptr<MessageData::ItemData>>();
    }

    auto row_count() { return data_root()->row_count(); }

    int get_item_message_row(const TreeItemData*item_message) const {
        const auto&index = get_item_message_index(item_message);
        if (!index.isValid()) { return -1; }
        return index.row();
    }

    QModelIndex get_item_message_index(const TreeItemData*item_message) const {
        do {
            if (!item_message) { break; }
            const auto&data_columns = item_message->get_data_columns();
            if ( data_columns.size() <= get_data_column()) { break; }
            const auto&data_column = data_columns.at(get_data_column());
            // if ( data_column.size() <= ENUMS::role_item_data()) { break; }
            if (!data_column.contains(ENUMS::role_item_data())) { break; }
            const auto&var_data = data_column.at(ENUMS::role_item_data());
            if (!var_data.canConvert<std::shared_ptr<MessageData::ItemData>>()) { break; }
            const auto&item_data = var_data.value<std::shared_ptr<MessageData::ItemData>>();
            return get_item_message_index(item_data);
        } while (false);
        return QModelIndex();
    }

    QModelIndex get_item_message_index(const std::shared_ptr<MessageData::ItemData>&item_data) const {
        do {
            if (!item_data) { break; }
            const auto&message_id = item_data->message_id;
            if (!message_id.isEmpty()) { break; }
            if (!m_cache_message_id_index.contains(message_id)) { break; }
            const auto&index = m_cache_message_id_index.value(message_id);
            return index;
        } while (false);
        return QModelIndex();
    }

    // 更新消息显示
    void update_message(const std::shared_ptr<MessageData::ItemData>&message_data) {
        do {
            auto index = get_item_message_index(message_data);
            if (!index.isValid()) { break; }
            int row = index.row();
            if (row < 0) { break; }  // 没有找到时
            emit dataChanged(index, index);
        } while (false);
    }

    std::shared_ptr<MessageData::ItemData> get_message(const QString&message_id) const {
        do {
            if (!m_cache_message_id_index.contains(message_id)) { break; }
            const auto&index =  m_cache_message_id_index.value(message_id);
            if (!index.isValid()) { break; }
            const auto&var_data = index.data(ENUMS::role_item_data());  // 空则返回默认值 QVariant{}
            if (!var_data.isValid()) { break; }
            if (!var_data.canConvert<std::shared_ptr<MessageData::ItemData>>()) { break; }
            const auto&item_data = var_data.value<std::shared_ptr<MessageData::ItemData>>();
            return item_data;
        } while (false);
        return nullptr;
    }
private:
    // 生成一个占位图标，否则获取图标大小会异常
    QIcon icon_init() const {
        static auto icons = Utils::FontIcons().update();
        return icons.icon.contacts;
    }
private:
    QMap<QString, QModelIndex> m_cache_message_id_index;  // 消息 ID 缓存
};

class ChatroomProxy : public QSortFilterProxyModel {
public:
protected:
    // 过滤函数
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override {
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
        // // 获取原始 Model 的 index
        // QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

        // // 获取自定义数据（例如在线状态）
        // bool is_online = index.data(CustomRoles::OnlineRole).toBool();
        // QString name   = index.data(Qt::DisplayRole).toString();

        // // 仅显示在线且符合搜索关键字的联系人
        // return is_online && name.contains(filterRegularExpression());
    }

    // 仅比较时间
    // 排序函数，该函数返回正序，由 `QSortFilterProxyModel::sort(0, Qt::AscendingOrder);` 设置为倒序
    bool lessThan(const QModelIndex &left_index, const QModelIndex &right_index) const {
        do {
            // 校验 index
            if (!left_index .isValid()) { break; }
            if (!right_index.isValid()) { break; }
            // 只处理第 0 列
            if ( left_index .column() != get_data_column()) { break; }
            if ( right_index.column() != get_data_column()) { break; }
            // 获取列中存储的数据
            auto left_data  = left_index .data(ENUMS::role_item_data());
            auto right_data = right_index.data(ENUMS::role_item_data());
            // 校验数据是否可用
            if (!left_data .isValid()) { break; }
            if (!right_data.isValid()) { break; }
            // 校验数据类型
            if (!left_data .canConvert<std::shared_ptr<ChatroomData::ItemData>>()) { break; }
            if (!right_data.canConvert<std::shared_ptr<ChatroomData::ItemData>>()) { break; }
            // 获取实际数据
            auto left_chatroom  = left_data .value<std::shared_ptr<ChatroomData::ItemData>>();
            auto right_chatroom = right_data.value<std::shared_ptr<ChatroomData::ItemData>>();
            if (!left_chatroom ) { break; }
            if (!right_chatroom) { break; }
            // 获取时间
            if (!left_chatroom ->last_message_datetime.isValid()) { break; }
            if (!right_chatroom->last_message_datetime.isValid()) { break; }
            return left_chatroom->last_message_datetime < right_chatroom->last_message_datetime;  // 正序
        } while(false);
        return QSortFilterProxyModel::lessThan(left_index, right_index);
        // return QString::localeAwareCompare(left_name, right_name) < 0;
    }
};

