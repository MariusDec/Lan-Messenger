#include "stdlocation.h"
#include "globals.h"
#include "loggermanager.h"

QString StdLocation::defaultTransferHistorySavePath() {
#if !PORTABLE
    QString location =
            QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    if (location.isEmpty()) {
        LoggerManager::getInstance().writeError(QStringLiteral(
                                                    "StdLocation.transferHistoryPath-|- no writable location found"));
        return "";
    }
#else
    QString location = qApp->applicationDirPath() + "/" PortableResFolder;
#endif

    // create the dir if it does not exist
    QDir dir(location);
    if (!dir.exists())
        dir.mkpath(location);

    location.append("/transfers.lst");
    return location;
}

QStringList StdLocation::historyFiles() {
    QStringList historyFiles;
    QString path = Globals::getInstance().historySavePath();

    if (path.isEmpty()) {
#if !PORTABLE
        path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
        path = qApp->applicationDirPath() + "/" PortableResFolder;
#endif
        if (!path.endsWith('/'))
            path.append('/');

        path.append(QStringLiteral("history/"));
    }

    QDir dir(path);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    foreach (const QString &fileName, dir.entryList())
        historyFiles.append(dir.absoluteFilePath(fileName));

    return historyFiles;
}

QString StdLocation::defaultHistorySavePath() {
    QString path;

#if !PORTABLE
    path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
    path = qApp->applicationDirPath() + "/" PortableResFolder;
#endif
    if (!path.endsWith('/'))
        path.append('/');

    path.append(QStringLiteral("history/"));

    return path;
}

QString StdLocation::defaultFileStoragePath(const QString &sender) {
    QString location;

#if !PORTABLE
    location =
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (location.isEmpty()) {
        LoggerManager::getInstance().writeError(QStringLiteral(
                                                    "StdLocation.getFileStoragePath-|- no writable location found"));
        return "";
    }
#else
    location = qApp->applicationDirPath() + "/" PortableResFolder;
#endif

    location.append(
                (QString("/%1/Received files/").arg (QApplication::applicationName())));

    if (!location.endsWith('/'))
        location.append('/');

    if (!sender.isEmpty()) {
        bool userFolders = Globals::getInstance().createIndividualFolders();
        if (userFolders)
            location.append(QString("%1/").arg(sender));
    }

    // create the dir if it does not exist
    QDir dir(location);
    if (!dir.exists())
        dir.mkpath(location);

    return location;
}

QString StdLocation::getWritableDataDir(const QString &folderName) {
#if !PORTABLE
    QString location =
            QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
    QString location = qApp->applicationDirPath() + "/" PortableResFolder;
#endif

    if (location.isEmpty()) {
        LoggerManager::getInstance().writeError(QStringLiteral(
                                                    "StdLocation.getWritableCacheDir-|- no writable location found"));
        return "";
    }

    QDir dir(location.append(QString("/%1/").arg (folderName)));
    if (!dir.exists())
        dir.mkpath(location);

    return location;
}

QString StdLocation::getDataDir(const QString &folderName) {
#if !PORTABLE
    QStringList dataLocations =
            QStandardPaths::standardLocations(QStandardPaths::DataLocation);

    dataLocations.removeDuplicates();

    QFileInfo fileInfo;
    for (QString &location : dataLocations) {
        fileInfo.setFile(location.append(QString("/%1/").arg(folderName)));
        if (fileInfo.exists())
            return location;
    }
#else
    return qApp->applicationDirPath() + "/" PortableResFolder "/" + folderName + "/";
#endif
    return "";
}

QStringList StdLocation::getDataDirs() {
#if !PORTABLE
    QStringList dataLocations =
            QStandardPaths::standardLocations(QStandardPaths::DataLocation);

    dataLocations.removeDuplicates();
    for (QString &dataLocation : dataLocations)
        dataLocation.append ("/");
#else
    QStringList dataLocations;
    dataLocations.append(QString("%1/%2/").arg(qApp->applicationDirPath(), PortableResFolder));
#endif

    return dataLocations;
}

QString StdLocation::getWritableCacheDir() {
#if !PORTABLE
    QString location =
            QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
    QString location = qApp->applicationDirPath() + "/" PortableResFolder;
#endif

    if (location.isEmpty()) {
        LoggerManager::getInstance().writeError(QStringLiteral(
                                                    "StdLocation.getWritableCacheDir-|- no writable location found"));
        return "";
    }

    QDir dir(location.append(QStringLiteral("/cache/")));
    if (!dir.exists())
        dir.mkpath(location);

    return location;
}

QString StdLocation::getCacheDir() {
#if !PORTABLE
    QStringList dataLocations =
            QStandardPaths::standardLocations(QStandardPaths::DataLocation);

    dataLocations.removeDuplicates();

    QFileInfo fileInfo;
    for (QString &location : dataLocations) {
        fileInfo.setFile(location.append(QStringLiteral("/cache/")));
        if (fileInfo.exists())
            return (location);
    }
#else
    return qApp->applicationDirPath() + "/" PortableResFolder "/cache/";
#endif

    LoggerManager::getInstance().writeError(QStringLiteral(
                                                "StdLocation.getCacheDir-|- a suitable location could not be found"));

    return "";
}

QString StdLocation::getGenericDocumentsPath() {
#if !PORTABLE
    QString location =
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#else
    QString location = qApp->applicationDirPath() + "/" PortableResFolder;
#endif

    if (location.isEmpty()) {
        LoggerManager::getInstance().writeError(QStringLiteral(
                                                    "StdLocation.getGenericDocumentsPath-|- no writable location found"));
        return "";
    }

    QDir dir(location);
    if (!dir.exists())
        dir.mkpath(location);

    return location;
}

QString StdLocation::getDocumentsPath() {
#if !PORTABLE
    QString location =
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
#else
    QString location = qApp->applicationDirPath() + "/" PortableResFolder;
#endif

    if (location.isEmpty()) {
        LoggerManager::getInstance().writeError(QStringLiteral(
                                                    "StdLocation.getDocumentsPath-|- no writable location found"));
        return "";
    }

    QDir dir(location.append(QString("/%1/").arg (QApplication::applicationName())));
    if (!dir.exists())
        dir.mkpath(location);

    return location;
}

QString StdLocation::translationsPath() {
#if !PORTABLE
    QStringList dataLocations =
            QStandardPaths::standardLocations(QStandardPaths::DataLocation);

    dataLocations.removeDuplicates();

    QFileInfo fileInfo;
    for (QString &location : dataLocations) {
        fileInfo.setFile(location.append("/translations/"));
        if (fileInfo.exists())
            return (location);
    }
#else
    return qApp->applicationDirPath() + "/" PortableResFolder "/translations/";
#endif

    LoggerManager::getInstance().writeError(QStringLiteral(
                                                "StdLocation.translationsPath-|- a suitable location could not be found"));

    return "";
}

QString StdLocation::writableGroupsFile() {
#if !PORTABLE
    QString location =
            QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
    QString location = qApp->applicationDirPath() + "/" PortableResFolder;
#endif

    if (location.isEmpty()) {
        LoggerManager::getInstance().writeError(QStringLiteral(
                                                    "StdLocation.writableGroupsFile-|- no writable location found"));
        return "";
    }

    QDir dir(location);
    if (!dir.exists())
        dir.mkpath(location);

    return location.append(QStringLiteral("/groups.cfg"));

    LoggerManager::getInstance().writeError(QStringLiteral(
                                                "StdLocation.writableGroupsFile-|- a suitable location could not be found"));

    return "";
}

QString StdLocation::groupsFile() {
#if !PORTABLE
    QStringList dataLocations =
            QStandardPaths::standardLocations(QStandardPaths::DataLocation);

    dataLocations.removeDuplicates();

    QFileInfo fileInfo;
    for (QString &location : dataLocations) {
        fileInfo.setFile(location.append("/groups.cfg"));
        if (fileInfo.exists())
            return (location);
    }
#else
    return qApp->applicationDirPath() + "/" PortableResFolder "/groups.cfg";
#endif

    LoggerManager::getInstance().writeError(QStringLiteral(
                                                "StdLocation.groupsFile-|- an existing groups file could not be found"));

    return "";
}

QString StdLocation::logPath() {
#if !PORTABLE
    QString location =
            QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
    QString location = qApp->applicationDirPath() + "/" PortableResFolder;
#endif

    if (location.isEmpty()) {
        LoggerManager::getInstance().writeError(
                    QStringLiteral("StdLocation.logPath-|- no writable location found"));
        return "";
    }

    // create the dir if it does not exist
    QDir dir(location.append("/logs"));
    if (!dir.exists())
        dir.mkpath(location);

    return location;
}

QString StdLocation::tempConfigFile() {
    QString location =
            QStandardPaths::writableLocation (QStandardPaths::TempLocation);

    if (location.isEmpty ()) {
        LoggerManager::getInstance().writeError(QStringLiteral(
                                                    "StdLocation.tempConfigFile-|- no writable location found"));
        return "";
    }

    QDir dir(location);
    if (!dir.exists ())
        dir.mkpath (location);

    return location.append ("lmc_tmpConf.conf");
}
