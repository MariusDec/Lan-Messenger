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

#include "settingsdialog.h"
#include "thememanager.h"
#include "globals.h"

#include <QUrl>
#include <QSound>
#include <QSystemTrayIcon>
#include <QLocale>
#include <QAudioDeviceInfo>
#include <QMessageBox>
#include <QDesktopWidget>

lmcSettingsDialog::lmcSettingsDialog(QWidget *parent, Qt::WindowFlags flags) : QDialog(parent, flags) {
    ui.setupUi(this);
    setProperty("isWindow", true);
    ui.textEditFontColorText->setProperty("light", true);

    //	remove the help button from window button group
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    //	Destroy the window when it closes
    setAttribute(Qt::WA_DeleteOnClose, true);

    pMessageLog = new lmcMessageLog(ui.frameMessageLog);
    ui.logLayout->addWidget(pMessageLog);

    connect(ui.listWidgetCategories, &QListWidget::currentRowChanged, this, &lmcSettingsDialog::listWidgetCategories_currentRowChanged);
    connect(ui.buttonOK, &ThemedButton::clicked, this, &lmcSettingsDialog::buttonOk_clicked);
    connect(ui.buttonCancel, &ThemedButton::clicked, this, &lmcSettingsDialog::buttonCancel_clicked);
    connect(ui.checkBoxMessageTime, &QCheckBox::toggled, this, &lmcSettingsDialog::checkBoxMessageTime_toggled);
    connect(ui.checkBoxAllowLinks, &QCheckBox::toggled, this, &lmcSettingsDialog::checkBoxAllowLinks_toggled);
    connect(ui.radioButtonSysHistoryPath, &QRadioButton::toggled, this, &lmcSettingsDialog::radioButtonSysHistoryPath_toggled);
    connect(ui.buttonHistoryPath, &ThemedButton::clicked, this, &lmcSettingsDialog::buttonHistoryPath_clicked);
    connect(ui.checkBoxSysTrayMsg, &QCheckBox::toggled, this, &lmcSettingsDialog::checkBoxSysTrayMsg_toggled);
    connect(ui.buttonFilePath, &ThemedButton::clicked, this, &lmcSettingsDialog::buttonFilePath_clicked);
    connect(ui.buttonClearHistory, &ThemedButton::clicked, this, &lmcSettingsDialog::buttonClearHistory_clicked);
    connect(ui.buttonClearFileHistory, &ThemedButton::clicked, this, &lmcSettingsDialog::buttonClearFileHistory_clicked);
    connect(ui.buttonViewFiles, &ThemedButton::clicked, this, &lmcSettingsDialog::buttonViewFiles_clicked);
    connect(ui.checkBoxSound, &QCheckBox::toggled, this, &lmcSettingsDialog::checkBoxSound_toggled);
    connect(ui.checkBoxAutoShowFile, &QCheckBox::toggled, this, &lmcSettingsDialog::checkBoxAutoShowFile_toggled);
    connect(ui.buttonFont, &ThemedButton::clicked, this, &lmcSettingsDialog::buttonFont_clicked);
    connect(ui.buttonColor, &ThemedButton::clicked, this, &lmcSettingsDialog::buttonColor_clicked);
    connect(ui.buttonReset, &ThemedButton::clicked, this, &lmcSettingsDialog::buttonReset_clicked);
    connect(ui.comboBoxApplicationTheme, &ThemedComboBox::currentIndexChanged, this, &lmcSettingsDialog::comboBoxApplicationTheme_currentIndexChanged);
    connect(ui.comboBoxIconTheme, &ThemedComboBox::currentIndexChanged, this, &lmcSettingsDialog::comboBoxIconTheme_currentIndexChanged);
    connect(ui.comboBoxButtonTheme, &ThemedComboBox::currentIndexChanged, this, &lmcSettingsDialog::comboBoxButtonTheme_currentIndexChanged);
    connect(ui.comboBoxChatTheme, &ThemedComboBox::currentIndexChanged, this, &lmcSettingsDialog::comboBoxChatTheme_currentIndexChanged);
    connect(ui.listWidgetBroadcasts, &QListWidget::currentRowChanged, this, &lmcSettingsDialog::listWidgetBroadcasts_currentRowChanged);
    connect(ui.textBoxBroadcast, &QLineEdit::textEdited, this, &lmcSettingsDialog::textBoxBroadcast_textEdited);
    connect(ui.textBoxBroadcast, &QLineEdit::returnPressed, this, &lmcSettingsDialog::buttonAddBroadcast_clicked);
    connect(ui.buttonAddBroadcast, &ThemedButton::clicked, this, &lmcSettingsDialog::buttonAddBroadcast_clicked);
    connect(ui.buttonDeleteBroadcast, &ThemedButton::clicked, this, &lmcSettingsDialog::buttonDeleteBroadcast_clicked);
    connect(ui.listWidgetSounds, &QListWidget::currentRowChanged, this, &lmcSettingsDialog::listWidgetSounds_currentRowChanged);
    connect(ui.buttonPlaySound, &ThemedButton::clicked, this, &lmcSettingsDialog::buttonPlaySound_clicked);
    connect(ui.buttonSoundPath, &ThemedButton::clicked, this, &lmcSettingsDialog::buttonSoundPath_clicked);
    connect(ui.buttonResetSounds, &ThemedButton::clicked, this, &lmcSettingsDialog::buttonResetSounds_clicked);

    init();
}

lmcSettingsDialog::~lmcSettingsDialog() {
}

void lmcSettingsDialog::init() {
    pSettings = new lmcSettings();
    QMap<QString, QString> languages;
    //	Loop through available languages and add them to a map. This ensures that
    //	the languages are sorted alphabetically. After that add the sorted items
    //	to the combo box.
    for(int index = 0; index < Application::availableLanguages().count(); index++) {
        QString langCode = Application::availableLanguages().value(index);
        QLocale locale(langCode);
        QString language = QLocale::languageToString(locale.language());
        languages.insert(language, langCode);
    }
    for(int index = 0; index < languages.count(); index++)
        ui.comboBoxLanguage->addItem(languages.keys().value(index), languages.values().value(index));

    for(int index = 0; index < SE_Max; index++) {
        QListWidgetItem* pListItem = new QListWidgetItem(ui.listWidgetSounds);
        pListItem->setText(lmcStrings::soundDesc()[index]);
        pListItem->setData(Qt::UserRole, soundFile[index]);
        pListItem->setCheckState(IDS_SOUNDEVENT_VAL);
    }

    QStringList themeNames = ThemeManager::getInstance ().chatThemesList ();
    QStringList themePaths = ThemeManager::getInstance ().chatThemesPathList ();
    for(int index = 0; index < themeNames.size (); index++)
        ui.comboBoxChatTheme->addItem(themeNames[index], themePaths[index]);

    themeNames = ThemeManager::getInstance ().themesList ();
    themePaths = ThemeManager::getInstance ().themesPathList ();
    for(int index = 0; index < themeNames.size (); index++)
        ui.comboBoxApplicationTheme->addItem(themeNames[index], themePaths[index]);

    themeNames = ThemeManager::getInstance ().iconThemes ();
    for(int index = 0; index < themeNames.size (); index++)
        ui.comboBoxIconTheme->addItem(themeNames[index], index);

    ui.comboBoxButtonTheme->setEnabled (false);

    for(int index = 0; index < lmcStrings::userListView().size (); index++)
        ui.comboBoxUserListView->addItem(lmcStrings::userListView()[index], index);

    fontSize = 0;
    font = QApplication::font();
    color = QApplication::palette().text().color();
    ui.listWidgetCategories->setCurrentRow(0);

    setWindowIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("messenger"))));

    ui.listWidgetCategories->setUniformItemSizes(true);
    ui.listWidgetCategories->setIconSize(QSize(32, 32));
    ui.listWidgetCategories->item(0)->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("systemsettings"))));
    ui.listWidgetCategories->item(1)->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("usersettings"))));
    ui.listWidgetCategories->item(2)->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("messagesettings"))));
    ui.listWidgetCategories->item(3)->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("historysettings"))));
    ui.listWidgetCategories->item(4)->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("alertsettings"))));
    ui.listWidgetCategories->item(5)->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("networksettings"))));
    ui.listWidgetCategories->item(6)->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("transfersettings"))));
    ui.listWidgetCategories->item(7)->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("themesettings"))));
    ui.listWidgetCategories->item(8)->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("hotkeysettings"))));

    setPageHeaderStyle(ui.labelGeneralPage);
    setPageHeaderStyle(ui.labelAccountPage);
    setPageHeaderStyle(ui.labelMessagesPage);
    setPageHeaderStyle(ui.labelHistoryPage);
    setPageHeaderStyle(ui.labelAlertsPage);
    setPageHeaderStyle(ui.labelNetworkPage);
    setPageHeaderStyle(ui.labelFileTransferPage);
    setPageHeaderStyle(ui.labelThemePage);
    setPageHeaderStyle(ui.labelHotkeysPage);

        ui.buttonPlaySound->setIcon(QIcon(ThemeManager::getInstance().getAppIcon(QStringLiteral("play"))));

    pPortValidator = new QIntValidator(1, 65535, this);
    ui.textBoxUDPPort->setValidator(pPortValidator);
    ui.textBoxTCPPort->setValidator(pPortValidator);

    ipRegExp = QRegExp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    pIpValidator = new QRegExpValidator(ipRegExp, this);
    ui.textBoxMulticast->setValidator(pIpValidator);

    pMessageLog->setAutoScroll(false);

    setUIText();
    loadSettings();
}

void lmcSettingsDialog::settingsChanged() {
    loadSettings();
}

void lmcSettingsDialog::changeEvent(QEvent* pEvent) {
    switch(pEvent->type()) {
    case QEvent::LanguageChange:
        setUIText();
        break;
    default:
        break;
    }

    QDialog::changeEvent(pEvent);
}

void lmcSettingsDialog::moveEvent(QMoveEvent *event)
{
    if (!Globals::getInstance().windowSnapping()) {
        QWidget::moveEvent(event);
        return;
    }

    const QRect screen = QApplication::desktop()->availableGeometry(this);

    bool windowSnapped = false;

    if (std::abs(frameGeometry().left() - screen.left()) < 25) {
        move(screen.left(), frameGeometry().top());
        windowSnapped = true;
    } else if (std::abs(screen.right() - frameGeometry().right()) < 25) {
        move((screen.right() - frameGeometry().width() + 1), frameGeometry().top());
        windowSnapped = true;
    }

    if (std::abs(frameGeometry().top() - screen.top()) < 25) {
        move(frameGeometry().left(), screen.top());
        windowSnapped = true;
    } else if (std::abs(screen.bottom() - frameGeometry().bottom()) < 25) {
        move(frameGeometry().left(), (screen.bottom() - frameGeometry().height() + 1));
        windowSnapped = true;
    }

    if (!windowSnapped)
        QWidget::moveEvent(event);
}

void lmcSettingsDialog::listWidgetCategories_currentRowChanged(int currentRow) {
    ui.stackedWidget->setCurrentIndex(currentRow);
}

void lmcSettingsDialog::buttonOk_clicked() {
    saveSettings();
    ThemeManager::getInstance ().changeTheme (ui.comboBoxApplicationTheme->currentIndex (), ui.comboBoxButtonTheme->currentIndex ());
    ThemeManager::getInstance ().setIconTheme (ui.comboBoxIconTheme->currentText ());
    ThemeManager::getInstance ().endPreview ();
    accept();
}

void lmcSettingsDialog::buttonCancel_clicked()
{
    ThemeManager::getInstance ().endPreview ();
    reject ();
}

void lmcSettingsDialog::checkBoxMessageTime_toggled(bool checked) {
    ui.checkBoxMessageDate->setEnabled(checked);
}

void lmcSettingsDialog::checkBoxAllowLinks_toggled(bool checked) {
    ui.checkBoxPathToLink->setEnabled(checked);
}

void lmcSettingsDialog::checkBoxSysTrayMsg_toggled(bool checked) {
    ui.checkBoxNewMessageNotif->setEnabled(checked);
    ui.checkBoxNewPublicMessageNotif->setEnabled(checked);
}

void lmcSettingsDialog::radioButtonSysHistoryPath_toggled(bool checked) {
    ui.textBoxHistoryPath->setEnabled(!checked);
    ui.buttonHistoryPath->setEnabled(!checked);

    if(!checked)
        ui.textBoxHistoryPath->setText(History::historyFilesDir());
    else
        ui.textBoxHistoryPath->clear();
}

void lmcSettingsDialog::buttonHistoryPath_clicked() {
    QString historyPath = QFileDialog::getExistingDirectory(this, tr("Save History"),
        ui.textBoxHistoryPath->text());
    if(!historyPath.isEmpty())
        ui.textBoxHistoryPath->setText(historyPath);
}

void lmcSettingsDialog::buttonFilePath_clicked() {
    QString filePath = QFileDialog::getExistingDirectory(this, tr("Select folder"),
        ui.textBoxFilePath->text(), QFileDialog::ShowDirsOnly);
    if(!filePath.isEmpty())
        ui.textBoxFilePath->setText(filePath);
}

void lmcSettingsDialog::buttonClearHistory_clicked() {
    QDir dir(History::historyFilesDir());
    dir.setNameFilters(QStringList() << "*.xml");
    dir.setFilter(QDir::Files);
    foreach(QString dirFile, dir.entryList())
        dir.remove(dirFile);

    emit historyCleared();
}

void lmcSettingsDialog::buttonClearFileHistory_clicked() {
    QFile::remove(StdLocation::defaultTransferHistorySavePath());
    emit fileHistoryCleared();
}

void lmcSettingsDialog::checkBoxSound_toggled(bool checked) {
    ui.listWidgetSounds->setEnabled(checked);
}

void lmcSettingsDialog::checkBoxAutoShowFile_toggled(bool checked) {
    ui.radioButtonFileTop->setEnabled(checked);
    ui.radioButtonFileBottom->setEnabled(checked);
}

void lmcSettingsDialog::buttonViewFiles_clicked() {
    QString path = ui.textBoxFilePath->text();
    QDir dir(path);
    if (!dir.exists () && QMessageBox::question (this, QStringLiteral("Folder does not exist"), QStringLiteral("The folder does not exist. Do you want to create it now?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        dir.mkpath (path);
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(ui.textBoxFilePath->text()));
}

void lmcSettingsDialog::buttonFont_clicked() {
    bool ok;
    QFont newFont = QFontDialog::getFont(&ok, font, this, tr("Select Font"));
    if(ok) {
        font = newFont;
        ui.labelFontDescription->setText(QString("%1, %2pt").arg(font.family(), QString::number(font.pointSize())));
        ui.labelFontDescription->setFont(font);

        QString style = QString("color: %1; ").arg(color.name());
        ui.textEditFontColorText->setFont(font);
        ui.textEditFontColorText->setStyleSheet(style);

     //   QMessageBox::information(0, 0, ui.textEditFontColorText->font().toString() + "\n" + QString::number(ui.textEditFontColorText->fontItalic()) + "\n" + QString::number(ui.textEditFontColorText->fontWeight()));
    }
}

QString lmcSettingsDialog::getFontStyle() {
    QString style;

    if(font.italic())
        style.append("font-style: italic; ");
    if(font.bold())
        style.append("font-weight: bold; ");
    if(font.strikeOut())
        style.append("text-decoration: line-through; ");
    if(font.underline())
        style.append("text-decoration: underline; ");

    style.append(QString("font-family: \"%1\"; ").arg(font.family()));
    style.append(QString("font-size: %1pt; ").arg(QString::number(font.pointSize())));

    return style;
}

void lmcSettingsDialog::buttonColor_clicked() {
    QColor newColor = QColorDialog::getColor(color, this, tr("Select Color"));
    if(newColor.isValid()) {
        color = newColor;

        ui.labelFontColorDisplay->setStyleSheet(QString("border: 2px outset rgb(144, 144, 144); border-radius: 4px; background-color: %1; ").arg(color.name()));

        QString style = QString("color: %1; ").arg(color.name());
        ui.textEditFontColorText->setFont(font);
        ui.textEditFontColorText->setStyleSheet(style);
    }
}

void lmcSettingsDialog::buttonReset_clicked() {
    QString message = tr("Are you sure you want to reset your %1 preferences?");
    if(QMessageBox::question(this, tr("Reset Preferences"), message.arg(lmcStrings::appName()), QMessageBox::Yes, QMessageBox::No)
        == QMessageBox::Yes) {
        QFile::remove(pSettings->fileName());
        pSettings->saveDefaults();
        pSettings->sync();
        ThemeManager::getInstance ().changeTheme (0, 0); // switch to native theme
        ThemeManager::getInstance ().endPreview ();
        accept();
    }
}

void lmcSettingsDialog::comboBoxApplicationTheme_currentIndexChanged(int index) {
    QStringList buttonThemes = ThemeManager::getInstance ().buttonThemesList (index);
    QStringList buttonThemePaths = ThemeManager::getInstance ().buttonThemesPathList (index);

    ui.comboBoxButtonTheme->clear ();
    for(int index = 0; index < buttonThemes.size (); index++)
        ui.comboBoxButtonTheme->addItem(buttonThemes[index], buttonThemePaths[index]);

    ui.comboBoxButtonTheme->setEnabled (buttonThemes.size ());

    if (buttonThemes.size ())
        ui.comboBoxButtonTheme->setCurrentIndex (0);
}

void lmcSettingsDialog::comboBoxIconTheme_currentIndexChanged(int index)
{
    Q_UNUSED(index);

    QString currentTheme = ui.comboBoxIconTheme->currentText ();

    ui.labelIconPreviewAccept->setPixmap (QPixmap(ThemeManager::getInstance ().getAppIcon (QStringLiteral("accept"), currentTheme)));
    ui.labelIconPreviewAlert->setPixmap (QPixmap(ThemeManager::getInstance ().getAppIcon (QStringLiteral("alertsettings"), currentTheme)));
    ui.labelIconPreviewChatroom->setPixmap (QPixmap(ThemeManager::getInstance ().getAppIcon (QStringLiteral("chatroom"), currentTheme)));
    ui.labelIconPreviewSave->setPixmap (QPixmap(ThemeManager::getInstance ().getAppIcon (QStringLiteral("save"), currentTheme)));
    ui.labelIconPreviewSmiley->setPixmap (QPixmap(ThemeManager::getInstance ().getAppIcon (QStringLiteral("smiley"), currentTheme)));
    ui.labelIconPreviewTools->setPixmap (QPixmap(ThemeManager::getInstance ().getAppIcon (QStringLiteral("tools"), currentTheme)));
}

void lmcSettingsDialog::comboBoxButtonTheme_currentIndexChanged(int index) {
    ThemeManager::getInstance ().previewTheme (ui.comboBoxApplicationTheme->itemData (ui.comboBoxApplicationTheme->currentIndex ()).toString (), ui.comboBoxButtonTheme->itemData (index).toString ());
}

void lmcSettingsDialog::comboBoxChatTheme_currentIndexChanged(int index) {
    pMessageLog->localId = "Myself";
    pMessageLog->demoPeerId = "Jack";

    ThemeManager::getInstance ().loadPreviewChatTheme (index);
    pMessageLog->initMessageLog(true);

    XmlMessage msg;
    msg.addData(XN_TIME, QString::number(QDateTime::currentMSecsSinceEpoch()));
    msg.addData(XN_FONT, Globals::getInstance().messagesFontString());
    msg.addData(XN_COLOR, Globals::getInstance().messagesColor());

    QString userId = "Jack";
    QString userName = "Jack";

    msg.addData(XN_MESSAGE, "Hello, this is an incoming message.");
    pMessageLog->appendMessageLog(MT_Message, userId, userName, msg, true, false, false);

   // msg.removeData(XN_MESSAGE);
    msg.addData(XN_MESSAGE, "Hello, this is a consecutive incoming message.");
    pMessageLog->appendMessageLog(MT_Message, userId, userName, msg, true, false, false);

   // msg.removeData(XN_MESSAGE);
    msg.addData(XN_MESSAGE, "This is a broadcast message!");
    pMessageLog->appendMessageLog(MT_Broadcast, userId, userName, msg, true, false, false);

    userId = "Myself";
    userName = "Myself";

    //msg.removeData(XN_BROADCAST);
    msg.addData(XN_MESSAGE, "Hi, this is an outgoing message.");
    pMessageLog->appendMessageLog(MT_Message, userId, userName, msg, true, false, false);

    //msg.removeData(XN_MESSAGE);
    msg.addData(XN_MESSAGE, "Hi, this is a consecutive outgoing message.");
    pMessageLog->appendMessageLog(MT_Message, userId, userName, msg, true, false, false);

    userId = "Jack";
    userName = "Jack";

    msg.removeData(XN_MESSAGE);
    msg.addData(XN_MESSAGE, "This is another incoming message.");
    pMessageLog->appendMessageLog(MT_Message, userId, userName, msg, true, false, false);
}

void lmcSettingsDialog::listWidgetBroadcasts_currentRowChanged(int index) {
    ui.buttonDeleteBroadcast->setEnabled(!(index < 0));
}

void lmcSettingsDialog::textBoxBroadcast_textEdited(const QString& text) {
    ui.buttonAddBroadcast->setEnabled(ipRegExp.exactMatch(text));
}

void lmcSettingsDialog::buttonAddBroadcast_clicked() {
    QString address = ui.textBoxBroadcast->text();
    //	Do not add if not a valid ip address
    if(!ipRegExp.exactMatch(address))
        return;

    //	Check if the same address is already present in the list
    for(int index = 0; index < ui.listWidgetBroadcasts->count(); index++) {
        QString text = ui.listWidgetBroadcasts->item(index)->text();
        if(text.compare(address) == 0)
            return;
    }

    QListWidgetItem* item = new QListWidgetItem(ui.listWidgetBroadcasts);
    item->setText(address);

    ui.textBoxBroadcast->clear();
    ui.buttonAddBroadcast->setEnabled(false);
    ui.textBoxBroadcast->setFocus();
}

void lmcSettingsDialog::buttonDeleteBroadcast_clicked() {
    if(ui.listWidgetBroadcasts->currentRow() < 0)
        return;

    QListWidgetItem* item = ui.listWidgetBroadcasts->takeItem(ui.listWidgetBroadcasts->currentRow());
    delete item;
}

void lmcSettingsDialog::listWidgetSounds_currentRowChanged(int index) {
    ui.buttonPlaySound->setEnabled(!(index < 0));
    ui.buttonSoundPath->setEnabled(!(index < 0));

    if(index < 0) {
        ui.textBoxSoundFile->clear();
        return;
    }

    QFileInfo fileInfo(ui.listWidgetSounds->currentItem()->data(Qt::UserRole).toString());
    if(fileInfo.exists())
        ui.textBoxSoundFile->setText(fileInfo.fileName());
    else
        ui.textBoxSoundFile->setText(tr("<File Not Found>"));
}

void lmcSettingsDialog::buttonPlaySound_clicked() {
    if(ui.listWidgetSounds->currentRow() < 0)
        return;

    QSound::play(ui.listWidgetSounds->currentItem()->data(Qt::UserRole).toString());
}

void lmcSettingsDialog::buttonSoundPath_clicked() {
    if(ui.listWidgetSounds->currentRow() < 0)
        return;

    QString soundPath = QFileDialog::getOpenFileName(this, tr("Select sound"),
        ui.listWidgetSounds->currentItem()->data(Qt::UserRole).toString(), "Wave Files (*.wav)");
    if(!soundPath.isEmpty()) {
        ui.listWidgetSounds->currentItem()->setData(Qt::UserRole, soundPath);
        listWidgetSounds_currentRowChanged(ui.listWidgetSounds->currentRow());
    }
}

void lmcSettingsDialog::buttonResetSounds_clicked() {
    for(int index = 0; index < SE_Max; index++) {
        QListWidgetItem* pListItem = ui.listWidgetSounds->item(index);
        pListItem->setData(Qt::UserRole, soundFile[index]);
    }
    listWidgetSounds_currentRowChanged(ui.listWidgetSounds->currentRow());
}

void lmcSettingsDialog::setPageHeaderStyle(QLabel* pLabel) {
    QFont font = pLabel->font();
    int fontSize = pLabel->fontInfo().pixelSize();
    fontSize += (fontSize * 0.2);
    font.setPixelSize(fontSize);
    font.setBold(true);
    pLabel->setFont(font);
}

void lmcSettingsDialog::setUIText() {
    ui.retranslateUi(this);

    setWindowTitle(tr("Preferences"));

    ui.checkBoxAutoStart->setText(ui.checkBoxAutoStart->text().arg(lmcStrings::appName()));
    ui.checkBoxAutoShow->setText(ui.checkBoxAutoShow->text().arg(lmcStrings::appName()));
    ui.labelFinePrint->setText(ui.labelFinePrint->text().arg(lmcStrings::appName()));
    ui.labelNeedRestart->setText (ui.labelNeedRestart->text().arg(lmcStrings::appName()));

    if(!QSystemTrayIcon::isSystemTrayAvailable()) {
        ui.groupSysTray->setEnabled(false);
        ui.groupSysTray->setTitle(tr("System Tray (Not Available)"));
    }
    if(!QSystemTrayIcon::supportsMessages()) {
        ui.groupAlerts->setEnabled(false);
        ui.groupAlerts->setTitle(tr("Status Alerts (Not Available)"));
    }
    if(!QAudioDeviceInfo::availableDevices(QAudio::AudioOutput).isEmpty()) {
                ui.groupSounds->setEnabled(false);
                ui.groupSounds->setTitle(tr("Sounds (Not Available)"));
    }

    for(int index = 0; index < ui.listWidgetSounds->count(); index++)
        ui.listWidgetSounds->item(index)->setText(lmcStrings::soundDesc()[index]);

    for(int index = 0; index < ULV_Max; index++)
        ui.comboBoxUserListView->setItemText(index, lmcStrings::userListView()[index]);

//	QString updateLink = QString(IDA_DOMAIN"/downloads.php#translations");
//	ui.labelUpdateLink->setText("<a href='" + updateLink + "'><span style='text-decoration: underline; color:#0000ff;'>" +
//		tr("Check for updates") + "</span></a>");

    comboBoxChatTheme_currentIndexChanged(ui.comboBoxChatTheme->currentIndex());

#ifdef Q_OS_MAC
    ui.radioButtonEnter->setText("Return");
    ui.radioButtonCmdEnter->setText(QString(QChar(0x2318)) + " + Return"); // U+2318 is the hex code for Bowen Knot symbol
#else
    ui.radioButtonEnter->setText("Enter");
    ui.radioButtonCmdEnter->setText("Ctrl + Enter");
#endif

    //	set minimum possible size
    layout()->setSizeConstraint(QLayout::SetMinimumSize);
}

void lmcSettingsDialog::loadSettings() {
    //	Auto start function not implemented on Mac since Mac itself provides an easy UI for it
#ifdef Q_OS_MAC
        ui.checkBoxAutoStart->setChecked(false);
        ui.checkBoxAutoStart->hide();
#else
    ui.checkBoxAutoStart->setChecked(Globals::getInstance().autoStart());
#endif
    ui.checkBoxAutoShow->setChecked(Globals::getInstance().autoShow());
    ui.checkBoxEnableSnap->setChecked(Globals::getInstance().windowSnapping());
    ui.checkBoxSysTray->setChecked(Globals::getInstance().sysTray());
    ui.checkBoxMinimizeTray->setChecked(Globals::getInstance().minimizeToTray());
    ui.checkBoxSingleClickTray->setChecked(Globals::getInstance().singleClickTray());
    ui.checkBoxSysTrayMsg->setChecked(Globals::getInstance().sysTrayMessages());
    ui.checkBoxNewMessageNotif->setChecked(Globals::getInstance().sysTrayNewMessages());
    ui.checkBoxNewPublicMessageNotif->setChecked(Globals::getInstance().sysTrayNewPublicMessages());
    ui.checkBoxAllowSysTrayMin->setChecked(Globals::getInstance().sysTrayMinimize());
    ui.checkBoxRememberStatus->setChecked(Globals::getInstance().restoreStatus());
    QString langCode = Globals::getInstance().language();
    for(int index = 0; index < ui.comboBoxLanguage->count(); index ++) {
        QString code = ui.comboBoxLanguage->itemData(index, Qt::UserRole).toString();
        if(langCode.compare(code) == 0) {
            ui.comboBoxLanguage->setCurrentIndex(index);
            break;
        }
    }

    if (Globals::getInstance().defaultNewMessageAction() == 2)
        ui.radioSendInstantMessage->setChecked(true);
    else
        ui.radioOpenConversation->setChecked(true);

    ui.textBoxUserName->setText(Globals::getInstance().userName());
    ui.textBoxFirstName->setText(Globals::getInstance().userFirstName());
    ui.textBoxLastName->setText(Globals::getInstance().userLastName());
    ui.textBoxAbout->setPlainText(Globals::getInstance().userAbout());
    ui.spinBoxRefreshTime->setValue(Globals::getInstance().refreshInterval());

    ui.checkBoxMessageRaise->setChecked(Globals::getInstance().popOnNewMessage());
    ui.checkBoxPublicMessageRaise->setChecked(Globals::getInstance().popOnNewPublicMessage());
    ui.checkBoxEmoticon->setChecked(Globals::getInstance().showEmoticons());
    ui.checkBoxMessageTime->setChecked(Globals::getInstance().showMessageTime());
    ui.checkBoxMessageDate->setChecked(Globals::getInstance().showMessageDate());
    ui.checkBoxAllowLinks->setChecked(Globals::getInstance().showLinks());
    ui.checkBoxPathToLink->setChecked(Globals::getInstance().showPathsAsLinks());
    ui.checkBoxTrimMessage->setChecked(Globals::getInstance().trimMessages());
    ui.checkBoxAppendHistory->setChecked(Globals::getInstance().appendHistory());
    ui.checkBoxReadNotifications->setChecked(!Globals::getInstance().informReadMessage());
    font.fromString(Globals::getInstance().messagesFontString());
    color.setNamedColor(Globals::getInstance().messagesColor());
    ui.checkBoxOverrideIncoming->setChecked(Globals::getInstance().overrideInMessagesStyle());
    ui.checkBoxCharCount->setChecked(Globals::getInstance().showCharacterCount());

    ui.labelFontDescription->setText(QString("%1, %2pt").arg(font.family(), QString::number(font.pointSize())));
    ui.labelFontDescription->setFont(font);//->setStyleSheet(fontStyle);

    ui.labelFontColorDisplay->setStyleSheet(QString("border: 2px outset rgb(144, 144, 144); border-radius: 4px; background-color: %1; ").arg(color.name()));
    QString style = QString("color: %1; ").arg(color.name());
    ui.textEditFontColorText->setFont(font);
    ui.textEditFontColorText->setStyleSheet(style);

    ui.checkBoxHistory->setChecked(Globals::getInstance().saveHistory());
    ui.radioButtonSysHistoryPath->setChecked(Globals::getInstance().defaultHistorySavePath());
    ui.radioButtonCustomHistoryPath->setChecked(!Globals::getInstance().defaultHistorySavePath());
    ui.checkBoxFileHistory->setChecked(Globals::getInstance().saveFileHistory());

    ui.checkBoxAlert->setChecked(Globals::getInstance().enableAlerts());
    ui.checkBoxNoBusyAlert->setChecked(Globals::getInstance().noBusyAlerts());
    ui.checkBoxNoDNDAlert->setChecked(Globals::getInstance().noDNDAlerts());
    ui.checkBoxSound->setChecked(Globals::getInstance().enableSoundAlerts());
    // Check so that number of elements read from settings file does not exceed the number of elements
    // in the list view control. This prevents array out of bounds error.
    int size = qMin(pSettings->beginReadArray(IDS_SOUNDEVENTHDR), ui.listWidgetSounds->count());
    for(int index = 0; index < size; index++) {
        pSettings->setArrayIndex(index);
        ui.listWidgetSounds->item(index)->setCheckState((Qt::CheckState)pSettings->value(IDS_SOUNDEVENT).toInt());
    }
    pSettings->endArray();
    size = qMin(pSettings->beginReadArray(IDS_SOUNDFILEHDR), ui.listWidgetSounds->count());
    for(int index = 0; index < size; index++) {
        pSettings->setArrayIndex(index);
        ui.listWidgetSounds->item(index)->setData(Qt::UserRole, pSettings->value(IDS_SOUNDFILE).toString());
    }
    pSettings->endArray();
    ui.checkBoxNoBusySound->setChecked(Globals::getInstance().noBusySounds());
    ui.checkBoxNoDNDSound->setChecked(Globals::getInstance().noDNDSounds());

    ui.spinBoxTimeout->setValue(Globals::getInstance().connectionTimeout());
    ui.spinBoxMaxRetries->setValue(Globals::getInstance().connectionRetries());
    size = pSettings->beginReadArray(IDS_BROADCASTHDR);
    for(int index = 0; index < size; index++) {
        pSettings->setArrayIndex(index);
        QListWidgetItem* item = new QListWidgetItem(ui.listWidgetBroadcasts);
        item->setText(pSettings->value(IDS_BROADCAST).toString());
    }
    pSettings->endArray();
    ui.textBoxMulticast->setText(Globals::getInstance().multicastAddress());
    ui.textBoxUDPPort->setText(QString::number(Globals::getInstance().udpPort()));
    ui.textBoxTCPPort->setText(QString::number(Globals::getInstance().tcpPort()));
    ui.textBoxErpAddress->setText(Globals::getInstance().erpAddress());

    ui.checkBoxAutoFile->setChecked(Globals::getInstance().autoReceiveFile());
    ui.checkBoxAutoShowFile->setChecked(Globals::getInstance().autoShowTransfer());
    ui.radioButtonFileTop->setChecked(Globals::getInstance().displayNewTransfers());
    ui.radioButtonFileBottom->setChecked(!Globals::getInstance().displayNewTransfers());
    ui.textBoxFilePath->setText(Globals::getInstance().fileStoragePath());
    ui.checkBoxFolderForEach->setChecked(Globals::getInstance().createIndividualFolders());

    QString themeName = Globals::getInstance().chatTheme();
    ui.comboBoxChatTheme->setCurrentText(themeName);

    themeName = Globals::getInstance().applicationTheme();
    ui.comboBoxApplicationTheme->setCurrentText(themeName);

    themeName = Globals::getInstance().iconTheme();
    ui.comboBoxIconTheme->setCurrentText(themeName);

    themeName = Globals::getInstance().buttonsTheme();
    ui.comboBoxButtonTheme->setCurrentText(themeName);

    int userListView = Globals::getInstance().userListView();
    ui.comboBoxUserListView->setCurrentIndex(userListView);

    ui.radioButtonEnter->setChecked(Globals::getInstance().sendByEnter());
    ui.radioButtonCmdEnter->setChecked(!Globals::getInstance().sendByEnter());

    checkBoxMessageTime_toggled(ui.checkBoxMessageTime->isChecked());
    checkBoxAllowLinks_toggled(ui.checkBoxAllowLinks->isChecked());
    checkBoxSysTrayMsg_toggled(ui.checkBoxSysTrayMsg->isChecked());
    radioButtonSysHistoryPath_toggled(ui.radioButtonSysHistoryPath->isChecked());
    checkBoxAutoShowFile_toggled(ui.checkBoxAutoShowFile->isChecked());
}

void lmcSettingsDialog::saveSettings() {
    Globals::getInstance().setVersion(IDA_VERSION);
    Globals::getInstance().setAutoStart(ui.checkBoxAutoStart->isChecked());
    Globals::getInstance().setAutoShow(ui.checkBoxAutoShow->isChecked());
    Globals::getInstance().setWindowSnapping(ui.checkBoxEnableSnap->isChecked());
    Globals::getInstance().setSysTray(ui.checkBoxSysTray->isChecked());
    Globals::getInstance().setMinimizeToTray(ui.checkBoxMinimizeTray->isChecked());
    Globals::getInstance().setSingleClickTray(ui.checkBoxSingleClickTray->isChecked());
    Globals::getInstance().setSysTrayMessages(ui.checkBoxSysTrayMsg->isChecked());
    Globals::getInstance().setSysTrayNewMessages(ui.checkBoxNewMessageNotif->isChecked());
    Globals::getInstance().setSysTrayNewPublicMessages(ui.checkBoxNewPublicMessageNotif->isChecked());
    Globals::getInstance().setSysTrayMinimize(ui.checkBoxAllowSysTrayMin->isChecked());
    Globals::getInstance().setRestoreStatus(ui.checkBoxRememberStatus->isChecked());
    QString langCode = ui.comboBoxLanguage->itemData(ui.comboBoxLanguage->currentIndex(), Qt::UserRole).toString();
    Globals::getInstance().setLanguage(langCode);

    int defaultNewMessageAction = 1;
    if (ui.radioSendInstantMessage->isChecked())
        defaultNewMessageAction = 2;
    Globals::getInstance().setDefaultNewMessageAction(defaultNewMessageAction);

    Globals::getInstance().setUserName(ui.textBoxUserName->text());
    Globals::getInstance().setUserFirstName(ui.textBoxFirstName->text());
    Globals::getInstance().setUserLastName(ui.textBoxLastName->text());
    Globals::getInstance().setUserAbout(ui.textBoxAbout->toPlainText());
    Globals::getInstance().setRefreshInterval(ui.spinBoxRefreshTime->value());

    Globals::getInstance().setPopOnNewMessage(ui.checkBoxMessageRaise->isChecked());
    Globals::getInstance().setPopOnNewPublicMessage(ui.checkBoxPublicMessageRaise->isChecked());
    Globals::getInstance().setShowEmoticons(ui.checkBoxEmoticon->isChecked());
    Globals::getInstance().setShowMessageTime(ui.checkBoxMessageTime->isChecked());
    Globals::getInstance().setShowMessageDate(ui.checkBoxMessageDate->isChecked());
    Globals::getInstance().setShowLinks(ui.checkBoxAllowLinks->isChecked());
    Globals::getInstance().setShowPathsAsLinks(ui.checkBoxPathToLink->isChecked());
    Globals::getInstance().setTrimMessages(ui.checkBoxTrimMessage->isChecked());
    Globals::getInstance().setAppendHistory(ui.checkBoxAppendHistory->isChecked());
    Globals::getInstance().setMessagesFont(font.toString());
    Globals::getInstance().setMessagesColor(color.name());
    Globals::getInstance().setOverrideInMessagesStyle(ui.checkBoxOverrideIncoming->isChecked());
    Globals::getInstance().setInformReadMessage(!ui.checkBoxReadNotifications->isChecked());
    Globals::getInstance().setShowCharacterCount(ui.checkBoxCharCount->isChecked());

    Globals::getInstance().setSaveHistory(ui.checkBoxHistory->isChecked());
    Globals::getInstance().setDefaultHistorySavePath(ui.radioButtonSysHistoryPath->isChecked());
    Globals::getInstance().setHistorySavePath(ui.textBoxHistoryPath->text());
    Globals::getInstance().setSaveFileHistory(ui.checkBoxFileHistory->isChecked());

    Globals::getInstance().setEnableAlerts(ui.checkBoxAlert->isChecked());
    Globals::getInstance().setNoBusyAlerts(ui.checkBoxNoBusyAlert->isChecked());
    Globals::getInstance().setNoDNDAlerts(ui.checkBoxNoDNDAlert->isChecked());
    Globals::getInstance().setEnableSoundAlerts(ui.checkBoxSound->isChecked());
    int checkCount = 0;
    int soundFileCount = 0;
    if(ui.listWidgetSounds->count() > 0) {
        pSettings->beginWriteArray(IDS_SOUNDEVENTHDR);
        for(int index = 0; index < ui.listWidgetSounds->count(); index++) {
            pSettings->setArrayIndex(index);
            pSettings->setValue(IDS_SOUNDEVENT, ui.listWidgetSounds->item(index)->checkState());
            if(ui.listWidgetSounds->item(index)->checkState() == IDS_SOUNDEVENT_VAL)
                checkCount++;
        }
        pSettings->endArray();
        pSettings->beginWriteArray(IDS_SOUNDFILEHDR);
        for(int index = 0; index < ui.listWidgetSounds->count(); index++) {
            pSettings->setArrayIndex(index);
            pSettings->setValue(IDS_SOUNDFILE, ui.listWidgetSounds->item(index)->data(Qt::UserRole).toString());
            if(ui.listWidgetSounds->item(index)->data(Qt::UserRole).toString().compare(soundFile[index]) == 0)
                soundFileCount++;
        }
        pSettings->endArray();
    }
    if(ui.listWidgetSounds->count() == 0 || checkCount == ui.listWidgetSounds->count()) {
        pSettings->beginGroup(IDS_SOUNDEVENTHDR);
        pSettings->remove("");
        pSettings->endGroup();
    }
    if(ui.listWidgetSounds->count() == 0 || soundFileCount == ui.listWidgetSounds->count()) {
        pSettings->beginGroup(IDS_SOUNDFILEHDR);
        pSettings->remove("");
        pSettings->endGroup();
    }

    Globals::getInstance().setNoBusySounds(ui.checkBoxNoBusySound->isChecked());
    Globals::getInstance().setNoDNDSounds(ui.checkBoxNoDNDSound->isChecked());

    Globals::getInstance().setConnectionTimeout(ui.spinBoxTimeout->value());
    Globals::getInstance().setConnectionRetries(ui.spinBoxMaxRetries->value());
    //	If any broadcast address is specified, settings written to settings file
    //	Otherwise, the entire group is removed from the settings file
    if(ui.listWidgetBroadcasts->count() > 0) {
        pSettings->beginWriteArray(IDS_BROADCASTHDR);
        for(int index = 0; index < ui.listWidgetBroadcasts->count(); index++) {
            pSettings->setArrayIndex(index);
            pSettings->setValue(IDS_BROADCAST, ui.listWidgetBroadcasts->item(index)->text());
        }
        pSettings->endArray();
    }
    if(ui.listWidgetBroadcasts->count() == 0){
        pSettings->beginGroup(IDS_BROADCASTHDR);
        pSettings->remove("");
        pSettings->endGroup();
    }
    Globals::getInstance().setMulticastAddress(ui.textBoxMulticast->text());
    Globals::getInstance().setUdpPort(ui.textBoxUDPPort->text().toInt());
    Globals::getInstance().setTcpPort(ui.textBoxTCPPort->text().toInt());
    Globals::getInstance().setErpAddress(ui.textBoxErpAddress->text());

    Globals::getInstance().setAutoReceiveFile(ui.checkBoxAutoFile->isChecked());
    Globals::getInstance().setAutoShowTransfer(ui.checkBoxAutoShowFile->isChecked());
    Globals::getInstance().setDisplayNewTransfers(ui.radioButtonFileTop->isChecked());
    Globals::getInstance().setFileStoragePath(ui.textBoxFilePath->text());
    Globals::getInstance().setCreateIndividualFolders(ui.checkBoxFolderForEach->isChecked());

    Globals::getInstance().setChatTheme(ui.comboBoxChatTheme->currentText ());
    Globals::getInstance().setApplicationTheme(ui.comboBoxApplicationTheme->currentText ());
    Globals::getInstance().setIconTheme(ui.comboBoxIconTheme->currentText ());
    Globals::getInstance().setButtonsTheme(ui.comboBoxButtonTheme->currentText ());

    Globals::getInstance().setUserListView(static_cast<UserListView> (ui.comboBoxUserListView->currentIndex()));

    Globals::getInstance().setSendByEnter(ui.radioButtonEnter->isChecked());

    pSettings->sync();
}
