/*
 * SPDX-FileCopyrightText: 2021~2021 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "fcitxtheme.h"
#include "font.h"
#include <QDebug>
#include <QMargins>
#include <QPixmap>
#include <QSettings>
#include <QStandardPaths>

namespace fcitx {

bool readBool(const QSettings &settings, const QString &name,
              bool defaultValue) {
    return settings.value(name, defaultValue ? "True" : "False").toString() ==
           "True";
}

QMargins readMargin(const QSettings &settings) {
    settings.allKeys();
    return QMargins(settings.value("Left", 0).toInt(),
                    settings.value("Top", 0).toInt(),
                    settings.value("Right", 0).toInt(),
                    settings.value("Bottom", 0).toInt());
}

QColor readColor(const QSettings &settings, const QString &name,
                 const QString &defaultValue) {
    QString colorString = settings.value(name, defaultValue).toString();
    QColor color(defaultValue);
    if (colorString.startsWith("#")) {
        if (colorString.size() == 7) {
            // Parse #RRGGBB
            color = QColor(colorString.toUpper());
        } else if (colorString.size() == 9) {
            // Qt accept "#AARRGGBB"
            auto newColorString =
                QString("#%1%2")
                    .arg(colorString.mid(7, 2), colorString.mid(1, 6))
                    .toUpper();
            color = QColor(newColorString);
        }
    }
    return color;
}

void BackgroundImage::load(const QString &name, QSettings &settings) {
    settings.allKeys();
    image_ = QPixmap();
    overlay_ = QPixmap();
    if (auto image = settings.value("Image").toString(); !image.isEmpty()) {
        auto file = QStandardPaths::locate(
            QStandardPaths::GenericDataLocation,
            QString("fcitx5/themes/%1/%2").arg(name, image));
        image_.load(file);
    }
    if (auto image = settings.value("Overlay").toString(); !image.isEmpty()) {
        auto file = QStandardPaths::locate(
            QStandardPaths::GenericDataLocation,
            QString("fcitx5/themes/%1/%2").arg(name, image));
        overlay_.load(file);
    }

    settings.beginGroup("Margin");
    margin_ = readMargin(settings);
    settings.endGroup();

    if (image_.isNull()) {
        QColor color = readColor(settings, "Color", "#ffffff");
        QColor borderColor = readColor(settings, "BorderColor", "#00ffffff");
        int borderWidth = settings.value("BorderWidth", 0).toInt();
        fillBackground(borderColor, color, borderWidth);
    }

    settings.beginGroup("OverlayClipMargin");
    overlayClipMargin_ = readMargin(settings);
    settings.endGroup();

    hideOverlayIfOversize_ =
        settings.value("HideOverlayIfOversize", "False").toString() == "True";
    overlayOffsetX_ = settings.value("OverlayOffsetX", 0).toInt();
    overlayOffsetY_ = settings.value("OverlayOffsetY", 0).toInt();
    gravity_ = settings.value("Gravity", "TopLeft").toString();
}

void BackgroundImage::loadFromValue(const QColor &border,
                                    const QColor &background, QMargins margin,
                                    int borderWidth) {
    image_ = QPixmap();
    overlay_ = QPixmap();
    margin_ = margin;
    fillBackground(border, background, borderWidth);
    overlayClipMargin_ = QMargins();
    hideOverlayIfOversize_ = false;
    overlayOffsetX_ = 0;
    overlayOffsetY_ = 0;
    gravity_ = QString();
}

void BackgroundImage::fillBackground(const QColor &border,
                                     const QColor &background,
                                     int borderWidth) {
    image_ = QPixmap(margin_.left() + margin_.right() + 1,
                     margin_.top() + margin_.bottom() + 1);
    borderWidth = std::min({borderWidth, margin_.left(), margin_.right(),
                            margin_.top(), margin_.bottom()});
    borderWidth = std::max(0, borderWidth);

    QPainter painter;
    painter.begin(&image_);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    if (borderWidth) {
        painter.fillRect(image_.rect(), border);
    }
    painter.fillRect(QRect(borderWidth, borderWidth,
                           image_.width() - borderWidth * 2,
                           image_.height() - borderWidth * 2),
                     background);
    painter.end();
}

void ActionImage::load(const QString &name, QSettings &settings) {
    settings.allKeys();
    image_ = QPixmap();
    valid_ = false;
    if (auto image = settings.value("Image").toString(); !image.isEmpty()) {
        auto file = QStandardPaths::locate(
            QStandardPaths::GenericDataLocation,
            QString("fcitx5/themes/%1/%2").arg(name, image));
        image_.load(file);
        valid_ = !image_.isNull();
    }

    settings.beginGroup("ClickMargin");
    margin_ = readMargin(settings);
    settings.endGroup();
}

void ActionImage::reset() {
    image_ = QPixmap();
    valid_ = false;
    margin_ = QMargins(0, 0, 0, 0);
}

FcitxTheme::FcitxTheme(QObject *parent)
    : QObject(parent), configPath_(QStandardPaths::writableLocation(
                                       QStandardPaths::GenericConfigLocation)
                                       .append("/fcitx5/conf/classicui.conf")),
      watcher_(new QFileSystemWatcher) {
    connect(watcher_, &QFileSystemWatcher::fileChanged, this,
            &FcitxTheme::configChanged);
    watcher_->addPath(configPath_);

    configChanged();
}

FcitxTheme::~FcitxTheme() {}

void FcitxTheme::configChanged() {
    // Since fcitx is doing things like delete and move, we need to re-add the
    // path.
    watcher_->removePath(configPath_);
    watcher_->addPath(configPath_);
    QSettings settings(configPath_, QSettings::IniFormat);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    settings.setIniCodec("UTF-8");
#endif
    settings.childGroups();
    font_ = parseFont(settings.value("Font", "Sans Serif 9").toString());
    fontMetrics_ = QFontMetrics(font_);
    vertical_ =
        settings.value("Vertical Candidate List", "False").toString() == "True";
    wheelForPaging_ =
        settings.value("WheelForPaging", "True").toString() == "True";
    theme_ = settings.value("Theme", "default").toString();

    themeChanged();
}

void FcitxTheme::themeChanged() {
    if (!themeConfigPath_.isEmpty()) {
        watcher_->removePath(themeConfigPath_);
    }
    auto themeConfig = QString("/fcitx5/themes/%1/theme.conf").arg(theme_);
    themeConfigPath_ =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
            .append(themeConfig);
    auto file = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                       themeConfig);
    if (file.isEmpty()) {
        file = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                      "fcitx5/themes/default/theme.conf");
        themeConfigPath_ = QStandardPaths::writableLocation(
                               QStandardPaths::GenericDataLocation)
                               .append("fcitx5/themes/default/theme.conf");
        theme_ = "default";
    }

    watcher_->addPath(themeConfigPath_);

    // We can not locate default theme.
    if (file.isEmpty()) {
        normalColor_ = QColor("#000000");
        highlightCandidateColor_ = QColor("#ffffff");
        fullWidthHighlight_ = true;
        highlightColor_ = QColor("#ffffff");
        highlightBackgroundColor_ = QColor("#a5a5a5");
        contentMargin_ = QMargins{2, 2, 2, 2};
        textMargin_ = QMargins{5, 5, 5, 5};
        highlightClickMargin_ = QMargins{0, 0, 0, 0};
        shadowMargin_ = QMargins{0, 0, 0, 0};
        background_.loadFromValue(highlightBackgroundColor_, highlightColor_,
                                  contentMargin_, 2);
        highlight_.loadFromValue(highlightBackgroundColor_,
                                 highlightBackgroundColor_, textMargin_, 0);
        prev_.reset();
        next_.reset();
        return;
    }

    QSettings settings(file, QSettings::IniFormat);
    settings.childGroups();
    settings.beginGroup("InputPanel");
    normalColor_ = readColor(settings, "NormalColor", "#000000");
    highlightCandidateColor_ =
        readColor(settings, "HighlightCandidateColor", "#ffffff");
    fullWidthHighlight_ = readBool(settings, "FullWidthHighlight", true);
    highlightColor_ = readColor(settings, "HighlightColor", "#ffffff");
    highlightBackgroundColor_ =
        readColor(settings, "HighlightBackgroundColor", "#a5a5a5");
    buttonAlignment_ =
        settings.value("PageButtonAlignment", "Bottom").toString();

    settings.beginGroup("ContentMargin");
    contentMargin_ = readMargin(settings);
    settings.endGroup();
    settings.beginGroup("TextMargin");
    textMargin_ = readMargin(settings);
    settings.endGroup();
    settings.beginGroup("ShadowMargin");
    shadowMargin_ = readMargin(settings);
    settings.endGroup();

    settings.beginGroup("Background");
    background_.load(theme_, settings);
    settings.endGroup();

    settings.beginGroup("Highlight");
    highlight_.load(theme_, settings);
    settings.beginGroup("HighlightClickMargin");
    highlightClickMargin_ = readMargin(settings);
    settings.endGroup();
    settings.endGroup();

    settings.beginGroup("PrevPage");
    prev_.load(theme_, settings);
    settings.endGroup();

    settings.beginGroup("NextPage");
    next_.load(theme_, settings);
    settings.endGroup();
}

} // namespace fcitx

void fcitx::FcitxTheme::paint(QPainter *painter,
                              const fcitx::BackgroundImage &image,
                              QRect region) {
    auto marginTop = image.margin_.top();
    auto marginBottom = image.margin_.bottom();
    auto marginLeft = image.margin_.left();
    auto marginRight = image.margin_.right();
    int resizeHeight = image.image_.height() - marginTop - marginBottom;
    int resizeWidth = image.image_.width() - marginLeft - marginRight;

    if (resizeHeight <= 0) {
        resizeHeight = 1;
    }

    if (resizeWidth <= 0) {
        resizeWidth = 1;
    }

    if (region.height() < 0) {
        region.setHeight(resizeHeight);
    }

    if (region.width() < 0) {
        region.setWidth(resizeWidth);
    }

    /*
     * 7 8 9
     * 4 5 6
     * 1 2 3
     */

    if (marginLeft && marginBottom) {
        /* part 1 */
        painter->drawPixmap(
            QRect(0, region.height() - marginBottom, marginLeft, marginBottom)
                .translated(region.topLeft()),
            image.image_,
            QRect(0, marginTop + resizeHeight, marginLeft, marginBottom));
    }

    if (marginRight && marginBottom) {
        /* part 3 */
        painter->drawPixmap(
            QRect(region.width() - marginRight, region.height() - marginBottom,
                  marginRight, marginBottom)
                .translated(region.topLeft()),
            image.image_,
            QRect(marginLeft + resizeWidth, marginTop + resizeHeight,
                  marginRight, marginBottom));
    }

    if (marginLeft && marginTop) {
        /* part 7 */
        painter->drawPixmap(
            QRect(0, 0, marginLeft, marginTop).translated(region.topLeft()),
            image.image_, QRect(0, 0, marginLeft, marginTop));
    }

    if (marginRight && marginTop) {
        /* part 9 */
        painter->drawPixmap(
            QRect(region.width() - marginRight, 0, marginRight, marginTop)
                .translated(region.topLeft()),
            image.image_,
            QRect(marginLeft + resizeWidth, 0, marginRight, marginTop));
    }

    /* part 2 & 8 */
    if (marginTop) {
        painter->drawPixmap(
            QRect(marginLeft, 0, region.width() - marginLeft - marginRight,
                  marginTop)
                .translated(region.topLeft()),
            image.image_, QRect(marginLeft, 0, resizeWidth, marginTop));
    }

    if (marginBottom) {
        painter->drawPixmap(QRect(marginLeft, region.height() - marginBottom,
                                  region.width() - marginLeft - marginRight,
                                  marginBottom)
                                .translated(region.topLeft()),
                            image.image_,
                            QRect(marginLeft, marginTop + resizeHeight,
                                  resizeWidth, marginBottom));
    }

    /* part 4 & 6 */
    if (marginLeft) {
        painter->drawPixmap(QRect(0, marginTop, marginLeft,
                                  region.height() - marginTop - marginBottom)
                                .translated(region.topLeft()),
                            image.image_,
                            QRect(0, marginTop, marginLeft, resizeHeight));
    }

    if (marginRight) {
        painter->drawPixmap(QRect(region.width() - marginRight, marginTop,
                                  marginRight,
                                  region.height() - marginTop - marginBottom)
                                .translated(region.topLeft()),
                            image.image_,
                            QRect(marginLeft + resizeWidth, marginTop,
                                  marginRight, resizeHeight));
    }

    /* part 5 */
    {
        painter->drawPixmap(
            QRect(marginLeft, marginTop,
                  region.width() - marginLeft - marginRight,
                  region.height() - marginTop - marginBottom)
                .translated(region.topLeft()),
            image.image_,
            QRect(marginLeft, marginTop, resizeWidth, resizeHeight));
    }

    if (image.overlay_.isNull()) {
        return;
    }

    auto clipWidth = region.width() - image.overlayClipMargin_.left() -
                     image.overlayClipMargin_.right();
    auto clipHeight = region.height() - image.overlayClipMargin_.top() -
                      image.overlayClipMargin_.bottom();
    if (clipWidth <= 0 || clipHeight <= 0) {
        return;
    }
    QRect clipRect(region.topLeft() + QPoint(image.overlayClipMargin_.left(),
                                             image.overlayClipMargin_.top()),
                   QSize(clipWidth, clipHeight));

    int x = 0, y = 0;
    if (image.gravity_ == "Top Left" || image.gravity_ == "Center Left" ||
        image.gravity_ == "Bottom Left") {
        x = image.overlayOffsetX_;
    } else if (image.gravity_ == "Top Center" || image.gravity_ == "Center" ||
               image.gravity_ == "Bottom Center") {
        x = (region.width() - image.overlay_.width()) / 2 +
            image.overlayOffsetX_;
    } else {
        x = region.width() - image.overlay_.width() - image.overlayOffsetX_;
    }

    if (image.gravity_ == "Top Left" || image.gravity_ == "Top Center" ||
        image.gravity_ == "Top Right") {
        y = image.overlayOffsetY_;
    } else if (image.gravity_ == "Center Left" || image.gravity_ == "Center" ||
               image.gravity_ == "Center Right") {
        y = (region.height() - image.overlay_.height()) / 2 +
            image.overlayOffsetY_;
    } else {
        y = region.height() - image.overlay_.height() - image.overlayOffsetY_;
    }
    QRect rect(QPoint(x, y) + region.topLeft(), image.overlay_.size());
    QRect finalRect = rect.intersected(clipRect);
    if (finalRect.isEmpty()) {
        return;
    }

    if (image.hideOverlayIfOversize_ && !clipRect.contains(rect)) {
        return;
    }

    painter->save();
    painter->setClipRect(clipRect);
    painter->drawPixmap(rect, image.overlay_);
    painter->restore();
}

void fcitx::FcitxTheme::paint(QPainter *painter,
                              const fcitx::ActionImage &image, QPoint position,
                              float alpha) {
    painter->save();
    painter->setOpacity(alpha);
    painter->drawPixmap(position, image.image_);
    painter->restore();
}

QMargins fcitx::FcitxTheme::highlightMargin() const {
    return highlight_.margin_;
}
