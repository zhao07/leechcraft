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
#include <QTreeView>

struct libvlc_media_player_t;
struct libvlc_media_list_player_t;
struct libvlc_instance_t;
struct libvlc_media_list_t;

class QAction;

namespace LeechCraft
{
namespace vlc
{
	class PlaylistModel;
	
	class PlaylistWidget : public QTreeView
	{
		Q_OBJECT
		
		libvlc_media_list_player_t *Player_;
		libvlc_media_list_t *Playlist_;
		libvlc_media_player_t *NativePlayer_;
		libvlc_instance_t *Instance_;
		
		PlaylistModel *Model_;
		QAction *DeleteAction_;
	
	public:
		explicit PlaylistWidget (QWidget *parent = 0);
		
		void AddUrl (const QUrl&);
		bool NowPlaying ();
		void Init (libvlc_instance_t *instance, libvlc_media_player_t *player);
		void Clear ();
		
	private:
		void findAndDelete (QUrl);
		
	protected:
		void dragEnterEvent (QDragEnterEvent*);
		void dropEvent (QDropEvent*);
		
	private slots:
		void togglePlay ();
		void updateInterface ();
		void selectionChanged (const QModelIndex& current, const QModelIndex& previous);
		void createMenu (QPoint);
		void deleteRequested (QAction*);
	};
}
}
