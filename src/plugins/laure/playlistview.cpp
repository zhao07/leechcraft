/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "playlistview.h"
#include <QKeyEvent>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QMenu>
#include "nowplayingdelegate.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Laure
{	
	PlayListView::PlayListView (QStandardItemModel *model, QWidget *parent)
	: QTreeView (parent)
	, PlayListModel_ (model)
	, CurrentItem_ (-1)
	{
		setModel (PlayListModel_);
		
		PlayListModel_->setColumnCount (PlayListColumns::MAX);
		
		setEditTriggers (SelectedClicked);
		setSelectionMode (ContiguousSelection);
		setAlternatingRowColors (true);
		hideColumn (0);

		handleHideHeaders ();
		
		QList<QByteArray> propNames;
		
		for (int i = 0; i < PlayListModel_->columnCount () - 1; ++i)
			propNames.push_back ("Header" + QString::number (i).toAscii ());
		
		XmlSettingsManager::Instance ().RegisterObject (propNames, this,
				"handleHideHeaders");

		setItemDelegate (new NowPlayingDelegate (this));
		setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
		
		header ()->setResizeMode (QHeaderView::ResizeToContents);
		
		header ()->setContextMenuPolicy (Qt::CustomContextMenu);
		connect (header (),
				SIGNAL (customContextMenuRequested (QPoint)),
				this,
				SLOT (handleHeaderMenu (QPoint)));
		
		setContextMenuPolicy (Qt::CustomContextMenu);
		connect (this,
				SIGNAL (customContextMenuRequested (QPoint)),
				this,
				SLOT (handleMenu (QPoint)));
		
		connect (this,
				SIGNAL (doubleClicked (QModelIndex)),
				this,
				SLOT (handleDoubleClicked (QModelIndex)));
		
		QStringList headers;
		
		headers << tr ("Artist")
				<< tr ("Title")
				<< tr ("Album")
				<< tr ("Genre")
				<< tr ("Date")
				<< "";
		for (int i = 1, count = PlayListModel_->columnCount ();
				i < count; ++i)
			PlayListModel_->setHeaderData (i, Qt::Horizontal,
					headers [i - 1]);
	}
	
	void PlayListView::Init (boost::shared_ptr<VLCWrapper> wrapper)
	{
		VLCWrapper_ = wrapper;
		VLCWrapper *w = wrapper.get ();
		connect (this,
				SIGNAL (itemRemoved (int)),
				w,
				SLOT (removeRow (int)));
		connect (this,
				SIGNAL (playItem (int)),
				w,
				SLOT (playItem (int)));
	}
	
	void PlayListView::handleHeaderMenu (const QPoint& point)
	{
		QMenu menu;
		for (int i = 1; i < PlayListModel_->columnCount () - 1; ++i)
		{
			QAction *menuAction = new QAction (header ()->model ()
					->headerData (i, Qt::Horizontal).toString (), &menu);
			menuAction->setCheckable (true);
			menuAction->setData (i);
			
			if (!isColumnHidden (i))
				menuAction->setChecked (true);
			menu.addAction (menuAction);
		}
		
		QAction *selectedItem = menu.exec (mapToGlobal (point));
		if (selectedItem)
		{
			int columnIndex = selectedItem->data ().toInt ();
			XmlSettingsManager::Instance ().setProperty ("Header"
					+ QString::number (columnIndex).toAscii (), selectedItem->isChecked ());
			qDebug () << Q_FUNC_INFO << QString::number (columnIndex).toAscii () << selectedItem->isChecked ();
		}
	}
	
	void PlayListView::handleMenu (const QPoint& point)
	{
		if (!selectedIndexes ().size ())
			return;
		
		const int row = selectedIndexes ().first ().row ();
		
		bool found = false;
		Q_FOREACH (int val, VLCWrapper_->GetQueueListIndexes ())
		{
			if (val == row)
			{
				found = true;
				break;
			}
		}
		
		QMenu menu;
		QAction *menuAction = new QAction (tr (found ? "Unqueue" : "Queue"), &menu);
		menuAction->setData (found);
		
		menu.addAction (menuAction);
		QAction *action = menu.exec (mapToGlobal (point));
		if (!action)
			return;
		
		if (action->data ().toBool ())
			VLCWrapper_->removeFromQueue (row);
		else
			VLCWrapper_->addToQueue (row);
		
		UpdateQueueIndexes ();
	}
	
	void PlayListView::UpdateQueueIndexes ()
	{
		const QList<int> queueIndexes = VLCWrapper_->GetQueueListIndexes ();
		int i = 0;
		for (; i < PlayListModel_->columnCount (); ++i)
		{
			PlayListModel_->setData (PlayListModel_
					->index (i, PlayListColumns::QueueColumn), QString ());
		}
		
		i = 0;
		Q_FOREACH (const int index, queueIndexes)
		{
			PlayListModel_->setData (PlayListModel_
					->index (index, PlayListColumns::QueueColumn), "#" + QString::number (i++));
		}
	}
	
	void PlayListView::handleHideHeaders ()
	{
		NotHiddenColumnCount_ = 0;
		for (int i = 1; i < PlayListModel_->columnCount () - 1; ++i)
		{
			const QString& itemName = "Header" + QString::number (i);
			const bool checked = XmlSettingsManager::Instance ()
					.property (itemName.toAscii ()).toBool ();
			if (checked)
				++NotHiddenColumnCount_;
			
			setColumnHidden (i, !checked);
		}
	}
	
	void PlayListView::selectRow (int row)
	{
		setCurrentIndex (model ()->index (row, 0));
	}

	void PlayListView::AddItem (const MediaMeta& item, const QString& fileName)
	{
		QList<QStandardItem*> list;
		list << new QStandardItem (fileName);
		list << new QStandardItem (item.Artist_);
		list << new QStandardItem (item.Title_);
		list << new QStandardItem (item.Album_);
		list << new QStandardItem (item.Genre_);
		list << new QStandardItem (item.Date_);
		
		Q_FOREACH (QStandardItem *itemList, list)
			itemList->setFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled
					| Qt::ItemIsDropEnabled | Qt::ItemIsEditable);
		
		QStandardItem *queueItem = new QStandardItem ();
		queueItem->setFlags (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		list << queueItem;
		PlayListModel_->appendRow (list);
	}
	
	void PlayListView::MarkPlayingItem (int row)
	{
		auto it = PlayListModel_->item (CurrentItem_);
		if (it)
			it->setData (false, Roles::IsPlayingRole);
		it = PlayListModel_->item (row);
		it->setData (true, Roles::IsPlayingRole);
		CurrentItem_ = row;
	}

	void PlayListView::handleDoubleClicked (const QModelIndex& index)
	{
		emit playItem (index.row ());
	}
	
	void PlayListView::removeSelectedRows ()
	{
		const QModelIndexList& indexList = selectedIndexes ();
		const int c = indexList.count ();
		if (!c)
			return;
		
		const int first = indexList.first ().row ();
		const int rows = indexList.count () / NotHiddenColumnCount_;
		PlayListModel_->removeRows (first, rows);
		for (int i = 0; i < rows; ++i)
			emit itemRemoved (first);
		
		UpdateQueueIndexes ();
	}
	
	void PlayListView::keyPressEvent (QKeyEvent *event)
	{
		const QModelIndex& curIndex = currentIndex ();
		switch (event->key ())
		{
		case Qt::Key_Delete:
			removeSelectedRows ();
			break;
		case Qt::Key_Return:
			emit playItem (curIndex.row ());
			break;
		case Qt::Key_Up:
			setCurrentIndex (model ()->index (curIndex.row () - 1, 0));
			break;
		case Qt::Key_Down:
			setCurrentIndex (model ()->index (curIndex.row () + 1, 0));
			break;
		}
	}
}
}	