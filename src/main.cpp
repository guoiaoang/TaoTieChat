#include "MainWindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

void InitDebugOutputFormat() {
    const QString QT_MESSAGE_PATTERN =
        "[%{time yyyy-MM-dd hh:mm:ss.zzz ttt}"
        " %{if-debug}D%{endif}"
        "%{if-info}I%{endif}"
        "%{if-warning}W%{endif}"
        "%{if-critical}C%{endif}"
        "%{if-fatal}F%{endif}]"
        " %{file}:%{line} - %{function}"
        " >> %{message}";
    qSetMessagePattern(QT_MESSAGE_PATTERN);
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "TaoTieChat_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    InitDebugOutputFormat();

    MainWindow w;
    w.show();

    return a.exec();
}
