/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "syncer.h"
#include <QFileInfo>
#include <QStandardItem>
#include <QtDebug>
#include "interfaces/netstoremanager/istorageaccount.h"
#include "utils.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	Syncer::Syncer (const QString& dirPath, const QString& remotePath,
			IStorageAccount *isa, QObject *parent)
	: QObject (parent)
	, LocalPath_ (dirPath)
	, RemotePath_ (remotePath)
	, Started_ (false)
	, Account_ (isa)
	, SFLAccount_ (qobject_cast<ISupportFileListings*> (isa->GetQObject ()))
	{
	}

	QByteArray Syncer::GetAccountID () const
	{
		return Account_->GetUniqueID ();
	}

	QString Syncer::GetLocalPath () const
	{
		return LocalPath_;
	}

	QString Syncer::GetRemotePath () const
	{
		return RemotePath_;
	}

	bool Syncer::IsStarted () const
	{
		return Started_;
	}

	void Syncer::CreatePath (const QStringList& path)
	{
		if (Id2Path_.right.count (path.join ("/")))
			return;

		QStringList nonExistingPath;
		QStringList existingPath;
		for (int i = path.length () - 1; i >= 0; --i)
		{
			QStringList temp = path.mid (0, i + 1);
			if (Id2Path_.right.count (temp.join ("/")))
			{
				existingPath = temp;
				break;
			}
			else if (nonExistingPath.isEmpty ())
				nonExistingPath = temp;
		}

		if (nonExistingPath.isEmpty ())
			return;

		int lastPos = 0;
		for ( ; lastPos < std::min (existingPath .size (), nonExistingPath.size ()); ++lastPos)
			if (existingPath.at (lastPos) != nonExistingPath.at (lastPos))
				break;

		SFLAccount_->CreateDirectory (nonExistingPath.at (lastPos),
				existingPath.isEmpty () ?
					QByteArray () :
					Id2Path_.right.at (existingPath.join ("/")));
		if (lastPos != nonExistingPath.length () - 1)
			CallsQueue_.append ([this, nonExistingPath] (const QStringList&)
				{ CreatePath (nonExistingPath); });
	}

	void Syncer::start ()
	{
		Started_ = true;
		QStringList path = RemotePath_.split ('/');
		CallsQueue_.append ([this, path] (const QStringList&) { CreatePath (path); });
		CallsQueue_.dequeue () (QStringList ());
	}

	void Syncer::stop ()
	{
		CallsQueue_.clear ();
		Started_ = false;
	}

	void Syncer::handleGotItems (const QList<StorageItem>& items)
	{
		Id2Item_.clear ();
		boost::bimaps::bimap<QByteArray, QStandardItem*> id2StandardItem;
		for (const auto& item : items)
		{
			if (item.IsTrashed_)
				continue;

			Id2Item_ [item.ID_] = item;
			id2StandardItem.insert ({ item.ID_, new QStandardItem (item.Name_) });
		}

		QStandardItem *core = new QStandardItem;
		for (const auto& pair : id2StandardItem.left)
		{
			if (!Id2Item_.contains (Id2Item_ [pair.first].ParentID_))
				core->appendRow (pair.second);
			else
				id2StandardItem.left.at (Id2Item_ [pair.first].ParentID_)->appendRow (pair.second);
		}

		QList<QStandardItem*> parentItems = { core };
		QList<QStandardItem*> childItems;
		while (!parentItems.isEmpty ())
		{
			for (auto item : parentItems)
			{
				for (int i = 0; i < item->rowCount (); ++i)
					childItems << item->child (i);
			}

			for (auto item : childItems)
			{
				const auto& id = id2StandardItem.right.at (item);
				Id2Path_.insert ({ id,
					(Id2Item_.contains (Id2Item_ [id].ParentID_) ?
					(Id2Path_.left.at (Id2Item_ [id].ParentID_) + "/" ) :
					QString ())
					+ item->text () });
			}
			auto tempItems = parentItems;
			parentItems = childItems;
			childItems = tempItems;

			childItems.clear();
		}
	}

	void Syncer::handleGotNewItem(const StorageItem& item, const QByteArray& parentId)
	{
		Id2Item_ [item.ID_] = item;
		Id2Path_.insert ({ item.ID_, (Id2Item_.contains (parentId) ?
			(Id2Path_.left.at (parentId) + "/") :
			QString ())
			+ item.Name_ });

		if (!CallsQueue_.isEmpty ())
			CallsQueue_.dequeue () (QStringList ());
	}

	void Syncer::handleGotChanges (const QList<Change>& changes)
	{
		//TODO gotChanges
	}

	void Syncer::dirWasCreated (const QString& path)
	{
		if (!SFLAccount_)
			return;

		QString dirPath = path;
		dirPath.replace (LocalPath_, RemotePath_);

		if (CallsQueue_.isEmpty ())
			CreatePath (dirPath.split ("/"));
		else
			CallsQueue_ << [this, dirPath] (const QStringList&)
				{ CreatePath (dirPath.split ("/")); };
	}

	void Syncer::dirWasRemoved (const QString& path)
	{
		if (!SFLAccount_)
			return;

		QString dirPath = path;
		dirPath.replace (LocalPath_ + "/", "");
		if (Id2Path_.right.count (dirPath))
			return;

// 		GetParentID (dirPath);
// 		SFLAccount_->Delete ({ Id2Path_.right.at (dirPath) }, false);
	}

	void Syncer::fileWasCreated (const QString& path)
	{
		qDebug () << Q_FUNC_INFO << path;
	}

	void Syncer::fileWasRemoved (const QString& path)
	{
		qDebug () << Q_FUNC_INFO << path;
	}

	void Syncer::fileWasUpdated (const QString& path)
	{
		qDebug () << Q_FUNC_INFO << path;
	}

}
}
