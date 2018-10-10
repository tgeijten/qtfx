#include "QSettingsItemModel.h"
#include "xo/container/prop_node_tools.h"

using xo::prop_node;
using xo::settings;

std::string get_item_id( const QModelIndex& index )
{
	QString id = index.data().toString();
	for ( auto idx = index.parent(); idx.isValid(); idx = idx.parent() )
		id = idx.data().toString() + "." + id;
	return id.toStdString();
}

std::pair< int, const prop_node* > find_parent_node( const prop_node* pn, const prop_node* child )
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

QModelIndex QSettingsItemModel::index( int row, int column, const QModelIndex &parent ) const
{
	if ( parent.isValid() )
	{
		auto* pn = (prop_node*)parent.internalPointer();
		auto* ch = &pn->get_child( row );
		return createIndex( row, column, (void*)ch );
	}
	else return createIndex( row, column, (void*)&settings_.schema().get_child( row ) );
}

QModelIndex QSettingsItemModel::parent( const QModelIndex &child ) const
{
	prop_node* pn = (prop_node*)child.internalPointer();
	auto parent = find_parent_node( &settings_.schema(), pn );
	if ( parent.second != &settings_.schema() )
		return createIndex( parent.first, 0, (void*)parent.second );
	else return QModelIndex();
}

int QSettingsItemModel::rowCount( const QModelIndex &parent ) const
{
	if ( parent.isValid() )
	{
		auto* pn = reinterpret_cast<prop_node*>( parent.internalPointer() );
		if ( pn->has_key( "default" ) )
			return 0;
		else return (int)pn->size();
	}
	else return (int)settings_.schema().size();
}

int QSettingsItemModel::columnCount( const QModelIndex &parent ) const
{
	return 3;
}

QVariant QSettingsItemModel::data( const QModelIndex &index, int role ) const
{
	if ( role == Qt::DisplayRole || role == Qt::EditRole )
	{
		auto* pn = reinterpret_cast<prop_node*>( index.internalPointer() );
		auto parent = find_parent_node( &settings_.schema(), pn );

		if ( index.column() == 0 )
		{
			auto parent_key = parent.second->get_key( parent.first );
			return QVariant( parent_key.c_str() );
		}
		else if ( index.column() == 1 && pn->has_key( "label" ) )
		{
			return QVariant( QString( pn->get< std::string >( "label" ).c_str() ) );
		}
		else if ( index.column() == 2 && pn->has_key( "default" ) )
		{
			auto id = xo::find_query_to_node( &settings_.schema(), pn ).second;
			return QVariant( QString( settings_.get< std::string >( id ).c_str() ) );
		}
	}

	return QVariant();
}

bool QSettingsItemModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
	if ( role == Qt::EditRole )
	{
		auto* pn = reinterpret_cast<prop_node*>( index.internalPointer() );
		auto id = xo::find_query_to_node( &settings_.schema(), pn ).second;
		auto id_check = get_item_id( index );
		settings_.set( id, value.toString().toStdString() );
		return true;
	}
	else return false;
}

Qt::ItemFlags QSettingsItemModel::flags( const QModelIndex &index ) const
{
	if ( !index.isValid() )
		return 0;
	else if ( index.column() == 2 )
		return Qt::ItemIsEditable | QAbstractItemModel::flags( index );
	else return QAbstractItemModel::flags( index );
}
