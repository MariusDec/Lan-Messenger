#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QString>
#include <QStringList>
#include <vector>

struct ChatThemeStruct {
  QString themePath;
  QString document;
  QString inMsg;
  QString inNextMsg;
  QString outMsg;
  QString outNextMsg;
  QString pubMsg;
  QString sysMsg;
  QString sysNextMsg;
  QString reqMsg;
  QString stateMsg;
};

class ThemeManager
{
private:

  int                       _themeIndex;
  int                       _buttonThemeIndex;
  int                       _chatThemeIndex;
  bool                      _useSystemTheme;

  QString                   _themeStyleSheet;
  QString                   _currentIconTheme;

  ChatThemeStruct           _currentChatTheme;
  ChatThemeStruct          *_currentPreviewChatTheme = nullptr;
  QStringList               _chatThemeNames;
  QStringList               _chatThemePaths;

  QStringList               _themeNames;
  QStringList               _themeFolders;           // the path to the themes
  QStringList               _themeSearchLocations;
  std::vector<QStringList>  _buttonThemeNames;
  QStringList               _iconThemes;

  bool                      _previewTheme;
  QString                   _tempApplicationThemePath;
  QString                   _tempButtonThemePath;
  QString                   _tempThemeStyleSheet;

  const QString            _docTemplate;
public:
  ThemeManager();
  void init() {
      readAvailableThemes();
      readAvailableIconThemes();
      readAvailableChatThemes();
  }

  static ThemeManager &getInstance() {
    static ThemeManager themeManager;
    return themeManager;
  }

  void addThemeFolder(const QString &themeFolder);

  void setUseSystemTheme(bool nativeTheme)  { _useSystemTheme = nativeTheme; }
  bool useSystemTheme() { return _previewTheme ? _tempApplicationThemePath.isEmpty () : _useSystemTheme; }

  QString currentThemePath () const  {
      if (_previewTheme)
          return _tempApplicationThemePath;

    return _themeFolders[_themeIndex];
  }

  QString currentButtonThemePath () const  {
      if (_previewTheme)
          return _tempButtonThemePath;

    if (_themeFolders.empty () or _buttonThemeNames[_themeIndex].empty ())
      return "";

    return  (_themeFolders[_themeIndex] + _buttonThemeNames[_themeIndex][_buttonThemeIndex] + "/");
  }

  QString currentChatThemePath () const {
    return (_chatThemePaths.empty () ? "" : _chatThemePaths[_chatThemeIndex]);
  }

  const ChatThemeStruct &currentChatTheme(bool themePreview = false) const {
      if (themePreview && _currentPreviewChatTheme)
          return *_currentPreviewChatTheme;

    return _currentChatTheme;
  }

  const QString& currentThemeName () const  {
    return _themeNames[_themeIndex];
  }

  QString currentButtonThemeName() const  {
    if (_themeIndex > 0)
      return _buttonThemeNames[_themeIndex][_buttonThemeIndex];
    return 0;
  }

  QString currentChatThemeName () const {
    return (_chatThemeNames.empty () ? "" :  _chatThemeNames[_chatThemeIndex]);
  }

  const int& currentThemeIndex () const  {
    return _themeIndex;
  }

  const int& currentButtonThemeIndex () const  {
    return _buttonThemeIndex;
  }

  const int &currentChatThemeIndex () const {
    return _chatThemeIndex;
  }

  const QStringList& themesList () const  {
    return _themeNames;
  }

  const QStringList& themesPathList () const  {
    return _themeFolders;
  }

  const QStringList &iconThemes () const {
      return _iconThemes;
  }
  void setIconTheme (const QString &icon) {
      if (_iconThemes.contains (icon))
          _currentIconTheme = icon;
  }

  QStringList buttonThemesList (unsigned index) const  {
      if (index < _buttonThemeNames.size ())
          return _buttonThemeNames[index];

      return QStringList();
  }

  QStringList buttonThemesPathList (int index) const  {
      if (_themeFolders.empty () or index < 0 or index >= _themeFolders.size ())
        return QStringList();

      QStringList themes;

      for (const QString &theme : _buttonThemeNames[index]) {
          themes.append ((_themeFolders[index] + theme + "/"));
      }
      return themes;
  }

  const QStringList& chatThemesList () const {
    return _chatThemeNames;
  }

  const QStringList& chatThemesPathList () const {
    return _chatThemePaths;
  }

  QString getStyleSheet() const  {
      if (_previewTheme)
          return _tempThemeStyleSheet;

      return _themeStyleSheet;
  }

  QString loadStyleSheet();
  void reloadStyleSheet() { loadStyleSheet(); }
  void updateWidgetStyle(QWidget *widget);

  void changeTheme (const QString &theme, const QString &buttonTheme);
  void changeTheme (const int &themeIndex, const int &buttonThemeIndex);
  void changeChatTheme (const QString &theme);
  void loadPreviewChatTheme (int themeIndex);

  void previewTheme (const QString &themePath, const QString &buttonThemePath);
  void endPreview ();

  QString getThemePath(int theme); // returns a path to the theme specified by index

  bool readAvailableThemes();
  bool readAvailableIconThemes();
  bool readAvailableChatThemes();
  bool readAvailableButtonThemes(const QString &path);

  ChatThemeStruct loadChatTheme(const QString &path);

  QString getAppIcon(const QString &icon, QString path = QString()); // gets the icon path to the icon used in the application (from images folder)
  QString getBubbleIcon(const QString &icon); // gets the icon path to the icon used in the status (from icons folder)
  QString getThemeIconPath (const QString &icon);  // gets the icon path to the icon contained in the current theme folder

  QString getIconFromFolder (const QString &icon, const QString &path, bool absolutePath = false);
};

#endif // THEMEMANAGER_H
