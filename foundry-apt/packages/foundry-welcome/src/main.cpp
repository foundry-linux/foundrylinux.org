#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("foundry-welcome");
    app.setOrganizationName("foundrylinux");

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

    // Write FirstRunDone=true so the X-KDE-autostart-condition in
    // /etc/xdg/autostart/foundry-welcome.desktop skips us on subsequent logins.
    // kreadconfig6 reads ~/.config/foundry-welcome; QSettings IniFormat writes
    // the same path when given an absolute filename.
    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                               + "/foundry-welcome";
    QSettings settings(configPath, QSettings::IniFormat);
    settings.beginGroup("General");
    settings.setValue("FirstRunDone", QString("true"));

    return 0;
}
