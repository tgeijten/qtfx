#pragma once

#include <QString>
#include <string>
#include "xo/filesystem/path.h"
#include "QColor"
#include "xo/utility/color.h"

inline QString to_qt( const std::string& s ) { return QString( s.c_str() ); }
inline QString to_qt( const xo::path& s ) { return QString( s.string().c_str() ); }
inline QColor to_qt( const xo::color& c ) { return QColor( 255 * c.r, 255 * c.g, 255 * c.b ); }

namespace xo {
	template<> struct string_cast<QString, void> {
		static QString from( const string& value ) { return QString( value.c_str() ); }
		static string to( const QString& s ) { return s.toStdString(); }
	};
}
