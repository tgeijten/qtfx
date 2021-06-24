#pragma once

#include <QWidget>

inline bool isVisible( QWidget* w ) { return w && w->isVisible() && w->visibleRegion().isEmpty(); }
