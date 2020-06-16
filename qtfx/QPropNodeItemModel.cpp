#include "QPropNodeItemModel.h"
#include "xo/container/prop_node_tools.h"
#include "xo/system/log.h"

using xo::prop_node;

static std::pair< int, const prop_node* > find_parent_node( const prop_node* pn, const prop_node* child )
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

void QPropNodeItemModel::setData( const xo::prop_node& pn )
{
	beginResetModel();
	props_ = pn;
	endResetModel();
}

QModelIndex QPropNodeItemModel::index( int row, int column, const QModelIndex &parent ) const
{
	if ( parent.isValid() )
	{
		auto* pn = (prop_node*)parent.internalPointer();
		auto* ch = &pn->get_child( row );
		return createIndex( row, column, (void*)ch );
	}
	else if ( row < props_.size() )
	{
		return createIndex( row, column, (void*)&props_.get_child( row ) );
	}
	else
	{
		xo::log::debug( "Invalid model index, row=", row, " column=", column );
		return QModelIndex();
	}
}

QModelIndex QPropNodeItemModel::parent( const QModelIndex &child ) const
{
	prop_node* pn = ( prop_node* )child.internalPointer();
	auto parent = find_parent_node( &props_, pn );
	if ( parent.second != &props_ )
		return createIndex( parent.first, 0, ( void* )parent.second );
	else return QModelIndex();
}

int QPropNodeItemModel::rowCount( const QModelIndex &parent ) const
{
	if ( parent.isValid() )
	{
		auto* pn = reinterpret_cast< prop_node* >( parent.internalPointer() );
		return pn->size();
	}
	else return props_.size();
}

int QPropNodeItemModel::columnCount( const QModelIndex &parent ) const
{
	return 2;
}

QVariant QPropNodeItemModel::data( const QModelIndex &index, int role ) const
{
	if ( role == Qt::DisplayRole || role == Qt::EditRole )
	{
		auto* pn = reinterpret_cast< prop_node* >( index.internalPointer() );
		if ( index.column() == 0 )
		{
			auto parent = find_parent_node( &props_, pn );
			return QVariant( parent.second->get_key( parent.first ).c_str() );
		}
		else return QVariant( QString( pn->raw_value().c_str() ) );
	}
	else return QVariant();
}

QVariant QPropNodeItemModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	static const QStringList column_names = { "Property", "Value" };
	if ( role == Qt::DisplayRole && orientation == Qt::Horizontal )
		return column_names[ section ];
	else return QVariant();
}

bool QPropNodeItemModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
	if ( role == Qt::EditRole )
	{
		auto* pn = reinterpret_cast< prop_node* >( index.internalPointer() );
		pn->set_value( value.toString().toStdString() );
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
