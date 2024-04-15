/*
 * SPDX-FileCopyrightText: 2013~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "filelistmodel.h"
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpath.h>
#include <fcntl.h>

fcitx::FileListModel::FileListModel(QObject *parent)
    : QAbstractListModel(parent) {}

fcitx::FileListModel::~FileListModel() {}

int fcitx::FileListModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : fileList_.size();
}

QVariant fcitx::FileListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= fileList_.size())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        if (fileList_[index.row()] == QUICK_PHRASE_CONFIG_FILE) {
            return _("Default");
        } else {
            // remove "data/quickphrase.d/"
            const size_t length = strlen(QUICK_PHRASE_CONFIG_DIR);
            return fileList_[index.row()].mid(length + 1,
                                              fileList_[index.row()].size() -
                                                  length - strlen(".mb") - 1);
        }
    case Qt::UserRole:
        return fileList_[index.row()];
    default:
        break;
    }
    return QVariant();
}

void fcitx::FileListModel::loadFileList() {
    beginResetModel();
    fileList_.clear();
    fileList_.append(QUICK_PHRASE_CONFIG_FILE);
    auto files = StandardPath::global().multiOpen(
        StandardPath::Type::PkgData, QUICK_PHRASE_CONFIG_DIR, O_RDONLY,
        filter::Suffix(".mb"));

    for (auto &file : files) {
        fileList_.append(QString::fromLocal8Bit(
            stringutils::joinPath(QUICK_PHRASE_CONFIG_DIR, file.first).data()));
    }

    endResetModel();
}

int fcitx::FileListModel::findFile(const QString &lastFileName) {
    int idx = fileList_.indexOf(lastFileName);
    if (idx < 0) {
        return 0;
    }
    return idx;
}
