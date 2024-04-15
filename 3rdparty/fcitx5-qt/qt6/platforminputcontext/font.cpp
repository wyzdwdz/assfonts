/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "font.h"
#include <QMap>

QFont fcitx::parseFont(const QString &string) {
#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    auto list = string.split(" ", QString::SkipEmptyParts);
#else
    auto list = string.split(" ", Qt::SkipEmptyParts);
#endif
    int size = 9; // Default size.
    if (!list.empty()) {
        bool ok = false;
        auto fontSize = list.back().toInt(&ok);
        if (ok) {
            if (fontSize > 0) {
                size = fontSize;
            }
            list.pop_back();
        }
    }

    QFont::Style style = QFont::StyleNormal;
    QFont::Weight weight = QFont::Normal;
    const QMap<QString, QFont::Weight> strToWeight = {
        {"Thin", QFont::Thin},
        {"Ultra-Light", QFont::Thin},
        {"Extra-Light", QFont::ExtraLight},
        {"Light", QFont::Light},
        {"Semi-Light", QFont::Light},
        {"Demi-Light", QFont::Light},
        {"Book", QFont::Light},
        {"Regular", QFont::Normal},
        {"Medium", QFont::Medium},
        {"Semi-Bold", QFont::Medium},
        {"Demi-Bold", QFont::DemiBold},
        {"Bold", QFont::Bold},
        {"Ultra-Bold", QFont::Bold},
        {"Extra-Bold", QFont::ExtraBold},
        {"Black", QFont::Black},
        {"Ultra-Black", QFont::Black},
        {"Extra-Black", QFont::Black},
    };
    const QMap<QString, QFont::Style> strToStyle = {
        {"Italic", QFont::StyleItalic}, {"Oblique", QFont::StyleOblique}};
    while (!list.empty()) {
        if (strToWeight.contains(list.back())) {
            weight = strToWeight.value(list.back(), QFont::Normal);
            list.pop_back();
        } else if (strToStyle.contains(list.back())) {
            style = strToStyle.value(list.back(), QFont::StyleNormal);
            list.pop_back();
        } else {
            break;
        }
    }
    QString family = list.join(" ");
    QFont font;
    font.setFamily(family);
    font.setWeight(weight);
    font.setStyle(style);
    font.setPointSize(size);
    return font;
}
