/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#ifndef _QUICKPHRASE_EDITOR_MAIN_H_
#define _QUICKPHRASE_EDITOR_MAIN_H_

#include "fcitxqtconfiguiplugin.h"

class QuickPhraseEditorPlugin : public fcitx::FcitxQtConfigUIPlugin {
    Q_OBJECT
public:
    Q_PLUGIN_METADATA(IID FcitxQtConfigUIFactoryInterface_iid FILE
                      "quickphrase-editor.json")
    explicit QuickPhraseEditorPlugin(QObject *parent = 0);
    fcitx::FcitxQtConfigUIWidget *create(const QString &key) override;
};

#endif // _QUICKPHRASE_EDITOR_MAIN_H_
