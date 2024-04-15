/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#ifndef _QUICKPHRASE_EDITOR_EDITOR_H_
#define _QUICKPHRASE_EDITOR_EDITOR_H_

#include "fcitxqtconfiguiwidget.h"
#include "model.h"
#include "ui_editor.h"
#include <QDir>
#include <QMainWindow>
#include <QMutex>

class QAbstractItemModel;

namespace fcitx {

class FileListModel;

class ListEditor final : public FcitxQtConfigUIWidget, Ui::Editor {
    Q_OBJECT
public:
    explicit ListEditor(QWidget *parent = 0);

    void load() override;
    void save() override;
    QString title() override;
    bool asyncSave() override;

    void loadFileList();

public Q_SLOTS:
    void batchEditAccepted();
    void removeFileTriggered();
    void addFileTriggered();
    void refreshListTriggered();
    void changeFile(int);

private Q_SLOTS:
    void addWord();
    void batchEditWord();
    void deleteWord();
    void deleteAllWord();
    void itemFocusChanged();
    void addWordAccepted();
    void importData();
    void exportData();
    void importFileSelected();
    void exportFileSelected();

private:
    void load(const QString &file);
    void save(const QString &file);
    QString currentFile();
    QString currentName();
    QuickPhraseModel *model_;
    FileListModel *fileListModel_;
    QMenu *operationMenu_;
    QString lastFile_;
};
} // namespace fcitx

#endif // _QUICKPHRASE_EDITOR_EDITOR_H_
