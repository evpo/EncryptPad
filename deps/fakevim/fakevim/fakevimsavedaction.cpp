#include "fakevimsavedaction.h"

namespace Utils
{

SavedAction::SavedAction(void *)
{
}

void SavedAction::setValue(const QVariant &value)
{
    m_value = value;
    valueChanged(value);
}

QVariant SavedAction::value() const
{
    return m_value;
}

void SavedAction::setDefaultValue(const QVariant &value)
{
    m_defaultValue = value;
}

QVariant SavedAction::defaultValue() const
{
    return m_defaultValue;
}
void SavedAction::setSettingsKey(const QString &group, const QString &key)
{
    m_settingsGroup = group;
    m_settingsKey = key;
}

QString SavedAction::settingsKey() const
{
    return m_settingsKey;
}

}
