#include "file_request_service.h"
#include "file_name_helper.h"
#include <QFileDialog>
#include "plog/Log.h"


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

    if(GetDontUseNativeDialog())
        dlg.setOption(QFileDialog::DontUseNativeDialog);

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

    QFileDialog dlg(parent,
                    title,
                    !file_name.isEmpty() ? file_name : GetValidDirectory(),
                    filter);
    dlg.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
    dlg.setFileMode(QFileDialog::AnyFile);
    if(!ret_val.filter.isEmpty())
        dlg.selectNameFilter(ret_val.filter);

    if(GetDontUseNativeDialog())
        dlg.setOption(QFileDialog::DontUseNativeDialog);

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
