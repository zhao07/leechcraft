/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2013  Vladislav Tyulbashev
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANusers/vtyulb/TY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "playlistwidget.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QVBoxLayout>
#include <QTimer>
#include <QString>
#include <QRect>
#include <QModelIndex>
#include <QAction>
#include <QMenu>
#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include <QFontMetrics>
#include <QStandardItem>
#include <QStringList>
#include <QEventLoop>
#include <vlc/vlc.h>
#include "playlistmodel.h"

namespace LeechCraft
{
namespace vlc
{
	PlaylistWidget::PlaylistWidget (QIcon playIcon, QWidget *parent)
	: QTreeView (parent)
	, PlayIcon_ (playIcon)
	, LastPlayingItem_ (nullptr)
	{
		setDragEnabled (true);
		setDropIndicatorShown (true);
		setAcceptDrops (true);
		setBaseSize (0, 0);
		setRootIsDecorated (false);
		setContextMenuPolicy (Qt::CustomContextMenu);
		
		connect (this,
				SIGNAL (customContextMenuRequested (QPoint)),
				this,
				SLOT (createMenu (QPoint)));
	}
	
	PlaylistWidget::~PlaylistWidget ()
	{
		QStringList save;
		int size = libvlc_media_list_count (Playlist_);
		for (int i = 0; i < size; i++)
			save << QString (libvlc_media_get_meta (libvlc_media_list_item_at_index (Playlist_, i), libvlc_meta_URL));
		
		if (LastPlayingItem_ == nullptr)
			save << "0";
		else
			save << QString::number (LastPlayingItem_->row ());
		
		emit savePlaylist (save);

		libvlc_media_list_release (Playlist_);
		libvlc_media_list_player_release (Player_);
		delete DeleteAction_;
	}
	
	void PlaylistWidget::Init (libvlc_instance_t *instance, libvlc_media_player_t *player)
	{
		Player_ = libvlc_media_list_player_new (instance);
		Instance_ = instance;
		libvlc_media_list_player_set_media_player (Player_, player);
		Playlist_ = libvlc_media_list_new (Instance_);
		libvlc_media_list_player_set_media_list (Player_, Playlist_);
		NativePlayer_ = player;
		
		Model_ = new PlaylistModel (this, Playlist_);
		setModel (Model_);
		
		QTimer *timer = new QTimer (this);
		timer->setInterval (300);
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (updateInterface ()));
		
		timer->start ();

		DeleteAction_ = new QAction (this);
	}
	
	void PlaylistWidget::AddUrl (const QUrl& url, bool start)
	{
		for (int i = 0; i < libvlc_media_list_count (Playlist_); i++)
			if (url.toEncoded () == libvlc_media_get_meta (libvlc_media_list_item_at_index (Playlist_, i), libvlc_meta_URL))
			{
				qWarning () << Q_FUNC_INFO << "Ignoring already added url";
				return;
			}
		libvlc_media_t *m = libvlc_media_new_path (Instance_, url.toEncoded ());
		libvlc_media_set_meta (m, libvlc_meta_URL, url.toEncoded ());
		libvlc_media_list_add_media (Playlist_, m);
		
		if (start)
			libvlc_media_list_player_play (Player_);
		
		updateInterface ();
	}
	
	bool PlaylistWidget::IsPlaying () const
	{
		return libvlc_media_list_player_is_playing (Player_);
	}
	
	void PlaylistWidget::togglePlay ()
	{
		if (IsPlaying ())
			libvlc_media_list_player_pause (Player_);
		else
			libvlc_media_list_player_play (Player_);
	}
	
	void PlaylistWidget::Clear ()
	{
		libvlc_media_list_player_stop (Player_);
		while (libvlc_media_list_count (Playlist_))
			libvlc_media_list_remove_index (Playlist_, 0);
	}
	
	void PlaylistWidget::updateInterface ()
	{
		Model_->updateTable ();
		int currentRow = libvlc_media_list_index_of_item (Playlist_, libvlc_media_player_get_media (NativePlayer_));
		fprintf (stderr, "%d", currentRow);
				
		bool find = false;
		for (int i = 0; i < Model_->GetPublicItems ()->size (); i++)
			if (LastPlayingItem_ == ((*Model_->GetPublicItems ()) [i]))
			{
				find = true;
				break;
			}
			
		if (!find)
			LastPlayingItem_ = nullptr;
		
		if (LastPlayingItem_)
			LastPlayingItem_->setIcon (QIcon ());
		
		if (currentRow == -1 || currentRow >= Model_->rowCount ())
			return;
				
		LastPlayingItem_ = Model_->item (currentRow, ColumnName);
		LastPlayingItem_->setIcon (QIcon (PlayIcon_));
		
		update ();
	}
	
	void PlaylistWidget::createMenu (QPoint p)
	{
		int index = indexAt (p).row ();
		if (index == -1)
			return;
		
		QMenu menu;
		QAction *action = new QAction (&menu);
		action->setText ("Delete");
		action->setData (QVariant (index));
		menu.addAction (action);

		connect (&menu,
				SIGNAL (triggered (QAction*)),
				this,
				SLOT (deleteRequested (QAction*)));

		menu.exec (QCursor::pos ());
	}
	
	void PlaylistWidget::deleteRequested (QAction *object)
	{
		libvlc_media_t *media = libvlc_media_list_item_at_index (Playlist_, object->data ().toInt ());
		libvlc_media_list_remove_index (Playlist_, object->data ().toInt ());
		libvlc_media_release (media);
		Model_->updateTable ();
	}
	
	void PlaylistWidget::mouseDoubleClickEvent (QMouseEvent *event)
	{
		int row = indexAt (event->pos ()).row ();
		if (row > -1 && row < libvlc_media_list_count (Playlist_))
			libvlc_media_list_player_play_item_at_index (Player_, row);
		
		updateInterface ();
		event->accept ();
	}
	
	void PlaylistWidget::resizeEvent (QResizeEvent *event)
	{
		QFontMetrics metrics (font ());
		int len =  (metrics.width ("00:00:00")) + 10;
		setColumnWidth (0, event->size ().width () - len);
		setColumnWidth (1, len);
	}
	
	void PlaylistWidget::SetCurrentMedia (int current)
	{
		libvlc_media_t *media = libvlc_media_list_item_at_index (Playlist_, current);
		if (current > -1 && current < libvlc_media_list_count (Playlist_))
		{
			int count = libvlc_media_list_count (Playlist_);
			libvlc_media_list_player_play_item (Player_, media);
			while (!libvlc_media_player_is_playing (NativePlayer_))
			{
				QEventLoop loop;
				QTimer::singleShot (5, &loop, SLOT (quit ()));
				loop.exec ();
			}
			
			libvlc_media_player_stop (NativePlayer_);
		}
	}
}
}
