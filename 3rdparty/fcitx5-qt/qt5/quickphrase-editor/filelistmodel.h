/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#ifndef _QUICKPHRASE_EDITOR_FILELISTMODEL_H_
#define _QUICKPHRASE_EDITOR_FILELISTMODEL_H_

#include <QAbstractListModel>
#include <QStringList>

#define QUICK_PHRASE_CONFIG_DIR "data/quickphrase.d"
#define QUICK_PHRASE_CONFIG_FILE "data/QuickPhrase.mb"

namespace fcitx {

class FileListModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit FileListModel(QObject *parent = 0);
    virtual ~FileListModel();

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void loadFileList();
    int findFile(const QString &lastFileName);

private:
    QStringList fileList_;
};
} // namespace fcitx

#endif // _QUICKPHRASE_EDITOR_FILELISTMODEL_H_
