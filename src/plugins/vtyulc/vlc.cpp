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

#include <QIcon>
#include <QShortcut>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/entitytesthandleresult.h>
#include <util/shortcuts/shortcutmanager.h>
#include <util/util.h>
#include "vlc.h"

namespace LeechCraft
{
namespace vlc
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("vtyulc");

		Proxy_ = proxy;

		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (), "vtyulcsettings.xml");

		Manager_ = new Util::ShortcutManager (proxy);
		Manager_->SetObject (this);

		Manager_->RegisterActionInfo ("org.vtyulc.navigate_left",
				ActionInfo (tr ("Navigate left in DVD menu"),
						QKeySequence (Qt::Key_Left),
						Proxy_->GetIcon ("arrow-left")));

		Manager_->RegisterActionInfo ("org.vtyulc.navigate_right",
				ActionInfo (tr ("Navigate right in DVD menu"),
						QKeySequence (Qt::Key_Right),
						Proxy_->GetIcon ("arrow-right")));

		Manager_->RegisterActionInfo ("org.vtyulc.navigate_up",
				ActionInfo (tr ("Navigate up in DVD menu"),
						QKeySequence (Qt::Key_Up),
						Proxy_->GetIcon ("arrow-up")));

		Manager_->RegisterActionInfo ("org.vtyulc.navigate_down",
				ActionInfo (tr ("Navigate down in DVD menu"),
						QKeySequence (Qt::Key_Down),
						Proxy_->GetIcon ("arrow-down")));

		Manager_->RegisterActionInfo ("org.vtyulc.navigate_enter",
				ActionInfo (tr ("Activate current in DVD menu"),
						QKeySequence (Qt::Key_Enter),
						Proxy_->GetIcon ("key-enter")));

		Manager_->RegisterActionInfo ("org.vtyulc.toggle_fullscreen",
				ActionInfo (tr ("Toggle fullscreen"),
						QKeySequence (Qt::Key_F),
						Proxy_->GetIcon ("view-fullscreen")));

		Manager_->RegisterActionInfo ("org.vtyulc.toggle_play",
				ActionInfo (tr ("Switch play/pause"),
						QKeySequence (Qt::Key_Space),
						Proxy_->GetIcon ("media-playback-start")));

		Manager_->RegisterActionInfo ("org.vtyulc.volume_increase",
				ActionInfo (tr ("Increase volume"),
						QKeySequence (Qt::Key_Plus),
						Proxy_->GetIcon ("audio-volume-high")));

		Manager_->RegisterActionInfo ("org.vtyulc.volume_decrease",
				ActionInfo (tr ("Decrease volume"),
						QKeySequence (Qt::Key_Minus),
						Proxy_->GetIcon ("audio-volume-low")));

		Manager_->RegisterActionInfo ("org.vtyulc.plus_3_percent",
				ActionInfo (tr ("3% seek forward"),
						QKeySequence (Qt::Key_Asterisk),
						QIcon ()));

		Manager_->RegisterActionInfo ("org.vtyulc.minus_3_percent",
				ActionInfo (tr ("3% seek backward"),
						QKeySequence (Qt::Key_Slash),
						QIcon ()));

		Manager_->RegisterActionInfo ("org.vtyulc.plus_10_seconds",
				ActionInfo (tr ("10 seconds seek forward"),
						QKeySequence (Qt::Key_0),
						QIcon ()));

		Manager_->RegisterActionInfo ("org.vtyulc.minus_10_seconds",
				ActionInfo (tr ("10 seconds seek backward"),
						QKeySequence (Qt::Key_9),
						QIcon ()));

		Manager_->RegisterActionInfo ("org.vtyulc.next",
				ActionInfo (tr ("Next in playlist"),
						QKeySequence (Qt::Key_R),
						QIcon ()));

		Manager_->RegisterActionInfo ("org.vtyulc.prev",
				ActionInfo (tr ("Prev in playlist"),
						QKeySequence (Qt::Key_T),
						QIcon ()));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.vtyulc";
	}

	void Plugin::Release ()
	{
		for (int i = 0; i < Tabs_.size (); i++)
			delete Tabs_ [i];
	}

	QString Plugin::GetName () const
	{
		return "VtyuLC";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Video player, based on VLC");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	void Plugin::TabOpenRequested (const QByteArray& tabClass)
	{
		VlcWidget *widget = new VlcWidget (Proxy_, Manager_);
		XmlSettingsManager::Instance ().RegisterObject ("Autostart", widget, "autostartChanged");
		Tabs_ << widget;
		emit addNewTab ("VtyuLC", widget);
		emit raiseTab (widget);
		connect (widget,
				SIGNAL (deleteMe (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)));

		connect (widget,
				SIGNAL (deleteMe (QWidget*)),
				this,
				SLOT (deleteDeleted (QWidget*)));
	}

	LeechCraft::TabClasses_t Plugin::GetTabClasses () const
	{
		return { VlcWidget::GetTabInfo () };
	}

	QMap<QString, ActionInfo> Plugin::GetActionInfo () const
	{
		return Manager_->GetActionInfo ();
	}

	void Plugin::SetShortcut (const QString &id, const QKeySequences_t &shortcuts)
	{
		Manager_->SetShortcut (id, shortcuts);
	}

	void Plugin::deleted (QWidget *widget)
	{
		for (auto i = Tabs_.begin (); i != Tabs_.end (); i++)
			if (*i == widget)
			{
				Tabs_.erase (i);
				return;
			}
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& entity) const
	{
		if (entity.Mime_ == "x-leechcraft/power-state-changed")
			return EntityTestHandleResult (EntityTestHandleResult::PHigh);
		else
			return EntityTestHandleResult ();
	}

	void Plugin::Handle (Entity entity)
	{
		if (entity.Entity_ == "Sleeping")
			for (int i = 0; i < Tabs_.size (); i++)
				Tabs_ [i]->Pause ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_vlc, LeechCraft::vlc::Plugin);