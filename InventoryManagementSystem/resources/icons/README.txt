HOW TO CHANGE THE DASHBOARD ICONS
========================================

The icons you drop in this folder are compiled directly into the app
(via resources/resources.qrc), NOT loaded from disk at runtime. This is
because IDEs like Qt Creator build into a separate "shadow build" folder
(e.g. build/Desktop_Qt_6_.../Debug/) rather than this source folder, and
the app actually runs from there - so a plain disk-relative path never
finds files sitting here. Compiling the icons into the binary sidesteps
that problem entirely; they'll work no matter which folder the .exe runs
from.

WHAT THIS MEANS FOR YOU
    To change an icon: replace the PNG file in this folder (keep the
    exact same file name), then REBUILD the project. A restart alone is
    NOT enough - Qt Creator: Build > Rebuild Project (or just Build, since
    it will notice resources.qrc's dependency changed and re-embed it).

REQUIRED FILENAMES (already in place if you followed the earlier steps)
    box.png       -> "Total Products" card
    menu.png      -> "Menu Items" card
    receipt.png   -> "Total Orders" card
    clock.png     -> "Active in Kitchen" card
    warning.png   -> "Low Stock Items" card + "Low Stock Alerts" panel heading

FILE SPEC (for a crisp, consistent look)
    Format:     PNG, transparent background
    Size:       at least 64x64 px (the app displays them at 32x32; your
                current files are 512x512, which is plenty)
    Style:      single flat color, simple line/outline icon works best
                next to the colored accent bar on each card - avoid
                multi-color icons, they'll clash

ADDING A NEW ICON FILENAME (not just replacing one of the 5 above)
    If you ever want a 6th icon with a new filename, you also need to
    add one line to resources/resources.qrc, e.g.:
        <file alias="newicon.png">icons/newicon.png</file>
    inside the <qresource prefix="/icons"> block. The 5 icons above are
    already wired up, so you only need this step for something new.

WHAT HAPPENS IF A FILE IS MISSING
    IconWidget falls back to a small hand-drawn vector icon for that
    concept if the compiled-in resource isn't found - the app never
    shows a broken image, it just uses the placeholder.
