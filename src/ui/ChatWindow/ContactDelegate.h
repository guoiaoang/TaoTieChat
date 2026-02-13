#pragma once

#include <QStyledItemDelegate>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QtDebug>
#include <QFont>
#include <QFontMetrics>
#include <QColor>
#include <QSize>

#include "data.h"

class ContactDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit ContactDelegate(QObject *parent = nullptr);
    ~ContactDelegate();

public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
};
