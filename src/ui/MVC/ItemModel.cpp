#include "ItemModel.h"

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent)) { return QModelIndex(); }
    const auto data_parent = parent.isValid() ? static_cast<TreeItemData*>(parent.internalPointer()) : data_root();
    if (row < 0 || row >= static_cast<int>(data_parent->get_item_childs().size())) { return QModelIndex(); }
    auto data_child = data_parent->get_item_childs().at(row);
    return data_child ? createIndex(row, column, data_child) : QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &child) const {
    if (!child.isValid()) { return QModelIndex(); }

    // auto data_parent = static_cast<TreeItemData*>(child.internalPointer())->get_item_parent();
    // return (data_parent != m_data_root && data_parent != nullptr) ? createIndex(data_parent->row_index(), 0, data_parent) : QModelIndex();

    auto child_item = static_cast<TreeItemData*>(child.internalPointer());
    if (!child_item) { return QModelIndex(); }

    auto data_parent = child_item->get_item_parent();
    if (!data_parent || data_parent == m_data_root) { return QModelIndex(); }

    return createIndex(data_parent->row_index(), 0, data_parent);
}

// 获取 parent 的子列数
int TreeModel::rowCount(const QModelIndex &parent) const {
    if (parent.column() > 0) { return 0; }  // 只有第一列数据允许有子节点
    const auto data_parent = parent.isValid() ? static_cast<TreeItemData*>(parent.internalPointer()) : data_root();
    return static_cast<int>(data_parent->get_item_childs().size());
}

int TreeModel::columnCount(const QModelIndex &parent) const {
    const auto data_parent = parent.isValid() ? static_cast<TreeItemData*>(parent.internalPointer()) : data_root();
    return static_cast<int>(data_parent->get_data_columns().size());
}

QVariant TreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) { return QVariant(); }
    TreeItemData *data_item = static_cast<TreeItemData*>(index.internalPointer());
    if (index.column() < 0 || index.column() >= static_cast<int>(data_item->get_data_columns().size())) { return QVariant(); }
    auto var = data_item->get_data_columns().at(index.column());
    if(!var.contains(role)) { return QVariant{}; }
    return var.at(role);
}
// 
// // 添加数据
// void ItemModel::add(const std::shared_ptr<ItemDelegate>&item) {
//     int row = rowCount();
//     beginInsertRows(QModelIndex(), row, row);
//     get().emplace_back(item);  // TODO:这里应该进行数据拷贝，防止外部更改影响到内部
//     endInsertRows();
// }
// 
// // 移除数据
// void ItemModel::remove(const std::shared_ptr<ItemDelegate>&item) {
//     auto row = get(item);
//     beginRemoveRows(QModelIndex(), row, row);
//     auto items = get();
//     auto it = items.begin();
//     std::advance(it, row);
//     items.erase(it);
//     endRemoveRows();
// }
// 
// // 清空或替换
// void ItemModel::reset(const std::vector<std::shared_ptr<ItemDelegate>>&items) {
//     beginResetModel();
//     get() = items;
//     endResetModel();
// }
// 
// // 获取数据组
// const std::vector<std::shared_ptr<ItemDelegate>>& ItemModel::get() const {
//     return m_items;
// }
// 
// // 获取数据组
// std::vector<std::shared_ptr<ItemDelegate>>& ItemModel::get() {
//     return m_items;
// }
// 
// //获取位置所在数据
// std::shared_ptr<ItemDelegate> ItemModel::get(int pos) const {
//     if (pos < 0 || pos >= rowCount()) { return nullptr; }
//     return get().at(pos);
// }
// 
// // 替换指定位置的元素
// bool ItemModel::replace(int pos, std::shared_ptr<ItemDelegate>&provider) {
//     if (pos < 0 || pos >= rowCount()) { return false; }
//     get()[pos] = provider;
//     return true;
// }
// 
// // 获取数据所在位置
// int ItemModel::get(const std::shared_ptr<ItemDelegate>&item_providers) const {
//     int pos = -1;
//     int index = 0;
//     for(auto&ite : get()) {
//         if (item_providers == ite) { return index; }  // 指针还是内容？？
//         index++;
//     }
//     return pos;
// }
