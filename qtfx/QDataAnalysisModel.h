#pragma once

#include <vector>
#include <utility>

#include "QString"
#include "xo/container/storage_tools.h"

typedef std::vector< std::pair< float, float > > DataSeries;

class QDataAnalysisModel
{
public:
	QDataAnalysisModel() {}
	virtual ~QDataAnalysisModel() {}

	virtual size_t seriesCount() const = 0;
	virtual QString label( int idx ) const = 0;
	virtual double value( int idx, double time ) const = 0;
	virtual DataSeries getSeries( int idx, double min_interval = 0.0 ) const = 0;
	virtual double timeStart() const { return 0.0; }
	virtual double timeFinish() const { return 0.0; }
};

template< typename T >
class StorageDataAnalysisModel : public QDataAnalysisModel
{
public:
	StorageDataAnalysisModel( const xo::storage< T >* s = nullptr ) : sto_( s ) {}

	void setStorage( const xo::storage< T >* s ) { sto_ = s; }
	virtual QString label( int idx ) const override { return QString( sto_->get_label( idx ).c_str() ); }
	virtual DataSeries getSeries( int idx, double min_interval = 0.0 ) const override {
		std::vector< std::pair< float, float > > series;
		if ( sto_->channel_size() < 1 )
			return series;
		series.reserve( sto_->frame_size() );
		for ( size_t i = 0; i < sto_->frame_size(); ++i )
			series.emplace_back( static_cast<float>( (*sto_)( i, 0 ) ), static_cast<float>( ( *sto_ )( i, idx ) ) );
		return series;
	}
	virtual double timeFinish() const override { return sto_->empty() ? 0.0 : sto_->back()[ 0 ]; }
	virtual double timeStart() const override { return sto_->empty() ? 0.0 : sto_->front()[ 0 ]; }
	virtual double value( int channel, double time ) const override {
		// TODO: make this more efficient with binary search (and use iterators)
		auto frame_idx = xo::find_frame_index( *sto_, float( time ), 0 );
		return (*sto_)( frame_idx, channel );
	}
	virtual size_t seriesCount() const override { return sto_->empty() ? 0 : sto_->channel_size(); }
private:
	const xo::storage< T >* sto_;
};
