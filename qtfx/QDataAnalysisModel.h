#pragma once

#include <vector>
#include <utility>

#include <QString>

#include "xo/container/storage_tools.h"
#include "xo/xo_types.h"

class QDataAnalysisModel
{
public:
	using Series = std::vector< std::pair< float, float > >;

	QDataAnalysisModel() {}
	virtual ~QDataAnalysisModel() {}

	virtual int channelCount() const = 0;
	virtual int frameCount() const = 0;
	virtual QString label( int channel ) const = 0;
	virtual double value( int channel, double time ) const = 0;
	virtual double value( int channel, int frame ) const = 0;

	virtual Series getSeries( int channel, double min_interval = 0.0 ) const = 0;

	virtual double timeStart() const { return 0.0; }
	virtual double timeFinish() const { return 0.0; }
	virtual int timeIndex( double time ) const = 0;
	virtual double timeValue( int frame ) const = 0;

	bool hasData() const { return channelCount() > 0; }
};

template< typename T >
class StorageDataAnalysisModel : public QDataAnalysisModel
{
public:
	StorageDataAnalysisModel( const xo::storage< T >* s = nullptr ) : sto_( s ) {}
	void setStorage( const xo::storage< T >* s ) { sto_ = s; }

	virtual int channelCount() const override { return sto_->empty() ? 0 : sto_->channel_size(); }
	virtual QString label( int idx ) const override { return QString( sto_->get_label( idx ).c_str() ); }
	virtual double value( int channel, double time ) const override { return ( *sto_ )( timeIndex( time ), channel ); }
	virtual double value( int channel, int frame ) const override { return ( *sto_ )( frame, channel ); }
	virtual Series getSeries( int idx, double min_interval = 0.0 ) const override;

	virtual double timeStart() const override { return sto_->empty() ? 0.0 : sto_->front()[ 0 ]; }
	virtual double timeFinish() const override { return sto_->empty() ? 0.0 : sto_->back()[ 0 ]; }
	virtual int timeIndex( double time ) const override { return xo::find_frame_index( *sto_, float( time ), 0 ); }
	virtual double timeValue( int idx ) const override { return ( *sto_ )( idx, 0 ); }

private:
	const xo::storage< T >* sto_;
};

template< typename T >
QDataAnalysisModel::Series StorageDataAnalysisModel<T>::getSeries( int idx, double min_interval ) const
{
	std::vector< std::pair< float, float > > series;
	if ( sto_->channel_size() < 1 )
		return series;
	series.reserve( sto_->frame_size() );
	for ( size_t i = 0; i < sto_->frame_size(); ++i )
		series.emplace_back( static_cast<float>( ( *sto_ )( i, 0 ) ), static_cast<float>( ( *sto_ )( i, idx ) ) );
	return series;
}
