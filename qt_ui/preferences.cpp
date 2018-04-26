#include "preferences.h"
#include <QFontDatabase>
#include "encryptmsg/openpgp_conversions.h"
#include "encryptmsg/algo_spec.h"

namespace
{
    const int kDefaultRecentFiles = 5;
    const bool kDefaultSaveLastUsedDirectory = true;
    const bool kDefaultenableBakFiles = true;
    const bool kDefaultWordWrap = true;
    const int kDefaultS2KResultsPoolSize = 8;
    const int kDefaultTabSize = 4;
    const EncryptMsg::Compression kDefaultKeyFileCompression
        = EncryptMsg::Compression::Uncompressed;
#if defined(__MINGW__) || defined(__MINGW32__)
    const bool kDefaultWindowsEol = true;
#else
    const bool kDefaultWindowsEol = false;
#endif

    enum class MapperDirection
    {
        Read,
        Write
    };

    class SettingMapper
    {
        private:
            QSettings &settings_;
            MapperDirection direction_;
        public:
            SettingMapper(QSettings &settings, MapperDirection direction):
                settings_(settings), direction_(direction)
            {
            }

        private:

        void Assign(bool &value, const QVariant &setting)
        {
            value = setting.toBool();
        }

        void Assign(int &value, const QVariant &setting)
        {
            value = setting.toInt();
        }

        void Assign(QString &value, const QVariant &setting)
        {
            value = setting.toString();
        }

        template<class T>
            void Assign(T &algo, const QVariant &setting)
            {
                algo = static_cast<T>(setting.toInt());
            }

        template<class T>
            void ReadSetting(const QString &key, T& value)
            {
                if(!settings_.contains(key))
                    return;

                Assign(value, settings_.value(key));
            }

        template<class T>
            QVariant ToVariant(const T& value)
            {
                return QVariant(value);
            }

        QVariant ToVariant(EncryptMsg::Compression &value)
        {
            return QVariant(static_cast<int>(value));
        }

        QVariant ToVariant(EncryptMsg::CipherAlgo &value)
        {
            return QVariant(static_cast<int>(value));
        }

        QVariant ToVariant(EncryptMsg::HashAlgo &value)
        {
            return QVariant(static_cast<int>(value));
        }

        template<class T>
            void WriteSetting(const QString &key, T& value)
            {
                settings_.setValue(key, ToVariant(value));
            }
        public:
        template<class T>
        void Map(const QString &key, T &value)
        {
            if(direction_ == MapperDirection::Read)
            {
                ReadSetting(key, value);
            }
            else
            {
                WriteSetting(key, value);
            }
        }
    };

    void Map(SettingMapper &mapper, PersistentPreferences &preferences)
    {
        mapper.Map("recent_files", preferences.recentFiles);
        mapper.Map("enable_bak_files", preferences.enableBakFiles);
        mapper.Map("save_last_used_directory", preferences.saveLastUsedDirectory);
        mapper.Map("word_wrap", preferences.wordWrap);
        mapper.Map("libcurl_path", preferences.libCurlPath);
        mapper.Map("libcurl_parameters", preferences.libCurlParameters);
        mapper.Map("s2k_results_pool_size", preferences.s2kResultsPoolSize);

        mapper.Map("default_cipher_algo", preferences.defaultFileProperties.cipher_algo);
        mapper.Map("default_hash_algo", preferences.defaultFileProperties.hash_algo);
        mapper.Map("default_compression", preferences.defaultFileProperties.compression);
        mapper.Map("default_iterations", preferences.defaultFileProperties.iterations);

        mapper.Map("key_file_cipher_algo", preferences.keyFileProperties.cipher_algo);
        mapper.Map("key_file_hash_algo", preferences.keyFileProperties.hash_algo);
        mapper.Map("key_file_compression", preferences.keyFileProperties.compression);
        mapper.Map("key_file_iterations", preferences.keyFileProperties.iterations);
        mapper.Map("windows_eol", preferences.windowsEol);
        mapper.Map("key_file_key_length", preferences.kfKeyLength);
        mapper.Map("tab_size", preferences.tabSize);
    }
}

void SetDefaultPreferences(PersistentPreferences &preferences)
{
    preferences.recentFiles = kDefaultRecentFiles;
    preferences.saveLastUsedDirectory = kDefaultSaveLastUsedDirectory;
    preferences.enableBakFiles = kDefaultenableBakFiles;
    preferences.windowsEol = kDefaultWindowsEol;
    preferences.s2kResultsPoolSize = kDefaultS2KResultsPoolSize;
    preferences.wordWrap = kDefaultWordWrap;
    preferences.kfKeyLength = EncryptPad::kDefaultKeyFileKeyLength;

    preferences.defaultFileProperties.cipher_algo = EncryptPad::kDefaultCipherAlgo;
    preferences.defaultFileProperties.hash_algo = EncryptPad::kDefaultHashAlgo;
    preferences.defaultFileProperties.compression = EncryptPad::kDefaultCompression;
    preferences.defaultFileProperties.iterations = EncryptPad::kDefaultIterations;

    preferences.keyFileProperties.cipher_algo = EncryptPad::kDefaultCipherAlgo;
    preferences.keyFileProperties.hash_algo = EncryptPad::kDefaultHashAlgo;
    preferences.keyFileProperties.compression = kDefaultKeyFileCompression;;
    preferences.keyFileProperties.iterations = EncryptPad::kDefaultIterations;

    preferences.font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    preferences.tabSize = kDefaultTabSize;
}


void ReadPreferences(QSettings &settings, PersistentPreferences &preferences)
{
    using namespace EncryptMsg;
    if(settings.contains("font"))
    {
        QFont font;
        font.fromString(settings.value("font").toString());
        preferences.font = font;
    }

    SettingMapper mapper(settings, MapperDirection::Read);
    Map(mapper, preferences);

    preferences.defaultFileProperties.iterations = DecodeS2KIterations(EncodeS2KIterations(
                preferences.defaultFileProperties.iterations));
    preferences.keyFileProperties.iterations = DecodeS2KIterations(EncodeS2KIterations(
                preferences.keyFileProperties.iterations));
}

void WritePreferences(QSettings &settings, PersistentPreferences &preferences)
{
    settings.setValue("font", QVariant(preferences.font.toString()));
    SettingMapper mapper(settings, MapperDirection::Write);
    Map(mapper, preferences);
}

void CopyMetadataPreferences(EncryptPad::PacketMetadata &source, EncryptPad::PacketMetadata &target)
{
    target.cipher_algo = source.cipher_algo;
    target.hash_algo = source.hash_algo;
    target.iterations = source.iterations;
    target.compression = source.compression;
}
