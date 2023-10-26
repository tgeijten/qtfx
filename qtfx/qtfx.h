#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QFont>

QWidget* createVBoxWidget( QWidget* parent, int margin = 4, int spacing = 4 );
QWidget* createHBoxWidget( QWidget* parent, int margin = 4, int spacing = 4 );
QFont getMonospaceFont( int pointSize = -1, int weight = -1 );

void cycleTabWidget( QTabWidget* wdg, int offset = 1 );

bool darkMode();
Qt::GlobalColor textColor( Qt::GlobalColor );