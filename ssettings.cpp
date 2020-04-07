#include <ssettings.h>

bool SSection::isConfirmation(const SSection &sect)
{
    for (auto field : sect.fields) {
        if (field.contains("<") && field.contains(">"))
            return true;
    }
    return false;
}

QList<QString> SSection::getErrorFields(const SSection &sect)
{
    QList<QString> res;
    for (auto it = sect.fields.begin(); it != sect.fields.end(); it++) {
        bool empty = it.value().isEmpty();
        bool confirmed = it.value().contains(SECTIONVAL_CONFIRMED);
        bool serialName = it.key() == SERIAL_IFACE;
        if (!empty && !confirmed && !serialName)
            res.push_back(it.key());
    }
    return res;
}

bool SSection::isEmpty(const SSection &sect)
{
    for (auto field : sect.fields) {
        if (!field.isEmpty())
            return false;
    }
    return true;
}
