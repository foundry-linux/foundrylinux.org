#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QUrl>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("foundry-welcome");
    app.setOrganizationName("foundrylinux");

    const QString sentinel = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                             + "/foundry-welcome-shown";
    if (QFile::exists(sentinel)) {
        return 0;
    }

    // Allow overriding QML location for development testing.
    const char *qmlEnv = qgetenv("FOUNDRY_WELCOME_QML_DIR").constData();
    const QString qmlDir = qmlEnv && *qmlEnv
                           ? QString::fromUtf8(qmlEnv)
                           : QStringLiteral("/usr/share/foundry-welcome/qml");

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::quit, &app, &QGuiApplication::quit);

    engine.load(QUrl::fromLocalFile(qmlDir + "/main.qml"));
    if (engine.rootObjects().isEmpty()) {
        return 1;
    }

    app.exec();

    // Write sentinel so the welcome screen doesn't appear again.
    QDir().mkpath(QFileInfo(sentinel).absolutePath());
    QFile f(sentinel);
    (void)f.open(QIODevice::WriteOnly);

    return 0;
}
