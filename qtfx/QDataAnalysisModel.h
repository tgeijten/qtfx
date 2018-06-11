#pragma once

#include <vector>
#include <utility>

#include "QString"

typedef std::vector< std::pair< float, float > > DataSeries;

class QDataAnalysisModel
{
public:
	QDataAnalysisModel() {}
	virtual ~QDataAnalysisModel() {}

	virtual size_t getSeriesCount() const = 0;
	virtual QString getLabel( int idx ) const = 0;
	virtual double getValue( int idx, double time ) const = 0;
	virtual DataSeries getSeries( int idx, double min_interval = 0.0 ) const = 0;
	virtual double getTimeStart() const { return 0.0; }
	virtual double getTimeFinish() const { return 0.0; }
};

template< typename T >
class StorageDataAnalysisModel : public QDataAnalysisModel
{
public:
	StorageDataAnalysisModel( const xo::storage< T >* s = nullptr ) : sto_( s ) {}

	void setStorage( const xo::storage< T >* s ) { sto_ = s; }
	virtual QString getLabel( int idx ) const override { return QString( sto_->get_label( idx + 1 ).c_str() ); }
	virtual DataSeries getSeries( int idx, double min_interval = 0.0 ) const override {
		std::vector< std::pair< float, float > > series;
		if ( sto_->channel_size() < 2 )
			return series;
		series.reserve( sto_->frame_size() );
		for ( size_t i = 0; i < sto_->frame_size(); ++i )
			series.emplace_back( static_cast<float>( (*sto_)( i, 0 ) ), static_cast<float>( ( *sto_ )( i, idx + 1 ) ) );
		return series;
	}
	virtual double getTimeFinish() const override { return sto_->empty() ? 0.0 : sto_->back()[ 0 ]; }
	virtual double getTimeStart() const override { return sto_->empty() ? 0.0 : sto_->front()[ 0 ]; }
	virtual double getValue( int idx, double time ) const override {
		auto frequency = double( sto_->frame_size() - 1 ) / ( sto_->back()[ 0 ] - sto_->front()[ 0 ] );
		int frame_idx = xo::clamped< int >( round( time * frequency ), 0, sto_->frame_size() - 1 );
		return (*sto_)( frame_idx, idx + 1 );
	}
	virtual size_t getSeriesCount() const override { return sto_->empty() ? 0 : sto_->channel_size() - 1; }
private:
	const xo::storage< T >* sto_;
};
