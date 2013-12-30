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

#include "shortcutmanager.h"
#include <QAction>
#include <QShortcut>
#include "util/util.h"
#include "interfaces/ihaveshortcuts.h"
#include "interfaces/core/ientitymanager.h"

namespace LeechCraft
{
namespace Util
{
	ShortcutManager::ShortcutManager (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, CoreProxy_ (proxy)
	, ContextObj_ (0)
	{
	}

	void ShortcutManager::SetObject (QObject *obj)
	{
		ContextObj_ = obj;
	}

	void ShortcutManager::RegisterAction (const QString& id, QAction *act)
	{
		Actions_ [id] << act;
		connect (act,
				SIGNAL (destroyed ()),
				this,
				SLOT (handleActionDestroyed ()));

		const QIcon& icon = act->icon ().isNull () ?
				CoreProxy_->GetIcon (act->property ("ActionIcon").toString ()) :
				act->icon ();
		RegisterActionInfo (id,
				{ act->text (), act->shortcuts (), icon });

		if (CoreProxy_->GetShortcutProxy ()->HasObject (ContextObj_))
			SetShortcut (id,
					CoreProxy_->GetShortcutProxy ()->GetShortcuts (ContextObj_, id));
	}

	void ShortcutManager::RegisterShortcut (const QString& id, const ActionInfo& info, QShortcut* shortcut)
	{
		Shortcuts_ [id] << shortcut;
		connect (shortcut,
				SIGNAL (destroyed ()),
				this,
				SLOT (handleShortcutDestroyed ()));

		RegisterActionInfo (id, info);

		if (CoreProxy_->GetShortcutProxy ()->HasObject (ContextObj_))
			SetShortcut (id,
					CoreProxy_->GetShortcutProxy ()->GetShortcuts (ContextObj_, id));
	}

	void ShortcutManager::RegisterActionInfo (const QString& id, const ActionInfo& info)
	{
		if (!ActionInfo_.contains (id) ||
				ActionInfo_ [id].UserVisibleText_.isEmpty ())
			ActionInfo_ [id] = info;
	}

	void ShortcutManager::RegisterGlobalShortcut (const QString& id,
			QObject *target, const QByteArray& method, const ActionInfo& info)
	{
		Entity e = Util::MakeEntity ({}, {}, 0,
				"x-leechcraft/global-action-register");
		e.Additional_ ["Receiver"] = QVariant::fromValue (target);
		e.Additional_ ["ActionID"] = id;
		e.Additional_ ["Method"] = method;
		e.Additional_ ["Shortcut"] = QVariant::fromValue (info.Seqs_.value (0));
		Globals_ [id] = e;

		ActionInfo_ [id] = info;
	}

	void ShortcutManager::AnnounceGlobalShorcuts ()
	{
		for (const auto& entity : Globals_)
			CoreProxy_->GetEntityManager ()->HandleEntity (entity);
	}

	void ShortcutManager::SetShortcut (const QString& id, const QKeySequences_t& seqs)
	{
		for (auto act : Actions_ [id])
			act->setShortcuts (seqs);

		for (auto sc : Shortcuts_ [id])
			sc->setKey (seqs.value (0));

		if (Globals_.contains (id))
		{
			auto& e = Globals_ [id];
			e.Additional_ ["Shortcut"] = QVariant::fromValue (seqs.value (0));
			CoreProxy_->GetEntityManager ()->HandleEntity (e);
		}
	}

	QMap<QString, ActionInfo> ShortcutManager::GetActionInfo () const
	{
		return ActionInfo_;
	}

	ShortcutManager& ShortcutManager::operator<< (const QPair<QString, QAction*>& pair)
	{
		RegisterAction (pair.first, pair.second);
		return *this;
	}

	void ShortcutManager::handleActionDestroyed ()
	{
		auto act = static_cast<QAction*> (sender ());
		for (const auto& id : Actions_.keys ())
			Actions_ [id].removeAll (act);
	}

	void ShortcutManager::handleShortcutDestroyed()
	{
		auto sc = static_cast<QShortcut*> (sender ());
		for (const auto& id : Shortcuts_.keys ())
			Shortcuts_ [id].removeAll (sc);
	}
}
}
