#pragma once

#include "QAbstractItemModel"
#include "xo\container\prop_node.h"

class QPropNodeItemModel : public QAbstractItemModel
{
public:
	QPropNodeItemModel( xo::prop_node& pn ) : QAbstractItemModel(), props_( pn ) {}
	virtual ~QPropNodeItemModel() {}

	Q_INVOKABLE virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
	Q_INVOKABLE virtual QModelIndex parent( const QModelIndex &child ) const override;
	Q_INVOKABLE virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
	Q_INVOKABLE virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
	Q_INVOKABLE virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
	Q_INVOKABLE virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

private:
	xo::prop_node& props_;
};
