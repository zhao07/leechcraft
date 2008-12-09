#include "favoritesmodel.h"
#include <algorithm>
#include <QTimer>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "filtermodel.h"
#include "core.h"

bool FavoritesModel::FavoritesItem::operator== (const FavoritesModel::FavoritesItem& item) const
{
	return Title_ == item.Title_ &&
		URL_ == item.URL_ &&
		Tags_ == item.Tags_;
}

FavoritesModel::FavoritesModel (QObject *parent)
: QAbstractItemModel (parent)
{
	ItemHeaders_ << tr ("Title")
		<< tr ("URL")
		<< tr ("Tags");
	QTimer::singleShot (0, this, SLOT (loadData ()));
}

FavoritesModel::~FavoritesModel ()
{
}

int FavoritesModel::columnCount (const QModelIndex&) const
{
	return ItemHeaders_.size ();
}

QVariant FavoritesModel::data (const QModelIndex& index, int role) const
{
    if (!index.isValid ())
        return QVariant ();

	switch (role)
	{
		case Qt::DisplayRole:
			switch (index.column ())
			{
				case ColumnTitle:
					return Items_ [index.row ()].Title_;
				case ColumnURL:
					return Items_ [index.row ()].URL_;
				case ColumnTags:
					return Items_ [index.row ()].Tags_.join (" ");
				default:
					return QVariant ();
			}
		case TagsRole:
			return Items_ [index.row ()].Tags_;
		default:
			return QVariant ();
	}
}

Qt::ItemFlags FavoritesModel::flags (const QModelIndex& index) const
{
	Qt::ItemFlags result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	if (index.column () == ColumnTags)
		result |= Qt::ItemIsEditable;
	return result;
}

QVariant FavoritesModel::headerData (int column, Qt::Orientation orient,
		int role) const
{
    if (orient == Qt::Horizontal && role == Qt::DisplayRole)
        return ItemHeaders_.at (column);
    else
        return QVariant ();
}

QModelIndex FavoritesModel::index (int row, int column,
		const QModelIndex& parent) const
{
    if (!hasIndex (row, column, parent))
        return QModelIndex ();

    return createIndex (row, column);
}

QModelIndex FavoritesModel::parent (const QModelIndex&) const
{
    return QModelIndex ();
}

int FavoritesModel::rowCount (const QModelIndex& index) const
{
	return index.isValid () ? 0 : Items_.size ();
}

bool FavoritesModel::setData (const QModelIndex& index,
		const QVariant& value, int)
{
	if (index.column () != ColumnTags)
		return false;

	Items_ [index.row ()].Tags_ = value.toStringList ();
	Core::Instance ().GetStorageBackend ()->UpdateFavorites (Items_ [index.row ()]);
	return true;
}

void FavoritesModel::AddItem (const QString& title, const QString& url,
	   const QStringList& tags)
{
	FavoritesItem item =
	{
		title,
		url,
		tags
	};

	Core::Instance ().GetStorageBackend ()->AddToFavorites (item);
}

void FavoritesModel::removeItem (const QModelIndex& index)
{
	Core::Instance ().GetStorageBackend ()->RemoveFromFavorites (Items_ [index.row ()]);
}

void FavoritesModel::handleItemAdded (const FavoritesModel::FavoritesItem& item)
{
	beginInsertRows (QModelIndex (), 0, 0);
	Items_.push_back (item);
	endInsertRows ();
}

namespace
{
	struct ItemFinder
	{
		const QString& URL_;

		ItemFinder (const QString& url)
		: URL_ (url)
		{
		}

		bool operator() (const FavoritesModel::FavoritesItem& item) const
		{
			return item.URL_ == URL_;
		}
	};
};

void FavoritesModel::handleItemUpdated (const FavoritesModel::FavoritesItem& item)
{
	items_t::iterator pos =
		std::find_if (Items_.begin (), Items_.end (), ItemFinder (item.URL_));
	if (pos == Items_.end ())
	{
		qWarning () << Q_FUNC_INFO << "not found updated item";
		return;
	}

	*pos = item;

	int n = std::distance (Items_.begin (), pos);

	emit dataChanged (index (n, 2), index (n, 2));
}

void FavoritesModel::handleItemRemoved (const FavoritesModel::FavoritesItem& item)
{
	items_t::iterator pos =
		std::find (Items_.begin (), Items_.end (), item);
	if (pos == Items_.end ())
	{
		qWarning () << Q_FUNC_INFO << "not found removed item";
		return;
	}

	int n = std::distance (Items_.begin (), pos);
	beginRemoveRows (QModelIndex (), n, n);
	Items_.erase (pos);
	endRemoveRows ();
}

void FavoritesModel::loadData ()
{
	items_t items;
	Core::Instance ().GetStorageBackend ()->LoadFavorites (items);

	if (!items.size ())
		return;

	beginInsertRows (QModelIndex (), 0, items.size () - 1);
	for (items_t::const_reverse_iterator i = items.rbegin (),
			end = items.rend (); i != end; ++i)
		Items_.push_back (*i);
	endInsertRows ();
}

