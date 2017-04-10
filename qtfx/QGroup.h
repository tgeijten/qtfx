#pragma once

#include <QWidget>

class QGroup : public QWidget
{
	Q_OBJECT

public:
	QGroup( QWidget* parent );
	void add( QWidget* w );
	QGroup& operator<<( QWidget* w );
};

class QVGroup : public QGroup
{
	Q_OBJECT

public:
	QVGroup( QWidget* parent, int margin = 4, int spacing = 4 );
};

class QHGroup : public QGroup
{
	Q_OBJECT

public:
	QHGroup( QWidget* parent, int margin = 4, int spacing = 4 );
};
