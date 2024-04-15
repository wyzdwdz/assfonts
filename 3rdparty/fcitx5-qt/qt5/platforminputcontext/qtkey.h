/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _PLATFORMINPUTCONTEXT_QTKEY_H_
#define _PLATFORMINPUTCONTEXT_QTKEY_H_

#include <QString>
#include <cstdint>

int keysymToQtKey(uint32_t keysym, const QString &text);

#endif // _PLATFORMINPUTCONTEXT_QTKEY_H_
