#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "AuthWindow.h"

// 由于 `QStackedWidget::addWidget()` 会修改子窗口大小，
// 为保障子窗口大小不被意外修改，改为 QStackedWidget 最多只持有当前子窗口，
// 且先将主窗口大小设为子窗口大小，这样在拉伸主窗口时可以有效的修改子窗口的大小，
// 同时 QStackedWidget 不会影响其他子窗口的大小

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    // this->installEventFilter(this);  // 用于调试
    ui->setupUi(this);
    menuBar()->hide();
    statusBar()->hide();

    // // 1.创建
    m_auth_window = new AuthWindow(this);
    m_chat_window = new ChatWindow(this);

    // // 2.连接
    // 根据子窗口来调整父窗口的大小和标题等内容
    connect(ui->main_stacked, &QStackedWidget::currentChanged, this, [&](){ this->on_show_widget_change(); });
    connect(m_auth_window   , &AuthWindow    ::accepted      , this, [&](){ this->show_chat_window     (); });
    connect(m_chat_window   , &ChatWindow    ::accepted      , this, [&](){ this->show_auth_window     (); });  // 用于测试页面切换
    connect(m_chat_window   , &ChatWindow    ::signout       , this, [&](){ this->show_auth_window     (); });  // 退出登录

    // // 3.设置
    // m_auth_window->installEventFilter(this);  // debug
    // m_chat_window->installEventFilter(this);  // debug
    m_auth_window->setMinimumSize(1,1); // 设置为 (0,0) 并不能缩到最小
    m_chat_window->setMinimumSize(1,1);
    // 先调整主窗口大小再添加显示
    // 默认显示登陆页面
    this->resize(m_auth_window->size());
    ui->main_stacked->addWidget(m_auth_window);
    ui->main_stacked->setCurrentWidget(m_auth_window);

    // TODO:登陆检查
    QTimer::singleShot(0, this, &MainWindow::on_init);

    // // 4.显示
    // this->resize(600, 400);
}

MainWindow::~MainWindow() {
    delete ui;
    ui = nullptr; // 必须置空
    if(m_auth_window) {
        delete m_auth_window;
        m_auth_window = nullptr;
    }
    if(m_chat_window) {
        delete m_chat_window;
        m_chat_window = nullptr;
    }
}

// 用于调试
bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    qDebug() << " className():" << obj->metaObject()->className()
             << " event->type():" << QMetaEnum::fromType<QEvent::Type>().valueToKey(event->type());
    return QObject::eventFilter(obj, event);
}

void MainWindow::on_init() {
    // 检查本地是否有可用的登陆记录并执行自动登陆
    if(!this->check_login()) {
        show_auth_window();
    }
}

void MainWindow::on_show_widget_change() {
    if(!ui || !ui->main_stacked || !ui->main_stacked->currentWidget()) {
        return;  // 销毁时会触发
    }
    QWidget *widget = ui->main_stacked->currentWidget();
    // qDebug() << widget->metaObject()->className() << " size : " << widget->size();  // debug
    // qDebug() << this  ->metaObject()->className() << " size : " << this  ->size();  // debug
    // this->resize(widget->size());  // 这里设置就晚了
    this->setWindowTitle   (widget->windowTitle   ());
    this->setWindowIcon    (widget->windowIcon    ());
    this->setWindowIconText(widget->windowIconText());
    connect(widget, &QWidget::windowTitleChanged   , this, [&](const QString &title   ){ this->setWindowTitle   (title   ); });
    connect(widget, &QWidget::windowIconChanged    , this, [&](const QIcon   &icon    ){ this->setWindowIcon    (icon    ); });
    connect(widget, &QWidget::windowIconTextChanged, this, [&](const QString &iconText){ this->setWindowIconText(iconText); });
    // connect(widget, &QWidget::customContextMenuRequested, this, [&](const QPoint &pos){this->setWindowIcon(icon);});
}

bool MainWindow::check_login() {
    return false;  // 无有效的登陆信息，需要用户手动登陆
}

bool MainWindow::show_window(QWidget*widget) {
    if(!widget) {
        return false;
    }
    // 先清除已经添加的其他子窗口，防止意外的更改
    for(int i = 0; i< ui->main_stacked->count(); ++i) {
        ui->main_stacked->removeWidget(ui->main_stacked->widget(i));
    }
    // 先调整主窗口大小再添加显示
    this->resize(widget->size());
    // this->setGeometry(widget->geometry());  // 子控件在 (0,0) 位置，该方式不可用
    ui->main_stacked->addWidget(widget);
    ui->main_stacked->setCurrentWidget(widget);

    // 设置居中显示窗口
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, this->size(), qApp->primaryScreen()->availableGeometry()));
    return true;
}

bool MainWindow::show_auth_window() {
    return show_window(m_auth_window);
}

bool MainWindow::show_chat_window() {
    return show_window(m_chat_window);
}
