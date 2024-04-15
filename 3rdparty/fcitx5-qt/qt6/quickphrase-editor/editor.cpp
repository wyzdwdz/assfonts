/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include "editor.h"
#include "batchdialog.h"
#include "editordialog.h"
#include "filelistmodel.h"
#include "model.h"
#include <QCloseEvent>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QtConcurrentRun>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpath.h>

namespace fcitx {

ListEditor::ListEditor(QWidget *parent)
    : FcitxQtConfigUIWidget(parent), model_(new QuickPhraseModel(this)),
      fileListModel_(new FileListModel(this)) {
    setupUi(this);

    macroTableView->setModel(model_);
    fileListComboBox->setModel(fileListModel_);

    operationMenu_ = new QMenu(this);
    operationMenu_->addAction(_("Add File"), this,
                              &ListEditor::addFileTriggered);
    operationMenu_->addAction(_("Remove File"), this,
                              &ListEditor::removeFileTriggered);
    operationMenu_->addAction(_("Refresh List"), this,
                              &ListEditor::refreshListTriggered);
    operationButton->setMenu(operationMenu_);

    loadFileList();
    itemFocusChanged();

    connect(addButton, &QPushButton::clicked, this, &ListEditor::addWord);
    connect(batchEditButton, &QPushButton::clicked, this,
            &ListEditor::batchEditWord);
    connect(deleteButton, &QPushButton::clicked, this, &ListEditor::deleteWord);
    connect(clearButton, &QPushButton::clicked, this,
            &ListEditor::deleteAllWord);
    connect(importButton, &QPushButton::clicked, this, &ListEditor::importData);
    connect(exportButton, &QPushButton::clicked, this, &ListEditor::exportData);

    connect(fileListComboBox, qOverload<int>(&QComboBox::activated), this,
            &ListEditor::changeFile);

    connect(macroTableView->selectionModel(),
            &QItemSelectionModel::selectionChanged, this,
            &ListEditor::itemFocusChanged);
    connect(model_, &QuickPhraseModel::needSaveChanged, this,
            &ListEditor::changed);
}

void ListEditor::load() {
    lastFile_ = currentFile();
    model_->load(currentFile(), false);
}

void ListEditor::load(const QString &file) { model_->load(file, true); }

void ListEditor::save(const QString &file) { model_->save(file); }

void ListEditor::save() {
    // QFutureWatcher< bool >* futureWatcher =
    // m_model->save("data/QuickPhrase.mb");
    QFutureWatcher<bool> *futureWatcher = model_->save(currentFile());
    connect(futureWatcher, &QFutureWatcherBase::finished, this,
            &ListEditor::saveFinished);
}

bool ListEditor::asyncSave() { return true; }

void ListEditor::changeFile(int) {
    if (model_->needSave()) {
        int ret = QMessageBox::question(
            this, _("Save Changes"),
            _("The content has changed.\n"
              "Do you want to save the changes or discard them?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            // save(fileListComboBox->itemText(lastFileIndex));
            save(lastFile_);
        } else if (ret == QMessageBox::Cancel) {
            fileListComboBox->setCurrentIndex(
                fileListModel_->findFile(lastFile_));
            return;
        }
    }
    load();
}

QString ListEditor::title() { return _("Quick Phrase Editor"); }

void ListEditor::itemFocusChanged() {
    deleteButton->setEnabled(macroTableView->currentIndex().isValid());
}

void ListEditor::deleteWord() {
    if (!macroTableView->currentIndex().isValid())
        return;
    int row = macroTableView->currentIndex().row();
    model_->deleteItem(row);
}

void ListEditor::deleteAllWord() { model_->deleteAllItem(); }

void ListEditor::addWord() {
    EditorDialog *dialog = new EditorDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->open();
    connect(dialog, &QDialog::accepted, this, &ListEditor::addWordAccepted);
}

void ListEditor::batchEditWord() {
    BatchDialog *dialog = new BatchDialog(this);
    QString text;
    QTextStream stream(&text);
    model_->saveDataToStream(stream);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->setText(text);
    dialog->open();
    connect(dialog, &QDialog::accepted, this, &ListEditor::batchEditAccepted);
}

void ListEditor::addWordAccepted() {
    const EditorDialog *dialog =
        qobject_cast<const EditorDialog *>(QObject::sender());

    model_->addItem(dialog->key(), dialog->value());
    QModelIndex last = model_->index(model_->rowCount() - 1, 0);
    macroTableView->setCurrentIndex(last);
    macroTableView->scrollTo(last);
}

void ListEditor::batchEditAccepted() {
    const BatchDialog *dialog =
        qobject_cast<const BatchDialog *>(QObject::sender());

    QString s = dialog->text();
    QTextStream stream(&s);

    model_->loadData(stream);
    QModelIndex last = model_->index(model_->rowCount() - 1, 0);
    macroTableView->setCurrentIndex(last);
    macroTableView->scrollTo(last);
}

void ListEditor::importData() {
    QFileDialog *dialog = new QFileDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->setFileMode(QFileDialog::ExistingFile);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->open();
    connect(dialog, &QDialog::accepted, this, &ListEditor::importFileSelected);
}

void ListEditor::exportData() {
    QFileDialog *dialog = new QFileDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->open();
    connect(dialog, &QDialog::accepted, this, &ListEditor::exportFileSelected);
}

void ListEditor::importFileSelected() {
    const QFileDialog *dialog =
        qobject_cast<const QFileDialog *>(QObject::sender());
    if (dialog->selectedFiles().length() <= 0)
        return;
    QString file = dialog->selectedFiles()[0];
    load(file);
}

void ListEditor::exportFileSelected() {
    const QFileDialog *dialog =
        qobject_cast<const QFileDialog *>(QObject::sender());
    if (dialog->selectedFiles().length() <= 0)
        return;
    QString file = dialog->selectedFiles()[0];
    save(file);
}

void ListEditor::loadFileList() {
    int row = fileListComboBox->currentIndex();
    int col = fileListComboBox->modelColumn();
    QString lastFileName =
        fileListModel_->data(fileListModel_->index(row, col), Qt::UserRole)
            .toString();
    fileListModel_->loadFileList();
    fileListComboBox->setCurrentIndex(fileListModel_->findFile(lastFileName));
    load();
}

QString ListEditor::currentFile() {
    int row = fileListComboBox->currentIndex();
    int col = fileListComboBox->modelColumn();
    return fileListModel_->data(fileListModel_->index(row, col), Qt::UserRole)
        .toString();
}

QString ListEditor::currentName() {
    int row = fileListComboBox->currentIndex();
    int col = fileListComboBox->modelColumn();
    return fileListModel_
        ->data(fileListModel_->index(row, col), Qt::DisplayRole)
        .toString();
}

void ListEditor::addFileTriggered() {
    bool ok;
    QString filename = QInputDialog::getText(
        this, _("Create new file"), _("Please input a filename for newfile"),
        QLineEdit::Normal, "newfile", &ok);

    if (filename.contains('/')) {
        QMessageBox::warning(this, _("Invalid filename"),
                             _("File name should not contain '/'."));
        return;
    }

    filename.append(".mb");
    if (!StandardPath::global().safeSave(
            StandardPath::Type::PkgData,
            stringutils::joinPath(QUICK_PHRASE_CONFIG_DIR,
                                  filename.toLocal8Bit().constData()),
            [](int) { return true; })) {
        QMessageBox::warning(
            this, _("File Operation Failed"),
            QString(_("Cannot create file %1.")).arg(filename));
        return;
    }

    fileListModel_->loadFileList();
    fileListComboBox->setCurrentIndex(fileListModel_->findFile(
        filename.prepend(QUICK_PHRASE_CONFIG_DIR "/")));
    load();
}

void ListEditor::refreshListTriggered() { loadFileList(); }

void ListEditor::removeFileTriggered() {
    QString filename = currentFile();
    QString curName = currentName();
    auto fullname = stringutils::joinPath(
        StandardPath::global().userDirectory(StandardPath::Type::PkgData),
        filename.toLocal8Bit().constData());
    QFile f(fullname.data());
    if (!f.exists()) {
        int ret = QMessageBox::question(
            this, _("Cannot remove system file"),
            QString(_("%1 is a system file, do you want to delete all phrases "
                      "instead?"))
                .arg(curName),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (ret == QMessageBox::Yes) {
            deleteAllWord();
        }
        return;
    }

    int ret = QMessageBox::question(
        this, _("Confirm deletion"),
        QString(_("Are you sure to delete %1?")).arg(curName),
        QMessageBox::Ok | QMessageBox::Cancel);

    if (ret == QMessageBox::Ok) {
        bool ok = f.remove();
        if (!ok) {
            QMessageBox::warning(
                this, _("File Operation Failed"),
                QString(_("Error while deleting %1.")).arg(curName));
        }
    }
    loadFileList();
    load();
}
} // namespace fcitx
