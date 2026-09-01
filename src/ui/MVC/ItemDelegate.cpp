#include "ItemDelegate.h"
#include "ItemModel.h"
#include "Accounts.h"
#include "MessageDelegate.h"

void ItemDelegate::register_delegates() const {
    if (!m_map_delegates.contains(ENUMS::ItemDataType::GROUP)) {
        m_map_delegates[ENUMS::ItemDataType::GROUP] = std::make_shared<ItemDelegateGroup>();
    }
    if (!m_map_delegates.contains(ENUMS::ItemDataType::CONTACT)) {
        m_map_delegates[ENUMS::ItemDataType::CONTACT] = std::make_shared<ItemDelegateContact>();
    }
    if (!m_map_delegates.contains(ENUMS::ItemDataType::CHATROOM)) {
        m_map_delegates[ENUMS::ItemDataType::CHATROOM] = std::make_shared<ItemDelegateChatroom>();
    }
    // if (!m_map_delegates.contains(ENUMS::ItemDataType::MESSAGE)) {
    //     m_map_delegates[ENUMS::ItemDataType::MESSAGE] = std::make_shared<ItemDelegateMessage>();
    // }
    if (!m_map_delegates.contains(ENUMS::ItemDataType::MESSAGE)) {
        m_map_delegates[ENUMS::ItemDataType::MESSAGE] = std::make_shared<MessageDelegate>();
    }
    // std::map<ENUMS::ItemDataType, std::shared_ptr<QStyledItemDelegate>> m;
    // m.insert(std::make_pair<ENUMS::ItemDataType, std::shared_ptr<QStyledItemDelegate>>(ENUMS::ItemDataType::GROUP, std::make_shared<ItemDelegateGroup>()));
    // m[ENUMS::ItemDataType::MESSAGE] = std::make_shared<ItemDelegateMessage>();

    // std::map<ENUMS::ItemDataType, QStyledItemDelegate*> p;
    // p.insert(std::make_pair<ENUMS::ItemDataType, QStyledItemDelegate*>(ENUMS::ItemDataType::GROUP, new ItemDelegateGroup));

    // m_map_delegates.insert(std::make_pair<ENUMS::ItemDataType, std::shared_ptr<QStyledItemDelegate>>(ENUMS::ItemDataType::GROUP, std::make_shared<ItemDelegateGroup>()));
    // qDebug().noquote().nospace() << "...";
}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    do{
        if (!index.isValid()) { break; }

        QVariant variant = index.data(ENUMS::role_item_type());
        if (!variant.isValid() || !variant.canConvert<ENUMS::ItemDataType>()) { break; }

        register_delegates();

        auto type = variant.value<ENUMS::ItemDataType>();
        // auto delegate = factory.create<type>();
        if (!m_map_delegates.contains(type)) { break; }

        m_map_delegates[type]->paint(painter, option, index);
        return;  // 子控件绘制完成后不需要基础控件再次绘制
    } while(false);
    QStyledItemDelegate::paint(painter, option, index);
}

// **注意：`sizeHint()` 返回的是初始大小，不会跟随 `resize()` 调整，宽度 `-1` 表示不关心，由系统实时计算。**
QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    do{
        if (!index.isValid()) { break; }

        QVariant variant = index.data(ENUMS::role_item_type());
        if (!variant.isValid() || !variant.canConvert<ENUMS::ItemDataType>()) { break; }

        register_delegates();

        auto type = variant.value<ENUMS::ItemDataType>();
        if (!m_map_delegates.contains(type)) { break; }

        return m_map_delegates[type]->sizeHint(option, index);;
    } while(false);
    return QStyledItemDelegate::sizeHint(option, index);
}

bool ItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) {
    do{
        if (!index.isValid()) { break; }

        QVariant variant = index.data(ENUMS::role_item_type());
        if (!variant.isValid() || !variant.canConvert<ENUMS::ItemDataType>()) { break; }

        register_delegates();

        auto type = variant.value<ENUMS::ItemDataType>();
        if (!m_map_delegates.contains(type)) { break; }

        return m_map_delegates[type]->editorEvent(event, model, option, index);
    } while(false);
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
