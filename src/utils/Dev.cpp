#include "Dev.h"
#include "MediaWindow.h"
#include "ChatWindow.h"

// 开发测试代码
namespace Dev{

MediaWindowDev::MediaWindowDev(MediaWindow*media_window, QWidget *parent) {
    // 原生标题栏不支持显示自定义字体

    QLabel *label_widget = new QLabel;

    QSize size;
    size = label_widget->size();
    // size = media_window->screen()->availableGeometry().size();

    QString label_text;
    // // 非空格，实际为 unicode 图标字体字符 // 运行流畅 // 原文字符
    label_text = QString("󰤱󰤰");  // `\uf435`, `\udb82\udd31`, `\udb82\udd30`
    // // 直接赋值  // 运行卡顿，仅绘制出了第一个字符
    // label_text = QString::fromUcs4(U"\uf435 \U00018d31 \U00018d30");
    // // 使用 UTF-16 转义，需确保编译器支持  // 编译失败
    // label_text = QString::fromUtf16(u"\uf435 \udb82\udd31 \udb82\udd30");

    QFont label_font = Utils::get_suitable_font_size("Inconsolata Nerd Font Mono", size, label_text);

    QPalette label_palette = label_widget->palette();
    label_palette.setBrush(QPalette::WindowText, QBrush(QColor(Qt::red)));

    label_widget->setAlignment(Qt::AlignCenter);
    label_widget->setFont   (label_font   );
    label_widget->setText   (label_text   );
    label_widget->setPalette(label_palette);

    media_window->setCentralWidget(label_widget);
}

MediaWindowDev::~MediaWindowDev() {
}

// ========================================================================== //

ChatWindowDev::ChatWindowDev(ChatWindow*chat_window, QWidget *parent) {
    // list path ":/"
    if(0)
    {
        QList<QFileInfo> file_infos = Utils::dir_contents(":/", true);
        qDebug().noquote().nospace() << "Total files: " << file_infos.size();

        int width = QString::number(file_infos.size()).length();

        for(int i = 0; i < file_infos.size(); ++i) {
            QFileInfo file_info = file_infos.at(i);
            QString indexStr = QString("%1").arg(i, width, 10, QChar(' '));
            QString typeStr = file_info.isFile() ? " [File]" : (file_info.isDir() ? "  [Dir]" : "[other]");
            qDebug().noquote().nospace() << indexStr << typeStr << " " << file_info.absoluteFilePath();
        }
    }
    // contact test data
    {
        // ContactManager
        {
            // // Contact contact = ContactManager::create();
            // Contact contact;
            // auto p = &contact;
            // qDebug().noquote().nospace() << "p : " << (void*)p;
            // // 地址不同，赋值也只是作用于临时对象
            // [](Contact contact = nullptr) {
            //     auto p = &contact;
            //     qDebug().noquote().nospace() << "p : " << (void*)p;
            //     if(contact == nullptr) {
            //         contact = ContactManager::create();
            //     }
            // }(contact);
            // // 引用不能默认为 nullptr  // ok
            // [](Contact&contact) {
            //     if(contact == nullptr) {
            //         contact = ContactManager::create();
            //     }
            // }(contact);
            // [](Contact &contact = Contact()) {}(contact); // 无法绑定左值引用
            // [](Contact&&contact = Contact()) {}(contact); // 调用参数不匹配
            // // 太折腾
            // [](Contact *contact = nullptr) {
            //     if(contact == nullptr) {
            //         return;
            //     }
            //     if(*contact == nullptr) {
            //         *contact = ContactManager::create();
            //     }
            // }(&contact);

            ContactManager cm(ContactManager::create());
            cm.set_account_id ("Contact ID 01"      );
            // cm.set_avatar_pixmap(QPixmap());
            cm.set_avatar_path("Contact Avatar 01"  );
            cm.set_nick_name  ("Contact NickName 01");
            cm.set_notes      ("Contact Notes 01"   );
            chat_window->add_contact(cm.contact());

            chat_window->add_contact(ContactManager("Contact ID 02", QPixmap(), "Contact Avatar 02", "Contact NickName 02", "Contact Notes 02").contact());
            chat_window->add_contact(ContactManager("Contact ID 03", QPixmap(), "Contact Avatar 03", "Contact NickName 03", "Contact Notes 03").contact());
            chat_window->add_contact(
                ContactManager(
                    "Contact ID 04",
                    QPixmap(":/qt-project.org/qmessagebox/images/qtlogo-64.png"),
                    ":/qt-project.org/qmessagebox/images/qtlogo-64.png",
                    "Contact NickName 04",
                    "Contact Notes 04"
                    ).contact()
                );
        }

        // test data
        {
#ifdef DEBUG
            // ":/"
            {
                for(QDirListing::DirEntry file : QDirListing(":/", QDirListing::IteratorFlag::Recursive | QDirListing::IteratorFlag::FilesOnly)) {
                    QString path = file.absoluteFilePath();
                    // if(!pixmap.isNull()) {
                    QString count = QString::number(chat_window->m_contact_model->contacts_size());
                    chat_window->add_contact(ContactManager("Contact :/ ID " + count, QPixmap(path), path, "Contact :/ NickName " + count, path).contact());
                }
            }
            // enum QStyle::StandardPixmap
            {
                QMetaEnum metaEnum = QMetaEnum::fromType<QStyle::StandardPixmap>();
                for(int index = 0; index < metaEnum.keyCount(); ++index) {
                    QPixmap pixmap = style()->standardPixmap(QStyle::StandardPixmap(index));
                    QString path   = QString("QStyle::StandardPixmap::%1").arg(metaEnum.valueToKey(index));
                    QString count  = QString::number(chat_window->m_contact_model->contacts_size());
                    chat_window->add_contact(ContactManager("Contact standardPixmap ID " + count, pixmap, path, "Contact standardPixmap NickName " + count, path).contact());
                }
            }
            // [since 6.7] enum class QIcon::ThemeIcon
            {
                // 最后一个元素不能使用，会造成程序静默退出
                QMetaEnum metaEnum = QMetaEnum::fromType<Rebuild::QIcon::ThemeIcon>();
                // int count_len = QString::number(metaEnum.keyCount()).length();
                for(int index = 0; index < metaEnum.keyCount() - 1; ++index) {
                    // QPixmap::fromImage(QImage(64, 64, QImage::Format_ARGB32_Premultiplied));
                    QIcon icon = QIcon::fromTheme(QIcon::ThemeIcon(index));
                    QPixmap pixmap = icon.pixmap(256);
                    QString path   = QString("QIcon::ThemeIcon::%1").arg(metaEnum.valueToKey(index));
                    QString count  = QString::number(chat_window->m_contact_model->contacts_size());
                    chat_window->add_contact(ContactManager("Contact ThemeIcon ID " + count, pixmap, path, "Contact ThemeIcon NickName " + count, path).contact());
                }
            }
#endif // DEBUG
        }
    }

    // select
    if(0)
    {
        // 推荐 `QItemSelectionModel::currentChanged`
        // currentChanged：侧重于“焦点”在哪一行（通过键盘上下键移动焦点时触发，即便没有勾选选框）。
        // selectionChanged：侧重于“高亮/勾选”状态的变化（在多选模式下，即使用户只是增加了一个选中项，也会触发）。

        // // C++ 实现示例
        // connect(ui->page_list_contact->selectionModel(), &QItemSelectionModel::currentChanged, this, [&](const QModelIndex &current, const QModelIndex &previous) {
        //             int currentRow = current.row();
        //             // 在这里处理行号变化
        //             qDebug() << "current row : " << currentRow;
        //         });
        // // 连接信号
        // connect(ui->page_list_contact->selectionModel(), &QItemSelectionModel::selectionChanged, this, [&](const QItemSelection &selected, const QItemSelection &deselected) {
        //             // 获取所有当前选中的索引
        //             QModelIndexList indexes = ui->page_list_contact->selectionModel()->selectedIndexes();
        //             for (const QModelIndex &index : indexes) {
        //                 qDebug() << "current row : " << index.row();
        //             }
        //         });

        // // 鼠标左键单击会先后触发 pressed, clicked;
        // // 鼠标左键双击击会先后触发 pressed, clicked, doubleClicked, activated;
        // // 回车仅会触发 activated;  // 微信回车键无反应
        // // 空格及方向键均无消息。
        // // 上下方向键不触发 clicked 等信号，但会触发 currentChanged
        // connect(ui->page_list_contact, &QListView::pressed        , this, [&](const QModelIndex &index) { qDebug() << "QListView::pressed         " << index; } );
        // connect(ui->page_list_contact, &QListView::clicked        , this, [&](const QModelIndex &index) { qDebug() << "QListView::clicked         " << index; } );
        // connect(ui->page_list_contact, &QListView::doubleClicked  , this, [&](const QModelIndex &index) { qDebug() << "QListView::doubleClicked   " << index; } );
        // connect(ui->page_list_contact, &QListView::activated      , this, [&](const QModelIndex &index) { qDebug() << "QListView::activated       " << index; } );
        // connect(ui->page_list_contact, &QListView::entered        , this, [&](const QModelIndex &index) { qDebug() << "QListView::entered         " << index; } );
        // connect(ui->page_list_contact, &QListView::viewportEntered, this, [&](                        ) { qDebug() << "QListView::viewportEntered "         ; } );
        // connect(ui->page_list_contact, &QListView::iconSizeChanged, this, [&](const QSize       &size ) { qDebug() << "QListView::iconSizeChanged " << size ; } );

        // 使用 currentChanged 信号支持键盘上下键导航和鼠标点击
        // connect(ui->page_list_contact->selectionModel(), &QItemSelectionModel::currentChanged, this, &ChatWindow::on_contact_selection_changed);
    }
}

ChatWindowDev::~ChatWindowDev() {
}

// ========================================================================== //

}
