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

#include "useravatarmanager.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include "pubsubmanager.h"
#include "useravatardata.h"
#include "useravatarmetadata.h"
#include "core.h"
#include "clientconnection.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	UserAvatarManager::UserAvatarManager (ClientConnection *conn)
	: QObject (conn)
	, Manager_ (conn->GetPubSubManager ())
	, Conn_ (conn)
	{
		connect (Manager_,
				SIGNAL (gotEvent (QString, PEPEventBase*)),
				this,
				SLOT (handleEvent (QString, PEPEventBase*)));

		Manager_->RegisterCreator<UserAvatarData> ();
		Manager_->RegisterCreator<UserAvatarMetadata> ();
		Manager_->SetAutosubscribe<UserAvatarMetadata> (true);
	}

	void UserAvatarManager::PublishAvatar (const QImage& avatar)
	{
		if (!avatar.isNull ())
		{
			UserAvatarData data (avatar);

			Manager_->PublishEvent (&data);
		}

		UserAvatarMetadata metadata (avatar);
		Manager_->PublishEvent (&metadata);
	}

	void UserAvatarManager::handleEvent (const QString& from, PEPEventBase *event)
	{
		UserAvatarMetadata *mdEvent = dynamic_cast<UserAvatarMetadata*> (event);
		if (mdEvent)
		{
			if (mdEvent->GetID ().isEmpty ())
			{
				emit avatarUpdated (from, QImage ());
				return;
			}

			QString bare;
			QString resource;
			ClientConnection::Split (from, &bare, &resource);

			ICLEntry *entry = qobject_cast<ICLEntry*> (Conn_->GetCLEntry (bare, resource));
			if (entry && !entry->GetAvatar ().isNull ())
			{
				UserAvatarMetadata md (entry->GetAvatar ());
				if (mdEvent->GetID () == md.GetID ())
					return;
			}

			if (mdEvent->GetURL ().isValid ())
			{
				QNetworkAccessManager *mgr = Core::Instance ()
						.GetProxy ()->GetNetworkAccessManager ();

				QNetworkReply *rep = mgr->get (QNetworkRequest (mdEvent->GetURL ()));
				rep->setProperty ("Azoth/From", from);
				connect (rep,
						SIGNAL (finished ()),
						this,
						SLOT (handleHTTPFinished ()));
			}
			else
				Manager_->RequestItem (bare,
						UserAvatarData::GetNodeString (),
						mdEvent->GetID ());

			return;
		}

		UserAvatarData *dEvent = dynamic_cast<UserAvatarData*> (event);
		if (dEvent)
			emit avatarUpdated (from, dEvent->GetImage ());
	}

	void UserAvatarManager::handleHTTPFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "isn't a QNetworkReply";
			return;
		}

		reply->deleteLater ();

		const QString& from = reply->property ("Azoth/From").toString ();
		if (from.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty from";
			return;
		}

		emit avatarUpdated (from, QImage::fromData (reply->readAll ()));
	}
}
}
}
