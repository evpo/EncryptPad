#pragma once
#include <QFont>
#include <QSettings>
#include "packet_composer.h"

struct PersistentPreferences
{
    EncryptPad::PacketMetadata defaultFileProperties;
    EncryptPad::PacketMetadata keyFileProperties;
    bool saveLastUsedDirectory;
    bool enableBakFiles;
    bool windowsEol;
    bool wordWrap;
    int recentFiles;
    int s2kResultsPoolSize;
    QFont font;
    QString libCurlPath;
    QString libCurlParameters;
    int kfKeyLength;
};

void SetDefaultPreferences(PersistentPreferences &preferences);
void ReadPreferences(QSettings &settings, PersistentPreferences &preferences);
void WritePreferences(QSettings &settings, PersistentPreferences &preferences);
