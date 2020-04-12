//**********************************************************************************
//EncryptPad Copyright 2016 Evgeny Pokhilko
//<http://www.evpo.net/encryptpad>
//
//This file is part of EncryptPad
//
//EncryptPad is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 2 of the License, or
//(at your option) any later version.
//
//EncryptPad is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with EncryptPad.  If not, see <http://www.gnu.org/licenses/>.
//**********************************************************************************
#ifndef RECENT_FILES_SERVICE_H
#define RECENT_FILES_SERVICE_H

#include "assert.h"
#include <QMenu>
#include <QAction>
#include <QStringList>
#include <QFileInfo>
#include <QWidget>
#include <QObject>
#include <QDebug>
#include <QSignalMapper>

class RecentFilesService
        : public QObject
{
    Q_OBJECT
private:
    QWidget *parent_;
    QMenu *menu_;
    int begin_index_;
    QAction *end_;
    int max_files_;
    int file_count_;

    QAction *CreateAction(const QString &name, const QString &path);
    void RemoveAction(QAction *action);
signals:
    void FileSelected(const QString &path);
public:
    RecentFilesService(QWidget *parent);
    virtual ~RecentFilesService();

    // begin - separator before recent files, end - next menu item after recent files separator
    void Init(QMenu *menu, QAction *begin, QAction *end);
    void Serialize(QStringList &list);
    void Deserialize(const QStringList &list, int max_files);
    void PushFile(const QString &file_path);
    void SetMaxFiles(int max_files);
    int GetMaxFiles() const;
    void TriggerFileSelected(QString path);
};

#endif // RECENT_FILES_SERVICE_H
