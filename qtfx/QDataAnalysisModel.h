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
	StorageDataAnalysisModel( const xo::storage< T >& s ) : storage( s ) {}
	const xo::storage< T >& storage;
	virtual QString getLabel( int idx ) const override { return QString( storage.get_label( idx ).c_str() ); }
	virtual DataSeries getSeries( int idx, double min_interval = 0.0 ) const override {
		std::vector< std::pair< float, float > > series;
		if ( storage.channel_size() < 2 )
			return series;

		series.reserve( storage.frame_size() );
		for ( size_t i = 0; i < storage.frame_size(); ++i ) {
			series.emplace_back( static_cast<float>( storage( i, 0 ) ), static_cast<float>( storage( i, idx + 1 ) ) );
		}
		return series;
	}
	virtual double getTimeFinish() const override { return storage[ 0 ]; }
	virtual double getTimeStart() const override { return storage( 0, 0 ); }
	virtual double getValue( int idx, double time ) const override {
		auto frequency = double( storage.frame_size() - 1 ) / storage( storage.frame_size() - 1, 0 ) - storage( 0, 0 );
		int frame_idx = xo::clamped< int >( round( time * frequency ), 0, storage.frame_size() - 1 );
		return storage( frame_idx, idx );
	}

	virtual size_t getSeriesCount() const override { return storage.empty() ? 0 : storage.channel_size() - 1; }
};
