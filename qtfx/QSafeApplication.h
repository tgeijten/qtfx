#pragma once

#include <QApplication>
#include "QMessageBox"

class QSafeApplication : public QApplication
{
public:
	QSafeApplication::QSafeApplication( int& argc, char** argv ) :
		QApplication( argc, argv )
	{}

	bool notify( QObject* receiver, QEvent* event ) override
	{
		try
		{
			return QApplication::notify( receiver, event );
		}
		catch ( std::exception& e )
		{
			QMessageBox::critical( nullptr, "Error", QString( e.what() ) );
		}
		return false;
	}
};
