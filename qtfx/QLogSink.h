#pragma once

#include <QPlainTextEdit>
#include <QThread>

#include "flut/system/log_sink.hpp"
#include "flut/system/types.hpp"

class QLogSink : public QPlainTextEdit, public flut::log::sink
{
	Q_OBJECT

public:
	QLogSink( QWidget* parent = 0, flut::log::level level = flut::log::info_level );
	virtual ~QLogSink() {}

	virtual void send_log_message( flut::log::level l, const flut::string& msg ) override;

	bool enabled() const { return enabled_; }
	void enabled( bool val ) { enabled_ = val; }

public slots:
	void update();

private:
	void append_message( flut::log::level l, const flut::string& msg );
	bool enabled_;
	Qt::HANDLE qt_thread_id_;

	std::vector< std::pair< flut::log::level, std::string > > buffer_data_;
	QMutex buffer_mutex_;
	QTimer update_timer_;
};
