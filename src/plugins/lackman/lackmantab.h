/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include "ui_lackmantab.h"

class QStringListModel;

namespace LeechCraft
{
namespace Util
{
	class ShortcutManager;
}
namespace LackMan
{
	class TypeFilterProxyModel;
	class StringFilterModel;

	class LackManTab : public QWidget
					 , public ITabWidget
					 , public IRecoverableTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab)

		Ui::LackManTab Ui_;

		const TabClassInfo TC_;
		QObject * const ParentPlugin_;

		Util::ShortcutManager * const ShortcutMgr_;

		QStringListModel * const TagsModel_;

		StringFilterModel * const FilterString_;
		TypeFilterProxyModel * const TypeFilter_;

		QAction *UpdateAll_;
		QAction *UpgradeAll_;
		QAction *Apply_;
		QAction *Cancel_;
		QToolBar *Toolbar_;
	public:
		LackManTab (Util::ShortcutManager*, const TabClassInfo&, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;

		QByteArray GetTabRecoverData () const;
		QIcon GetTabRecoverIcon () const;
		QString GetTabRecoverName () const;

		void SetFilterTags (const QStringList&);
		void SetFilterString (const QString&);
	private:
		void BuildPackageTreeShortcuts ();
		void BuildActions ();
	private slots:
		void toggleSelected ();

		void handlePackageSelected (const QModelIndex&);
		void handleFetchListUpdated (const QList<int>&);
		void handleTagsUpdated (const QStringList&);
		void on_PackageStatus__currentIndexChanged (int);
	signals:
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void changeTooltip (QWidget*, QWidget*);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);

		void tabRecoverDataChanged ();
	};
}
}
