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

#pragma once

#include <QObject>
#include <QHash>
#include <QSet>
#include <QFutureWatcher>
#include <QIcon>
#include "interfaces/lmp/collectiontypes.h"
#include "interfaces/lmp/ilocalcollection.h"
#include "mediainfo.h"

class QStandardItemModel;
class QStandardItem;
class QAbstractItemModel;
class QModelIndex;

namespace LeechCraft
{
namespace LMP
{
	class AlbumArtManager;
	class LocalCollectionStorage;
	class LocalCollectionWatcher;
	class LocalCollectionModel;
	class Player;

	class LocalCollection : public QObject
						  , public ILocalCollection
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::LMP::ILocalCollection)

		bool IsReady_;

		QStringList RootPaths_;

		LocalCollectionStorage * const Storage_;
		LocalCollectionModel * const CollectionModel_;
		LocalCollectionWatcher * const FilesWatcher_;

		AlbumArtManager * const AlbumArtMgr_;

		Collection::Artists_t Artists_;

		QSet<QString> PresentPaths_;
		QHash<QString, int> Path2Track_;
		QHash<int, QString> Track2Path_;

		QHash<int, int> Track2Album_;
		QHash<int, Collection::Album_ptr> AlbumID2Album_;
		QHash<int, int> AlbumID2ArtistID_;

		QFutureWatcher<MediaInfo> *Watcher_;
		QList<QSet<QString>> NewPathsQueue_;

		int UpdateNewArtists_;
		int UpdateNewAlbums_;
		int UpdateNewTracks_;
	public:
		enum class DynamicPlaylist
		{
			Random50,
			LovedTracks,
			BannedTracks
		};

		enum class DirStatus
		{
			RootPath,
			SubPath,
			None
		};

		enum class StaticRating
		{
			Loved,
			Banned
		};

		LocalCollection (QObject* = 0);
		void FinalizeInit ();

		bool IsReady () const;

		AlbumArtManager* GetAlbumArtManager () const;
		LocalCollectionStorage* GetStorage () const;
		LocalCollectionModel* GetCollectionModel () const;

		void Clear ();

		void Scan (const QString&, bool root = true);
		void Unscan (const QString&);
		void Rescan ();

		DirStatus GetDirStatus (const QString&) const;
		QStringList GetDirs () const;

		int FindArtist (const QString&) const;

		int FindAlbum (const QString& artist, const QString& album) const;
		void SetAlbumArt (int, const QString&);
		Collection::Album_ptr GetAlbum (int albumId) const;

		int FindTrack (const QString& path) const;
		int GetTrackAlbumId (int trackId) const;
		Collection::Album_ptr GetTrackAlbum (int trackId) const;

		QList<int> GetDynamicPlaylist (DynamicPlaylist) const;
		QStringList TrackList2PathList (const QList<int>&) const;

		void AddTrackTo (int, StaticRating);

		Collection::TrackStats GetTrackStats (const QString&) const;

		QList<int> GetAlbumArtists (int) const;
		Collection::Artist GetArtist (int) const;
		Collection::Artists_t GetAllArtists () const;

		void RemoveTrack (const QString&);
	private:
		void HandleExistingInfos (const QList<MediaInfo>&);
		void HandleNewArtists (const Collection::Artists_t&);
		void RemoveAlbum (int);
		Collection::Artists_t::iterator RemoveArtist (Collection::Artists_t::iterator);

		void AddRootPaths (QStringList);
		void RemoveRootPaths (const QStringList&);

		void CheckRemovedFiles (const QSet<QString>& scanned, const QString& root);

		void InitiateScan (const QSet<QString>&);
	public slots:
		void recordPlayedTrack (const QString&);
	private slots:
		void rescanOnLoad ();
		void handleLoadFinished ();
		void handleIterateFinished ();
		void handleScanFinished ();
		void saveRootPaths ();
	signals:
		void scanStarted (int);
		void scanProgressChanged (int);
		void scanFinished ();

		void collectionReady ();

		void rootPathsChanged (const QStringList&);
	};
}
}
