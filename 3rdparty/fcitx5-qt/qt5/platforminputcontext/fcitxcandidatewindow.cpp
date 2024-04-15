/*
 * SPDX-FileCopyrightText: 2021~2021 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "fcitxcandidatewindow.h"
#include "fcitxflags.h"
#include "fcitxtheme.h"
#include "qfcitxplatforminputcontext.h"
#include <QDebug>
#include <QExposeEvent>
#include <QFont>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPalette>
#include <QResizeEvent>
#include <QScreen>
#include <QTextLayout>
#include <QVariant>
#include <QtMath>
#include <utility>

#if defined(FCITX_ENABLE_QT6_WAYLAND_WORKAROUND) &&                            \
    QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
#include <QtGui/private/qhighdpiscaling_p.h>
#include <QtWaylandClient/private/qwayland-xdg-shell.h>
#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandintegration_p.h>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <QtWaylandClient/private/wayland-xdg-shell-client-protocol.h>
#include <qpa/qplatformnativeinterface.h>
#endif

namespace fcitx {

namespace {

#if defined(FCITX_ENABLE_QT6_WAYLAND_WORKAROUND) &&                            \
    QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
class XdgWmBase : public QtWayland::xdg_wm_base {
public:
    using xdg_wm_base::xdg_wm_base;

protected:
    // This is required for all xdg_wm_base bind.
    void xdg_wm_base_ping(uint32_t serial) override { pong(serial); }
};

#endif

void doLayout(QTextLayout &layout) {
    QFontMetrics fontMetrics(layout.font());
    auto minH = fontMetrics.ascent() + fontMetrics.descent();
    layout.setCacheEnabled(true);
    layout.beginLayout();
    int height = 0;
    while (1) {
        QTextLine line = layout.createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(INT_MAX);
        line.setLeadingIncluded(true);

        line.setPosition(
            QPoint(0, height - line.ascent() + fontMetrics.ascent()));
        height += minH;
    }
    layout.endLayout();
}

} // namespace

class MultilineText {
public:
    MultilineText(const QFont &font, const QString &text) {
        QStringList lines = text.split("\n");
        int width = 0;
        QFontMetrics fontMetrics(font);
        fontHeight_ = fontMetrics.ascent() + fontMetrics.descent();
        for (const auto &line : lines) {
            layouts_.emplace_back(std::make_unique<QTextLayout>(line));
            layouts_.back()->setFont(font);
            doLayout(*layouts_.back());
            width = std::max(width,
                             layouts_.back()->boundingRect().toRect().width());
        }
        boundingRect_.setTopLeft(QPoint(0, 0));
        boundingRect_.setSize(QSize(width, lines.size() * fontHeight_));
    }

    bool isEmpty() const { return layouts_.empty(); }

    void draw(QPainter *painter, QColor color, QPoint position) {
        painter->save();
        painter->setPen(color);
        int currentY = 0;
        for (const auto &layout : layouts_) {
            layout->draw(painter, position + QPoint(0, currentY));
            currentY += fontHeight_;
        }
        painter->restore();
    }

    QRect boundingRect() { return boundingRect_; }

private:
    std::vector<std::unique_ptr<QTextLayout>> layouts_;
    int fontHeight_;
    QRect boundingRect_;
};

FcitxCandidateWindow::FcitxCandidateWindow(QWindow *window,
                                           QFcitxPlatformInputContext *context)
    : QRasterWindow(), context_(context), theme_(context->theme()),
      parent_(window) {
    constexpr Qt::WindowFlags commonFlags = Qt::FramelessWindowHint |
                                            Qt::WindowDoesNotAcceptFocus |
                                            Qt::NoDropShadowWindowHint;
    if (isWayland_) {
        // Qt::ToolTip ensures wayland doesn't grab focus.
        // Not using Qt::BypassWindowManagerHint ensures wayland handle
        // fractional scale.
        setFlags(Qt::ToolTip | commonFlags);
#if defined(FCITX_ENABLE_QT6_WAYLAND_WORKAROUND) &&                            \
    QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
        if (auto instance = QtWaylandClient::QWaylandIntegration::instance()) {
            for (QtWaylandClient::QWaylandDisplay::RegistryGlobal global :
                 instance->display()->globals()) {
                if (global.interface == QLatin1String("xdg_wm_base")) {
                    xdgWmBase_.reset(
                        new XdgWmBase(instance->display()->wl_registry(),
                                      global.id, global.version));
                    break;
                }
            }
        }
        setProperty("_q_waylandPopupAnchor",
                    QVariant::fromValue(Qt::BottomEdge | Qt::LeftEdge));
        setProperty("_q_waylandPopupGravity",
                    QVariant::fromValue(Qt::BottomEdge | Qt::RightEdge));
        setProperty(
            "_q_waylandPopupConstraintAdjustment",
            static_cast<unsigned int>(
                QtWayland::xdg_positioner::constraint_adjustment_slide_x |
                QtWayland::xdg_positioner::constraint_adjustment_flip_y));
#endif
    } else {
        // Qt::Popup ensures X11 doesn't apply tooltip animation under kwin.
        setFlags(Qt::Popup | Qt::BypassWindowManagerHint | commonFlags);
    }
    if (isWayland_) {
        setTransientParent(parent_);
    }
    QSurfaceFormat surfaceFormat = format();
    surfaceFormat.setAlphaBufferSize(8);
    setFormat(surfaceFormat);
    connect(this, &QWindow::visibleChanged, this, [this] { hoverIndex_ = -1; });
}

FcitxCandidateWindow::~FcitxCandidateWindow() {}

bool FcitxCandidateWindow::event(QEvent *event) {
    if (event->type() == QEvent::Leave) {
        auto oldHighlight = highlight();
        hoverIndex_ = -1;
        if (highlight() != oldHighlight) {
            update();
        }
    }
    return QRasterWindow::event(event);
}

void FcitxCandidateWindow::render(QPainter *painter) {
    theme_->paint(painter, theme_->background(),
                  QRect(QPoint(0, 0), actualSize_));
    auto contentMargin = theme_->contentMargin();

    const QPoint topLeft(contentMargin.left(), contentMargin.top());
    painter->setPen(theme_->normalColor());
    auto minH =
        theme_->fontMetrics().ascent() + theme_->fontMetrics().descent();
    auto textMargin = theme_->textMargin();
    auto extraW = textMargin.left() + textMargin.right();
    auto extraH = textMargin.top() + textMargin.bottom();
    size_t currentHeight = 0;
    if (!upperLayout_.text().isEmpty()) {
        upperLayout_.draw(
            painter, topLeft + QPoint(textMargin.left(), textMargin.top()));
        // Draw cursor
        currentHeight += minH + extraH;
        if (cursor_ >= 0) {
            auto line = upperLayout_.lineForTextPosition(cursor_);
            if (line.isValid()) {
                int cursorX = line.cursorToX(cursor_);
                line.lineNumber();
                painter->save();
                QPen pen = painter->pen();
                pen.setWidth(2);
                painter->setPen(pen);
                QPoint start = topLeft + QPoint(textMargin.left() + cursorX + 1,
                                                textMargin.top() +
                                                    line.lineNumber() * minH);
                painter->drawLine(start, start + QPoint(0, minH));
                painter->restore();
            }
        }
    }
    if (!lowerLayout_.text().isEmpty()) {
        lowerLayout_.draw(painter,
                          topLeft + QPoint(textMargin.left(),
                                           textMargin.top() + currentHeight));
        currentHeight += minH + extraH;
    }

    bool vertical = theme_->vertical();
    if (layoutHint_ == FcitxCandidateLayoutHint::Vertical) {
        vertical = true;
    } else if (layoutHint_ == FcitxCandidateLayoutHint::Horizontal) {
        vertical = false;
    }

    candidateRegions_.clear();
    candidateRegions_.reserve(labelLayouts_.size());
    size_t wholeW = 0, wholeH = 0;

    // size of text = textMargin + actual text size.
    // HighLight = HighLight margin + TEXT.
    // Click region = HighLight - click

    for (size_t i = 0; i < labelLayouts_.size(); i++) {
        int x, y;
        if (vertical) {
            x = 0;
            y = currentHeight + wholeH;
        } else {
            x = wholeW;
            y = currentHeight;
        }
        x += textMargin.left();
        y += textMargin.top();
        int labelW = 0, labelH = 0, candidateW = 0, candidateH = 0;
        if (!labelLayouts_[i]->isEmpty()) {
            auto size = labelLayouts_[i]->boundingRect();
            labelW = size.width();
            labelH = size.height();
        }
        if (!candidateLayouts_[i]->isEmpty()) {
            auto size = candidateLayouts_[i]->boundingRect();
            candidateW = size.width();
            candidateH = size.height();
        }
        int vheight;
        if (vertical) {
            vheight = std::max({minH, labelH, candidateH});
            wholeH += vheight + extraH;
        } else {
            vheight = candidatesHeight_ - extraH;
            wholeW += candidateW + labelW + extraW;
        }
        QMargins highlightMargin = theme_->highlightMargin();
        QMargins clickMargin = theme_->highlightClickMargin();
        auto highlightWidth = labelW + candidateW;
        bool fullWidthHighlight = true;
        if (fullWidthHighlight && vertical) {
            // Last candidate, fill.
            highlightWidth = actualSize_.width() - contentMargin.left() -
                             contentMargin.right() - textMargin.left() -
                             textMargin.right();
        }
        const int highlightIndex = highlight();
        QColor color = theme_->normalColor();
        if (highlightIndex >= 0 && i == static_cast<size_t>(highlightIndex)) {
            // Paint highlight
            theme_->paint(
                painter, theme_->highlight(),
                QRect(topLeft + QPoint(x, y) -
                          QPoint(highlightMargin.left(), highlightMargin.top()),
                      QSize(highlightWidth + highlightMargin.left() +
                                highlightMargin.right(),
                            vheight + highlightMargin.top() +
                                highlightMargin.bottom())));
            color = theme_->highlightCandidateColor();
        }
        QRect candidateRegion(
            topLeft + QPoint(x, y) -
                QPoint(highlightMargin.left(), highlightMargin.right()) +
                QPoint(clickMargin.left(), clickMargin.right()),
            QSize(highlightWidth + highlightMargin.left() +
                      highlightMargin.right() - clickMargin.left() -
                      clickMargin.right(),
                  vheight + highlightMargin.top() + highlightMargin.bottom() -
                      clickMargin.top() - clickMargin.bottom()));
        candidateRegions_.push_back(candidateRegion);
        if (!labelLayouts_[i]->isEmpty()) {
            labelLayouts_[i]->draw(painter, color, topLeft + QPoint(x, y));
        }
        if (!candidateLayouts_[i]->isEmpty()) {
            candidateLayouts_[i]->draw(painter, color,
                                       topLeft + QPoint(x + labelW, y));
        }
    }
    prevRegion_ = QRect();
    nextRegion_ = QRect();
    if (labelLayouts_.size() && (hasPrev_ || hasNext_)) {
        if (theme_->prev().valid() && theme_->next().valid()) {
            int prevY = 0, nextY = 0;
            if (theme_->buttonAlignment() == "Top") {
                prevY = contentMargin.top();
                nextY = contentMargin.top();
            } else if (theme_->buttonAlignment() == "First Candidate") {
                prevY = candidateRegions_.front().top() +
                        (candidateRegions_.front().height() -
                         theme_->prev().height()) /
                            2.0;
                nextY = candidateRegions_.front().top() +
                        (candidateRegions_.front().height() -
                         theme_->next().height()) /
                            2.0;
            } else if (theme_->buttonAlignment() == "Center") {
                prevY = contentMargin.top() +
                        (actualSize_.height() - contentMargin.top() -
                         contentMargin.bottom() - theme_->prev().height()) /
                            2.0;
                nextY = contentMargin.top() +
                        (actualSize_.height() - contentMargin.top() -
                         contentMargin.bottom() - theme_->next().height()) /
                            2.0;
            } else if (theme_->buttonAlignment() == "Last Candidate") {
                prevY = candidateRegions_.back().top() +
                        (candidateRegions_.back().height() -
                         theme_->prev().height()) /
                            2.0;
                nextY = candidateRegions_.back().top() +
                        (candidateRegions_.back().height() -
                         theme_->next().height()) /
                            2.0;
            } else {
                prevY = actualSize_.height() - contentMargin.bottom() -
                        theme_->prev().height();
                nextY = actualSize_.height() - contentMargin.bottom() -
                        theme_->next().height();
            }
            nextRegion_ =
                QRect(QPoint(actualSize_.width() - contentMargin.right() -
                                 theme_->prev().width(),
                             nextY),
                      theme_->next().size());
            double alpha = 1.0;
            if (!hasNext_) {
                alpha = 0.3;
            } else if (nextHovered_) {
                alpha = 0.7;
            }
            theme_->paint(painter, theme_->next(), nextRegion_.topLeft(),
                          alpha);
            nextRegion_ = nextRegion_.marginsRemoved(theme_->next().margin());
            prevRegion_ = QRect(
                QPoint(actualSize_.width() - contentMargin.right() -
                           theme_->next().width() - theme_->prev().width(),
                       prevY),
                theme_->prev().size());
            alpha = 1.0;
            if (!hasPrev_) {
                alpha = 0.3;
            } else if (prevHovered_) {
                alpha = 0.7;
            }
            theme_->paint(painter, theme_->prev(), prevRegion_.topLeft(),
                          alpha);
            prevRegion_ = prevRegion_.marginsRemoved(theme_->prev().margin());
        }
    }
}

void UpdateLayout(QTextLayout &layout, const FcitxTheme &theme,
                  std::initializer_list<
                      std::reference_wrapper<const FcitxQtFormattedPreeditList>>
                      texts) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    layout.clearFormats();
#endif
    layout.setFont(theme.font());
    QVector<QTextLayout::FormatRange> formats;
    QString str;
    int pos = 0;
    for (const auto &text : texts) {
        for (const auto &preedit : text.get()) {
            str += preedit.string();
            QTextCharFormat format;
            if (preedit.format() & FcitxTextFormatFlag_Underline) {
                format.setUnderlineStyle(QTextCharFormat::DashUnderline);
            }
            if (preedit.format() & FcitxTextFormatFlag_Strike) {
                format.setFontStrikeOut(true);
            }
            if (preedit.format() & FcitxTextFormatFlag_Bold) {
                format.setFontWeight(QFont::Bold);
            }
            if (preedit.format() & FcitxTextFormatFlag_Italic) {
                format.setFontItalic(true);
            }
            if (preedit.format() & FcitxTextFormatFlag_HighLight) {
                format.setBackground(theme.highlightBackgroundColor());
                format.setForeground(theme.highlightColor());
            }
            formats.append(QTextLayout::FormatRange{
                pos, static_cast<int>(preedit.string().length()), format});
            pos += preedit.string().length();
        }
    }
    layout.setText(str);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    layout.setFormats(formats);
#endif
}

void FcitxCandidateWindow::updateClientSideUI(
    const FcitxQtFormattedPreeditList &preedit, int cursorpos,
    const FcitxQtFormattedPreeditList &auxUp,
    const FcitxQtFormattedPreeditList &auxDown,
    const FcitxQtStringKeyValueList &candidates, int candidateIndex,
    int layoutHint, bool hasPrev, bool hasNext) {
    bool preeditVisible = !preedit.isEmpty();
    bool auxUpVisbile = !auxUp.isEmpty();
    bool auxDownVisible = !auxDown.isEmpty();
    bool candidatesVisible = !candidates.isEmpty();
    bool visible =
        preeditVisible || auxUpVisbile || auxDownVisible || candidatesVisible;
    auto window = context_->focusWindowWrapper();
    if (!theme_ || !visible || !window || window != parent_) {
        hide();
        return;
    }

    UpdateLayout(upperLayout_, *theme_, {auxUp, preedit});
    if (cursorpos >= 0) {
        int auxUpLength = 0;
        for (const auto &auxUpText : auxUp) {
            auxUpLength += auxUpText.string().length();
        }
        // Get the preedit part
        auto preeditString = upperLayout_.text().mid(auxUpLength).toUtf8();
        preeditString = preeditString.mid(0, cursorpos);
        cursor_ = auxUpLength + QString::fromUtf8(preeditString).length();
    } else {
        cursor_ = -1;
    }
    doLayout(upperLayout_);
    UpdateLayout(lowerLayout_, *theme_, {auxDown});
    doLayout(lowerLayout_);
    labelLayouts_.clear();
    candidateLayouts_.clear();
    for (int i = 0; i < candidates.size(); i++) {
        labelLayouts_.emplace_back(std::make_unique<MultilineText>(
            theme_->font(), candidates[i].key()));
        candidateLayouts_.emplace_back(std::make_unique<MultilineText>(
            theme_->font(), candidates[i].value()));
    }
    highlight_ = candidateIndex;
    hasPrev_ = hasPrev;
    hasNext_ = hasNext;
    layoutHint_ = static_cast<FcitxCandidateLayoutHint>(layoutHint);

    actualSize_ = sizeHint();

    if (actualSize_.width() <= 0 || actualSize_.height() <= 0) {
        hide();
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QSize sizeWithoutShadow = actualSize_.shrunkBy(theme_->shadowMargin());
#else
    QSize sizeWithoutShadow =
        actualSize_ -
        QSize(theme_->shadowMargin().left() + theme_->shadowMargin().right(),
              theme_->shadowMargin().top() + theme_->shadowMargin().bottom());
#endif
    if (sizeWithoutShadow.width() < 0) {
        sizeWithoutShadow.setWidth(0);
    }
    if (sizeWithoutShadow.height() < 0) {
        sizeWithoutShadow.setHeight(0);
    }

    if (size() != actualSize_) {
        resize(actualSize_);
    }
    update();

    QRect cursorRect = context_->cursorRectangleWrapper();
    QRect screenGeometry;

#if defined(FCITX_ENABLE_QT6_WAYLAND_WORKAROUND) &&                            \
    QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    if (isWayland_) {
        auto *waylandWindow =
            static_cast<QtWaylandClient::QWaylandWindow *>(window->handle());
        const auto windowMargins = waylandWindow->windowContentMargins() -
                                   waylandWindow->clientSideMargins();
        auto windowGeometry = waylandWindow->windowContentGeometry();
        if (!cursorRect.isValid()) {
            if (cursorRect.width() <= 0) {
                cursorRect.setWidth(1);
            }
            if (cursorRect.height() <= 0) {
                cursorRect.setHeight(1);
            }
        }
        QRect nativeCursorRect = QHighDpi::toNativePixels(cursorRect, this);
        // valid the anchor rect.
        if (!nativeCursorRect.intersects(windowGeometry)) {
            if (nativeCursorRect.right() < windowGeometry.left()) {
                nativeCursorRect.setLeft(windowGeometry.left());
                nativeCursorRect.setWidth(1);
            }
            if (nativeCursorRect.left() > windowGeometry.right()) {
                nativeCursorRect.setLeft(windowGeometry.right());
                nativeCursorRect.setWidth(1);
            }
            if (nativeCursorRect.bottom() < windowGeometry.top()) {
                nativeCursorRect.setTop(windowGeometry.top());
                nativeCursorRect.setWidth(1);
            }
            if (nativeCursorRect.top() > windowGeometry.bottom()) {
                nativeCursorRect.setTop(windowGeometry.bottom());
                nativeCursorRect.setWidth(1);
            }
        }
        bool wasVisible = isVisible();
        bool cursorRectChanged = false;
        if (property("_q_waylandPopupAnchorRect") != nativeCursorRect) {
            cursorRectChanged = true;
            setProperty("_q_waylandPopupAnchorRect", nativeCursorRect);
        }
        // This try to ensure xdg_popup is available.
        show();
        xdg_popup *xdgPopup = static_cast<xdg_popup *>(
            QGuiApplication::platformNativeInterface()->nativeResourceForWindow(
                "xdg_popup", this));
        if (xdgWmBase_ && xdgPopup &&
            xdg_popup_get_version(xdgPopup) >=
                XDG_POPUP_REPOSITION_SINCE_VERSION) {
            nativeCursorRect.translate(-windowMargins.left(),
                                       -windowMargins.top());
            auto *positioner =
                new QtWayland::xdg_positioner(xdgWmBase_->create_positioner());
            positioner->set_anchor_rect(
                nativeCursorRect.x(), nativeCursorRect.y(),
                nativeCursorRect.width(), nativeCursorRect.height());
            positioner->set_anchor(
                QtWayland::xdg_positioner::anchor_bottom_left);
            positioner->set_gravity(
                QtWayland::xdg_positioner::gravity_bottom_right);

            auto *waylandCandidateWindow =
                static_cast<QtWaylandClient::QWaylandWindow *>(handle());
            QRect nativeGeometry =
                waylandCandidateWindow->windowContentGeometry();
            positioner->set_size(nativeGeometry.width(),
                                 nativeGeometry.height());
            positioner->set_reactive();
            positioner->set_constraint_adjustment(
                QtWayland::xdg_positioner::constraint_adjustment_slide_x |
                QtWayland::xdg_positioner::constraint_adjustment_flip_y);
            xdg_popup_reposition(xdgPopup, positioner->object(),
                                 repositionToken_++);
            positioner->destroy();
            return;
        }
        // Check if we need remap.
        // If it was invisible, nothing need to be done.
        // If cursor rect changed, the window must be remapped.
        // If adjustment is already happening (flip/slide), then remap.
        // If we predict adjustment may be happening, then remap.
        const auto predictGeometry =
            QRect(QPoint(cursorRect.x(), cursorRect.y() + cursorRect.height()),
                  actualSize_);

        if (wasVisible &&
            (cursorRectChanged || position() != predictGeometry.topLeft() ||
             !windowGeometry.contains(predictGeometry))) {
            hide();
            show();
        }
        return;
    }
#endif
    // Try to apply the screen edge detection over the window, because if we
    // intent to use this with wayland. It we have no information above screen
    // edge.
    if (isWayland_) {
        screenGeometry = window->frameGeometry();
        cursorRect.translate(window->framePosition());
        auto margins = window->frameMargins();
        cursorRect.translate(margins.left(), margins.top());
    } else {
        screenGeometry = window->screen()->geometry();
        auto pos = window->mapToGlobal(cursorRect.topLeft());
        cursorRect.moveTo(pos);
    }

    int x = cursorRect.left();
    int y = cursorRect.bottom();
    if (cursorRect.left() + sizeWithoutShadow.width() >
        screenGeometry.right()) {
        x = screenGeometry.right() - sizeWithoutShadow.width() + 1;
    }

    if (x < screenGeometry.left()) {
        x = screenGeometry.left();
    }

    if (y + sizeWithoutShadow.height() > screenGeometry.bottom()) {
        if (y > screenGeometry.bottom()) {
            y = screenGeometry.bottom() - sizeWithoutShadow.height() - 40;
        } else { /* better position the window */
            y = y - sizeWithoutShadow.height() -
                ((cursorRect.height() == 0) ? 40 : cursorRect.height());
        }
    }

    if (y < screenGeometry.top()) {
        y = screenGeometry.top();
    }

    QPoint newPosition(x, y);
    newPosition -=
        QPoint(theme_->shadowMargin().left(), theme_->shadowMargin().top());
    if (newPosition != position()) {
        if (isWayland_ && isVisible()) {
            hide();
        }
        setPosition(newPosition);
    }
    show();
}

void FcitxCandidateWindow::mouseMoveEvent(QMouseEvent *event) {
    bool needRepaint = false;

    bool prevHovered = false;
    bool nextHovered = false;
    auto oldHighlight = highlight();
    hoverIndex_ = -1;

    prevHovered = prevRegion_.contains(event->pos());
    if (!prevHovered) {
        nextHovered = nextRegion_.contains(event->pos());
        if (!nextHovered) {
            for (int idx = 0, e = candidateRegions_.size(); idx < e; idx++) {
                if (candidateRegions_[idx].contains(event->pos())) {
                    hoverIndex_ = idx;
                    break;
                }
            }
        }
    }

    needRepaint = needRepaint || prevHovered_ != prevHovered;
    prevHovered_ = prevHovered;

    needRepaint = needRepaint || nextHovered_ != nextHovered;
    nextHovered_ = nextHovered;

    needRepaint = needRepaint || oldHighlight != highlight();
    if (needRepaint) {
        update();
    }
}

void FcitxCandidateWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) {
        return;
    }

    if (prevRegion_.contains(event->pos())) {
        Q_EMIT prevClicked();
        return;
    }

    if (nextRegion_.contains(event->pos())) {
        Q_EMIT nextClicked();
        return;
    }

    for (int idx = 0, e = candidateRegions_.size(); idx < e; idx++) {
        if (candidateRegions_[idx].contains(event->pos())) {
            Q_EMIT candidateSelected(idx);
            return;
        }
    }
}

QSize FcitxCandidateWindow::sizeHint() {
    auto minH =
        theme_->fontMetrics().ascent() + theme_->fontMetrics().descent();

    size_t width = 0;
    size_t height = 0;
    auto updateIfLarger = [](size_t &m, size_t n) {
        if (n > m) {
            m = n;
        }
    };
    auto textMargin = theme_->textMargin();
    auto extraW = textMargin.left() + textMargin.right();
    auto extraH = textMargin.top() + textMargin.bottom();
    if (!upperLayout_.text().isEmpty()) {
        auto size = upperLayout_.boundingRect();
        height += minH + extraH;
        updateIfLarger(width, size.width() + extraW);
    }
    if (!lowerLayout_.text().isEmpty()) {
        auto size = lowerLayout_.boundingRect();
        height += minH + extraH;
        updateIfLarger(width, size.width() + extraW);
    }

    bool vertical = theme_->vertical();
    if (layoutHint_ == FcitxCandidateLayoutHint::Vertical) {
        vertical = true;
    } else if (layoutHint_ == FcitxCandidateLayoutHint::Horizontal) {
        vertical = false;
    }

    size_t wholeH = 0, wholeW = 0;
    for (size_t i = 0; i < labelLayouts_.size(); i++) {
        size_t candidateW = 0, candidateH = 0;
        if (!labelLayouts_[i]->isEmpty()) {
            auto size = labelLayouts_[i]->boundingRect();
            candidateW += size.width();
            updateIfLarger(candidateH,
                           std::max(minH, qCeil(size.height())) + extraH);
        }
        if (!candidateLayouts_[i]->isEmpty()) {
            auto size = candidateLayouts_[i]->boundingRect();
            candidateW += size.width();
            updateIfLarger(candidateH,
                           std::max(minH, qCeil(size.height())) + extraH);
        }
        candidateW += extraW;

        if (vertical) {
            wholeH += candidateH;
            updateIfLarger(wholeW, candidateW);
        } else {
            wholeW += candidateW;
            updateIfLarger(wholeH, candidateH);
        }
    }
    updateIfLarger(width, wholeW);
    candidatesHeight_ = wholeH;
    height += wholeH;

    auto contentMargin = theme_->contentMargin();
    width += contentMargin.left() + contentMargin.right();
    height += contentMargin.top() + contentMargin.bottom();

    if (!labelLayouts_.empty() && (hasPrev_ || hasNext_)) {
        if (theme_->prev().valid() && theme_->next().valid()) {
            width += theme_->prev().width() + theme_->next().width();
        }
    }

    return {static_cast<int>(width), static_cast<int>(height)};
}

void FcitxCandidateWindow::wheelEvent(QWheelEvent *event) {
    if (!theme_ || !theme_->wheelForPaging()) {
        return;
    }
    accAngle_ += event->angleDelta().y();
    auto angleForClick = 120;
    while (accAngle_ >= angleForClick) {
        accAngle_ -= angleForClick;
        Q_EMIT prevClicked();
    }
    while (accAngle_ <= -angleForClick) {
        accAngle_ += angleForClick;
        Q_EMIT nextClicked();
    }
}

void FcitxCandidateWindow::paintEvent(QPaintEvent *) {
    QPainter p(this);
    render(&p);
}
} // namespace fcitx
