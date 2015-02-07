#ifndef GLOBALS_H
#define GLOBALS_H

#include "vector"

#include <QString>
#include <QCoreApplication>

enum StatusTypeEnum {
    StatusOnline = 0,
    StatusBusy,
    StatusAway,
    StatusOffline
};

struct StatusStruct {
    QString icon;
    QString description;
    QString uiDescription;
    StatusTypeEnum statusType;

    StatusStruct(const QString &icon, const QString &description, const QString &uiDescription, StatusTypeEnum statusType) : icon(icon), description(description), uiDescription(uiDescription), statusType(statusType) {}
    StatusStruct() {}
};

class Globals
{    
    Q_DECLARE_TR_FUNCTIONS(Globals)

    std::vector<StatusStruct> _statuses = { StatusStruct("online", "Available", tr("Available"), StatusTypeEnum::StatusOnline), StatusStruct("busy", "Busy", tr("Busy"), StatusTypeEnum::StatusBusy), StatusStruct("nodisturb", "Do Not Disturb", tr("Do Not Disturb"), StatusTypeEnum::StatusBusy), StatusStruct("away", "Be Right Back", tr("Be Right Back"), StatusTypeEnum::StatusAway), StatusStruct("away", "Away", tr("Away"), StatusTypeEnum::StatusAway), StatusStruct("offline", "Appear Offline", tr("Appear Offline"), StatusTypeEnum::StatusOffline) };

public:

    Globals();
    ~Globals();

    static Globals &getInstance() {
        static Globals globals;
        return globals;
    }

    std::vector<StatusStruct> &getStatuses() { return _statuses; }
    StatusStruct *getStatus(const QString &description, int *index = nullptr);
    StatusTypeEnum getStatusType(const QString &description);
    bool statusExists(const QString &description);
};

#endif // GLOBALS_H
