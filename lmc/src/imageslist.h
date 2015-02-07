#ifndef SMILEYSLIST_H
#define SMILEYSLIST_H

#include "chathelper.h"

#include <QString>
#include <QCoreApplication>
#include <QRegularExpression>

struct ImagesStruct {
    QString code;
    QString codeHtmlSafe;
    QString description;
    QString icon;
    QString group;

    ImagesStruct() {}

    ImagesStruct(const QString &code, const QString &description,
                 const QString &imagePath, const QString &tab)
        : code(code), description(description), icon(imagePath), group(tab) {
        codeHtmlSafe = ChatHelper::makeHtmlSafe(code);
    }

    ImagesStruct(const QString &code, const QString &description)
        : code(code), description(description) { codeHtmlSafe = ChatHelper::makeHtmlSafe(code); }

    ImagesStruct(const QString &imagePath) : icon(imagePath) {}

    void clear() {
        code.clear();
        description.clear();
        icon.clear();
        group.clear();
    }
};

class ImagesList {
  Q_DECLARE_TR_FUNCTIONS(ImagesList)

  std::vector<ImagesStruct> _smileys;
  std::vector<ImagesStruct> _emojis;
  std::vector<ImagesStruct> _avatars;
  bool                      _smileysLoaded;
  bool                      _avatarsLoaded;
  QRegularExpression        _smileysRegex;
  QRegularExpression        _emojisRegex;

  void loadSmileysInternal(const QString &path);
  void loadEmojisInternal(const QString &path);

public:
  enum TypeEnum { Avatars, Smileys, Emojis };

  ImagesList();
  ~ImagesList();

  static ImagesList &getInstance() {
    static ImagesList smileys;
    return smileys;
  }

  void loadSmileys();
  void loadAvatars();

 const std::vector<ImagesStruct> &getSmileys() {
    if (!_smileysLoaded)
      loadSmileys();

    return _smileys;
  }
  const std::vector<ImagesStruct> &getEmojis() {
    if (!_smileysLoaded)
      loadSmileys();

    return _emojis;
  }

  const std::vector<ImagesStruct> &getAvatars() {
    if (!_avatarsLoaded)
      loadAvatars();

    return _avatars;
  }

  const QRegularExpression &getSmileysRegex() {
      return _smileysRegex;
  }

  const QRegularExpression &getEmojisRegex() {
      return _emojisRegex;
  }

  QStringList getTabs (TypeEnum type);
  std::vector<ImagesStruct> getSmileysByGroup(const QString &group);
  bool getSmileyByCode(const QString &code, const ImagesStruct *smiley);
  bool getEmojiByCode(const QString &code, const ImagesStruct *emoji);
  QString getAvatar(const QString &avatarName);
  QString getAvatar(int avatarIndex);
  int getAvatarIndex(const QString &imagePath);
  QString getDefaultAvatar();
  QString addAvatar(const QString &icon);

private:
  void buildRegex(const std::vector<ImagesStruct> &imagesList, QRegularExpression &regex);
};

#endif // SMILEYSLIST_H
