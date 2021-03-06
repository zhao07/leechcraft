/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
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

#include "collectionwidget.h"
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <util/gui/clearlineeditaddon.h>
#include <util/util.h>
#include "core.h"
#include "localcollection.h"
#include "localcollectionmodel.h"
#include "palettefixerfilter.h"
#include "collectiondelegate.h"
#include "audiopropswidget.h"
#include "util.h"
#include "albumartmanagerdialog.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		class CollectionFilterModel : public QSortFilterProxyModel
		{
		public:
			CollectionFilterModel (QObject *parent = 0)
			: QSortFilterProxyModel (parent)
			{
				setDynamicSortFilter (true);
			}
		protected:
			bool filterAcceptsRow (int sourceRow, const QModelIndex& sourceParent) const
			{
				const auto& source = sourceModel ()->index (sourceRow, 0, sourceParent);
				const auto type = source.data (LocalCollectionModel::Role::Node).toInt ();

				const auto& pattern = filterRegExp ().pattern ();

				if (type != LocalCollectionModel::NodeType::Track)
					for (int i = 0, rc = sourceModel ()->rowCount (source); i < rc; ++i)
						if (filterAcceptsRow (i, source))
							return true;

				return source.data ().toString ().contains (pattern, Qt::CaseInsensitive);
			}
		};
	}

	CollectionWidget::CollectionWidget (QWidget *parent)
	: QWidget { parent }
	, Player_ { Core::Instance ().GetPlayer () }
	, CollectionFilterModel_ { new CollectionFilterModel { this } }
	{
		Ui_.setupUi (this);

		new Util::ClearLineEditAddon (Core::Instance ().GetProxy (), Ui_.CollectionFilter_);
		new PaletteFixerFilter (Ui_.CollectionTree_);

		connect (Core::Instance ().GetLocalCollection (),
				SIGNAL (scanStarted (int)),
				Ui_.ScanProgress_,
				SLOT (setMaximum (int)));
		connect (Core::Instance ().GetLocalCollection (),
				SIGNAL (scanProgressChanged (int)),
				this,
				SLOT (handleScanProgress (int)));
		connect (Core::Instance ().GetLocalCollection (),
				SIGNAL (scanFinished ()),
				Ui_.ScanProgress_,
				SLOT (hide ()));
		Ui_.ScanProgress_->hide ();

		Ui_.CollectionTree_->setItemDelegate (new CollectionDelegate (Ui_.CollectionTree_));
		auto collMgr = Core::Instance ().GetCollectionsManager ();
		CollectionFilterModel_->setSourceModel (collMgr->GetModel ());
		Ui_.CollectionTree_->setModel (CollectionFilterModel_);

		QAction *addToPlaylist = new QAction (tr ("Add to playlist"), this);
		addToPlaylist->setProperty ("ActionIcon", "list-add");
		connect (addToPlaylist,
				SIGNAL (triggered ()),
				this,
				SLOT (loadFromCollection ()));
		Ui_.CollectionTree_->addAction (addToPlaylist);

		CollectionShowTrackProps_ = new QAction (tr ("Show track properties"), Ui_.CollectionTree_);
		CollectionShowTrackProps_->setProperty ("ActionIcon", "document-properties");
		connect (CollectionShowTrackProps_,
				SIGNAL (triggered ()),
				this,
				SLOT (showCollectionTrackProps ()));
		Ui_.CollectionTree_->addAction (CollectionShowTrackProps_);

		CollectionShowAlbumArt_ = new QAction (tr ("Show album art"), Ui_.CollectionTree_);
		CollectionShowAlbumArt_->setProperty ("ActionIcon", "media-optical");
		connect (CollectionShowAlbumArt_,
				SIGNAL (triggered ()),
				this,
				SLOT (showCollectionAlbumArt ()));
		Ui_.CollectionTree_->addAction (CollectionShowAlbumArt_);

		CollectionShowAAManager_ = new QAction (tr ("Album art manager..."), Ui_.CollectionTree_);
		connect (CollectionShowAAManager_,
				SIGNAL (triggered ()),
				this,
				SLOT (showAlbumArtManager ()));
		Ui_.CollectionTree_->addAction (CollectionShowAAManager_);

		Ui_.CollectionTree_->addAction (Util::CreateSeparator (Ui_.CollectionTree_));

		CollectionRemove_ = new QAction (tr ("Remove from collection..."), Ui_.CollectionTree_);
		CollectionRemove_->setProperty ("ActionIcon", "list-remove");
		connect (CollectionRemove_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleCollectionRemove ()));
		Ui_.CollectionTree_->addAction (CollectionRemove_);

		CollectionDelete_ = new QAction (tr ("Delete from disk..."), Ui_.CollectionTree_);
		CollectionDelete_->setProperty ("ActionIcon", "edit-delete");
		connect (CollectionDelete_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleCollectionDelete ()));
		Ui_.CollectionTree_->addAction (CollectionDelete_);

		connect (Ui_.CollectionTree_,
				SIGNAL (doubleClicked (QModelIndex)),
				this,
				SLOT (loadFromCollection ()));

		connect (Ui_.CollectionTree_->selectionModel (),
				SIGNAL (currentRowChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleCollectionItemSelected (QModelIndex)));

		connect (Ui_.CollectionFilter_,
				SIGNAL (textChanged (QString)),
				CollectionFilterModel_,
				SLOT (setFilterFixedString (QString)));
	}

	void CollectionWidget::showCollectionTrackProps ()
	{
		const auto& index = Ui_.CollectionTree_->currentIndex ();
		const auto& info = index.data (LocalCollectionModel::Role::TrackPath).toString ();
		if (info.isEmpty ())
			return;

		AudioPropsWidget::MakeDialog ()->SetProps (info);
	}

	void CollectionWidget::showCollectionAlbumArt ()
	{
		const auto& index = Ui_.CollectionTree_->currentIndex ();
		const auto& path = index.data (LocalCollectionModel::Role::AlbumArt).toString ();
		if (path.isEmpty ())
			return;

		ShowAlbumArt (path, QCursor::pos ());
	}

	void CollectionWidget::showAlbumArtManager ()
	{
		auto aamgr = Core::Instance ().GetLocalCollection ()->GetAlbumArtManager ();

		const auto& index = Ui_.CollectionTree_->currentIndex ();
		const auto& album = index.data (LocalCollectionModel::Role::AlbumName).toString ();
		const auto& artist= index.data (LocalCollectionModel::Role::ArtistName).toString ();

		auto dia = new AlbumArtManagerDialog (artist, album, aamgr, this);
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();
	}

	namespace
	{
		template<typename T>
		QList<T> CollectFromModel (const QModelIndex& root, int role)
		{
			QList<T> result;

			const auto& var = root.data (role);
			if (!var.isNull ())
				result << var.value<T> ();

			auto model = root.model ();
			for (int i = 0; i < model->rowCount (root); ++i)
				result += CollectFromModel<T> (root.child (i, 0), role);

			return result;
		}
	}

	void CollectionWidget::handleCollectionRemove ()
	{
		const auto& index = Ui_.CollectionTree_->currentIndex ();
		const auto& paths = CollectFromModel<QString> (index, LocalCollectionModel::Role::TrackPath);
		if (paths.isEmpty ())
			return;

		auto response = QMessageBox::question (this,
				"LeechCraft",
				tr ("Are you sure you want to remove %n track(s) from your collection?<br/><br/>"
					"Please note that if tracks remain on your disk they will be re-added next "
					"time collection is scanned, but you will lose the statistics.",
					0,
					paths.size ()),
					QMessageBox::Yes | QMessageBox::No);
		if (response != QMessageBox::Yes)
			return;

		auto collection = Core::Instance ().GetLocalCollection ();
		Q_FOREACH (const auto& path, paths)
			collection->RemoveTrack (path);
	}

	void CollectionWidget::handleCollectionDelete ()
	{
		const auto& index = Ui_.CollectionTree_->currentIndex ();
		const auto& paths = CollectFromModel<QString> (index, LocalCollectionModel::Role::TrackPath);
		if (paths.isEmpty ())
			return;

		auto response = QMessageBox::question (this,
				"LeechCraft",
				tr ("Are you sure you want to erase %n track(s)? This action cannot be undone.",
					0,
					paths.size ()),
					QMessageBox::Yes | QMessageBox::No);
		if (response != QMessageBox::Yes)
			return;

		Q_FOREACH (const auto& path, paths)
			QFile::remove (path);
	}

	void CollectionWidget::loadFromCollection ()
	{
		const auto& idxs = Ui_.CollectionTree_->selectionModel ()->selectedRows ();

		QModelIndexList mapped;
		for (const auto& src : idxs)
		{
			const auto& index = CollectionFilterModel_->mapToSource (src);
			if (index.isValid ())
				mapped << index;
		}

		Core::Instance ().GetCollectionsManager ()->Enqueue (mapped, Player_);
	}

	void CollectionWidget::handleCollectionItemSelected (const QModelIndex& index)
	{
		const int nodeType = index.data (LocalCollectionModel::Role::Node).value<int> ();
		CollectionShowTrackProps_->setEnabled (nodeType == LocalCollectionModel::NodeType::Track);
		CollectionShowAlbumArt_->setEnabled (nodeType == LocalCollectionModel::NodeType::Album);
		CollectionShowAAManager_->setEnabled (nodeType == LocalCollectionModel::NodeType::Album);
	}

	void CollectionWidget::handleScanProgress (int progress)
	{
		if (progress >= Ui_.ScanProgress_->maximum ())
		{
			Ui_.ScanProgress_->hide ();
			return;
		}

		if (!Ui_.ScanProgress_->isVisible ())
			Ui_.ScanProgress_->show ();
		Ui_.ScanProgress_->setValue (progress);
	}
}
}
