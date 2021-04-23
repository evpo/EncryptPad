#ifndef FILE_REQUEST_SERVICE_H
#define FILE_REQUEST_SERVICE_H

#include <QDialog>

struct FileRequestSelection
{
    bool cancelled;
    QString file_name;
    QString filter;
};

class FileRequestService
{
private:
    QString current_directory_;
    bool dont_use_native_dialog_ = false;

    const QString &GetValidDirectory();
    void SetDirectoryFromFile(const QString &path);
public:
    FileRequestService(){}
    FileRequestService(const FileRequestService&) = delete;
    const FileRequestService &operator=(const FileRequestService &) = delete;

    const QString &get_current_directory() const
    {
        return current_directory_;
    }

    void set_current_directory(const QString &dir)
    {
        current_directory_ = dir;
    }

    FileRequestSelection RequestExistingFile(QWidget *parent, const QString &title, const QString &file_name,
                                             const QString &filter, QString *selected_filter = nullptr);
    FileRequestSelection RequestNewFile(QWidget *parent, const QString &title, const QString &file_name,
                                        const QString &filter, QString *selected_filter = nullptr);

    bool GetDontUseNativeDialog() const
    {
        return dont_use_native_dialog_;
    }

    void SetDontUseNativeDialog(bool flag)
    {
        dont_use_native_dialog_ = flag;
    }

};

#endif // FILE_REQUEST_SERVICE_H
