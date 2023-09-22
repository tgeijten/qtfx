#pragma once

#include "QAbstractItemModel"
#include "xo/system/settings.h"

class QSettingsItemModel : public QAbstractItemModel
{
public:
	QSettingsItemModel( xo::settings& pn ) : QAbstractItemModel(), settings_( pn ) {}
	virtual ~QSettingsItemModel() {}

	virtual QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const override;
	virtual QModelIndex parent( const QModelIndex& child ) const override;
	virtual int rowCount( const QModelIndex& parent = QModelIndex() ) const override;
	virtual int columnCount( const QModelIndex& parent = QModelIndex() ) const override;
	virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;
	virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole ) override;
	virtual Qt::ItemFlags flags( const QModelIndex& index ) const override;
	virtual QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;

private:
	xo::settings& settings_;
};
