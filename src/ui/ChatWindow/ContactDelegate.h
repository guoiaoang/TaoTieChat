#pragma once

#include "Headers.h"
#include "data.h"
#include "Utils.h"

class ContactDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit ContactDelegate(QObject *parent = nullptr);
    ~ContactDelegate();

public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    // 无法在绘图函数中传出参数，需要预先计算高度
    void set_height(int height) { m_height = height; }
    int get_height() const { return m_height; }

private:
    int m_height = 80;
};
