#pragma once

#include <QStyledItemDelegate>
#include <QPainter>

#include "data.h"

class ContentDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit ContentDelegate(QObject *parent = nullptr);
    ~ContentDelegate();

public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
};
