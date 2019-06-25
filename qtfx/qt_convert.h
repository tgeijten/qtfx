#pragma once

#include <QString>
#include <string>
#include "xo/filesystem/path.h"
#include "QColor"
#include "xo/utility/color.h"

inline QString to_qt( const std::string& s ) { return QString( s.c_str() ); }
inline QString to_qt( const xo::path& s ) { return QString( s.str().c_str() ); }
inline QColor to_qt( const xo::color& c ) { return QColor( 255 * c.r, 255 * c.g, 255 * c.b ); }
inline xo::path path_from_qt( const QString& f ) { return xo::path( f.toStdString() ); }
