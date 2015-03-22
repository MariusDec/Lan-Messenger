/****************************************************************************
**
** This file is part of LAN Messenger.
**
** Copyright (c) 2010 - 2012 Qualia Digital Solutions.
**
** Contact:  qualiatech@gmail.com
**
** LAN Messenger is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** LAN Messenger is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with LAN Messenger.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "messaging.h"
#include "loggermanager.h"
#include "stdlocation.h"
#include "globals.h"

#include <QDirIterator>

void lmcMessaging::receiveProgress(QString userId, QString userName, QString data) {
    XmlMessage xmlMessage(data);
    int fileMode = Helper::indexOf(FileModeNames, FM_Max, xmlMessage.data(XN_MODE));
    int fileOp = Helper::indexOf(FileOpNames, FO_Max, xmlMessage.data(XN_FILEOP));
    int fileType = Helper::indexOf(FileTypeNames, FT_Max, xmlMessage.data(XN_FILETYPE));
    QString fileId = xmlMessage.data(XN_FILEID);

    //	determine type of message to be sent to app layer based on file type
    MessageType type;
    switch(fileType) {
    case FT_Normal:
    case FT_Folder:
        type = MT_File;
        break;
    case FT_Avatar:
        type = MT_Avatar;
        break;
    default:
        type = MT_Blank;
        break;
    }

    XmlMessage reply;

    switch(fileOp) {
    case FO_Error:
    {
        reply.addData(XN_MODE, FileModeNames[fileMode]);
        reply.addData(XN_FILETYPE, FileTypeNames[fileType]);
        reply.addData(XN_FILEOP, FileOpNames[FO_Abort]);
        reply.addData(XN_FILEID, fileId);
        sendMessage(type, userId, reply);

        if(updateFileTransfer(static_cast<FileMode>(fileMode), static_cast<FileOp>(fileOp), userId, userName, xmlMessage))
            emit messageReceived(type, userId, xmlMessage);
    }
        break;
    case FO_Progress:
    case FO_Complete:
        if(updateFileTransfer(static_cast<FileMode>(fileMode), static_cast<FileOp>(fileOp), userId, userName, xmlMessage))
            emit messageReceived(type, userId, xmlMessage);
        break;
    }
}

void lmcMessaging::prepareFile(const QString &userId, XmlMessage &message) {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.prepareFile started"));

    int fileOp = Helper::indexOf(FileOpNames, FO_Max, message.data(XN_FILEOP));
    int fileMode = Helper::indexOf(FileModeNames, FM_Max, message.data(XN_MODE));

    User* user = getUser(userId);
    QString messageString;

    LoggerManager::getInstance().writeInfo(QString("lmcMessaging.prepareFile -|- Sending file message type %1 (%2) to user %3, mode: %4 (%5)").arg(QString::number(fileOp), (fileOp >= 0 ? FileOpNames[fileOp] : ""), userId, QString::number(fileMode), (fileMode >= 0 ? FileModeNames[fileMode] : "")));

    switch(fileOp) {
    case FO_Request:
        //  New file transfer request, add to file transfer list.
        addFileTransfer(FM_Send, userId, message);
        break;
    case FO_Accept:
        if (user) {
            updateFileTransfer(FM_Receive, (FileOp)fileOp, userId, user->name, message);
            // pMessage now contains the generated id, file mode and other details.
            // Convert this to string now.
            messageString = message.toString();
            pNetwork->initReceiveFile(user->id, user->name, user->address, messageString);
        }
        break;
    case FO_Decline:
        if (user) {
            updateFileTransfer(FM_Receive, (FileOp)fileOp, userId, user->name, message);
        }
        break;
    case FO_Cancel:
        if (user) {
            updateFileTransfer((FileMode)fileMode, (FileOp)fileOp, userId, user->name, message);
            messageString = message.toString();
            pNetwork->fileOperation((FileMode)fileMode, user->id, messageString);
        }
        break;
    }

    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.prepareFile ended"));
}

void lmcMessaging::prepareFolder(const QString &userId, XmlMessage &message) {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.prepareFolder started"));

    int folderOp = Helper::indexOf(FileOpNames, FO_Max, message.data(XN_FILEOP));
    int folderMode = Helper::indexOf(FileModeNames, FM_Max, message.data(XN_MODE));

    LoggerManager::getInstance().writeInfo(QString("lmcMessaging.prepareFolder -|- Sending folder message type %1 to user %2, mode: %3").arg(QString::number(folderOp), userId, QString::number(folderMode)));

    switch(folderOp) {
    case FO_Request:
        //  New file transfer request, add to file transfer list.
        addFolderTransfer(FM_Send, userId, message);
        break;
    case FO_Accept:
        //  Upper layers send folder id in FileId field, copy to FolderId field
        message.addData(XN_FOLDERID, message.data(XN_FILEID));
        updateFolderTransfer(FM_Receive, (FileOp)folderOp, userId, message);
        break;
    case FO_Decline:
        message.addData(XN_FOLDERID, message.data(XN_FILEID));
        updateFolderTransfer(FM_Receive, (FileOp)folderOp, userId, message);
        break;
    case FO_Cancel:
        message.addData(XN_FOLDERID, message.data(XN_FILEID));
        updateFolderTransfer((FileMode)folderMode, (FileOp)folderOp, userId, message);
        break;
    }
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.prepareFolder ended"));
}

void lmcMessaging::processFile(const MessageHeader &header, XmlMessage &message) {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.processFile started"));

    int fileMode = Helper::indexOf(FileModeNames, FM_Max, message.data(XN_MODE));
    int fileOp = Helper::indexOf(FileOpNames, FO_Max, message.data(XN_FILEOP));
    QString messageString;

    //  Reverse file mode to match local mode. ie, message from sender with mode Send
    //  will be translated to Receive at receiver side.
    fileMode = fileMode == FM_Send ? FM_Receive : FM_Send;
    message.removeData(XN_MODE);
    message.addData(XN_MODE, FileModeNames[fileMode]);

    User *user = getUser(header.userId);

    LoggerManager::getInstance().writeInfo(QString("lmcMessaging.processFile -|- Processing file message type %1 to user %2, mode: %3").arg(QString::number(fileOp), header.userId, QString::number(fileMode)));

    switch(fileOp) {
    case FO_Request:
        if(addFileTransfer(FM_Receive, header.userId, message))
            emit messageReceived(header.type, header.userId, message);
        break;
    case FO_Accept:
        if (user) {
            if(updateFileTransfer(FM_Send, static_cast<FileOp>(fileOp), header.userId, user->name, message))
                emit messageReceived(header.type, header.userId, message);
            messageString = message.toString();
            pNetwork->initSendFile(header.userId, user->name, header.address, messageString);
        }
        break;
    case FO_Decline:
        if (user) {
            if(updateFileTransfer(FM_Send, static_cast<FileOp>(fileOp), header.userId, user->name, message))
                emit messageReceived(header.type, header.userId, message);
        }
        break;
    case FO_Cancel:
    case FO_Abort:
        if (user) {
            if(updateFileTransfer(static_cast<FileMode>(fileMode), static_cast<FileOp>(fileOp), header.userId, user->name, message))
                emit messageReceived(header.type, header.userId, message);
            messageString = message.toString();
            pNetwork->fileOperation(static_cast<FileMode>(fileMode), header.userId, messageString);
        }
        break;
    default:
        break;
    }
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.processFile ended"));
}

void lmcMessaging::processFolder(const MessageHeader &header, XmlMessage &message) {
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.processFolder started"));

    int fileMode = Helper::indexOf(FileModeNames, FM_Max, message.data(XN_MODE));
    int fileOp = Helper::indexOf(FileOpNames, FO_Max, message.data(XN_FILEOP));

    //  Reverse file mode to match local mode. ie, message from sender with mode Send
    //  will be translated to Receive at receiver side.
    fileMode = fileMode == FM_Send ? FM_Receive : FM_Send;
    message.removeData(XN_MODE);
    message.addData(XN_MODE, FileModeNames[fileMode]);

    LoggerManager::getInstance().writeInfo(QString("lmcMessaging.processFolder -|- Processing folder message type %1 to user %2, mode: %3").arg(QString::number(fileOp), header.userId, QString::number(fileMode)));

    switch(fileOp) {
    case FO_Request:
        if(addFolderTransfer(FM_Receive, header.userId, message))
            emit messageReceived(header.type, header.userId, message);
        break;
    case FO_Accept:
        if(updateFolderTransfer(FM_Send, static_cast<FileOp>(fileOp), header.userId, message))
            emit messageReceived(header.type, header.userId, message);
        break;
    case FO_Decline:
        if(updateFolderTransfer(FM_Send, static_cast<FileOp>(fileOp), header.userId, message))
            emit messageReceived(header.type, header.userId, message);
        break;
    case FO_Cancel:
    case FO_Abort:
        if(updateFolderTransfer(static_cast<FileMode>(fileMode), static_cast<FileOp>(fileOp), header.userId, message))
            emit messageReceived(header.type, header.userId, message);
        break;
    default:
        break;
    }
    LoggerManager::getInstance().writeInfo(QStringLiteral("lmcMessaging.processFolder ended"));
}

bool lmcMessaging::addFileTransfer(FileMode fileMode, const QString &userId, XmlMessage &message) {
    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcTransferWindow.addFileTransfer started"));

    int fileType = Helper::indexOf(FileTypeNames, FT_Max, message.data(XN_FILETYPE));

    QString fileId, folderId;
    QFileInfo fileInfo;
    XmlMessage xmlMessage;

    bool emitMsg = true;
    switch(fileMode) {
    case FM_Send:
        fileId = Helper::getUuid();
        fileInfo.setFile(message.data(XN_FILEPATH));
        fileList.append(TransFile(fileId, QString::null, userId, fileInfo.filePath(),
            fileInfo.fileName(), fileInfo.size(), fileMode, FO_Request, (FileType)fileType));

        message.addData(XN_FILEID, fileId);
        message.addData(XN_MODE, FileModeNames[fileMode]);
        message.addData(XN_FILENAME, fileInfo.fileName());
        message.addData(XN_FILESIZE, QString::number(fileInfo.size()));
        xmlMessage = message.clone();
        switch(fileType) {
        case FT_Normal:
            emit messageReceived(MT_File, userId, xmlMessage);
            break;
        case FT_Folder:
            folderId = message.data(XN_FOLDERID);
            fileList.last().folderId = folderId;
            message.addData(XN_RELPATH,
                QDir(getFolderPath(folderId, userId, FM_Send)).relativeFilePath(fileInfo.filePath()));
            updateFolderTransfer(FM_Send, FO_Init, userId, message);
            break;
        default:
            break;
        }
        break;
    case FM_Receive:
        fileList.append(TransFile(message.data(XN_FILEID), QString::null, userId, QString::null,
            message.data(XN_FILENAME), message.data(XN_FILESIZE).toLongLong(),
            fileMode, FO_Request, (FileType)fileType));
        switch(fileType) {
        case FT_Avatar:
        {
            xmlMessage.addData(XN_MODE, FileModeNames[FM_Receive]);
            xmlMessage.addData(XN_FILETYPE, FileTypeNames[FT_Avatar]);
            xmlMessage.addData(XN_FILEOP, FileOpNames[FO_Accept]);
            xmlMessage.addData(XN_FILEID, message.data(XN_FILEID));
            xmlMessage.addData(XN_FILESIZE, message.data(XN_FILESIZE));
            sendMessage(MT_Avatar, userId, xmlMessage);
            emitMsg = false;    // Suppress emitting message to upper layers
        }
            break;
        case FT_Folder:
        {
            fileList.last().folderId = message.data(XN_FOLDERID);
            fileList.last().relPath = message.data(XN_RELPATH);
            xmlMessage.addData(XN_MODE, FileModeNames[FM_Receive]);
            xmlMessage.addData(XN_FILETYPE, FileTypeNames[FT_Folder]);
            xmlMessage.addData(XN_FILEOP, FileOpNames[FO_Accept]);
            xmlMessage.addData(XN_FILEID, message.data(XN_FILEID));
            xmlMessage.addData(XN_FILESIZE, message.data(XN_FILESIZE));
            sendMessage(MT_File, userId, xmlMessage);
            emitMsg = false;    // Suppress emitting message to upper layers
        }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcTransferWindow.addFileTransfer ended"));

    return emitMsg;
}

bool lmcMessaging::updateFileTransfer(FileMode fileMode, FileOp fileOp, const QString &userId, const QString &userName, XmlMessage &message) {
    QString fileId = message.data(XN_FILEID);
    int fileType = Helper::indexOf(FileTypeNames, FT_Max, message.data(XN_FILETYPE));

    bool emitMsg = false;
    for(int index = 0; index < fileList.count(); index++) {
        TransFile transferedFile = fileList.at(index);
        if(transferedFile.userId == userId && transferedFile.id == fileId && transferedFile.mode == fileMode) {
            QString filePath, fileName, folderId;
            QDir cacheDir;
            XmlMessage xmlMessage;
            emitMsg = true;
            switch(fileOp) {
            case FO_Accept:
                if(fileMode == FM_Send) {
                    fileName = fileList[index].name;
                    filePath = fileList[index].path;
                    message.removeData(XN_FILEPATH);
                    message.addData(XN_FILEPATH, filePath);
                    message.removeData(XN_FILENAME);
                    message.addData(XN_FILENAME, fileName);
                    switch(fileType) {
                    case FT_Avatar:
                    case FT_Folder:
                        emitMsg = false;
                        break;
                    default:
                        break;
                    }
                } else {
                    switch(fileType) {
                    case FT_Normal:
                    {
                        QString fileStoragePath = Globals::getInstance().fileStoragePath(userName);
                        //  set valid free file name and correct path
                        fileName = getFreeFileName(transferedFile.name);
                        filePath = fileStoragePath.isEmpty () ? "" : QDir(fileStoragePath).absoluteFilePath(fileName);
                        fileList[index].name = fileName;
                        fileList[index].path = filePath;
                        message.removeData(XN_FILEPATH);
                        message.addData(XN_FILEPATH, filePath);
                        message.removeData(XN_FILENAME);
                        message.addData(XN_FILENAME, fileName);
                        xmlMessage.setContent(message.toString());
                        emit messageReceived(MT_File, userId, xmlMessage);
                    }
                        break;
                    case FT_Folder:
                        folderId = transferedFile.folderId;
                        fileName = transferedFile.relPath;
                        filePath = QDir(getFolderPath(folderId, userId, FM_Receive)).absoluteFilePath(fileName);
                        fileList[index].path = filePath;
                        message.removeData(XN_FILEPATH);
                        message.addData(XN_FILEPATH, filePath);
                        break;
                    case FT_Avatar:
                        cacheDir = QDir(StdLocation::getWritableCacheDir());
                        fileName = "avt_" + userId + "_part.png";
                        filePath = cacheDir.absoluteFilePath(fileName);
                        fileList[index].name = fileName;
                        fileList[index].path = filePath;
                        message.addData(XN_FILEPATH, filePath);
                        message.addData(XN_FILENAME, fileName);
                        emitMsg = false;
                        break;
                    default:
                        break;
                    }
                }
                break;
            case FO_Decline:
                switch(fileType) {
                case FT_Avatar:
                    emitMsg = false;
                default:
                    break;
                }

                fileList.removeAt(index);
                break;
            case FO_Cancel:
                fileList[index].op = fileOp;
                switch(fileType) {
                case FT_Normal:
                    xmlMessage.setContent(message.toString());
                    emit messageReceived(MT_File, userId, xmlMessage);
                    break;
                case FT_Avatar:
                case FT_Folder:
                    emitMsg = false;
                default:
                    break;
                }
                fileList.removeAt(index);
                break;
            case FO_Complete:
                fileList[index].op = fileOp;
                if(fileMode == FM_Send) {
                    switch(fileType) {
                    case FT_Avatar:
                        emitMsg = false;
                    case FT_Folder:
                        message.addData(XN_FOLDERID, transferedFile.folderId);
                        updateFolderTransfer(FM_Send, FO_Next, userId, message);
                        emitMsg = false;
                        break;
                    default:
                        break;
                    }
                } else {
                    switch(fileType) {
                    case FT_Avatar:
                        cacheDir = QDir(StdLocation::getWritableCacheDir());
                        fileName = "avt_" + userId + ".png";
                        filePath = cacheDir.absoluteFilePath(fileName);

                        QFile::remove(filePath);
                        QFile::rename(fileList[index].path, filePath);
                        fileList[index].path = filePath;

                        updateUser(MT_Avatar, userId, filePath);
                        message.removeData(XN_FILEPATH);
                        message.addData(XN_FILEPATH, filePath);
                        break;
                    case FT_Folder:
                        message.addData(XN_FOLDERID, transferedFile.folderId);
                        updateFolderTransfer(FM_Receive, FO_Next, userId, message);
                        emitMsg = false;
                        break;
                    default:
                        break;
                    }
                }
                fileList.removeAt(index);
                break;
            case FO_Error:
            case FO_Abort:
                switch(fileType) {
                case FT_Avatar:
                    emitMsg = false;
                case FT_Folder:
                    message.addData(XN_FOLDERID, transferedFile.folderId);
                    updateFolderTransfer(fileMode, fileOp, userId, message);
                    emitMsg = false;
                    break;
                default:
                    break;
                }
                if(fileMode == FM_Send) {
                    //  The file transfer cannot be removed from the list in this case
                    //  because when receiver cancels, the sender gets an error first
                    //  and then the cancel message.
                } else
                    fileList.removeAt(index);
                break;
            case FO_Progress:
                fileList[index].pos = message.data(XN_FILESIZE).toLongLong();
                switch(fileType) {
                case FT_Avatar:
                    emitMsg = false;
                    break;
                case FT_Folder:
                    message.addData(XN_FOLDERID, transferedFile.folderId);
                    updateFolderTransfer(fileMode, fileOp, userId, message);
                    emitMsg = false;
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
            break;
        }
    }

    return emitMsg;
}

QString lmcMessaging::getFreeFileName(const QString &fileName) {
    QString freeFileName = fileName;

    QString fileDir = Globals::getInstance().fileStoragePath();
    QDir dir(fileDir);
    QString filePath = dir.absoluteFilePath(fileName);
    QString baseName = fileName.mid(0, fileName.lastIndexOf("."));
    QString ext = fileName.mid(fileName.lastIndexOf("."));

    int fileCount = 0;
    while(QFile::exists(filePath)) {
        fileCount++;
        freeFileName = baseName + " [" + QString::number(fileCount) + "]" + ext;
        filePath = dir.absoluteFilePath(freeFileName);
    }

    return freeFileName;
}

bool lmcMessaging::addFolderTransfer(FileMode folderMode, const QString &userId, XmlMessage &message) {
    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcTransferWindow.addFolderTransfer started"));

    int folderType = Helper::indexOf(FileTypeNames, FT_Max, message.data(XN_FILETYPE));

    QString folderId;
    QFileInfo fileInfo;
    XmlMessage xmlMessage;

    bool emitMsg = true;
    QDirIterator iterator(QDir(message.data(XN_FILEPATH)), QDirIterator::Subdirectories);
    switch(folderMode) {
    case FM_Send:
        folderId = Helper::getUuid();
        fileInfo.setFile(message.data(XN_FILEPATH));
        folderList.append(TransFolder(folderId, userId, fileInfo.filePath(),
            fileInfo.fileName(), 0, static_cast<FileMode>(folderMode), FO_Request, static_cast<FileType>(folderType), 0));
        //  fetch list of files to be sent
        while(iterator.hasNext()) {
            iterator.next();
            if(iterator.fileInfo().isFile()) {
                folderList.last().fileList.append(iterator.filePath());
                folderList.last().size += iterator.fileInfo().size();
                folderList.last().fileCount++;
            }
        }
        message.addData(XN_FOLDERID, folderId);
        message.addData(XN_FILEID, folderId); //  Assign the folder id to FileId field for uniformity
        message.addData(XN_MODE, FileModeNames[folderMode]);
        message.addData(XN_FILENAME, fileInfo.fileName());
        message.addData(XN_FILESIZE, QString::number(folderList.last().size));
        message.addData(XN_FILECOUNT, QString::number(folderList.last().fileCount));
        xmlMessage.setContent(message.toString());
        emit messageReceived(MT_Folder, userId, xmlMessage);
        break;
    case FM_Receive:
        folderList.append(TransFolder(message.data(XN_FOLDERID), userId, QString::null,
            message.data(XN_FILENAME), message.data(XN_FILESIZE).toLongLong(),
            folderMode, FO_Request, static_cast<FileType>(folderType), message.data(XN_FILECOUNT).toInt()));
        break;
    default:
        break;
    }

    LoggerManager::getInstance().writeInfo(
        QStringLiteral("lmcTransferWindow.addFolderTransfer ended"));

    return emitMsg;
}

bool lmcMessaging::updateFolderTransfer(FileMode folderMode, FileOp folderOp, const QString &userId, XmlMessage &message) {
    QString folderId = message.data(XN_FOLDERID);

    bool emitMsg = false;
    for(int index = 0; index < folderList.count(); index++) {
        TransFolder transFolder = folderList.at(index);
        if(transFolder.userId == userId && transFolder.id == folderId && transFolder.mode == folderMode) {
            emitMsg = true;
            QString folderName, folderPath;
            int fileIndex;
            XmlMessage xmlMessage;
            switch(folderOp) {
            case FO_Accept:
                if(folderMode == FM_Send) {
                    //  Send the first file in the list
                    if(!folderList[index].fileList.isEmpty()) {
                        xmlMessage.addData(XN_FOLDERID, folderList[index].id);
                        xmlMessage.addData(XN_MODE, FileModeNames[FM_Send]);
                        xmlMessage.addData(XN_FILETYPE, FileTypeNames[FT_Folder]);
                        xmlMessage.addData(XN_FILEOP, FileOpNames[FO_Request]);
                        xmlMessage.addData(XN_FILEPATH, folderList[index].fileList.first());
                        sendMessage(MT_File, userId, xmlMessage);
                    }
                    folderName = folderList[index].name;
                    folderPath = folderList[index].path;
                    message.removeData(XN_FILEPATH);
                    message.addData(XN_FILEPATH, folderPath);
                    message.removeData(XN_FILENAME);
                    message.addData(XN_FILENAME, folderName);
                } else {
                    //  set valid free folder name and correct path
                    QString fileStoragePath = Globals::getInstance().fileStoragePath();
                    folderName = getFreeFolderName(transFolder.name);
                    folderPath = fileStoragePath.isEmpty () ? "" : fileStoragePath + folderName;
                    folderList[index].name = folderName;
                    folderList[index].path = folderPath;
                    folderList[index].currentFile = message.data(XN_FILEID);
                    message.removeData(XN_FILEPATH);
                    message.addData(XN_FILEPATH, folderPath);
                    message.removeData(XN_FILENAME);
                    message.addData(XN_FILENAME, folderName);
                    if (!fileStoragePath.isEmpty ())
                        QDir(fileStoragePath).mkpath (folderName);
                    emit messageReceived(MT_Folder, userId, message);
                }
                folderList[index].lastUpdated = QDateTime::currentDateTime();
                break;
            case FO_Decline:
                folderList.removeAt(index);
                break;
            case FO_Cancel:
            {
                xmlMessage.setContent(message.toString());
                xmlMessage.addData(XN_FILEID, transFolder.currentFile);
                emit messageReceived(MT_File, userId, xmlMessage);
                sendMessage(MT_File, userId, xmlMessage);
                folderList.removeAt(index);
            }
                break;
            case FO_Complete:   //  This will never be executed
                folderList.removeAt(index);
                break;
            case FO_Error:
            case FO_Abort:
                message.removeData(XN_FILEID);
                message.addData(XN_FILEID, message.data(XN_FOLDERID));
                emit messageReceived(MT_Folder, userId, message);
                if(folderMode == FM_Send) {
                    //  Cannot be removed, same reason as file transfer
                } else
                    folderList.removeAt(index);
                break;
            case FO_Progress:
                message.removeData(XN_FILEID);
                message.addData(XN_FILEID, message.data(XN_FOLDERID));
                folderList[index].filePos = message.data(XN_FILESIZE).toLongLong();
                message.removeData(XN_FILESIZE);
                message.addData(XN_FILESIZE, QString::number(folderList[index].pos + folderList[index].filePos));
                emit messageReceived(MT_Folder, userId, message);
                folderList[index].lastUpdated = QDateTime::currentDateTime();
                break;
            case FO_Init:
                folderList[index].currentFile = message.data(XN_FILEID);
                break;
            case FO_Next:
                fileIndex = ++folderList[index].fileIndex;
                message.removeData(XN_FILEID);
                message.addData(XN_FILEID, message.data(XN_FOLDERID));
                if(fileIndex < folderList[index].fileCount) {
                    if(folderMode == FM_Send) {
                        xmlMessage.addData(XN_FOLDERID, folderList[index].id);
                        xmlMessage.addData(XN_MODE, FileModeNames[FM_Send]);
                        xmlMessage.addData(XN_FILETYPE, FileTypeNames[FT_Folder]);
                        xmlMessage.addData(XN_FILEOP, FileOpNames[FO_Request]);
                        xmlMessage.addData(XN_FILEPATH, folderList[index].fileList.at(fileIndex));
                        sendMessage(MT_File, userId, xmlMessage);
                    }
                    folderList[index].pos += QFileInfo(message.data(XN_FILEPATH)).size();
                    //  If the files are small, the progress timer will not have time to trigger since
                    //  the file transfer will be completed within its timeout period. In such cases
                    //  we manually emit a progress message if a progress update has not been sent for
                    //  more than 1 second (the timeout period).
                    if(folderList[index].lastUpdated.msecsTo(QDateTime::currentDateTime()) > PROGRESS_TIMEOUT) {
                        message.removeData(XN_FILEOP);
                        message.addData(XN_FILEOP, FileOpNames[FO_Progress]);
                        message.removeData(XN_FILESIZE);
                        message.addData(XN_FILESIZE, QString::number(folderList[index].pos));
                        emit messageReceived(MT_Folder, userId, message);
                        folderList[index].lastUpdated = QDateTime::currentDateTime();
                    }
                } else {
                    message.removeData(XN_FILEPATH);
                    message.addData(XN_FILEPATH, folderList[index].path);
                    folderList[index].op = FO_Complete;
                    emit messageReceived(MT_Folder, userId, message);
                    folderList.removeAt(index);
                }
                break;
            default:
                break;
            }
            break;
        }
    }

    return emitMsg;
}

QString lmcMessaging::getFreeFolderName(const QString &folderName) {
    QString freeFolderName = folderName;

    QString fileDir = Globals::getInstance().fileStoragePath();
    QDir dir(fileDir + folderName);

    int fileCount = 0;
    while(dir.exists()) {
        fileCount++;
        freeFolderName = folderName + " [" + QString::number(fileCount) + "]";
        dir.setPath(fileDir + "/" + freeFolderName);
    }

    return freeFolderName;
}

QString lmcMessaging::getFolderPath(const QString &folderId, const QString &userId, FileMode mode) {
    for(int index = 0; index < folderList.count(); index++) {
        if(folderList[index].id == folderId && folderList[index].userId == userId
                && folderList[index].mode == mode)
            return folderList[index].path;
    }

    return QString::null;
}
