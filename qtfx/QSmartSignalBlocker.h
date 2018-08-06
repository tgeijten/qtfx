#pragma once

template< typename T >
class QSmartSignalBlocker
{
public:
	T* operator->() { return widget_; }

	QSmartSignalBlocker( T* w ) : widget_( w ) {
		blockState_ = widget_->blockSignals( true );
	}
	~QSmartSignalBlocker() {
		widget_->blockSignals( blockState_ );
	}
private:
	T* widget_;
	bool blockState_;
};

template< typename T > QSmartSignalBlocker< T > qt_block_signals( T* w ) { return QSmartSignalBlocker< T >( w ); }
