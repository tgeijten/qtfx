#pragma once

#include <QPlainTextEdit>
#include "flut/system/log_sink.hpp"
#include "flut/system/types.hpp"

class QLogSink : public QPlainTextEdit, flut::log::sink
{
public:
	QLogSink( QWidget* parent = 0, flut::log::level level = flut::log::info_level );
	virtual ~QLogSink() {}

	virtual void send_log_message( flut::log::level l, const flut::string& msg ) override;

	bool enabled() const { return enabled_; }
	void enabled( bool val ) { enabled_ = val; }

private:
	bool enabled_;

};
