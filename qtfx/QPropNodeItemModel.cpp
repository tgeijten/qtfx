#include "QPropNodeItemModel.h"
#include "xo\container\prop_node_tools.h"
#include "xo\system\log.h"

using xo::prop_node;

std::pair< int, prop_node* > find_parent_node( prop_node* pn, prop_node* child )
{
	for ( int row = 0; row < pn->size(); ++row )
	{
		if ( &pn->get_child( row ) == child )
			return std::make_pair( row, pn );
	}

	for ( auto& child_pair : *pn )
	{
		auto result = find_parent_node( &child_pair.second, child );
		if ( result.second )
			return result;
	}

	return std::pair< int, prop_node* >( -1, nullptr );
}

QModelIndex QPropNodeItemModel::index( int row, int column, const QModelIndex &parent ) const
{
	if ( parent.isValid() )
	{
		auto* pn = ( prop_node* )parent.internalPointer();
		auto* ch = &pn->get_child( row );
		//xo::log::trace( "Creating index for ", ch->get_value() );
		return createIndex( row, column, (void*)ch );
	}
	else return createIndex( row, column, ( void* )&props_.get_child( row ) );
}

QModelIndex QPropNodeItemModel::parent( const QModelIndex &child ) const
{
	prop_node* pn = ( prop_node* )child.internalPointer();
	if ( pn != &props_ )
	{
		auto parent = find_parent_node( &props_, pn );
		return createIndex( parent.first, 0, ( void* )parent.second );
	}
	else return QModelIndex();
}

int QPropNodeItemModel::rowCount( const QModelIndex &parent /*= QModelIndex() */ ) const
{
	if ( parent.isValid() )
	{
		auto* pn = reinterpret_cast< prop_node* >( parent.internalPointer() );
		//xo::log::trace( "rowCount=", pn->size() );
		return pn->size();
	}
	else return props_.size();
}

int QPropNodeItemModel::columnCount( const QModelIndex &parent /*= QModelIndex() */ ) const
{
	return 2;
}

QVariant QPropNodeItemModel::data( const QModelIndex &index, int role /*= Qt::DisplayRole */ ) const
{
	if ( role == Qt::DisplayRole || role == Qt::EditRole )
	{
		auto* pn = reinterpret_cast< prop_node* >( index.internalPointer() );
		if ( index.column() == 0 )
			return QVariant( QString( "Label" ) );
		else return QVariant( QString( pn->get_value().c_str() ) );
	}
	else return QVariant();
}

bool QPropNodeItemModel::setData( const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole */ )
{
	if ( role == Qt::EditRole )
	{
		auto str = value.toString().toStdString();
		xo::log::trace( "Setting value to ", str );
		return true;
	}
	else return false;
}

Qt::ItemFlags QPropNodeItemModel::flags( const QModelIndex &index ) const
{
	if ( !index.isValid() )
		return 0;
	else if ( index.column() == 1 )
		return Qt::ItemIsEditable | QAbstractItemModel::flags( index );
	else return QAbstractItemModel::flags( index );
}
