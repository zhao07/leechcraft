/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "roomclentry.h"
#include <QImage>
#include <QtDebug>
#include <gloox/mucroom.h>
#include "glooxaccount.h"
#include "roompublicmessage.h"
#include "roomhandler.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{
namespace Xoox
{
	RoomCLEntry::RoomCLEntry (RoomHandler *rh, GlooxAccount *account)
	: QObject (account)
	, Account_ (account)
	, RH_ (rh)
	{
	}

	QObject* RoomCLEntry::GetObject ()
	{
		return this;
	}

	QObject* RoomCLEntry::GetParentAccount () const
	{
		return Account_;
	}

	ICLEntry::Features RoomCLEntry::GetEntryFeatures () const
	{
		return FSessionEntry;
	}

	ICLEntry::EntryType RoomCLEntry::GetEntryType () const
	{
		return ETMUC;
	}

	QString RoomCLEntry::GetEntryName () const
	{
		return RH_->GetRoomJID ();
	}

	void RoomCLEntry::SetEntryName (const QString&)
	{
	}

	QByteArray RoomCLEntry::GetEntryID () const
	{
		return RH_->GetRoomJID ().toUtf8 ();
	}

	QString RoomCLEntry::GetHumanReadableID () const
	{
		return RH_->GetRoomJID ();
	}

	QStringList RoomCLEntry::Groups () const
	{
		return QStringList () << tr ("Multiuser chatrooms");
	}

	QStringList RoomCLEntry::Variants () const
	{
		QStringList result;
		result << "";
		return result;
	}

	QObject* RoomCLEntry::CreateMessage (IMessage::MessageType type,
			const QString& variant, const QString& text)
	{
		if (variant == "")
			return new RoomPublicMessage (text, this);
		else
			return 0;
	}

	QList<QObject*> RoomCLEntry::GetAllMessages () const
	{
		return AllMessages_;
	}

	EntryStatus RoomCLEntry::GetStatus (const QString&) const
	{
		return EntryStatus (SOnline, QString ());
	}

	QList<QAction*> RoomCLEntry::GetActions () const
	{
		return QList<QAction*> ();
	}

	QImage RoomCLEntry::GetAvatar () const
	{
		return QImage ();
	}

	QString RoomCLEntry::GetRawInfo () const
	{
		return QString ();
	}

	void RoomCLEntry::ShowInfo ()
	{
	}

	QMap<QString, QVariant> RoomCLEntry::GetClientInfo (const QString& var) const
	{
		return QMap<QString, QVariant> ();
	}

	IMUCEntry::MUCFeatures RoomCLEntry::GetMUCFeatures () const
	{
		return MUCFCanBeConfigured;
	}

	QString RoomCLEntry::GetMUCSubject () const
	{
		return RH_->GetSubject ();
	}

	void RoomCLEntry::SetMUCSubject (const QString& subj)
	{
		RH_->SetSubject (subj);
	}

	QList<QObject*> RoomCLEntry::GetParticipants ()
	{
		return RH_->GetParticipants ();
	}

	void RoomCLEntry::Leave (const QString& msg)
	{
		RH_->Leave (msg);
	}

	QString RoomCLEntry::GetNick () const
	{
		return RH_->GetOurNick ();
	}

	void RoomCLEntry::SetNick (const QString& nick)
	{
		RH_->SetOurNick (nick);
	}

	bool RoomCLEntry::MayChangeAffiliation (QObject *participant, MUCAffiliation aff) const
	{
		MUCAffiliation ourAff = GetAffiliation (0);
		if (ourAff < MUCAAdmin)
			return false;

		if (ourAff == MUCAOwner)
			return true;

		MUCAffiliation partAff = GetAffiliation (participant);
		if (partAff >= ourAff)
			return false;

		if (aff >= MUCAAdmin)
			return false;

		return true;
	}

	bool RoomCLEntry::MayChangeRole (QObject *participant, MUCRole newRole) const
	{
		MUCAffiliation ourAff = GetAffiliation (0);
		MUCRole ourRole = GetRole (0);

		MUCAffiliation aff = GetAffiliation (participant);
		MUCRole role = GetRole (participant);

		if (role == MUCRInvalid ||
				ourRole == MUCRInvalid ||
				newRole == MUCRInvalid ||
				aff == MUCAInvalid ||
				ourAff == MUCAInvalid)
			return false;

		if (ourRole != MUCRModerator)
			return false;

		if (ourAff <= aff)
			return false;

		return true;
	}

	IMUCEntry::MUCAffiliation RoomCLEntry::GetAffiliation (QObject *participant) const
	{
		if (!participant)
			participant = RH_->GetSelf ();

		RoomParticipantEntry *entry = qobject_cast<RoomParticipantEntry*> (participant);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< participant
					<< "is not a RoomParticipantEntry";
			return MUCAInvalid;
		}

		return static_cast<MUCAffiliation> (entry->GetAffiliation ());
	}

	void RoomCLEntry::SetAffiliation (QObject *participant,
			MUCAffiliation newAff, const QString& reason)
	{
		RoomParticipantEntry *entry = qobject_cast<RoomParticipantEntry*> (participant);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< participant
					<< "is not a RoomParticipantEntry";
			return;
		}

		RH_->SetAffiliation (entry, newAff, reason);
	}

	IMUCEntry::MUCRole RoomCLEntry::GetRole (QObject *participant) const
	{
		if (!participant)
			participant = RH_->GetSelf ();

		RoomParticipantEntry *entry = qobject_cast<RoomParticipantEntry*> (participant);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< participant
					<< "is not a RoomParticipantEntry";
			return MUCRInvalid;
		}

		return static_cast<MUCRole> (entry->GetRole ());
	}

	void RoomCLEntry::SetRole (QObject *participant,
			MUCRole newRole, const QString& reason)
	{
		RoomParticipantEntry *entry = qobject_cast<RoomParticipantEntry*> (participant);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< participant
					<< "is not a RoomParticipantEntry";
			return;
		}

		RH_->SetRole (entry, newRole, reason);
	}

	void RoomCLEntry::HandleMessage (RoomPublicMessage *msg)
	{
		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	void RoomCLEntry::HandleNewParticipants (const QList<ICLEntry*>& parts)
	{
		QObjectList objs;
		Q_FOREACH (ICLEntry *e, parts)
			objs << e->GetObject ();
		emit gotNewParticipants (objs);
	}

	void RoomCLEntry::HandleSubjectChanged (const QString& subj)
	{
		emit mucSubjectChanged (subj);
	}
}
}
}
}
}
