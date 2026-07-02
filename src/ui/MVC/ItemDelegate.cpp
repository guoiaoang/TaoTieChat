#include "ItemDelegate.h"
#include "ItemModel.h"

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    do{
        if(!index.isValid()) { break; }

        auto index_data = index.data(Qt::UserRole + 1);
        if(!index_data.canConvert<std::shared_ptr<ItemDelegate>>()) { break; }

        auto item_provider = index_data.value<std::shared_ptr<ItemDelegate>>();
        if(!item_provider) { break; }

        if(item_provider.get() == this) { break; }

        item_provider->paint(painter, option, index);
        return;
    } while(false);
    QStyledItemDelegate::paint(painter, option, index);
}

// **注意：`sizeHint()` 返回的是初始大小，不会跟随 `resize()` 调整，宽度 `-1` 表示不关心，由系统实时计算。**
QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    do{
        if(!index.isValid()) { break; }

        auto index_data = index.data(Qt::UserRole + 1);
        if(!index_data.canConvert<std::shared_ptr<ItemDelegate>>()) { break; }

        auto item_provider = index_data.value<std::shared_ptr<ItemDelegate>>();
        if(!item_provider) { break; }

        if(item_provider.get() == this) { break; }

        return item_provider->sizeHint(option, index);
    } while(false);
    // return QSize(option.rect.width(), 80);
    return QStyledItemDelegate::sizeHint(option, index);
}

bool ItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    do{
        if(!index.isValid()) { break; }

        auto index_data = index.data(Qt::UserRole + 1);
        if(!index_data.canConvert<std::shared_ptr<ItemDelegate>>()) { break; }

        auto item_provider = index_data.value<std::shared_ptr<ItemDelegate>>();
        if(!item_provider) { break; }

        if(item_provider.get() == this) { break; }

        return item_provider->editorEvent(event, model, option, index);
        // return true;  // 已经处理时直接返回
    } while(false);
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
