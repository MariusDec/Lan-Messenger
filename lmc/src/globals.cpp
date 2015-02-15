#include "globals.h"

Globals::Globals()
{

}

Globals::~Globals()
{

}

StatusStruct *Globals::getStatus(const QString &description, int *index)
{
    StatusStruct *currentStatus = nullptr;
    int tempIndex = 0;

    for (StatusStruct &statusItem : _statuses) {
        if (!statusItem.description.compare (description)) {
            currentStatus = &statusItem;
            if (index)
                *index = tempIndex;
            break;
        }
        ++tempIndex;
    }
    if (!currentStatus && index)
        *index = -1;

    return currentStatus;
}

StatusTypeEnum Globals::getStatusType(const QString &description)
{
    StatusStruct *currrentStatus = getStatus (description);
    if (!currrentStatus)
        return StatusTypeEnum::StatusOnline;

    return currrentStatus->statusType;
}

bool Globals::statusExists(const QString &description)
{
    for (const StatusStruct &statusItem : _statuses)
        if (statusItem.description.compare (description))
            return true;

    return false;
}

