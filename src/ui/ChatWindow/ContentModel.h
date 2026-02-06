#pragma once

#include <QAbstractListModel>
#include "data.h"

class ContentModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit ContentModel(QObject *parent = nullptr);
    explicit ContentModel(Contents contents, QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // ====================================================================== //

    void add_content(Content content);
    void remove_content(Content content);

private:
    Contents m_contents;
};
