#include "MediaWindow.h"
#include "ui_MediaWindow.h"

MediaWindow::MediaWindow(QWidget *parent)
    : QMainWindow(parent), m_ui(std::make_shared<Ui::MediaWindow>()) {
    m_ui->setupUi(this);

    // new
    m_toolbar            = std::make_shared<QToolBar>();
    m_menu_save          = std::make_shared<QMenu>();
    m_action_group       = std::make_shared<QActionGroup>(this);
    // new action
    m_action_open        = std::make_shared<QAction>();  // 打开
    m_action_save        = std::make_shared<QAction>();  // 保存
    m_action_save_as     = std::make_shared<QAction>();  // 另存为
    m_action_prev        = std::make_shared<QAction>();  // 上一个
    m_action_next        = std::make_shared<QAction>();  // 下一个
    m_action_fit         = std::make_shared<QAction>();  // 适应窗口
    m_action_zoom_in     = std::make_shared<QAction>();  // 放大
    m_action_zoom_out    = std::make_shared<QAction>();  // 缩小
    m_action_normal_size = std::make_shared<QAction>();  // 原始大小
    m_action_full_screen = std::make_shared<QAction>();  // 全屏显示
    m_action_pin         = std::make_shared<QAction>();  // 窗口置顶

    // // icon
    m_action_open        ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpen  ));
    m_action_save        ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSave  ));
    m_action_save_as     ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSaveAs));
    m_action_prev        ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoPrevious    ));
    m_action_next        ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoNext        ));
    m_action_fit         ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ZoomFitBest   ));
    m_action_zoom_in     ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ZoomIn        ));
    m_action_zoom_out    ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ZoomOut       ));
    m_action_normal_size ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ViewRestore   ));
    m_action_full_screen ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ViewFullscreen));
    m_action_pin         ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoUp          ));

    QColor icon_color = this->palette().color(QPalette::WindowText);
    static QIcon icon_nf_oct_pin                      = Utils::to_icon("", icon_color);
    static QIcon icon_nf_md_pin_outline               = Utils::to_icon("󰤱", icon_color);
    static QIcon icon_nf_md_pin_off_outline           = Utils::to_icon("󰤰", icon_color);
    static QIcon icon_nf_md_menu                      = Utils::to_icon("󰍜", icon_color);
    static QIcon icon_nf_md_menu_open                 = Utils::to_icon("󰮫", icon_color);
    static QIcon icon_nf_md_fullscreen                = Utils::to_icon("󰊓", icon_color);
    static QIcon icon_nf_md_window_minimize           = Utils::to_icon("󰖰", icon_color);
    static QIcon icon_nf_md_window_maximize           = Utils::to_icon("󰖯", icon_color);
    static QIcon icon_nf_md_window_restore            = Utils::to_icon("󰖲", icon_color);
    static QIcon icon_nf_oct_zoom_in                  = Utils::to_icon("", icon_color);
    static QIcon icon_nf_oct_zoom_out                 = Utils::to_icon("", icon_color);
    static QIcon icon_nf_cod_zoom_in                  = Utils::to_icon("", icon_color);
    static QIcon icon_nf_cod_zoom_out                 = Utils::to_icon("", icon_color);
    static QIcon icon_nf_md_folder_open_outline       = Utils::to_icon("󰷏", icon_color);
    static QIcon icon_nf_md_content_save_outline      = Utils::to_icon("󰠘", icon_color);
    static QIcon icon_nf_md_content_save_edit_outline = Utils::to_icon("󰳼", icon_color);
    static QIcon icon_nf_fa_arrow_right               = Utils::to_icon("", icon_color);
    static QIcon icon_nf_fa_arrow_left                = Utils::to_icon("", icon_color);
    static QIcon icon_nf_md_fit_to_screen_outline     = Utils::to_icon("󱣵", icon_color);
    static QIcon icon_nf_md_restore                   = Utils::to_icon("󰦛", icon_color);

    // icon
    m_action_open        ->setIcon(icon_nf_md_folder_open_outline      );
    m_action_save        ->setIcon(icon_nf_md_content_save_outline     );
    m_action_save_as     ->setIcon(icon_nf_md_content_save_edit_outline);
    m_action_prev        ->setIcon(icon_nf_fa_arrow_left               );
    m_action_next        ->setIcon(icon_nf_fa_arrow_right              );
    m_action_fit         ->setIcon(icon_nf_md_fit_to_screen_outline    );
    m_action_zoom_in     ->setIcon(icon_nf_oct_zoom_in                 );
    m_action_zoom_out    ->setIcon(icon_nf_oct_zoom_out                );
    m_action_normal_size ->setIcon(icon_nf_md_restore                  );
    m_action_full_screen ->setIcon(icon_nf_md_fullscreen               );
    m_action_pin         ->setIcon(icon_nf_md_pin_outline              );

    connect(m_action_full_screen.get(), &QAction::triggered, this, [this]() {
        // 用于恢复之前的窗口大小状态
        static auto window_state_old = this->windowState();
        if(m_action_full_screen->isChecked()) {
            // 更新保存之前的窗口大小状态
            window_state_old = this->windowState();
            // 执行命令
            this->showFullScreen();
            // m_action_full_screen ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ListRemove));
            m_action_full_screen ->setIcon(icon_nf_md_window_restore);
        } else {
            // 还原窗口状态
            if(window_state_old == Qt::WindowNoState) {
                this->showNormal();
            }
            else if(window_state_old == Qt::WindowMinimized) {
                this->showMinimized();
            }
            else if(window_state_old == Qt::WindowMaximized) {
                // TODO: QT 的 showFullScreen() 状态到 showMaximized() 状态，会先恢复常规大小后再最大化
                this->showMaximized();
            }
            else if(window_state_old == Qt::WindowFullScreen) {
                this->showFullScreen();
            }
            // m_action_full_screen ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ViewFullscreen));
            m_action_full_screen ->setIcon(icon_nf_md_fullscreen);
        }
    });
    connect(m_action_pin.get(), &QAction::triggered, this, [this]() {
        if(m_action_pin->isChecked()) {
            this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
            // m_action_pin ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoDown));
            m_action_pin ->setIcon(icon_nf_md_pin_off_outline);
        } else {
            this->setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
            // m_action_pin ->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoUp));
            m_action_pin ->setIcon(icon_nf_md_pin_outline);
        }
        show(); // 必须调用 show() 重新刷新窗口状态
    });
    connect(m_action_fit.get()        , &QAction::triggered, this, [this](){ m_fit_to_window = true ; update_media(); });
    connect(m_action_normal_size.get(), &QAction::triggered, this, [this](){ m_fit_to_window = false; update_media(); });

    connect(m_action_open.get()   , &QAction::triggered, this, &MediaWindow::on_media_open   );
    connect(m_action_save.get()   , &QAction::triggered, this, &MediaWindow::on_media_save   );
    connect(m_action_save_as.get(), &QAction::triggered, this, &MediaWindow::on_media_save_as);

    // text
    m_action_open        ->setText(tr("Open"         ));
    m_action_save        ->setText(tr("Save"         ));
    m_action_save_as     ->setText(tr("Save As"      ));
    m_action_prev        ->setText(tr("Prev"         ));
    m_action_next        ->setText(tr("Next"         ));
    m_action_fit         ->setText(tr("Fit To Window"));
    m_action_zoom_in     ->setText(tr("Zoom In"      ));
    m_action_zoom_out    ->setText(tr("Zoom Out"     ));
    m_action_normal_size ->setText(tr("Normal Size"  ));
    m_action_full_screen ->setText(tr("Full Screen"  ));
    m_action_pin         ->setText(tr("Pin"          ));

    // shortcut
    m_action_open       ->setShortcut(QKeySequence::Open      );
    m_action_save       ->setShortcut(QKeySequence::Save      );
    m_action_save_as    ->setShortcut(QKeySequence::SaveAs    );
    m_action_prev       ->setShortcut(QKeySequence::Back      );
    m_action_next       ->setShortcut(QKeySequence::Forward   );
    m_action_zoom_in    ->setShortcut(QKeySequence::ZoomIn    ); // Qt::CTRL | Qt::Key_Plus );  // zoom in
    m_action_zoom_out   ->setShortcut(QKeySequence::ZoomOut   ); // Qt::CTRL | Qt::Key_Minus);  // zoom out
    m_action_full_screen->setShortcut(QKeySequence::FullScreen);

    // property // Enabled
    m_action_prev->setEnabled(false);
    m_action_next->setEnabled(false);
    // // 功能未实现，先禁用
    // m_action_open        ->setEnabled(false);
    // m_action_save        ->setEnabled(false);  // 会同时禁用菜单
    // m_action_save_as     ->setEnabled(false);
    m_action_prev        ->setEnabled(false);
    m_action_next        ->setEnabled(false);
    // m_action_fit         ->setEnabled(false);
    m_action_zoom_in     ->setEnabled(false);
    m_action_zoom_out    ->setEnabled(false);
    // m_action_normal_size ->setEnabled(false);
    // m_action_full_screen ->setEnabled(false);
    // m_action_pin         ->setEnabled(false);
    // property // Checkable
    m_action_fit        ->setCheckable(true);
    m_action_zoom_in    ->setCheckable(true);
    m_action_zoom_out   ->setCheckable(true);
    m_action_normal_size->setCheckable(true);
    m_action_full_screen->setCheckable(true);
    m_action_pin        ->setCheckable(true);
    // property // Checked
    m_action_fit->setChecked(true);

    // add to group
    m_action_group->addAction(m_action_fit        .get());
    m_action_group->addAction(m_action_zoom_in    .get());
    m_action_group->addAction(m_action_zoom_out   .get());
    m_action_group->addAction(m_action_normal_size.get());

    // action saveas  // 另存为按钮作为扩展按钮 - 保存按钮的菜单
    m_menu_save->addAction(m_action_save_as.get());
    m_action_save->setMenu(m_menu_save.get());
    // m_menu_save->setStyle();
    m_action_open       ->setMenuRole(QAction::ApplicationSpecificRole);
    m_action_full_screen->setMenuRole(QAction::TextHeuristicRole);

    // add to toolbar
    m_toolbar->addAction (m_action_open.get()            );
    // m_toolbar->addAction (m_action_save.get()            ); m_toolbar->addSeparator();
    m_toolbar->addAction (m_action_save_as.get()         ); m_toolbar->addSeparator(); // 不添加保存按钮，只添加另存为按钮
    m_toolbar->addAction (m_action_prev.get()            );
    m_toolbar->addAction (m_action_next.get()            ); m_toolbar->addSeparator();
    m_toolbar->addActions(m_action_group->actions()); m_toolbar->addSeparator();
    m_toolbar->addAction (m_action_full_screen.get()     );
    m_toolbar->addAction (m_action_pin.get()             );
    // toolbar options
    m_toolbar->setWindowTitle(tr("Toolbar"));
    m_toolbar->setMovable(false);
    m_toolbar->setIconSize(QSize(32, 32)); // 这里设置显示的图标大小
    m_toolbar->setContextMenuPolicy(Qt::PreventContextMenu);  // 不显示右键菜单
    // add toolbar to window
    this->addToolBar(m_toolbar.get());

    connect(this, &MediaWindow::media_changed, m_ui->label, &QLabel::setPixmap);
    m_ui->label->setAlignment(Qt::AlignCenter);
    // 设置 label 的尺寸策略，使其能够收缩以避免出现滚动条
    m_ui->label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    // 不自动缩放内容
    m_ui->label->setScaledContents(false);
    // m_ui->label->installEventFilter(this);
    // m_ui->layout_central->installEventFilter(this);
    m_ui->scrollArea->viewport()->installEventFilter(this);
    // m_ui->scrollArea->setWidgetResizable(true);

    // 窗口最大化、还原时触发图片自动拉伸
    this->installEventFilter(this);
}

MediaWindow::~MediaWindow() {
    if (m_toolbar           ) { m_toolbar           .reset(); }
    if (m_menu_save         ) { m_menu_save         .reset(); }
    if (m_action_group      ) { m_action_group      .reset(); }
    if (m_action_open       ) { m_action_open       .reset(); }
    if (m_action_save       ) { m_action_save       .reset(); }
    if (m_action_save_as    ) { m_action_save_as    .reset(); }
    if (m_action_prev       ) { m_action_prev       .reset(); }
    if (m_action_next       ) { m_action_next       .reset(); }
    if (m_action_fit        ) { m_action_fit        .reset(); }
    if (m_action_zoom_in    ) { m_action_zoom_in    .reset(); }
    if (m_action_zoom_out   ) { m_action_zoom_out   .reset(); }
    if (m_action_normal_size) { m_action_normal_size.reset(); }
    if (m_action_pin        ) { m_action_pin        .reset(); }
    if (m_ui                ) { m_ui                .reset(); }
}

void MediaWindow::clean() {
    m_media_path   = QString();
    m_media_pixmap = QPixmap();
    m_media_name   = QString();
    update_title();
}

bool MediaWindow::load(const QString&media_path) {
    QPixmap pixmap(media_path);
    if(load(pixmap)) {
        m_media_path = media_path;
        return true;
    }
    return false;
}

bool MediaWindow::load(const QPixmap&media_pixmap) {
    clean();
    m_media_pixmap = media_pixmap;
    emit media_changed(media_pixmap);
    update_media();
    update_title();

    if(!media_pixmap.isNull()) {
        return true;
    }
    return false;
}

void MediaWindow::set_media_name(const QString&media_name) {
    m_media_name = media_name;
    update_title();
}

bool MediaWindow::update_media() {
    if(m_fit_to_window) {
        // 更新缩放图片：使其适应 Label 大小，并保持纵横比
        // Qt::KeepAspectRatio 保证图片不变形
        // Qt::SmoothTransformation 保证缩放后的图片清晰（消除锯齿）
        QPixmap pixmap_scaled = m_media_pixmap.scaled(m_ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_ui->label->setPixmap(pixmap_scaled);
    }
    else {
        // 检测精度
        const double ff = 0.001;
        if(m_scale_factory > 1.0 - ff && m_scale_factory < 1.0 + ff) {
            // 如果缩放倍数为 1 附近的值，那么直接显示原图，避免不必要的缩放计算和可能的模糊
            m_ui->label->setPixmap(m_media_pixmap);
        }
        else {
            // 缩放倍数不为 1 时需要进行处理
            QPixmap pixmap_scaled = m_media_pixmap.scaled(m_media_pixmap.size() * m_scale_factory, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_ui->label->setPixmap(pixmap_scaled);
        }
    }
    return true;

    // // 更新缩放图片：使其适应 ScrollArea 的 viewport 大小，并保持纵横比
    // // 使用 scrollArea->viewport()->size() 而不是 label->size()
    // // 这样当窗口缩小时，图片也会相应缩小
    // // Qt::KeepAspectRatio 保证图片不变形
    // // Qt::SmoothTransformation 保证缩放后的图片清晰（消除锯齿）
    // QSize viewport_size = ui->scrollArea->viewport()->size();
    // QPixmap pixmap_scaled = m_media_pixmap.scaled(viewport_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    // ui->label->setPixmap(pixmap_scaled);
    // ui->label->resize(pixmap_scaled.size()); // 调整 label 大小以匹配图片
    // return true;
}

bool MediaWindow::update_title() {
    QPixmap label_pixmap = m_ui->label->pixmap();
    if(!m_media_name.isEmpty()) {
        this->setWindowTitle(label_pixmap.isNull() ? tr("%1 : Load failed!").arg(m_media_name) : m_media_name);
    }
    else if(m_media_path.isEmpty()) {
        this->setWindowTitle(label_pixmap.isNull() ? tr("%1 : Load failed!").arg(m_media_path) : m_media_path);
    }
    else {
        this->setWindowTitle(label_pixmap.isNull() ? QApplication::applicationName() : tr("Pixmap"));
    }
    return true;
}

bool MediaWindow::eventFilter(QObject *watched, QEvent *event) {
    if(event->type() == QEvent::Resize) {
        if(watched == m_ui->label) {
            update_media();
        }
        else if(watched == m_ui->scrollArea->viewport()) {
            update_media();
        }
    }
    else if(event->type() == QEvent::WindowStateChange) {
        // 窗口最大化与还原时可能需要手动触发图片缩放的更新
        update_media();
    }
    return QMainWindow::eventFilter(watched, event);
}

void MediaWindow::on_media_open() {
    QFileDialog dialog(this, tr("Open File"));
    QFileDialog::AcceptMode acceptMode = QFileDialog::AcceptOpen;

    const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    for (const QByteArray &mimeTypeName : supportedMimeTypes) {
        mimeTypeFilters.append(mimeTypeName);
    }
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    if (acceptMode == QFileDialog::AcceptSave) {
        dialog.setDefaultSuffix("jpg");
    }

    auto loadFile = [this](const QString &fileName) {
        QPixmap pixmap(fileName);
        if(pixmap.isNull()) {
            QMessageBox::information(this, QGuiApplication::applicationDisplayName(), tr("Cannot load %1").arg(QDir::toNativeSeparators(fileName)));
            return false;
        }

        m_media_pixmap = pixmap;

        setWindowFilePath(fileName);

        const QString message = tr("Opened \"%1\", %2x%3, Depth: %4").arg(QDir::toNativeSeparators(fileName)).arg(pixmap.width()).arg(pixmap.height()).arg(pixmap.depth());
        statusBar()->showMessage(message);
        return true;
    };

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}

void MediaWindow::on_media_save() {
    // QFileInfo file_info(m_media_path);
}

void MediaWindow::on_media_save_as() {
    QFileDialog dialog(this, tr("Save File As"));
    QFileDialog::AcceptMode acceptMode = QFileDialog::AcceptSave;

    const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    for (const QByteArray &mimeTypeName : supportedMimeTypes) {
        mimeTypeFilters.append(mimeTypeName);
    }
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    if (acceptMode == QFileDialog::AcceptSave) {
        dialog.setDefaultSuffix("jpg");
    }

    auto saveFile = [this](const QString &fileName) {
        if(!m_media_pixmap.save(fileName)) {
            QMessageBox::information(this, QGuiApplication::applicationDisplayName(), tr("Cannot write %1").arg(QDir::toNativeSeparators(fileName)));
            return false;
        }
        const QString message = tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName));
        statusBar()->showMessage(message);
        return true;
    };

    while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first())) {}
}
