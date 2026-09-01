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

protected:
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

// 联系人列表
class ContactModel : public TreeModel {
public:
    using TreeModel::TreeModel;
// #ifdef _DEBUG
//     ContactModel() {
//         for(int i = 0; i <= 300; ++i) {
//             auto contact = std::make_shared<ContactInfo>();
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
        auto column_index = 0;
        auto text_diaplay = QMetaEnum::fromType<ENUMS::ItemGroupType>().valueToKey(static_cast<int>(group_type));
        std::map<int, QVariant> var_item;
        var_item[ENUMS::role_item_type()] = QVariant::fromValue<ENUMS::ItemDataType>(ENUMS::ItemDataType::GROUP);
        var_item[ENUMS::role_item_data()] = QVariant::fromValue<ENUMS::ItemGroupType>(group_type);
        // 设置一些默认显示的数据，防止未设置 delegete 显示为空
        var_item[Qt::DisplayRole  ] = text_diaplay;
        var_item[Qt::EditRole     ] = text_diaplay;
        // var_item[Qt::ToolTipRole  ] = text_diaplay;
        // var_item[Qt::StatusTipRole] = text_diaplay;  // 效果不好
        var_item[Qt::WhatsThisRole] = text_diaplay;
        // 为了展示不同类型，给群组设置背景颜色
        // var_item[Qt::BackgroundRole] = QVariant::fromValue<QColor>(Qt::darkCyan);
        // var_item[Qt::ForegroundRole] = QVariant::fromValue<QColor>(Qt::darkMagenta);
        var_item[Qt::DecorationRole] = icon_init(); // 设置默认图标样式
        auto row = static_cast<int>(data_root()->get_item_childs().size());
        beginInsertRows(QModelIndex(), row, row);
        auto item_group = data_root()->add_child(new TreeItemData(column_index, var_item));
        endInsertRows();
        return item_group;
    }

    // 返回添加的成员节点  // 1.分组不重复；2.contact 必须有 account_id 且同样不允许重复
    auto add_member(ENUMS::ItemGroupType group_type, const std::shared_ptr<ContactInfo> contact) -> TreeItemData* {
        // ============================ 过滤重复 ============================ //

        // 获取账号 ID 作为重复判断条件
        auto account_id = (contact && contact->hasBaseInfo() && !contact->baseInfo().accountId().isEmpty()) ? contact->baseInfo().accountId() : "";
        if (account_id.isEmpty()) {
            qWarning() << "Contact account id is EMPTY!";
            return nullptr;
        }

        auto func_cmp_columns = [account_id](const std::map<int, QVariant>&data_column) {
            using T = std::shared_ptr<ContactInfo>;
            // 不是目标数据
            if (!data_column.contains(ENUMS::role_item_data())) { return false; }
            // 获取目标数据容器
            const QVariant&var_column = data_column.at(ENUMS::role_item_data());
            if (!var_column.isValid()) { return false; }
            if (!var_column.canConvert<T>()) { return false; }
            // 对比目标数据容器内数据
            T val = var_column.value<T>();
            // if (var_column.value<T>() == group_type) { return true; }
            if (!val->hasBaseInfo()) { return false; }
            auto&base_info = val->baseInfo();
            auto&account_id_text = base_info.accountId();
            // if (account_id.isEmpty()) { return false; }
            if (account_id == account_id_text) { return true; }
            return false;
        };
        auto func_cmp_childs = [func_cmp_columns](TreeItemData*item_child) {
            auto item_columns = item_child->get_data_columns() | std::views::filter(func_cmp_columns);
            if (!item_columns.empty()) { return true; }
            return false;
        };
        auto has_dupe_account = [this, func_cmp_childs](ENUMS::ItemGroupType group_type, const std::shared_ptr<ContactInfo> contact) {
            auto row = row_group(group_type);
            if (row == -1) { return false; }
            auto item_group = data_root()->get_item_childs().at(row);
            if (!item_group) { return false; }
            auto item_childs = item_group->get_item_childs() | std::views::filter(func_cmp_childs);  // 空数组不会进入 views
            if (!item_childs.empty()) { return true; }
            return false;
        };
        if (has_dupe_account(group_type, contact)) {
            return nullptr;
        }

        // // 支持空信息的添加，如果不是空信息时才校验重复
        // if (account_id.isEmpty()) {
        //     qWarning() << "Contact account id is EMPTY!";
        //     account_id = contact ? tr("Account ID is EMPTY!") : tr("[Contact is EMPTY!]");
        // }
        // else {
        //     if (has_dupe_account(group_type, contact)) { return nullptr; }
        // }

        // ============================ 添加成员 ============================ //

        // 获取分组节点
        auto row = row_group(group_type);
        TreeItemData*item_group = row == -1 ? nullptr : data_root()->get_item_childs().at(row);
        // 没有该分组时添加该分组节点
        if (!item_group) {
            item_group = add_group(group_type);
        }

        // 分组节点添加成员  // 未校验重复
        auto column_index = 0;
        auto text_diaplay = account_id;
        std::map<int, QVariant> var_item;
        var_item[ENUMS::role_item_type()] = QVariant::fromValue<ENUMS::ItemDataType>(ENUMS::ItemDataType::CONTACT);
        var_item[ENUMS::role_item_data()] = QVariant::fromValue<std::shared_ptr<ContactInfo>>(contact);
        // 设置一些默认显示的数据，防止未设置 delegete 显示为空
        var_item[Qt::DisplayRole  ] = text_diaplay;
        var_item[Qt::EditRole     ] = text_diaplay;
        // var_item[Qt::ToolTipRole  ] = text_diaplay;
        // var_item[Qt::StatusTipRole] = text_diaplay;  // 效果不好
        var_item[Qt::WhatsThisRole] = text_diaplay;
        var_item[Qt::DecorationRole] = icon_init(); // 设置默认图标样式，实际不使用

        // 查找视图中的插入位置  // 判断条件是先判断昵称，昵称相同再判断用户 ID
        auto func_find_pos = [](const std::vector<TreeItemData*>&childs, const std::shared_ptr<ContactInfo>&contact) -> int {
            // 无效值不插入
            if (!contact->hasBaseInfo()) { return -1; }
            auto nn  = contact->baseInfo().nickname();
            auto uid = contact->baseInfo().userId();
            if (nn.isEmpty() && uid.isEmpty()) { return -1; }

            // 空值前置  // 插入位置在 ite 前
            auto ite = childs.begin();
            for (; ite != childs.end(); ++ite) {
                // 跳过已有的无效值
                TreeItemData*tree_item_data = *ite;
                if (!tree_item_data) { continue; }

                bool finded = false;
                for (auto&data_column : tree_item_data->get_data_columns()) {
                    using T = std::shared_ptr<ContactInfo>;
                    // 不是目标数据
                    if (!data_column.contains(ENUMS::role_item_data())) { continue; }
                    // 获取目标数据容器
                    const QVariant&var_column = data_column.at(ENUMS::role_item_data());
                    if (!var_column.isValid()) { continue; }
                    if (!var_column.canConvert<T>()) { continue; }
                    // 对比目标数据容器内数据
                    T val = var_column.value<T>();
                    // if (var_column.value<T>() == group_type) { return true; }
                    if (!val->hasBaseInfo()) { continue; }
                    auto&base_info = val->baseInfo();
                    auto&nickname  = base_info.nickname();
                    auto&user_id   = base_info.userId();
                    // 使用 Qt 的 localeAwareCompare 实现支持中文拼音/本地化排序的对比升序排序
                    int rnn = QString::localeAwareCompare(nn , nickname);
                    int rid = QString::localeAwareCompare(uid, user_id );
                    // 昵称一样时才比较账号 ID
                    if (rnn < 0 || (rnn == 0 && rid < 0)) { finded = true; break; }
                }
                if (finded) { break; }
            }
            return static_cast<int>(std::distance(childs.begin(), ite));
        };

        int pos_insert = static_cast<int>(item_group->get_item_childs().size()); // 默认不排序
        // 判断分组，查找插入位置
        if (group_type == ENUMS::ItemGroupType::FRIENDS) {
            pos_insert = func_find_pos(item_group->get_item_childs(), contact);
            qDebug().noquote().nospace() << "insert pos : " << pos_insert << ", contact nickname : " << contact->baseInfo().nickname();
        }
        if (pos_insert < 0) { return nullptr; }

        auto row_item = static_cast<int>(pos_insert);
        const QModelIndex parent_index = index_group(group_type);
        beginInsertRows(parent_index, row_item, row_item);
        // 需要支持按位置插入
        // auto item_member = item_group->add_child(new TreeItemData(column_index, var_item));
        auto item_member = item_group->insert_child(pos_insert, new TreeItemData(column_index, var_item));
        endInsertRows();
        return item_member;

        // auto item_member = item_group->add_child(new TreeItemData(0, ENUMS::role_item_data(), QVariant::fromValue<std::shared_ptr<ContactInfo>>(contact)));
        // return item_member;
    }

    QModelIndex index_group(ENUMS::ItemGroupType group_type) {
        auto row = row_group(group_type);
        return row == -1 ? QModelIndex() : createIndex(row, 0, data_root()->get_item_childs().at(row));
        return QModelIndex();
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
            // return item_group->add_child(new TreeItemData(0, ENUMS::role_item_data(), QVariant::fromValue<std::shared_ptr<ContactInfo>>(contact)));
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

    // 生成一个占位图标，否则获取图标大小会异常
    QIcon icon_init() const {
        static auto icons = Utils::FontIcons().update();
        return icons.icon.contacts;
    }
};

// 搜索结果
class SearchModel : public TreeModel {
public:
    using TreeModel::TreeModel;
// #ifdef _DEBUG
//     SearchModel() {
//         for(int i = 0; i <= 300; ++i) {
//             auto contact = std::make_shared<ContactInfo>();
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
    auto add_member(const std::shared_ptr<ContactInfo> contact) -> TreeItemData* {
        // ============================ 过滤重复 ============================ //

        // 获取账号 ID 作为重复判断条件
        auto account_id = (contact && contact->hasBaseInfo() && !contact->baseInfo().accountId().isEmpty()) ? contact->baseInfo().accountId() : "";
        if (account_id.isEmpty()) {
            qWarning() << "Contact account id is EMPTY!";
            return nullptr;
        }

        auto func_check_dupe = [account_id](const std::map<int, QVariant>&data_column) {
            using T = std::shared_ptr<ContactInfo>;
            // 不是目标数据
            if (!data_column.contains(ENUMS::role_item_data())) { return false; }
            // 获取目标数据容器
            const QVariant&var_column = data_column.at(ENUMS::role_item_data());
            if (!var_column.canConvert<T>()) { return false; }
            // 对比目标数据容器内数据
            T val = var_column.value<T>();
            // if (var_column.value<T>() == group_type) { return true; }
            if (!val->hasBaseInfo()) { return false; }
            auto&base_info = val->baseInfo();
            auto&account_id_text = base_info.accountId();
            // if (account_id.isEmpty()) { return false; }
            if (account_id == account_id_text) { return true; }
            return false;
            // return col.contains(ENUMS::role_item_data()) && col.at(ENUMS::role_item_data()).value<ENUMS::ItemGroupType>() == group_type;
        };
        auto func_cmp_dupe = [contact, func_check_dupe](const TreeItemData* item_ite) {
            if (!item_ite) { return false; }
            auto vec_filter = item_ite->get_data_columns() | std::views::filter(func_check_dupe);
            return !vec_filter.empty();
            return false;
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
        auto column_index = 0;
        auto text_diaplay = account_id;
        std::map<int, QVariant> var_item;
        var_item[ENUMS::role_item_type()] = QVariant::fromValue<ENUMS::ItemDataType>(ENUMS::ItemDataType::CONTACT);
        var_item[ENUMS::role_item_data()] = QVariant::fromValue<std::shared_ptr<ContactInfo>>(contact);
        // 设置一些默认显示的数据，防止未设置 delegete 显示为空
        var_item[Qt::DisplayRole  ] = text_diaplay;
        var_item[Qt::EditRole     ] = text_diaplay;
        // var_item[Qt::ToolTipRole  ] = text_diaplay;
        // var_item[Qt::StatusTipRole] = text_diaplay;  // 效果不好
        var_item[Qt::WhatsThisRole] = text_diaplay;
        var_item[Qt::DecorationRole] = icon_init(); // 设置默认图标样式，实际不使用
        auto row = static_cast<int>(item_group->get_item_childs().size());
        beginInsertRows(QModelIndex(), row, row);
        auto item_member = item_group->add_child(new TreeItemData(column_index, var_item));
        endInsertRows();
        return item_member;
    }

    // 生成一个占位图标，否则获取图标大小会异常
    QIcon icon_init() const {
        static auto icons = Utils::FontIcons().update();
        return icons.icon.contacts;
    }
};

// // 聊天列表所需的数据
// struct ChatroomData {
//     std::shared_ptr<ContactInfo> contact;
//     QString   text    ;  // last_message_text
//     QDateTime datetime;  // last_message_datetime
// };

class ChatroomModel : public TreeModel {
public:
    using TreeModel::TreeModel;

private:

    // 添加对话
    TreeItemData* add_contact_chatroom_item(const std::shared_ptr<ChatroomData>&chatroom_data) {
        TreeItemData* chatroom = nullptr;
        if (!chatroom_data) { return chatroom; }
        chatroom = get_contact_chatroom_item(chatroom_data);
        if (chatroom) { return chatroom; }

        auto get_display_name = [](const std::shared_ptr<ContactInfo>&contact) -> QString {
            QString display_name;
            if (!contact) { return display_name; }

            // 备注 > 昵称 > 账号 ID(DEBUG)
            if (contact->hasFriendInfo()) {
                auto friend_info = contact->friendInfo();
                auto notes = friend_info.notes();
                if (!notes.isEmpty()) { display_name = notes; }
            }
            else if (contact->hasBaseInfo()) {
                auto base_info = contact->baseInfo();
                auto nickname = base_info.nickname();
                if (!nickname.isEmpty()) { display_name = nickname; }
#ifdef _DEBUG
                auto account_id = base_info.accountId();
                if (!account_id.isEmpty()) { display_name = account_id; }
#endif // _DEBUG
            }
            return display_name;
        };
        // 填充数据
        constexpr auto column_index = 0;
        auto text_diaplay = get_display_name(chatroom_data->contact);
        std::map<int, QVariant> var_item;
        var_item[ENUMS::role_item_type()] = QVariant::fromValue<ENUMS::ItemDataType>(ENUMS::ItemDataType::CHATROOM);
        var_item[ENUMS::role_item_data()] = QVariant::fromValue<std::shared_ptr<ChatroomData>>(chatroom_data);
        // 设置一些默认显示的数据，防止未设置 delegete 显示为空
        var_item[Qt::DisplayRole  ] = text_diaplay;
        var_item[Qt::EditRole     ] = text_diaplay;
        // var_item[Qt::ToolTipRole  ] = text_diaplay;
        // var_item[Qt::StatusTipRole] = text_diaplay;  // 效果不好
        var_item[Qt::WhatsThisRole] = text_diaplay;
        var_item[Qt::DecorationRole] = icon_init(); // 设置默认图标样式，实际不使用

        // 插入到第一行即可，更新时间时再进行排序
        auto row_item = 0;
        const QModelIndex parent_index = get_item_chatroom_index(chatroom);
        beginInsertRows(parent_index, row_item, row_item);
        // 需要支持按位置插入
        chatroom = data_root()->insert_child(row_item, new TreeItemData(column_index, var_item));
        endInsertRows();
        return chatroom;
    }

    // 获取数据
    TreeItemData* get_contact_chatroom_item(const std::shared_ptr<ChatroomData>&chatroom_data) const {
        TreeItemData* chatroom = nullptr;
        if (!chatroom_data) { return chatroom; }
        auto&contact = chatroom_data->contact;
        // 获取传入对象的账号 ID
        if (!contact) { return chatroom; }
        if (!contact->hasBaseInfo()) { return chatroom; }
        // if ( contact->baseInfo().accountId().isEmpty()) { return chatroom; }
        auto account_id = contact->baseInfo().accountId();
        return get_contact_chatroom_item(account_id);
    }

    // 获取数据
    TreeItemData* get_contact_chatroom_item(const QString&account_id) const {
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
                if (!item_var_data.canConvert<std::shared_ptr<ChatroomData>>()) { continue; }
                auto chatroom_data = item_var_data.value<std::shared_ptr<ChatroomData>>();
                // 比较聊天室 ID
                auto contact = chatroom_data->contact;
                if (!contact) { continue; }
                if (!contact->hasBaseInfo()) { continue; }
                if ( contact->baseInfo().accountId().isEmpty()) { continue; }
                auto item_account_id = contact->baseInfo().accountId();
                if (account_id == item_account_id) {
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

    // // 更新时间触发重新排序
    // TreeItemData* updata_last_message_datetime(const std::shared_ptr<ContactInfo>&in_contact, const QString&message, const QDateTime&datetime) {
    //     TreeItemData* chatroom = nullptr;
    //     chatroom = get_contact_chatroom_item(in_contact);
    //     if (!chatroom) { return nullptr; }
    //     // TODO:更新内存及视图
    //     return chatroom;
    // }

    // 生成一个占位图标，否则获取图标大小会异常
    QIcon icon_init() const {
        static auto icons = Utils::FontIcons().update();
        return icons.icon.contacts;
    }

public:
    // 外部只需要知道行号即可
    int add_contact_chatroom_row(const std::shared_ptr<ChatroomData>&chatroom_data) {
        return get_item_chatroom_row(add_contact_chatroom_item(chatroom_data));
    }
    int get_contact_chatroom_row(const std::shared_ptr<ChatroomData>&chatroom_data) const {
        return get_item_chatroom_row(get_contact_chatroom_item(chatroom_data));
    }
    void update_last_message(const QString&chatroom_id, const QString&last_message, const QDateTime&datetime) {
        // 查找节点
        TreeItemData* chatroom = get_contact_chatroom_item(chatroom_id);
        if (!chatroom) { return; }
        // 获取节点
        auto column_data = chatroom->get_column_data(0);
        if (!column_data.size()) { return; }
        // 判断类型
        auto item_data = column_data.at(ENUMS::role_item_data());
        if (!item_data.canConvert<std::shared_ptr<ChatroomData>>()) { return; }
        // 获取数据
        auto chatroom_data = item_data.value<std::shared_ptr<ChatroomData>>();
        if (!chatroom_data) { return; }
        // 更新存储
        chatroom_data->text = last_message;
        chatroom_data->datetime = datetime;
        // 通知界面更新
        // 修改第 row 行、col 列的数据
        QModelIndex index = get_item_chatroom_index(chatroom);
        // 发射信号通知 View 刷新该单元格（也可指定受影响的 Roles 优化性能）
        // emit dataChanged(index, index, { Qt::DisplayRole, Qt::UserRole });        
        emit dataChanged(index, index);        
    }

// private:
//     std::map<QString, TreeItemData*> m_cache_chatrooms;
};

class MessageModel : public TreeModel {
public:
    using TreeModel::TreeModel;
public:
    TreeItemData* add_message(const std::shared_ptr<MessageContext>&message_context) {
        // 先获取消息列表，再进行排序，查找插入位置，与上一条消息间隔时间过长时添加一条时间提示

        // 去重
        auto client_message_id = message_context->clientMessageId();
        for (auto&item : data_root()->get_item_childs()) {
            for (auto&column : item->get_data_columns()) {
                auto data = column.at(ENUMS::role_item_data());
                if (!data.isValid() || !data.canConvert<std::shared_ptr<MessageContext>>()) { continue; }
                auto context = data.value<std::shared_ptr<MessageContext>>();
                auto item_id = context->clientMessageId();
                if (item_id == client_message_id) {
                    return nullptr;  // 重复消息
                }
            }
        }

        // 获取子节点列表
        auto item_childs = data_root()->get_item_childs();
        // 查找插入位置
        auto ite = item_childs.begin();
        for ( ; ite != item_childs.end(); ++ite) {
            ;
        }

        // 填充数据
        constexpr auto column_index = 0;
        auto text_diaplay = QString(); // get_display_name(in_contact);
        std::map<int, QVariant> var_item;
        var_item[ENUMS::role_item_type()] = QVariant::fromValue<ENUMS::ItemDataType>(ENUMS::ItemDataType::MESSAGE);
        var_item[ENUMS::role_item_data()] = QVariant::fromValue<std::shared_ptr<TaoTieChat::MessageContext>>(message_context);
        // 设置一些默认显示的数据，防止未设置 delegete 显示为空
        var_item[Qt::DisplayRole  ] = text_diaplay;
        var_item[Qt::EditRole     ] = text_diaplay;
        // var_item[Qt::ToolTipRole  ] = text_diaplay;
        // var_item[Qt::StatusTipRole] = text_diaplay;  // 效果不好
        var_item[Qt::WhatsThisRole] = text_diaplay;
        var_item[Qt::DecorationRole] = icon_init(); // 设置默认图标样式，实际不使用

        // 插入
        auto row_item = std::distance(item_childs.begin(), ite);
        const QModelIndex parent_index = QModelIndex(); // get_item_message_index(chatroom);
        beginInsertRows(parent_index, row_item, row_item);
        // 需要支持按位置插入
        TreeItemData* item_message = data_root()->insert_child(row_item, new TreeItemData(column_index, var_item));
        endInsertRows();
        return item_message;
        return nullptr;
    }

    // 获取最后一条消息以便列表显示
    const std::shared_ptr<MessageContext> get_last_message() {
        // 获取行号
        auto last_row_index = (int)data_root()->row_count() - 1;
        if ( last_row_index < 0) { return nullptr; }
        // 获取节点内容
        auto last_row_data = data_root()->row_data(last_row_index);
        if (!last_row_data) { return nullptr; }
        // 获取列中的数据
        auto column_data = last_row_data->get_column_data(0);
        if ( column_data.empty()) { return nullptr; }
        // 解析列中的数据
        auto column_variant = column_data.at(ENUMS::role_item_data());
        if (!column_variant.canConvert<std::shared_ptr<MessageContext>>()) { return nullptr; }
        return column_variant.value<std::shared_ptr<MessageContext>>();
    }

    int get_item_message_row(const TreeItemData*item_message) const {
        if (!item_message) { return -1; }
        const std::vector<TreeItemData*>& item_messages = data_root()->get_item_childs();
        auto ite = item_messages.begin();
        for (; ite != item_messages.end(); ++ite) {
            if (*ite == item_message) { break; }
        }
        if (ite != item_messages.end()) { return std::distance(item_messages.begin(), ite); }
        return -1;
    }

    QModelIndex get_item_message_index(const TreeItemData*item_message) const {
        if (!item_message) { return QModelIndex(); }
        auto row = get_item_message_row(item_message);
        return row == -1 ? QModelIndex() : createIndex(row, 0, data_root()->get_item_childs().at(row));
    }

private:
    // 生成一个占位图标，否则获取图标大小会异常
    QIcon icon_init() const {
        static auto icons = Utils::FontIcons().update();
        return icons.icon.contacts;
    }

};
