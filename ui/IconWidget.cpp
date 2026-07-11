#include "IconWidget.h"

#include <QPainter>
#include <QPainterPath>

IconWidget::IconWidget(Kind kind, const QString &fileName, QWidget *parent)
    : QWidget(parent)
    , kind(kind)
{
    setAttribute(Qt::WA_TranslucentBackground);

    QPixmap loaded(QStringLiteral(":/icons/") + fileName);
    if (!loaded.isNull())
        realIcon = loaded;
}

QSize IconWidget::sizeHint() const
{
    return QSize(32, 32);
}

void IconWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QRect r = rect();

    if (!realIcon.isNull()) {
        QPixmap scaled = realIcon.scaled(r.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QRect target(0, 0, scaled.width(), scaled.height());
        target.moveCenter(r.center());
        painter.drawPixmap(target, scaled);
        return;
    }

    drawFallback(painter, r);
}

// Original, hand-drawn line-art icons (not copied from any icon set) used
// only until real icon files are dropped into resources/icons/.
void IconWidget::drawFallback(QPainter &painter, const QRect &rect) const
{
    const QColor strokeColor("#4B5563"); // neutral slate gray
    QPen pen(strokeColor, 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    QRectF r = rect.adjusted(4, 4, -4, -4);

    switch (kind) {
    case Kind::Box: {
        // A simple package: outer square + a seam line + a "tape" line.
        painter.drawRoundedRect(r, 3, 3);
        painter.drawLine(QPointF(r.left(), r.top() + r.height() * 0.38),
                          QPointF(r.right(), r.top() + r.height() * 0.38));
        painter.drawLine(QPointF(r.center().x(), r.top() + r.height() * 0.38),
                          QPointF(r.center().x(), r.bottom()));
        break;
    }
    case Kind::Menu: {
        // A plate: two concentric circles.
        painter.drawEllipse(r);
        QRectF inner = r.adjusted(r.width() * 0.28, r.height() * 0.28,
                                   -r.width() * 0.28, -r.height() * 0.28);
        painter.drawEllipse(inner);
        break;
    }
    case Kind::Receipt: {
        // A receipt: rectangle with a zig-zag bottom edge + a few text lines.
        QRectF body = r.adjusted(r.width() * 0.12, 0, -r.width() * 0.12, -r.height() * 0.18);
        QPainterPath path;
        path.moveTo(body.topLeft());
        path.lineTo(body.topRight());
        path.lineTo(body.bottomRight());
        double zigW = body.width() / 5.0;
        double x = body.right();
        double yTop = body.bottom();
        double yBottom = body.bottom() + r.height() * 0.12;
        for (int i = 0; i < 5; ++i) {
            x -= zigW / 2.0;
            path.lineTo(x, (i % 2 == 0) ? yBottom : yTop);
            x -= zigW / 2.0;
            path.lineTo(x, (i % 2 == 0) ? yTop : yBottom);
        }
        path.lineTo(body.bottomLeft());
        path.closeSubpath();
        painter.drawPath(path);

        painter.drawLine(QPointF(body.left() + body.width() * 0.18, body.top() + body.height() * 0.30),
                          QPointF(body.right() - body.width() * 0.18, body.top() + body.height() * 0.30));
        painter.drawLine(QPointF(body.left() + body.width() * 0.18, body.top() + body.height() * 0.52),
                          QPointF(body.right() - body.width() * 0.18, body.top() + body.height() * 0.52));
        break;
    }
    case Kind::Clock: {
        painter.drawEllipse(r);
        QPointF center = r.center();
        painter.drawLine(center, QPointF(center.x(), r.top() + r.height() * 0.24));
        painter.drawLine(center, QPointF(center.x() + r.width() * 0.20, center.y()));
        break;
    }
    case Kind::Warning: {
        QPainterPath tri;
        tri.moveTo(r.center().x(), r.top());
        tri.lineTo(r.right(), r.bottom());
        tri.lineTo(r.left(), r.bottom());
        tri.closeSubpath();
        painter.drawPath(tri);

        painter.drawLine(QPointF(r.center().x(), r.top() + r.height() * 0.38),
                          QPointF(r.center().x(), r.top() + r.height() * 0.68));
        painter.setBrush(strokeColor);
        painter.drawEllipse(QPointF(r.center().x(), r.top() + r.height() * 0.80), 1.6, 1.6);
        break;
    }
    }
}
