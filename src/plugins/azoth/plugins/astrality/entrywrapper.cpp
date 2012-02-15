/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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

#include "entrywrapper.h"
#include <interfaces/azothutil.h>
#include "accountwrapper.h"
#include "astralityutil.h"
#include "msgwrapper.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Astrality
{
	EntryWrapper::EntryWrapper (Tp::ContactPtr c, AccountWrapper *aw)
	: QObject (aw)
	, AW_ (aw)
	, C_ (c)
	{
		connect (C_.data (),
				SIGNAL (presenceChanged (Tp::Presence)),
				this,
				SLOT (handlePresenceChanged ()));
		connect (C_.data (),
				SIGNAL (aliasChanged (QString)),
				this,
				SIGNAL (nameChanged (QString)));

		connect (this,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				AW_,
				SIGNAL (gotEntity (LeechCraft::Entity)));
	}

	void EntryWrapper::HandleMessage (MsgWrapper *msg)
	{
		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	QObject* EntryWrapper::GetObject ()
	{
		return this;
	}

	QObject* EntryWrapper::GetParentAccount () const
	{
		return AW_;
	}

	ICLEntry::Features EntryWrapper::GetEntryFeatures () const
	{
		return FPermanentEntry;
	}

	ICLEntry::EntryType EntryWrapper::GetEntryType () const
	{
		return ETChat;
	}

	QString EntryWrapper::GetEntryName () const
	{
		return C_->alias ();
	}

	void EntryWrapper::SetEntryName (const QString&)
	{
	}

	QString EntryWrapper::GetEntryID () const
	{
		return AW_->GetAccountID () + "." + C_->id ();
	}

	QString EntryWrapper::GetHumanReadableID () const
	{
		return C_->id ();
	}

	QStringList EntryWrapper::Groups () const
	{
		return C_->groups ();
	}

	void EntryWrapper::SetGroups (const QStringList& groups)
	{
		const auto& oldGroups = Groups ();
		Q_FOREACH (const QString& g, oldGroups)
			if (!groups.contains (g))
				C_->removeFromGroup (g);

		Q_FOREACH (const QString& g, groups)
			if (!oldGroups.contains (g))
				C_->addToGroup (g);
	}

	QStringList EntryWrapper::Variants () const
	{
		return QStringList (QString ());
	}

	QObject* EntryWrapper::CreateMessage (IMessage::MessageType mt, const QString&, const QString& body)
	{
		auto messenger = AW_->GetMessenger (GetHumanReadableID ());
		return new MsgWrapper (body, IMessage::DOut, messenger, this, mt);
	}

	QList<QObject*> EntryWrapper::GetAllMessages () const
	{
		QList<QObject*> result;
		Q_FOREACH (auto msg, AllMessages_)
			result << msg;
		return result;
	}

	void EntryWrapper::PurgeMessages (const QDateTime& before)
	{
		Util::StandardPurgeMessages (AllMessages_, before);
	}

	void EntryWrapper::SetChatPartState (ChatPartState, const QString&)
	{
	}

	EntryStatus EntryWrapper::GetStatus (const QString&) const
	{
		return Status2Azoth (C_->presence ());
	}

	QImage EntryWrapper::GetAvatar () const
	{
		return QImage ();
	}

	QString EntryWrapper::GetRawInfo () const
	{
		return QString ();
	}

	void EntryWrapper::ShowInfo ()
	{
	}

	QList<QAction*> EntryWrapper::GetActions() const
	{
		return QList<QAction*> ();
	}

	QMap<QString, QVariant> EntryWrapper::GetClientInfo (const QString&) const
	{
		return QMap<QString, QVariant> ();
	}

	void EntryWrapper::MarkMsgsRead ()
	{
	}

	void EntryWrapper::handlePresenceChanged ()
	{
		emit statusChanged (GetStatus (QString ()), QString ());
	}
}
}
}
