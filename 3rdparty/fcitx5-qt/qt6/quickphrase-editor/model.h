/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#ifndef _QUICKPHRASE_EDITOR_MODEL_H_
#define _QUICKPHRASE_EDITOR_MODEL_H_

#include <QAbstractTableModel>
#include <QFutureWatcher>
#include <QSet>
#include <QTextStream>

class QFile;
namespace fcitx {

typedef QList<QPair<QString, QString>> QStringPairList;

class QuickPhraseModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit QuickPhraseModel(QObject *parent = 0);
    virtual ~QuickPhraseModel();

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    void load(const QString &file, bool append);
    void loadData(QTextStream &stream);
    void addItem(const QString &macro, const QString &word);
    void deleteItem(int row);
    void deleteAllItem();
    QFutureWatcher<bool> *save(const QString &file);
    void saveDataToStream(QTextStream &dev);
    bool needSave();

Q_SIGNALS:
    void needSaveChanged(bool needSave);

private Q_SLOTS:
    void loadFinished();
    void saveFinished();

private:
    QStringPairList parse(const QString &file);
    bool saveData(const QString &file, const fcitx::QStringPairList &list);
    void setNeedSave(bool needSave);
    bool needSave_;
    QStringPairList list_;
    QFutureWatcher<QStringPairList> *futureWatcher_;
};
} // namespace fcitx

#endif // _QUICKPHRASE_EDITOR_MODEL_H_
