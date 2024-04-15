/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include "main.h"
#include "editor.h"
#include "model.h"

QuickPhraseEditorPlugin::QuickPhraseEditorPlugin(QObject *parent)
    : fcitx::FcitxQtConfigUIPlugin(parent) {}

fcitx::FcitxQtConfigUIWidget *
QuickPhraseEditorPlugin::create(const QString &key) {
    Q_UNUSED(key);
    return new fcitx::ListEditor;
}
