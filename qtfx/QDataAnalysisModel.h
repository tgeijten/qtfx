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
	virtual size_t getVariableCount() const = 0;
	virtual double getTimeStart() const { return 0.0; }
	virtual double getTimeFinish() const { return 0.0; }
	virtual QString getLabel( int idx ) const = 0;
	virtual double getValue( int idx, double time ) const = 0;
	virtual DataSeries getSeries( int idx, double min_interval = 0.0 ) const = 0;
};

template< typename T >
class StorageDataAnalysisModel : public QDataAnalysisModel
{
public:
	StorageDataAnalysisModel( const flut::storage< T >& s, double freq ) : storage( s ), frequency( freq ) {}
	const flut::storage< T >& storage;
	double frequency;
	virtual QString getLabel( int idx ) const override { return QString( storage.get_label( idx ).c_str() ); }
	virtual DataSeries getSeries( int idx, double min_interval = 0.0 ) const override {
		std::vector< std::pair< float, float > > series;
		size_t d = std::min< size_t >( 1, static_cast< size_t >( min_interval / ( 1 / frequency ) ) );
		series.reserve( storage.frame_size() / d );
		for ( size_t i = 0; i < storage.frame_size(); i += d ) {
			series.emplace_back( static_cast<float>( i / frequency ), static_cast<float>( storage( i, idx ) ) );
		}
		return series;
	}
	virtual double getTimeFinish() const override { return storage.frame_size() / frequency; }
	virtual double getTimeStart() const override { return 0.0; }
	virtual double getValue( int idx, double time ) const override {
		int frame_idx = flut::math::clamped< int >( round( time * frequency ), 0, storage.frame_size() - 1 );
		return storage( frame_idx, idx );
	}

	virtual size_t getVariableCount() const override { return storage.channel_size(); }
};
