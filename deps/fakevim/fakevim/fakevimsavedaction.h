#pragma once

#include <QVariant>
#include <QSettings>
#include "fakevimhandler.h"

namespace Utils
{

class SavedAction
{
public:
    SavedAction(void *parent);
    void setValue(const QVariant &value);
    QVariant value() const;
    void setDefaultValue(const QVariant &value);
    QVariant defaultValue() const;
    void setSettingsKey(const QString &group, const QString &key);
    QString settingsKey() const;
    void setCheckable(bool) {}

    void readSettings(QSettings *) {}
    void writeSettings(QSettings *) {}

    FakeVim::Internal::Signal<void(const QVariant&)> valueChanged;
private:

    QVariant m_value;
    QVariant m_defaultValue;
    QString m_settingsGroup;
    QString m_settingsKey;
};

}
