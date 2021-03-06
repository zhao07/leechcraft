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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QUrl>
#include <QListView>
#include <vlc/libvlc_structures.h>

struct libvlc_media_player_t;
struct libvlc_media_list_player_t;
struct libvlc_instance_t;
struct libvlc_media_list_t;
struct libvlc_media_t;

class QAction;
class QStandardItem;
class QStringList;

namespace LeechCraft
{
namespace vlc
{
	class PlaylistModel;
	
	struct QueueState
	{
		QStringList Playlist_;
		int Current_;
		libvlc_time_t Position_;
	};
	
	class PlaylistWidget : public QListView
	{
		Q_OBJECT
		
		libvlc_media_list_player_t *Player_;
		libvlc_media_list_t *Playlist_;
		libvlc_media_player_t *NativePlayer_;
		libvlc_instance_t *Instance_;
		
		QStandardItem *LastPlayingItem_;
		PlaylistModel *Model_;
		const QIcon PlayIcon_;
	
	public:
		explicit PlaylistWidget (QIcon playIcon, QWidget *parent = 0);
		~PlaylistWidget ();
		
		void SetCurrentMedia (int);
		void SetCurrentMedia (libvlc_media_t*);
		libvlc_media_t* AddUrl (const QUrl&, bool start);
		bool IsPlaying () const;
		void Init (libvlc_instance_t *instance, libvlc_media_player_t *player);
		void DeleteRequested (int index);
		
	protected:
		void mouseDoubleClickEvent (QMouseEvent*);
		void resizeEvent (QResizeEvent*);
		
	private:
		void updateModelConstants ();
		
	public slots:
		void clearPlaylist ();
		void next ();
		void prev ();
		void up ();
		void down ();
		
	private slots:
		void togglePlay ();
		void updateInterface ();
		void createMenu (QPoint);
		void deleteRequested (QAction*);
		
	signals:
		void savePlaylist (const QueueState&);
	};
}
}
