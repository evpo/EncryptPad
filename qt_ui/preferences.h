#pragma once
#include <QFont>
#include <QSettings>
#include "packet_composer.h"

struct PersistentPreferences
{
    enum class ThemeAppearanceConfig
    {
        Auto = 0,
        Light = 1,
        Dark = 2,
    };

    EncryptPad::PacketMetadata defaultFileProperties;
    EncryptPad::PacketMetadata keyFileProperties;
    bool saveLastUsedDirectory;
    bool enableBakFiles;
    bool windowsEol;
    bool displayLineNumbers;
    bool wordWrap;
    bool enableFakeVim;
    int recentFiles;
    int s2kResultsPoolSize;
    QFont font;
    int tabSize;
    QString libCurlPath;
    QString libCurlParameters;
    int kfKeyLength;
    ThemeAppearanceConfig themeAppearance;
};

void SetDefaultPreferences(PersistentPreferences &preferences);
void ReadPreferences(QSettings &settings, PersistentPreferences &preferences);
void WritePreferences(QSettings &settings, PersistentPreferences &preferences);
void CopyMetadataPreferences(EncryptPad::PacketMetadata &source, EncryptPad::PacketMetadata &target);
