#pragma once

#include <QWidget>

class QGroup : public QWidget
{
public:
	QGroup( QWidget* parent );
	void add( QWidget* w );
	QGroup& operator<<( QWidget* w );
	void clear();
};

class QVGroup : public QGroup
{
public:
	QVGroup( QWidget* parent, int margin = 4, int spacing = 4 );
};

class QHGroup : public QGroup
{
public:
	QHGroup( QWidget* parent, int margin = 4, int spacing = 4 );
};

class QFormGroup : public QGroup
{
public:
	QFormGroup( QWidget* parent, int margin = 4, int spacing = 4 );
	void addRow( const QString& label, QWidget* w );
};
