// #include "MainWindow.h"
#include "Utils.h"
#include "AuthWindow.h"
#include "ChatWindow.h"
#include "SignalInner.h"

#if defined(Q_OS_WIN) && defined(_DEBUG)
#include <QDebug>
#include <Windows.h>
// QtMessageHandler 消息回调
void MessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message) {
    // if (type == QtWarningMsg && message.contains("QTimer::start")) {
    if (type == QtWarningMsg) {
        OutputDebugStringW(QStringLiteral("Qt Warning: %1\n").arg(message).toStdWString().c_str());
        // __debugbreak(); // VS Code / Visual Studio 自动中断
    }
}
#endif // #if defined(Q_OS_WIN) && defined(_DEBUG)

int main(int argc, char *argv[]) {
    Q_INIT_RESOURCE_EXPAND(PROJECT_NAME);
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName  (TOSTRING(ORGANIZATION_NAME)  );
    QCoreApplication::setOrganizationDomain(TOSTRING(ORGANIZATION_DOMAIN));
    QCoreApplication::setApplicationName   (TOSTRING(PROJECT_NAME)       );

#if defined(Q_OS_WIN) && defined(_DEBUG)
    if (0) {
        QtMessageHandler default_message_handler = qInstallMessageHandler(MessageHandler);
    }
#endif // #if defined(Q_OS_WIN) && defined(_DEBUG)

    Utils::init_debug_output_format();
    Utils::load_translator(application);
    Utils::load_font(":/rc/fonts/InconsolataNerdFontMono-Regular.ttf");

    AuthWindow window_auth;
    ChatWindow window_chat;
    QObject::connect(SignalInner::instance(), &SignalInner::signal_signin, [&](){
        window_auth.hide();
        window_chat.reinit();  // 重新初始化界面及数据
        window_chat.show();
    });
    QObject::connect(SignalInner::instance(), &SignalInner::signal_signout, [&](){
        window_auth.view();
        window_chat.hide();
    });
    window_auth.view();

    return application.exec();
}
