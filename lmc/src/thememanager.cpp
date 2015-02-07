#include "thememanager.h"
#include "stdlocation.h"

#include <QApplication>
#include <QWidget>
#include <QStyle>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDirIterator>

ThemeManager::ThemeManager() :_themeIndex(-1), _buttonThemeIndex(-1), _currentIconTheme(QStringLiteral("Default")),
    _docTemplate(QStringLiteral("<html><head><style type='text/css'>%1</style></head><body style='-webkit-nbsp-mode: space; word-wrap:break-word;'></body></html>"))
{
    _themeSearchLocations.clear();
    _themeSearchLocations.append(StdLocation::getDataDirs());

    init();
    _previewTheme = false;
} // ThemeManager

void ThemeManager::addThemeFolder(const QString &themeFolder)
{
    _themeSearchLocations.push_back(themeFolder);
    if (!_themeSearchLocations.back().endsWith('/'))
        _themeSearchLocations.back().append('/');
} // addThemeFolder

QString ThemeManager::loadStyleSheet()
{
    QString styleSheet;

    QFile file((currentThemePath() + "stylesheet.qss"));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        styleSheet = file.readAll();
        file.close();
    } else {
        styleSheet.clear();
    }
    qApp->setStyleSheet(styleSheet);

    return styleSheet;
} // loadStyleSheet

void ThemeManager::updateWidgetStyle(QWidget *widget)
{
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
} // updateWidgetStyle

void ThemeManager::changeTheme (const QString &theme, const QString &buttonTheme) {
    int tempThemeIndex = _themeNames.indexOf(theme);
    int tempButtonThemeIndex = 0;

    if (tempThemeIndex > 0)
        tempButtonThemeIndex = _buttonThemeNames[tempThemeIndex].indexOf(buttonTheme);

    changeTheme(tempThemeIndex, tempButtonThemeIndex);
} // changeTheme

void ThemeManager::changeTheme (const int &themeIndex, const int &buttonThemeIndex)
{
    if (themeIndex <= 0) {
        _useSystemTheme = true;
        _themeIndex = 0;
        _buttonThemeIndex = 0;
    } else if (themeIndex > 0) {
        _useSystemTheme = false;

        _themeIndex = themeIndex;
        _buttonThemeIndex = buttonThemeIndex;
    }

    _themeStyleSheet = loadStyleSheet();
} // changeTheme

void ThemeManager::changeChatTheme (const QString &theme) {
    // TODO add option to load default theme in case it fails
    int tempThemeIndex = _chatThemeNames.indexOf(theme);

    if (tempThemeIndex >= 0)
        _currentChatTheme = loadChatTheme (_chatThemePaths[tempThemeIndex]);
}

void ThemeManager::loadPreviewChatTheme (int themeIndex) {
    if (themeIndex < 0 || themeIndex >= _chatThemePaths.size())
        return;

    if (!_currentPreviewChatTheme)
        _currentPreviewChatTheme = new ChatThemeStruct;

    *_currentPreviewChatTheme = loadChatTheme (_chatThemePaths[themeIndex]);
}

void ThemeManager::previewTheme(const QString &themePath, const QString &buttonThemePath)
{
    _previewTheme = true;

    _tempApplicationThemePath = themePath;
    _tempButtonThemePath = buttonThemePath;
    _tempThemeStyleSheet = loadStyleSheet ();
}

void ThemeManager::endPreview()
{
    _previewTheme = false;
    if (_themeIndex >= 0) // TODO remove this
        _themeStyleSheet = loadStyleSheet ();
}

QString ThemeManager::getThemePath(int theme)
{
    return _themeFolders[theme];
} // getThemePath

bool ThemeManager::readAvailableThemes()
{
    if (_themeNames.isEmpty()) {
        _themeNames.append(QStringLiteral("native"));
        _themeFolders.push_back(QStringLiteral(""));
        _buttonThemeNames.push_back(QStringList());
    }

    for (const QString &themeFolder : _themeSearchLocations)
    {
        QDir parrentDir ((themeFolder + "themes"));
        if (!parrentDir.exists() )
            continue;

        parrentDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

        QDir childDir;
        childDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

        foreach (const QString &dirName, parrentDir.entryList()) {
            childDir.setPath((parrentDir.path() + "/" + dirName));

            if (!readAvailableButtonThemes(childDir.absolutePath()) )
                continue;

            foreach (const QFileInfo &file, childDir.entryInfoList())
                if (!file.fileName().compare(QStringLiteral("stylesheet.qss")) ) {
                    _themeNames.append(childDir.dirName());
                    _themeFolders.push_back((childDir.absolutePath() + "/"));
                    break; // move to the next directory
                }
        }
    }
    return (_themeFolders.size () > 1);
} // readAvailableThemes

bool ThemeManager::readAvailableIconThemes()
{
    for (const QString &themeFolder : _themeSearchLocations)
    {
        QDir parrentDir ((themeFolder + "icons"));
        if (!parrentDir.exists() )
            continue;

        parrentDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

        QDir childDir;
        childDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

        foreach (const QString &dirName, parrentDir.entryList()) {
            childDir.setPath(QString("%1/%2").arg (parrentDir.path(), dirName));

            foreach (const QFileInfo &file, childDir.entryInfoList())
                if (!file.fileName().compare(QStringLiteral("theme")) ) {
                    _iconThemes.append(childDir.dirName());
                    break; // move to the next directory
                }
        }
    }
    return _iconThemes.size ();
} // readAvailableIconThemes

bool ThemeManager::readAvailableButtonThemes(const QString &path)
{
    QDir parrentDir(path);
    if (!parrentDir.exists())
        return false;

    QStringList buttonThemesList;

    parrentDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    QDir childDir;
    childDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    foreach (const QString &dirName, parrentDir.entryList()) {
        childDir.setPath((parrentDir.path() + "/" + dirName));

        foreach (const QFileInfo &file, childDir.entryInfoList())
            if (!file.fileName().compare(QStringLiteral("theme")) ) {
                buttonThemesList.append(dirName);
                break; // move to the next directory
            }
    }

    _buttonThemeNames.push_back(buttonThemesList);
    return buttonThemesList.size ();
} // readAvailableButtonThemes


bool ThemeManager::readAvailableChatThemes() {
    for (const QString &themeFolder : _themeSearchLocations)
    {
        QDir parrentDir ((themeFolder + "chat/themes"));
        if (!parrentDir.exists() )
            continue;

        parrentDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

        QDir childDir;
        childDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

        foreach (const QString &dirName, parrentDir.entryList()) {
            childDir.setPath((parrentDir.path() + "/" + dirName));

            foreach (const QFileInfo &file, childDir.entryInfoList())
                if (!file.fileName().compare(QStringLiteral("main.css")) ) {
                    _chatThemeNames.append(childDir.dirName());
                    _chatThemePaths.push_back((childDir.absolutePath() + "/"));
                    break; // move to the next directory
                }
        }
    }
    return _chatThemePaths.size ();
} // readAvailableChatThemes

ChatThemeStruct ThemeManager::loadChatTheme(const QString &path) {
    QFile file;
    ChatThemeStruct themeData;

    themeData.themePath = path;

    if (!themeData.themePath.endsWith('/'))
        themeData.themePath.append('/');

    file.setFileName(themeData.themePath + "main.css");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString style(file.readAll().constData());
        themeData.document = _docTemplate.arg(style);
        file.close();
    }

    file.setFileName(themeData.themePath + "Incoming/Content.html");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        themeData.inMsg = "<div class='_lmc_chatdiv'>" + QString(file.readAll().constData()) + "</div>";
        file.close();
    }

    file.setFileName(themeData.themePath + "Incoming/NextContent.html");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        themeData.inNextMsg = QString(file.readAll().constData());
        file.close();
    }

    file.setFileName(themeData.themePath + "Outgoing/Content.html");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        themeData.outMsg = "<div class='_lmc_chatdiv'>" + QString(file.readAll().constData()) + "</div>";
        file.close();
    }

    file.setFileName(themeData.themePath + "Outgoing/NextContent.html");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        themeData.outNextMsg = QString(file.readAll().constData());
        file.close();
    }

    file.setFileName(themeData.themePath + "Broadcast.html");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        themeData.pubMsg = "<div class='_lmc_publicdiv'>" + QString(file.readAll().constData()) + "</div>";
        file.close();
    }

    file.setFileName(themeData.themePath + "Status.html");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString sys = QString(file.readAll().constData());
        themeData.sysMsg = "<div class='_lmc_sysdiv'>" + sys + "</div>";
        themeData.stateMsg = "<div class='_lmc_statediv'>" + sys + "</div>";
        file.close();
    }

    file.setFileName(themeData.themePath + "NextStatus.html");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        themeData.sysNextMsg = QString(file.readAll().constData());
        file.close();
    }

    file.setFileName(themeData.themePath + "Request.html");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        themeData.reqMsg = "<div class='_lmc_reqdiv'>" + QString(file.readAll().constData()) + "</div>";
        file.close();
    }

    return themeData;
} // loadChatTheme

QString ThemeManager::getBubbleIcon(const QString &icon)
{
    return getIconFromFolder (icon, "images");
} // getApplicationIcon

QString ThemeManager::getAppIcon(const QString &icon, QString path)
{
    return getIconFromFolder (icon, QString("icons/%1").arg (!path.isEmpty () ? path : _currentIconTheme));
} // getStatusIcon

QString ThemeManager::getThemeIconPath(const QString &icon)
{
    return getIconFromFolder (icon, currentThemePath (), true);
} // getThemeIconPath

QString ThemeManager::getIconFromFolder(const QString &icon, const QString &path, bool absolutePath)
{
    if (!absolutePath) {
        for (const QString &themeFolder : _themeSearchLocations) {
            QDir dir ((themeFolder + path));
            if (!dir.exists() )
                continue;

            QDirIterator dirIterator(dir.absolutePath (), QStringList() << "*.jpg" << "*.png" << "*.ico", QDir::Files, QDirIterator::NoIteratorFlags);
            while (dirIterator.hasNext ()) {
                dirIterator.next ();
                if (!dirIterator.fileInfo ().baseName ().compare (icon))
                    return dirIterator.filePath ();
            }
        }
    } else {
        QDir dir (path);
        if (dir.exists ()) {
            QDirIterator dirIterator(dir.absolutePath (), QStringList() << "*.jpg" << "*.png" << "*.ico", QDir::Files, QDirIterator::NoIteratorFlags);
            while (dirIterator.hasNext ()) {
                dirIterator.next ();
                if (!dirIterator.fileInfo ().baseName ().compare (icon))
                    return dirIterator.filePath ();
            }
        }
    }

    return "";
} // getIconFromFolder
