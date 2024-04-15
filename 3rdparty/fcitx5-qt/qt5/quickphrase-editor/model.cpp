/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include "model.h"
#include "editor.h"
#include "filelistmodel.h"
#include <QApplication>
#include <QFile>
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpath.h>
#include <fcitx-utils/utf8.h>
#include <fcntl.h>

namespace fcitx {

namespace {

std::optional<std::pair<std::string, std::string>>
parseLine(const std::string &strBuf) {
    auto [start, end] = stringutils::trimInplace(strBuf);
    if (start == end) {
        return std::nullopt;
    }
    std::string_view text(strBuf.data() + start, end - start);
    if (!utf8::validate(text)) {
        return std::nullopt;
    }

    auto pos = text.find_first_of(FCITX_WHITESPACE);
    if (pos == std::string::npos) {
        return std::nullopt;
    }

    auto word = text.find_first_not_of(FCITX_WHITESPACE, pos);
    if (word == std::string::npos) {
        return std::nullopt;
    }

    std::string key(text.begin(), text.begin() + pos);
    auto wordString =
        stringutils::unescapeForValue(std::string_view(text).substr(word));
    if (!wordString) {
        return std::nullopt;
    }

    return std::make_pair(key, *wordString);
}

QString escapeValue(const QString &v) {
    return QString::fromStdString(stringutils::escapeForValue(v.toStdString()));
}

} // namespace

typedef QPair<QString, QString> ItemType;

QuickPhraseModel::QuickPhraseModel(QObject *parent)
    : QAbstractTableModel(parent), needSave_(false), futureWatcher_(0) {}

QuickPhraseModel::~QuickPhraseModel() {}

bool QuickPhraseModel::needSave() { return needSave_; }

QVariant QuickPhraseModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0)
            return _("Keyword");
        else if (section == 1)
            return _("Phrase");
    }
    return QVariant();
}

int QuickPhraseModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return list_.count();
}

int QuickPhraseModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 2;
}

QVariant QuickPhraseModel::data(const QModelIndex &index, int role) const {
    do {
        if ((role == Qt::DisplayRole || role == Qt::EditRole) &&
            index.row() < list_.count()) {
            if (index.column() == 0) {
                return list_[index.row()].first;
            } else if (index.column() == 1) {
                return list_[index.row()].second;
            }
        }
    } while (0);
    return QVariant();
}

void QuickPhraseModel::addItem(const QString &macro, const QString &word) {
    beginInsertRows(QModelIndex(), list_.size(), list_.size());
    list_.append(QPair<QString, QString>(macro, word));
    endInsertRows();
    setNeedSave(true);
}

void QuickPhraseModel::deleteItem(int row) {
    if (row >= list_.count())
        return;
    QPair<QString, QString> item = list_.at(row);
    QString key = item.first;
    beginRemoveRows(QModelIndex(), row, row);
    list_.removeAt(row);
    endRemoveRows();
    setNeedSave(true);
}

void QuickPhraseModel::deleteAllItem() {
    if (list_.count())
        setNeedSave(true);
    beginResetModel();
    list_.clear();
    endResetModel();
}

Qt::ItemFlags QuickPhraseModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return {};

    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool QuickPhraseModel::setData(const QModelIndex &index, const QVariant &value,
                               int role) {
    if (role != Qt::EditRole)
        return false;

    if (index.column() == 0) {
        list_[index.row()].first = value.toString();

        Q_EMIT dataChanged(index, index);
        setNeedSave(true);
        return true;
    } else if (index.column() == 1) {
        list_[index.row()].second = value.toString();

        Q_EMIT dataChanged(index, index);
        setNeedSave(true);
        return true;
    } else
        return false;
}

void QuickPhraseModel::load(const QString &file, bool append) {
    if (futureWatcher_) {
        return;
    }

    beginResetModel();
    if (!append) {
        list_.clear();
        setNeedSave(false);
    } else
        setNeedSave(true);
    futureWatcher_ = new QFutureWatcher<QStringPairList>(this);
    futureWatcher_->setFuture(
        QtConcurrent::run([this, file]() { return parse(file); }));
    connect(futureWatcher_, &QFutureWatcherBase::finished, this,
            &QuickPhraseModel::loadFinished);
}

QStringPairList QuickPhraseModel::parse(const QString &file) {
    QByteArray fileNameArray = file.toLocal8Bit();
    QStringPairList list;

    do {
        auto fp = fcitx::StandardPath::global().open(
            fcitx::StandardPath::Type::PkgData, fileNameArray.constData(),
            O_RDONLY);
        if (fp.fd() < 0)
            break;

        QFile file;
        if (!file.open(fp.fd(), QFile::ReadOnly)) {
            break;
        }
        QByteArray line;
        while (!(line = file.readLine()).isNull()) {
            auto l = line.toStdString();
            auto parsed = parseLine(l);
            if (!parsed)
                continue;
            auto [key, value] = *parsed;
            if (key.empty() || value.empty()) {
                continue;
            }
            list_.append(
                {QString::fromStdString(key), QString::fromStdString(value)});
        }

        file.close();
    } while (0);

    return list;
}

void QuickPhraseModel::loadFinished() {
    list_.append(futureWatcher_->future().result());
    endResetModel();
    futureWatcher_->deleteLater();
    futureWatcher_ = 0;
}

QFutureWatcher<bool> *QuickPhraseModel::save(const QString &file) {
    auto *futureWatcher = new QFutureWatcher<bool>(this);
    futureWatcher->setFuture(QtConcurrent::run(
        [this, file, list = list_]() { return saveData(file, list); }));
    connect(futureWatcher, &QFutureWatcherBase::finished, this,
            &QuickPhraseModel::saveFinished);
    return futureWatcher;
}

void QuickPhraseModel::saveDataToStream(QTextStream &dev) {
    for (int i = 0; i < list_.size(); i++) {
        dev << list_[i].first << "\t" << escapeValue(list_[i].second) << "\n";
    }
}

void QuickPhraseModel::loadData(QTextStream &stream) {
    beginResetModel();
    list_.clear();
    setNeedSave(true);
    QString s;
    while (!(s = stream.readLine()).isNull()) {
        auto line = s.toStdString();
        auto parsed = parseLine(line);
        if (!parsed)
            continue;
        auto [key, value] = *parsed;
        if (key.empty() || value.empty()) {
            continue;
        }
        list_.append(
            {QString::fromStdString(key), QString::fromStdString(value)});
    }
    endResetModel();
}

bool QuickPhraseModel::saveData(const QString &file,
                                const QStringPairList &list) {
    QByteArray filenameArray = file.toLocal8Bit();
    fs::makePath(stringutils::joinPath(
        StandardPath::global().userDirectory(StandardPath::Type::PkgData),
        QUICK_PHRASE_CONFIG_DIR));
    return StandardPath::global().safeSave(
        StandardPath::Type::PkgData, filenameArray.constData(),
        [&list](int fd) {
            QFile tempFile;
            if (!tempFile.open(fd, QIODevice::WriteOnly)) {
                return false;
            }
            for (int i = 0; i < list.size(); i++) {
                tempFile.write(list[i].first.toUtf8());
                tempFile.write("\t");
                tempFile.write(escapeValue(list[i].second).toUtf8());
                tempFile.write("\n");
            }
            tempFile.close();
            return true;
        });
}

void QuickPhraseModel::saveFinished() {
    QFutureWatcher<bool> *watcher =
        static_cast<QFutureWatcher<bool> *>(sender());
    QFuture<bool> future = watcher->future();
    if (future.result()) {
        setNeedSave(false);
    }
    watcher->deleteLater();
}

void QuickPhraseModel::setNeedSave(bool needSave) {
    if (needSave_ != needSave) {
        needSave_ = needSave;
        Q_EMIT needSaveChanged(needSave_);
    }
}
} // namespace fcitx
