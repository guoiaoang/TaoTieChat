#include "ItemModel.h"

int ItemModel::rowCount(const QModelIndex &parent) const {
    do{
        if (parent.isValid()) {
            return 0;
            // break;
        }
        return int(get().size());
    } while(false);
    // return QAbstractListModel::rowCount(parent);
    return 0;
}

// 从内部变量中获取数据
QVariant ItemModel::data(const QModelIndex &index, int role) const {
    do {
        // 无效索引
        if (!index.isValid()) {
            break;
        }

        // 无效角色
        if (role != Qt::UserRole + 1) {
            break;
        }

        // 索引位置存在数据
        auto item_providers = get(index.row());

        // 无效内容
        if(!item_providers) {
            break;
        }

        return QVariant::fromValue(item_providers);
    } while(false);
    // return QAbstractListModel::data(index, role);
    return QVariant{};
}

bool ItemModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    do {
        // 无效索引
        if (!index.isValid()) {
            return false;
        }

        // 无效角色
        if (role != Qt::UserRole + 1) {
            break;
        }

        // 内容格式异常
        if(!value.canConvert<std::shared_ptr<ItemDelegate>>()) {
            break;
        }

        // 保存数据并通知更新视图
        auto provider = value.value<std::shared_ptr<ItemDelegate>>();
        replace(index.row(), provider);
        emit dataChanged(index, index, { Qt::DisplayRole, Qt::UserRole + 1 });
        return true;
    } while(false);
    // return QAbstractListModel::setData(index, value, role);  // “QAbstractItemModel::setData”: 调用非静态成员函数需要一个对象
    return false;
}

// 添加数据
void ItemModel::add(const std::shared_ptr<ItemDelegate>&item) {
    int row = rowCount();
    beginInsertRows(QModelIndex(), row, row);
    get().emplace_back(item);  // TODO:这里应该进行数据拷贝，防止外部更改影响到内部
    endInsertRows();
}

// 移除数据
void ItemModel::remove(const std::shared_ptr<ItemDelegate>&item) {
    auto row = get(item);
    beginRemoveRows(QModelIndex(), row, row);
    auto items = get();
    auto it = items.begin();
    std::advance(it, row);
    items.erase(it);
    endRemoveRows();
}

// 清空或替换
void ItemModel::reset(const std::vector<std::shared_ptr<ItemDelegate>>&items) {
    beginResetModel();
    get() = items;
    endResetModel();
}

// 获取数据组
const std::vector<std::shared_ptr<ItemDelegate>>& ItemModel::get() const {
    return m_items;
}

// 获取数据组
std::vector<std::shared_ptr<ItemDelegate>>& ItemModel::get() {
    return m_items;
}

//获取位置所在数据
std::shared_ptr<ItemDelegate> ItemModel::get(int pos) const {
    if (pos < 0 || pos >= rowCount()) { return nullptr; }
    return get().at(pos);
}

// 替换指定位置的元素
bool ItemModel::replace(int pos, std::shared_ptr<ItemDelegate>&provider) {
    if (pos < 0 || pos >= rowCount()) { return false; }
    get()[pos] = provider;
    return true;
}

// 获取数据所在位置
int ItemModel::get(const std::shared_ptr<ItemDelegate>&item_providers) const {
    int pos = -1;
    int index = 0;
    for(auto&ite : get()) {
        if (item_providers == ite) { return index; }  // 指针还是内容？？
        index++;
    }
    return pos;
}
