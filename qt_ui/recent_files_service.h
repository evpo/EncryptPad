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
    QSignalMapper signal_mapper_;
    QWidget *parent_;
    QMenu *menu_;
    int begin_index_;
    QAction *end_;
    int max_files_;
    int file_count_;

    QAction *CreateAction(const QString &name, const QString &path)
    {
        QAction *new_action = new QAction(name, parent_);
        new_action->setData(QVariant(path));
        new_action->setToolTip(path);
        new_action->setStatusTip(path);
        signal_mapper_.setMapping(new_action, path);
        this->connect(new_action, SIGNAL(triggered()), &signal_mapper_, SLOT(map()));

        return new_action;
    }

    void RemoveAction(QAction *action)
    {
        menu_->removeAction(action);
        signal_mapper_.removeMappings(action);
        action->deleteLater();
    }
signals:
    void FileSelected(const QString &path);
public slots:
    void SignalMapped(const QString &str);
public:
    RecentFilesService(QWidget *parent);
    virtual ~RecentFilesService(){}

    // begin - separator before recent files, end - next menu item after recent files separator
    void Init(QMenu *menu, QAction *begin, QAction *end)
    {
        menu_ = menu;
        begin_index_ = menu->actions().indexOf(begin);
        end_ = end;
    }

    void Serialize(QStringList &list)
    {
        for(int i = begin_index_ + 1; i < begin_index_ + 1 + file_count_; i++)
        {
            QAction *action = menu_->actions().at(i);
            list.append(action->text());
            list.append(action->data().toString());
        }
    }


    void Deserialize(const QStringList &list, int max_files)
    {
        max_files_ = max_files;
        auto it = list.constBegin();
        while(it != list.constEnd() && file_count_ < max_files)
        {
            QString name = *it;
            it ++;
            QString path = *it;
            menu_->insertAction(end_, CreateAction(name, path));
            file_count_++;
            it ++;
        }

        if(file_count_ > 0)
            menu_->insertSeparator(end_);
    }

    void PushFile(const QString &file_path)
    {
        if(max_files_ == 0)
            return;

        QFileInfo info(file_path);
        QString name = info.fileName();
        QString path = info.absoluteFilePath();

        QAction *a = nullptr;

        // remove last separator if exists
        if(file_count_ > 0)
        {
            a = menu_->actions().at(menu_->actions().indexOf(end_) - 1);
            RemoveAction(a);
        }

        int i = begin_index_ + 1; // first recent file if exists
        a = menu_->actions().at(i);
        menu_->insertAction(a, CreateAction(name, path));
        file_count_ ++;
        while(a != end_ && a->data().toString() != path)
        {
            i ++;
            a = menu_->actions().at(i);
        }

        if(a != end_)
        {
            RemoveAction(a);
            file_count_ --;
        }

        menu_->insertSeparator(end_);

        SetMaxFiles(max_files_);
    }

    void SetMaxFiles(int max_files)
    {
        max_files_ = max_files;
        if(file_count_ <= max_files_)
            return;

        int index2remove = menu_->actions().indexOf(end_) - 1;
        // remove last separator
        QAction *a = menu_->actions().at(index2remove);
        RemoveAction(a);
        index2remove --; // move index to the last recent file

        // Remove recent files from bottom to top
        while(file_count_ != max_files_)
        {
            QAction *a = menu_->actions().at(index2remove);
            RemoveAction(a);
            file_count_ --;
            index2remove --;
        }

        // return the separator
        if(max_files_ != 0)
            menu_->insertSeparator(end_);
    }

    int GetMaxFiles() const
    {
        return max_files_;
    }
};

#endif // RECENT_FILES_SERVICE_H
