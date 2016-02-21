#include "file_request_service.h"
#include "file_name_helper.h"
#include <QFileDialog>


const QString &FileRequestService::GetValidDirectory()
{
    QDir dir(current_directory_);
    if(!dir.exists())
        current_directory_.clear();

    return current_directory_;
}

void FileRequestService::SetDirectoryFromFile(const QString &path)
{
    QFileInfo file_info(path);
    if(file_info.isRelative())
        return;

    current_directory_ = file_info.dir().absolutePath();
}

FileRequestSelection FileRequestService::RequestExistingFile(QWidget *parent, const QString &title, const QString &file_name,
                                                             const QString &filter, QString *selected_filter)
{
    FileRequestSelection ret_val;
    ret_val.cancelled = true;

    QFileDialog dlg(parent,
                    title,
                    !file_name.isEmpty() ? file_name : GetValidDirectory(),
                    filter);

    if(selected_filter)
        dlg.setNameFilter(*selected_filter);

    dlg.setFileMode(QFileDialog::ExistingFile);
    if(dlg.exec() && dlg.selectedFiles().size() > 0)
    {
        ret_val.cancelled = false;
        ret_val.file_name = dlg.selectedFiles().first();
        ret_val.filter = dlg.selectedNameFilter();

        ret_val.file_name = AppendExtensionForFileDialog(ret_val.file_name, ret_val.filter);
        SetDirectoryFromFile(ret_val.file_name);
    }

    return ret_val;
}

FileRequestSelection FileRequestService::RequestNewFile(QWidget *parent, const QString &title, const QString &file_name, const QString &filter, QString *selected_filter)
{
    FileRequestSelection ret_val;
    ret_val.cancelled = true;
    if(selected_filter)
        ret_val.filter = *selected_filter;

    ret_val.file_name = QFileDialog::getSaveFileName(parent, title,
                                            !file_name.isEmpty() ? file_name : GetValidDirectory(),
                                            filter, &ret_val.filter);

    ret_val.cancelled = ret_val.file_name.isNull();
    if(!ret_val.cancelled)
    {
        ret_val.file_name = AppendExtensionForFileDialog(ret_val.file_name, ret_val.filter);
        SetDirectoryFromFile(ret_val.file_name);
    }
    return ret_val;
}
