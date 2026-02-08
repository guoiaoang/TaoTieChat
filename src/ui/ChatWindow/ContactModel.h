#pragma once

#include <QAbstractListModel>
#include "data.h"

class ContactModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit ContactModel(QObject *parent = nullptr);
    explicit ContactModel(Contacts contacts, QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // ====================================================================== //

    void add_contact(Contact contact);
    void remove_contact(Contact contact);
    Contact contact(const QModelIndex &index) const;

private:
    Contacts m_contacts;
};
