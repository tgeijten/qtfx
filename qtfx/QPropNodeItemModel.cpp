#include "QPropNodeItemModel.h"
#include "xo/container/prop_node_tools.h"
#include "xo/system/log.h"

using xo::prop_node;

QPropNodeItemModel::QPropNodeItemModel( QObject* parent ) :
	QAbstractItemModel( parent ),
	props_(),
	default_icon_(),
	max_preview_children_()
{}

void QPropNodeItemModel::setData( const xo::prop_node& pn )
{
	beginResetModel();
	props_ = pn;
	endResetModel();
}

void QPropNodeItemModel::setData( xo::prop_node&& pn )
{
	beginResetModel();
	props_ = std::move( pn );
	endResetModel();
}

void QPropNodeItemModel::setDefaultIcon( const QIcon& icon )
{
	default_icon_ = icon;
}

QModelIndex QPropNodeItemModel::index( int row, int column, const QModelIndex& parent ) const
{
	if ( parent.isValid() )
	{
		if ( auto* pn = reinterpret_cast<prop_node*>( parent.internalPointer() ) )
			return createIndex( row, column, reinterpret_cast<void*>( &pn->get_child( row ) ) );
		else return QModelIndex();
	}
	else if ( row < props_.size() )
	{
		return createIndex( row, column, (void*)&props_.get_child( row ) );
	}
	else
	{
		xo::log::debug( "Invalid model index, row=", row, " column=", column, " parent=", parent.internalPointer() );
		return QModelIndex();
	}
}

QModelIndex QPropNodeItemModel::parent( const QModelIndex& child ) const
{
	if ( prop_node* pn = reinterpret_cast<prop_node*>( child.internalPointer() ) )
	{
		if ( auto p = props_.try_find_parent( *pn ); p.first ) {
			if ( auto pp = props_.try_find_parent( *p.first ); pp.first )
				return createIndex( pp.second, 0, (void*)p.first );
		}
	}

	return QModelIndex();
}

int QPropNodeItemModel::rowCount( const QModelIndex& parent ) const
{
	if ( parent.isValid() )
	{
		auto* pn = reinterpret_cast<prop_node*>( parent.internalPointer() );
		return pn->size();
	}
	else return props_.size();
}

int QPropNodeItemModel::columnCount( const QModelIndex& parent ) const
{
	return 2;
}

QVariant QPropNodeItemModel::data( const QModelIndex& index, int role ) const
{
	auto* pn = reinterpret_cast<prop_node*>( index.internalPointer() );
	if ( role == Qt::DisplayRole || role == Qt::EditRole )
	{
		if ( index.column() == 0 )
		{
			if ( auto parent = props_.try_find_parent( *pn ); parent.first )
				return QVariant( parent.first->get_key( parent.second ).c_str() );
			else return QVariant();
		}
		else
		{
			if ( !pn->get_str().empty() )
				return QVariant( QString( pn->get_str().c_str() ) );
			else if ( pn->size() > 0 && pn->count_children() <= max_preview_children_ )
				return QVariant( QString( make_str_from_prop_node( *pn ).c_str() ) );
			else return QVariant();
		}
	}
	else if ( role == Qt::DecorationRole )
	{
		// show icon if the pn has a value
		if ( index.column() == 0 && !default_icon_.isNull() && pn && !pn->get_str().empty() )
			return default_icon_;
		else return QVariant();
	}
	else return QVariant();
}

QVariant QPropNodeItemModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
	static const QStringList column_names = { "Property", "Value" };
	if ( role == Qt::DisplayRole && orientation == Qt::Horizontal )
		return column_names[section];
	else return QVariant();
}

bool QPropNodeItemModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
	if ( role == Qt::EditRole )
	{
		auto* pn = reinterpret_cast<prop_node*>( index.internalPointer() );
		pn->set_value( value.toString().toStdString() );
		return true;
	}
	else return false;
}

Qt::ItemFlags QPropNodeItemModel::flags( const QModelIndex& index ) const
{
	if ( !index.isValid() )
		return 0;
	else if ( index.column() == 1 )
		return Qt::ItemIsEditable | QAbstractItemModel::flags( index );
	else return QAbstractItemModel::flags( index );
}
