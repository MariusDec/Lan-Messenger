#include "imageslist.h"
#include "stdlocation.h"
#include "chathelper.h"

#include <QStandardPaths>
#include <QDir>
#include <QDirIterator>
#include <QTextStream>
#include <QPixmap>

ImagesList::ImagesList() : _smileysLoaded(false), _avatarsLoaded(false) {
    _smileysRegex.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption | QRegularExpression::CaseInsensitiveOption);
    _emojisRegex.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption | QRegularExpression::CaseInsensitiveOption);
}

ImagesList::~ImagesList() {}

void ImagesList::loadSmileysInternal(const QString &path) {
    QString filePath = path.left ((path.lastIndexOf ('/') + 1));
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream reader(&file);

        QString lineRead;
        QString key;
        QString value;
        int separatorIndex;
        ImagesStruct smiley;
        QFileInfo smileyImage;

        while (!reader.atEnd()) {
            lineRead = reader.readLine().trimmed();
            if ((separatorIndex = lineRead.indexOf (':')) >= 0) {
                key = lineRead.left (separatorIndex).trimmed ();
                value = lineRead.right ((lineRead.size () - separatorIndex - 1)).trimmed ();
            } else {
                key = lineRead;
            }

            if (!key.compare(QStringLiteral("[Smiley]")))
                smiley.clear();
            else if (!key.compare(QStringLiteral("[/Smiley]"))) {
                smileyImage.setFile(smiley.icon);
                if (smileyImage.exists())
                    _smileys.emplace_back(smiley.code, smiley.description,
                                          smiley.icon, smiley.group);
            } else if (!key.compare(QStringLiteral("Code")))
                smiley.code = value;
            else if (!key.compare(QStringLiteral("Description")))
                smiley.description = value;
            else if (!key.compare(QStringLiteral("Image")))
                smiley.icon = filePath + value;
            else if (!key.compare(QStringLiteral("Group")))
                smiley.group = value;
        }

        file.close();
    }
}

void ImagesList::loadEmojisInternal(const QString &path) {
    _emojis.emplace_back(QStringLiteral("(^._.^)ﾉ"), QStringLiteral("Test data"));
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream reader(&file);
        reader.setCodec("UTF-8");

        QString lineRead;
        QString key;
        QString value;
        QStringList key_value;
        ImagesStruct emoji;

        while (!reader.atEnd()) {
            lineRead = reader.readLine().trimmed();
            if (lineRead.contains(':')) {
                key_value = lineRead.split(':');
                key = key_value[0].trimmed();
                value = key_value[1].trimmed();
            } else {
                key = lineRead;
                value.clear();
            }

            if (!key.compare(QStringLiteral("[Emoji]")))
                emoji.clear();
            else if (!key.compare(QStringLiteral("[/Emoji]")))
                _emojis.emplace_back(emoji.code, emoji.description);
            else if (!key.compare(QStringLiteral("Code")))
                emoji.code = value;
            else if (!key.compare(QStringLiteral("Description")))
                emoji.description = value;
        }

        file.close();
    }
}

void ImagesList::loadSmileys() {
    QString path = StdLocation::getDataDir("smileys");

    QDir dir(path);
    if (dir.exists()) {
        QDirIterator iterator(dir.absolutePath (), QStringList() << "*.lst", QDir::Files,
                              QDirIterator::NoIteratorFlags);
        while (iterator.hasNext()) {
            iterator.next();
            QString fileName = iterator.fileName();

            if (!fileName.compare("smileys.lst"))
                loadSmileysInternal(iterator.filePath());
            else if (!fileName.compare("emojis.lst"))
                loadEmojisInternal(iterator.filePath());
        }
    }
    _smileysLoaded = true;

    buildRegex(_smileys, _smileysRegex);
    buildRegex(_emojis, _emojisRegex);

}

void ImagesList::buildRegex (const std::vector<ImagesStruct> &imagesList, QRegularExpression &regex) {
    QString regexString = "(";
    for (const ImagesStruct &image : imagesList)
        regexString += !image.code.isEmpty() ? QString("%1|").arg(QRegularExpression::escape(ChatHelper::makeHtmlSafe(image.code))) : QString::null;

    regexString.chop(1);
    regexString += ')';

    regex.setPattern(QString("(?<!\\S)%1(?!\\S)").arg(regexString));
    regex.optimize();
}

void ImagesList::loadAvatars()
{
    QString path = StdLocation::getDataDir("avatars");

    QDir dir(path);
    if (dir.exists()) {
        int tempIndex = 0;
        int defaultIndex = 0;

        QDirIterator iterator(dir.absolutePath (), QStringList() << "*.jpg" << "*.png" << "*.ico" << "*.gif", QDir::Files,
                              QDirIterator::NoIteratorFlags);

        while(iterator.hasNext ()) {
            _avatars.emplace_back(iterator.next ());
            if (iterator.fileName ().contains ("default")) {
                _avatars.back ().description = tr("Default avatar");
                defaultIndex = tempIndex;
            }
            ++tempIndex;
        }

        if (_avatars.size() > 0) {
            std::iter_swap(_avatars.begin (), (_avatars.begin () + defaultIndex));

            std::sort((_avatars.begin () + 1), _avatars.end (), [](const ImagesStruct &first, const ImagesStruct &second) {
                return first.icon < second.icon;
            });
        }
    }
    _avatarsLoaded = true;
}

QStringList ImagesList::getTabs(TypeEnum type)
{
    std::vector<ImagesStruct> *images = nullptr;

    switch (type) {
    case Avatars:
        images = &_avatars;
        break;
    case Smileys:
        images = &_smileys;
        break;
    case Emojis:
        images = &_emojis;
        break;
    }

    if (!images)
        return QStringList();

    QStringList tabs;
    for (const ImagesStruct &image : *images)
        if (!tabs.contains (image.group))
            tabs.append (image.group);

    return tabs;
}

std::vector<ImagesStruct> ImagesList::getSmileysByGroup(const QString &group) {
    if (!_smileysLoaded)
        loadSmileys();

    std::vector<ImagesStruct> list;

    for (const ImagesStruct &smiley : _smileys)
        if (!smiley.group.compare(group))
            list.emplace_back(smiley.code, smiley.description, smiley.icon,
                              smiley.group);

    return list;
}

bool ImagesList::getSmileyByCode(const QString &code, const ImagesStruct *smiley)
{
    if (!_smileysLoaded)
        loadSmileys();

    for (const ImagesStruct &smileyImg : _smileys)
        if (!code.compare(smileyImg.codeHtmlSafe, Qt::CaseInsensitive)) {
            smiley = &smileyImg;
            return true;
        }

    smiley = nullptr;
    return false;
}

bool ImagesList::getEmojiByCode(const QString &code, const ImagesStruct *emoji)
{
    if (!_smileysLoaded)
        loadSmileys();

    for (const ImagesStruct &emojiImg : _emojis)
        if (!code.compare(emojiImg.codeHtmlSafe, Qt::CaseInsensitive)) {
            emoji = &emojiImg;
            return true;
        }

    emoji = nullptr;
    return false;
}

QString ImagesList::getAvatar(const QString &avatarName)
{
    if (!_avatarsLoaded)
        loadAvatars();

    for (const ImagesStruct &avatar : _avatars) {
        int lastSeparator = avatar.icon.lastIndexOf ('/') + 1;
        QString fileName = avatar.icon.mid (lastSeparator, (avatar.icon.lastIndexOf ('.') - lastSeparator));
        if (!fileName.compare (avatarName))
            return avatar.icon;
    }
    return "";
}

QString ImagesList::getAvatar(int avatarIndex)
{
    if (!_avatarsLoaded)
        loadAvatars();

    QString avatar;

    if (avatarIndex >= 0 && avatarIndex < _avatars.size ())
        avatar = _avatars[avatarIndex].icon;

    if (avatar.isEmpty())
        avatar = getDefaultAvatar ();

    return avatar;
}

int ImagesList::getAvatarIndex(const QString &imagePath)
{
    if (!_avatarsLoaded)
        loadAvatars();

    for (int index = 0; index < _avatars.size(); ++index)
        if (_avatars[index].icon == imagePath)
            return index;

    return -1;
}

QString ImagesList::getDefaultAvatar()
{
    return getAvatar("default");
}

QString ImagesList::addAvatar(const QString &icon)
{
    LoggerManager::getInstance ().writeInfo (QStringLiteral("ImagesList.addAvatar started"));

    QPixmap avatar(icon);
    if (!avatar.isNull ()) {
        avatar = avatar.scaled(QSize(48, 48), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        QString fileName;
        QDir dir(StdLocation::getWritableDataDir ("avatars"));
        fileName = dir.absolutePath () + QString("/%1.png").arg (QString::number ((dir.entryList (QDir::Files).size () - 1)));

        LoggerManager::getInstance ().writeInfo (QString("ImagesList.addAvatar saved -|- file: %1").arg (fileName));
        avatar.save (fileName);
        _avatars.emplace_back(fileName);

        std::sort((_avatars.begin () + 1), _avatars.end (), [](const ImagesStruct &first, const ImagesStruct &second) {
            return first.icon < second.icon;
        });

        LoggerManager::getInstance ().writeInfo (QString("ImagesList.addAvatar ended -|- added avatar: \"%1\"").arg(icon));
        return fileName;
    }
    LoggerManager::getInstance ().writeInfo (QString("ImagesList.addAvatar ended -|- the avatar \"%1\" is invalid").arg(icon));

    return "";
}
