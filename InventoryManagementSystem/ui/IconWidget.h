#pragma once

#include <QWidget>
#include <QPixmap>
#include <QString>

// A small square icon used on Dashboard stat cards (and available for reuse
// elsewhere, e.g. KDS column headers).
//
// How icon sourcing works:
//   1. Icon files live in resources/icons/ and are compiled straight into
//      the app binary via resources/resources.qrc (same mechanism already
//      used for the app's stylesheet). This is deliberate: Qt Creator (and
//      most IDEs) build into a separate "shadow build" folder rather than
//      the source folder, so any path relative to the running .exe's
//      working directory is unreliable - compiling icons in avoids that
//      entirely, they just work regardless of where the exe runs from.
//   2. On construction, IconWidget tries to load ":/icons/<fileName>" from
//      that compiled-in resource.
//   3. If that resource is missing (nothing has been added to the .qrc yet,
//      or the file failed to load), IconWidget falls back to a small
//      hand-drawn vector icon for the same concept, so the UI never shows
//      a blank box or a missing-image glyph.
//
// To change an icon: replace the file in resources/icons/ (keep the same
// file name) and REBUILD the project - because the icon is compiled into
// the binary, a rebuild is required for the change to show up (a restart
// alone is not enough, unlike a plain-disk-file approach).
class IconWidget : public QWidget
{
    Q_OBJECT

public:
    enum class Kind { Box, Menu, Receipt, Clock, Warning };

    // fileName is just the file name (e.g. "box.png"), resolved against
    // the compiled-in ":/icons/" resource prefix.
    explicit IconWidget(Kind kind, const QString &fileName, QWidget *parent = nullptr);

    QSize sizeHint() const override;

    // True if a real icon resource was found and loaded (false = using fallback).
    bool usingRealIcon() const { return !realIcon.isNull(); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawFallback(class QPainter &painter, const QRect &rect) const;

    Kind kind;
    QPixmap realIcon; // null if no real icon resource was found
};
