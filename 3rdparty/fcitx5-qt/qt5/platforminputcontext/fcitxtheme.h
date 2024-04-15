/*
 * SPDX-FileCopyrightText: 2021~2021 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#ifndef _PLATFORMINPUTCONTEXT_FCITXTHEME_H_
#define _PLATFORMINPUTCONTEXT_FCITXTHEME_H_

#include <QColor>
#include <QFileSystemWatcher>
#include <QFont>
#include <QFontMetrics>
#include <QMargins>
#include <QObject>
#include <QPainter>
#include <QPixmap>
#include <QSettings>

namespace fcitx {

class BackgroundImage {
    friend class FcitxTheme;

public:
    void load(const QString &name, QSettings &settings);
    void loadFromValue(const QColor &border, const QColor &background,
                       QMargins margin, int borderWidth);

private:
    void fillBackground(const QColor &border, const QColor &background,
                        int borderWidth);

    QPixmap image_, overlay_;
    QMargins margin_, overlayClipMargin_;
    bool hideOverlayIfOversize_ = false;
    QString gravity_;
    int overlayOffsetX_ = 0;
    int overlayOffsetY_ = 0;
};

class ActionImage {
    friend class FcitxTheme;

public:
    void load(const QString &name, QSettings &settings);
    void reset();
    bool valid() const { return valid_; }
    int width() const { return image_.width(); }
    int height() const { return image_.height(); }
    QSize size() const { return image_.size(); }
    const QMargins &margin() const { return margin_; }

private:
    bool valid_ = false;
    QPixmap image_;
    QMargins margin_;
};

class FcitxTheme : public QObject {
    Q_OBJECT
public:
    FcitxTheme(QObject *parent = nullptr);
    ~FcitxTheme();

    void paint(QPainter *painter, const BackgroundImage &image, QRect region);
    void paint(QPainter *painter, const ActionImage &image, QPoint position,
               float alpha);

    const auto &background() const { return background_; }
    const auto &highlight() const { return highlight_; }
    const auto &prev() const { return prev_; }
    const auto &next() const { return next_; }
    const auto &font() const { return font_; }
    const auto &fontMetrics() const { return fontMetrics_; }
    const auto &highlightBackgroundColor() const {
        return highlightBackgroundColor_;
    }
    const auto &highlightColor() const { return highlightColor_; }
    const auto &buttonAlignment() const { return buttonAlignment_; }
    auto contentMargin() const { return contentMargin_; }
    auto textMargin() const { return textMargin_; }
    auto highlightClickMargin() const { return highlightClickMargin_; }
    QMargins highlightMargin() const;
    auto shadowMargin() const { return shadowMargin_; }
    auto normalColor() const { return normalColor_; }
    auto highlightCandidateColor() const { return highlightCandidateColor_; }
    auto vertical() const { return vertical_; }
    auto wheelForPaging() const { return wheelForPaging_; }

private Q_SLOTS:
    void configChanged();
    void themeChanged();

private:
    QString configPath_;
    QString themeConfigPath_;
    QFileSystemWatcher *watcher_;

    QFont font_;
    QFontMetrics fontMetrics_{font_};
    bool vertical_ = false;
    bool wheelForPaging_ = true;
    QString theme_ = "default";
    BackgroundImage background_;
    BackgroundImage highlight_;
    ActionImage prev_;
    ActionImage next_;

    QColor normalColor_{Qt::black};
    QColor highlightCandidateColor_{Qt::white};
    bool fullWidthHighlight_ = true;
    QColor highlightColor_{Qt::white};
    QColor highlightBackgroundColor_{0xa5, 0xa5, 0xa5};
    QString buttonAlignment_;
    QMargins highlightClickMargin_;
    QMargins contentMargin_;
    QMargins textMargin_;
    QMargins shadowMargin_;
};

} // namespace fcitx

#endif // _PLATFORMINPUTCONTEXT_FCITXTHEME_H_
