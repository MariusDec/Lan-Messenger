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


#include <QUrl>
#include <QSound>
#include <QSystemTrayIcon>
#include <QLocale>
#include <QAudioDeviceInfo>
#include <QMessageBox>
#include "settingsdialog.h"
#include "thememanager.h"

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
    // TODO change all connects to QT5 equivalents
    connect(ui.checkBoxSysTrayMsg, &QCheckBox::toggled, this, &lmcSettingsDialog::checkBoxSysTrayMsg_toggled);
    connect(ui.buttonFilePath, SIGNAL(clicked()), this, SLOT(buttonFilePath_clicked()));
    connect(ui.buttonClearHistory, SIGNAL(clicked()), this, SLOT(buttonClearHistory_clicked()));
    connect(ui.buttonClearFileHistory, SIGNAL(clicked()), this, SLOT(buttonClearFileHistory_clicked()));
    connect(ui.buttonViewFiles, SIGNAL(clicked()), this, SLOT(buttonViewFiles_clicked()));
    connect(ui.checkBoxSound, SIGNAL(toggled(bool)), this, SLOT(checkBoxSound_toggled(bool)));
    connect(ui.checkBoxAutoShowFile, SIGNAL(toggled(bool)), this, SLOT(checkBoxAutoShowFile_toggled(bool)));
    connect(ui.buttonFont, SIGNAL(clicked()), this, SLOT(buttonFont_clicked()));
    connect(ui.buttonColor, SIGNAL(clicked()), this, SLOT(buttonColor_clicked()));
    connect(ui.buttonReset, SIGNAL(clicked()), this, SLOT(buttonReset_clicked()));
    connect(ui.comboBoxApplicationTheme, &ThemedComboBox::currentIndexChanged, this, &lmcSettingsDialog::comboBoxApplicationTheme_currentIndexChanged);
    connect(ui.comboBoxIconTheme, &ThemedComboBox::currentIndexChanged, this, &lmcSettingsDialog::comboBoxIconTheme_currentIndexChanged);
    connect(ui.comboBoxButtonTheme, &ThemedComboBox::currentIndexChanged, this, &lmcSettingsDialog::comboBoxButtonTheme_currentIndexChanged);
    connect(ui.comboBoxChatTheme, &ThemedComboBox::currentIndexChanged, this, &lmcSettingsDialog::comboBoxChatTheme_currentIndexChanged);
    connect(ui.listWidgetBroadcasts, SIGNAL(currentRowChanged(int)), this, SLOT(listWidgetBroadcasts_currentRowChanged(int)));
    connect(ui.textBoxBroadcast, SIGNAL(textEdited(QString)), this, SLOT(textBoxBroadcast_textEdited(QString)));
    connect(ui.textBoxBroadcast, SIGNAL(returnPressed()), this, SLOT(buttonAddBroadcast_clicked()));
    connect(ui.buttonAddBroadcast, SIGNAL(clicked()), this, SLOT(buttonAddBroadcast_clicked()));
    connect(ui.buttonDeleteBroadcast, SIGNAL(clicked()), this, SLOT(buttonDeleteBroadcast_clicked()));
    connect(ui.listWidgetSounds, SIGNAL(currentRowChanged(int)), this, SLOT(listWidgetSounds_currentRowChanged(int)));
    connect(ui.buttonPlaySound, SIGNAL(clicked()), this, SLOT(buttonPlaySound_clicked()));
    connect(ui.buttonSoundPath, SIGNAL(clicked()), this, SLOT(buttonSoundPath_clicked()));
    connect(ui.buttonResetSounds, SIGNAL(clicked()), this, SLOT(buttonResetSounds_clicked()));
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
    QString historyPath = QFileDialog::getSaveFileName(this, tr("Save History"),
        ui.textBoxHistoryPath->text(), "Messenger DB (*.db)");
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
    QFile::remove(StdLocation::transferHistoryFilePath());
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
        ui.labelFontDescription->setText(QString("%1, %2px").arg(font.family(), QString::number(font.pointSize())));

        ui.labelFontDescription->setStyleSheet(getFontStyle(font));
    }
}

QString lmcSettingsDialog::getFontStyle(const QFont &font) {
    QString style;

    if(font.italic())
        style.append("font-style:italic; ");
    if(font.bold())
        style.append("font-weight:bold; ");
    if(font.strikeOut())
        style.append("text-decoration:line-through; ");
    if(font.underline())
        style.append("text-decoration:underline; ");

    style.append(QString("font-family:\"%1\"; ").arg(font.family()));
    style.append(QString("font-size:%1; ").arg(QString::number(font.pointSize())));

    return style;
}

void lmcSettingsDialog::buttonColor_clicked() {
    QColor newColor = QColorDialog::getColor(color, this, tr("Select Color"));
    if(newColor.isValid()) {
        color = newColor;

        ui.labelFontColorDisplay->setStyleSheet(QString("border: 2px outset rgb(144, 144, 144); border-radius: 4px; background-color: %1; ").arg(color.name()));
        ui.textEditFontColorText->setStyleSheet(QString("color: %1; ").arg(color.name()));
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
    pMessageLog->messageTime = true;

    ThemeManager::getInstance ().loadPreviewChatTheme (index);
    pMessageLog->initMessageLog(true);

    XmlMessage msg;
    msg.addData(XN_TIME, QString::number(QDateTime::currentMSecsSinceEpoch()));
    msg.addData(XN_FONT, pSettings->value(IDS_FONT, IDS_FONT_VAL).toString());
    msg.addData(XN_COLOR, pSettings->value(IDS_COLOR, IDS_COLOR_VAL).toString());

    QString userId = "Jack";
    QString userName = "Jack";

    msg.addData(XN_MESSAGE, "Hello, this is an incoming message.");
    pMessageLog->appendMessageLog(MT_Message, &userId, &userName, &msg, true, false, false);

   // msg.removeData(XN_MESSAGE);
    msg.addData(XN_MESSAGE, "Hello, this is a consecutive incoming message.");
    pMessageLog->appendMessageLog(MT_Message, &userId, &userName, &msg, true, false, false);

   // msg.removeData(XN_MESSAGE);
    msg.addData(XN_BROADCAST, "This is a broadcast message!");
    pMessageLog->appendMessageLog(MT_Broadcast, &userId, &userName, &msg, true, false, false);

    userId = "Myself";
    userName = "Myself";

    //msg.removeData(XN_BROADCAST);
    msg.addData(XN_MESSAGE, "Hi, this is an outgoing message.");
    pMessageLog->appendMessageLog(MT_Message, &userId, &userName, &msg, true, false, false);

    //msg.removeData(XN_MESSAGE);
    msg.addData(XN_MESSAGE, "Hi, this is a consecutive outgoing message.");
    pMessageLog->appendMessageLog(MT_Message, &userId, &userName, &msg, true, false, false);

    userId = "Jack";
    userName = "Jack";

    msg.removeData(XN_MESSAGE);
    msg.addData(XN_MESSAGE, "This is another incoming message.");
    pMessageLog->appendMessageLog(MT_Message, &userId, &userName, &msg, true, false, false);
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
    ui.checkBoxAutoStart->setChecked(pSettings->value(IDS_AUTOSTART, IDS_AUTOSTART_DEFAULT_VAL).toBool());
#endif
    ui.checkBoxAutoShow->setChecked(pSettings->value(IDS_AUTOSHOW, IDS_AUTOSHOW_DEFAULT_VAL).toBool());
    ui.checkBoxSysTray->setChecked(pSettings->value(IDS_SYSTRAY, IDS_SYSTRAY_VAL).toBool());
    ui.checkBoxMinimizeTray->setChecked(pSettings->value(IDS_MINIMIZETRAY, IDS_MINIMIZETRAY_VAL).toBool());
    ui.checkBoxSingleClickTray->setChecked(pSettings->value(IDS_SINGLECLICKTRAY, IDS_SINGLECLICKTRAY_VAL).toBool());
    ui.checkBoxSysTrayMsg->setChecked(pSettings->value(IDS_SYSTRAYMSG, IDS_SYSTRAYMSG_VAL).toBool());
    ui.checkBoxNewMessageNotif->setChecked(pSettings->value(IDS_SYSTRAYNEWMSG, IDS_SYSTRAYNEWMSG_VAL).toBool());
    ui.checkBoxNewPublicMessageNotif->setChecked(pSettings->value(IDS_SYSTRAYPUBNEWMSG, IDS_SYSTRAYPUBNEWMSG_VAL).toBool());
    ui.checkBoxAllowSysTrayMin->setChecked(pSettings->value(IDS_ALLOWSYSTRAYMIN, IDS_ALLOWSYSTRAYMIN_VAL).toBool());
    ui.checkBoxRememberStatus->setChecked(pSettings->value(IDS_RESTORESTATUS, IDS_RESTORESTATUS_VAL).toBool());
    QString langCode = pSettings->value(IDS_LANGUAGE, IDS_LANGUAGE_DEFAULT_VAL).toString();
    for(int index = 0; index < ui.comboBoxLanguage->count(); index ++) {
        QString code = ui.comboBoxLanguage->itemData(index, Qt::UserRole).toString();
        if(langCode.compare(code) == 0) {
            ui.comboBoxLanguage->setCurrentIndex(index);
            break;
        }
    }

    ui.textBoxUserName->setText(pSettings->value(IDS_USERNAME, IDS_USERNAME_VAL).toString());
    ui.textBoxFirstName->setText(pSettings->value(IDS_USERFIRSTNAME, IDS_USERFIRSTNAME_VAL).toString());
    ui.textBoxLastName->setText(pSettings->value(IDS_USERLASTNAME, IDS_USERLASTNAME_VAL).toString());
    ui.textBoxAbout->setPlainText(pSettings->value(IDS_USERABOUT, IDS_USERABOUT_VAL).toString());
    ui.spinBoxRefreshTime->setValue(pSettings->value(IDS_REFRESHTIME, IDS_REFRESHTIME_VAL).toInt());

    ui.radioButtonMessageTop->setChecked(pSettings->value(IDS_MESSAGEPOP, IDS_MESSAGEPOP_VAL).toBool());
    ui.radioButtonMessageBottom->setChecked(!pSettings->value(IDS_MESSAGEPOP, IDS_MESSAGEPOP_VAL).toBool());
    ui.checkBoxPublicMessagePop->setChecked(pSettings->value(IDS_PUBMESSAGEPOP, IDS_PUBMESSAGEPOP_VAL).toBool());
    ui.checkBoxEmoticon->setChecked(pSettings->value(IDS_EMOTICON, IDS_EMOTICON_VAL).toBool());
    ui.checkBoxMessageTime->setChecked(pSettings->value(IDS_MESSAGETIME, IDS_MESSAGETIME_VAL).toBool());
    ui.checkBoxMessageDate->setChecked(pSettings->value(IDS_MESSAGEDATE, IDS_MESSAGEDATE_VAL).toBool());
    ui.checkBoxAllowLinks->setChecked(pSettings->value(IDS_ALLOWLINKS, IDS_ALLOWLINKS_VAL).toBool());
    ui.checkBoxPathToLink->setChecked(pSettings->value(IDS_PATHTOLINK, IDS_PATHTOLINK_VAL).toBool());
    ui.checkBoxTrimMessage->setChecked(pSettings->value(IDS_TRIMMESSAGE, IDS_TRIMMESSAGE_VAL).toBool());
    ui.checkBoxAppendHistory->setChecked(pSettings->value(IDS_APPENDHISTORY, IDS_APPENDHISTORY_VAL).toBool());
    font.fromString(pSettings->value(IDS_FONT, IDS_FONT_VAL).toString());
    color.setNamedColor(pSettings->value(IDS_COLOR, IDS_COLOR_VAL).toString());
    ui.checkBoxOverrideIncoming->setChecked(pSettings->value(IDS_OVERRIDEINMSG, IDS_OVERRIDEINMSG_VAL).toBool());

    ui.labelFontDescription->setText(QString("%1, %2px").arg(font.family(), QString::number(font.pointSize())));
    ui.labelFontDescription->setStyleSheet(getFontStyle(font));

    ui.labelFontColorDisplay->setStyleSheet(QString("border: 2px outset rgb(144, 144, 144); border-radius: 4px; background-color: %1; ").arg(color.name()));
    ui.textEditFontColorText->setStyleSheet(QString("color: %1; ").arg(color.name()));

    ui.checkBoxHistory->setChecked(pSettings->value(IDS_HISTORY, IDS_HISTORY_VAL).toBool());
    ui.radioButtonSysHistoryPath->setChecked(pSettings->value(IDS_SYSHISTORYPATH, IDS_SYSHISTORYPATH_VAL).toBool());
    ui.radioButtonCustomHistoryPath->setChecked(!pSettings->value(IDS_SYSHISTORYPATH, IDS_SYSHISTORYPATH_VAL).toBool());
    ui.checkBoxFileHistory->setChecked(pSettings->value(IDS_FILEHISTORY, IDS_FILEHISTORY_VAL).toBool());

    ui.checkBoxAlert->setChecked(pSettings->value(IDS_ALERT, IDS_ALERT_VAL).toBool());
    ui.checkBoxNoBusyAlert->setChecked(pSettings->value(IDS_NOBUSYALERT, IDS_NOBUSYALERT_VAL).toBool());
    ui.checkBoxNoDNDAlert->setChecked(pSettings->value(IDS_NODNDALERT, IDS_NODNDALERT_VAL).toBool());
    ui.checkBoxSound->setChecked(pSettings->value(IDS_SOUND, IDS_SOUND_VAL).toBool());
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
    ui.checkBoxNoBusySound->setChecked(pSettings->value(IDS_NOBUSYSOUND, IDS_NOBUSYSOUND_VAL).toBool());
    ui.checkBoxNoDNDSound->setChecked(pSettings->value(IDS_NODNDSOUND, IDS_NODNDSOUND_VAL).toBool());

    ui.spinBoxTimeout->setValue(pSettings->value(IDS_TIMEOUT, IDS_TIMEOUT_VAL).toInt());
    ui.spinBoxMaxRetries->setValue(pSettings->value(IDS_MAXRETRIES, IDS_MAXRETRIES_VAL).toInt());
    size = pSettings->beginReadArray(IDS_BROADCASTHDR);
    for(int index = 0; index < size; index++) {
        pSettings->setArrayIndex(index);
        QListWidgetItem* item = new QListWidgetItem(ui.listWidgetBroadcasts);
        item->setText(pSettings->value(IDS_BROADCAST).toString());
    }
    pSettings->endArray();
    ui.textBoxMulticast->setText(pSettings->value(IDS_MULTICAST, IDS_MULTICAST_VAL).toString());
    ui.textBoxUDPPort->setText(pSettings->value(IDS_UDPPORT, IDS_UDPPORT_VAL).toString());
    ui.textBoxTCPPort->setText(pSettings->value(IDS_TCPPORT, IDS_TCPPORT_VAL).toString());
    ui.textBoxErpAddress->setText(pSettings->value(IDS_ERPADDRESS, IDS_ERPADDRESS_VAL).toString());

    ui.checkBoxAutoFile->setChecked(pSettings->value(IDS_AUTOFILE, IDS_AUTOFILE_VAL).toBool());
    ui.checkBoxAutoShowFile->setChecked(pSettings->value(IDS_AUTOSHOWFILE, IDS_AUTOSHOWFILE_VAL).toBool());
    ui.radioButtonFileTop->setChecked(pSettings->value(IDS_FILETOP, IDS_FILETOP_VAL).toBool());
    ui.radioButtonFileBottom->setChecked(!pSettings->value(IDS_FILETOP, IDS_FILETOP_VAL).toBool());
    ui.textBoxFilePath->setText(StdLocation::getFileStoragePath());
    ui.checkBoxFolderForEach->setChecked(pSettings->value(IDS_STORAGEUSERFOLDER, IDS_STORAGEUSERFOLDER_VAL).toBool());

    QString themeName = pSettings->value(IDS_THEME, IDS_THEME_VAL).toString();
    ui.comboBoxChatTheme->setCurrentText(themeName);

    themeName = pSettings->value(IDS_APPTHEME, IDS_APPTHEME_VAL).toString();
    ui.comboBoxApplicationTheme->setCurrentText(themeName);

    themeName = pSettings->value(IDS_APPICONTHEME, IDS_APPICONTHEME_VAL).toString();
    ui.comboBoxIconTheme->setCurrentText(themeName);

    themeName = pSettings->value(IDS_BUTTONTHEME, IDS_BUTTONTHEME_VAL).toString();
    ui.comboBoxButtonTheme->setCurrentText(themeName);

    int userListView = pSettings->value(IDS_USERLISTVIEW, IDS_USERLISTVIEW_VAL).toInt();
    ui.comboBoxUserListView->setCurrentIndex(userListView);

    ui.radioButtonEnter->setChecked(!pSettings->value(IDS_SENDKEYMOD, IDS_SENDKEYMOD_VAL).toBool());
    ui.radioButtonCmdEnter->setChecked(pSettings->value(IDS_SENDKEYMOD, IDS_SENDKEYMOD_VAL).toBool());
}

void lmcSettingsDialog::saveSettings() {
    pSettings->setValue(IDS_VERSION, IDA_VERSION);
    pSettings->setValue(IDS_AUTOSTART, ui.checkBoxAutoStart->isChecked());
    pSettings->setValue(IDS_AUTOSHOW, ui.checkBoxAutoShow->isChecked());
    pSettings->setValue(IDS_SYSTRAY, ui.checkBoxSysTray->isChecked());
    pSettings->setValue(IDS_MINIMIZETRAY, ui.checkBoxMinimizeTray->isChecked());
    pSettings->setValue(IDS_SINGLECLICKTRAY, ui.checkBoxSingleClickTray->isChecked());
    pSettings->setValue(IDS_SYSTRAYMSG, ui.checkBoxSysTrayMsg->isChecked());
    pSettings->setValue(IDS_SYSTRAYNEWMSG, ui.checkBoxNewMessageNotif->isChecked());
    pSettings->setValue(IDS_SYSTRAYPUBNEWMSG, ui.checkBoxNewPublicMessageNotif->isChecked());
    pSettings->setValue(IDS_ALLOWSYSTRAYMIN, ui.checkBoxAllowSysTrayMin->isChecked());
    pSettings->setValue(IDS_RESTORESTATUS, ui.checkBoxRememberStatus->isChecked());
    QString langCode = ui.comboBoxLanguage->itemData(ui.comboBoxLanguage->currentIndex(), Qt::UserRole).toString();
    pSettings->setValue(IDS_LANGUAGE, langCode);

    pSettings->setValue(IDS_USERNAME, ui.textBoxUserName->text());
    pSettings->setValue(IDS_USERFIRSTNAME, ui.textBoxFirstName->text());
    pSettings->setValue(IDS_USERLASTNAME, ui.textBoxLastName->text());
    pSettings->setValue(IDS_USERABOUT, ui.textBoxAbout->toPlainText());
    pSettings->setValue(IDS_REFRESHTIME, ui.spinBoxRefreshTime->value());

    pSettings->setValue(IDS_MESSAGEPOP, ui.radioButtonMessageTop->isChecked());
    pSettings->setValue(IDS_PUBMESSAGEPOP, ui.checkBoxPublicMessagePop->isChecked());
    pSettings->setValue(IDS_EMOTICON, ui.checkBoxEmoticon->isChecked());
    pSettings->setValue(IDS_MESSAGETIME, ui.checkBoxMessageTime->isChecked());
    pSettings->setValue(IDS_MESSAGEDATE, ui.checkBoxMessageDate->isChecked());
    pSettings->setValue(IDS_ALLOWLINKS, ui.checkBoxAllowLinks->isChecked());
    pSettings->setValue(IDS_PATHTOLINK, ui.checkBoxPathToLink->isChecked());
    pSettings->setValue(IDS_TRIMMESSAGE, ui.checkBoxTrimMessage->isChecked());
    pSettings->setValue(IDS_APPENDHISTORY, ui.checkBoxAppendHistory->isChecked());
    pSettings->setValue(IDS_FONT, font.toString());
    pSettings->setValue(IDS_COLOR, color.name());
    pSettings->setValue(IDS_OVERRIDEINMSG, ui.checkBoxOverrideIncoming->isChecked());

    pSettings->setValue(IDS_HISTORY, ui.checkBoxHistory->isChecked());
    pSettings->setValue(IDS_SYSHISTORYPATH, ui.radioButtonSysHistoryPath->isChecked());
    pSettings->setValue(IDS_HISTORYPATH, ui.textBoxHistoryPath->text());
    pSettings->setValue(IDS_FILEHISTORY, ui.checkBoxFileHistory->isChecked());

    pSettings->setValue(IDS_ALERT, ui.checkBoxAlert->isChecked());
    pSettings->setValue(IDS_NOBUSYALERT, ui.checkBoxNoBusyAlert->isChecked());
    pSettings->setValue(IDS_NODNDALERT, ui.checkBoxNoDNDAlert->isChecked());
    pSettings->setValue(IDS_SOUND, ui.checkBoxSound->isChecked());
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

    pSettings->setValue(IDS_NOBUSYSOUND, ui.checkBoxNoBusySound->isChecked());
    pSettings->setValue(IDS_NODNDSOUND, ui.checkBoxNoDNDSound->isChecked());

    pSettings->setValue(IDS_TIMEOUT, ui.spinBoxTimeout->value());
    pSettings->setValue(IDS_MAXRETRIES, ui.spinBoxMaxRetries->value());
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
    pSettings->setValue(IDS_MULTICAST, ui.textBoxMulticast->text());
    pSettings->setValue(IDS_UDPPORT, ui.textBoxUDPPort->text());
    pSettings->setValue(IDS_TCPPORT, ui.textBoxTCPPort->text());
    pSettings->setValue(IDS_ERPADDRESS, ui.textBoxErpAddress->text());

    pSettings->setValue(IDS_AUTOFILE, ui.checkBoxAutoFile->isChecked());
    pSettings->setValue(IDS_AUTOSHOWFILE, ui.checkBoxAutoShowFile->isChecked());
    pSettings->setValue(IDS_FILETOP, ui.radioButtonFileTop->isChecked());
    pSettings->setValue(IDS_FILESTORAGEPATH, ui.textBoxFilePath->text());
    pSettings->setValue(IDS_STORAGEUSERFOLDER, ui.checkBoxFolderForEach->isChecked());

    pSettings->setValue(IDS_THEME, ui.comboBoxChatTheme->currentText ());
    pSettings->setValue(IDS_APPTHEME, ui.comboBoxApplicationTheme->currentText ());
    pSettings->setValue(IDS_APPICONTHEME, ui.comboBoxIconTheme->currentText ());
    pSettings->setValue(IDS_BUTTONTHEME, ui.comboBoxButtonTheme->currentText ());

    pSettings->setValue(IDS_USERLISTVIEW, ui.comboBoxUserListView->currentIndex());

    pSettings->setValue(IDS_SENDKEYMOD, ui.radioButtonCmdEnter->isChecked());

    pSettings->sync();
}
