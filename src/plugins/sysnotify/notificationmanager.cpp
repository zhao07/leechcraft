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

#include "notificationmanager.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QApplication>
#include <QIcon>
#include <QtDebug>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>

namespace LeechCraft
{
namespace Sysnotify
{
	NotificationManager::NotificationManager (QObject *parent)
	: QObject (parent)
	{
		if (!QDBusConnection::sessionBus ().interface ()->
				isServiceRegistered ("org.freedesktop.Notifications"))
		{
			qWarning () << Q_FUNC_INFO
				<< QDBusConnection::sessionBus ().interface ()->registeredServiceNames ().value ();
			return;
		}

		Connection_.reset (new QDBusInterface ("org.freedesktop.Notifications",
					"/org/freedesktop/Notifications"));
		if (!Connection_->isValid ())
			qWarning () << Q_FUNC_INFO
					<< Connection_->lastError ();

		auto pendingSI = Connection_->asyncCall ("GetServerInformation");
		connect (new QDBusPendingCallWatcher (pendingSI, this),
				SIGNAL (finished (QDBusPendingCallWatcher*)),
				this,
				SLOT (handleGotServerInfo (QDBusPendingCallWatcher*)));

		connect (Connection_.get (),
				SIGNAL (ActionInvoked (uint, QString)),
				this,
				SLOT (handleActionInvoked (uint, QString)));
		connect (Connection_.get (),
				SIGNAL (NotificationClosed (uint, uint)),
				this,
				SLOT (handleNotificationClosed (uint)));
	}

	bool NotificationManager::CouldNotify (const Entity& e) const
	{
		return Connection_.get () &&
				Connection_->isValid () &&
				e.Mime_ == "x-leechcraft/notification" &&
				e.Additional_ ["Priority"].toInt () != PLog_ &&
				!e.Additional_ ["Text"].toString ().isEmpty ();
	}

	void NotificationManager::HandleNotification (const Entity& e)
	{
		if (!Connection_.get ())
			return;

		QStringList actions = e.Additional_ ["NotificationActions"].toStringList ();
		if (actions.isEmpty ())
		{
			DoNotify (e, false);
			return;
		}

		auto pending = Connection_->asyncCall ("GetCapabilities");
		auto watcher = new QDBusPendingCallWatcher (pending, this);
		Watcher2CapCheck_ [watcher] = { e };
		connect (watcher,
				SIGNAL (finished (QDBusPendingCallWatcher*)),
				this,
				SLOT (handleCapCheckCallFinished (QDBusPendingCallWatcher*)));
	}

	void NotificationManager::DoNotify (const Entity& e, bool hasActions)
	{
		Priority prio = static_cast<Priority> (e.Additional_ ["Priority"].toInt ());
		QString header = e.Entity_.toString ();
		QString text = e.Additional_ ["Text"].toString ();
		bool uus = e.Additional_ ["UntilUserSees"].toBool ();

		QStringList fmtActions;
		QStringList actions;
		if (hasActions)
		{
			actions = e.Additional_ ["NotificationActions"].toStringList ();
			int i = 0;
			Q_FOREACH (QString action, actions)
				fmtActions << QString::number (i++) << action;
		}

		if (prio == PLog_)
			return;

		int timeout = 0;
		if (!uus)
			timeout = 5000;

		QList<QVariant> arguments;
		arguments << header
			<< uint (0)
			<< QString ("leechcraft_main")
			<< QString ()
			<< text
			<< fmtActions
			<< QVariantMap ()
			<< timeout;

		ActionData ad =
		{
			e,
			e.Additional_ ["HandlingObject"].value<QObject_ptr> (),
			actions
		};

		auto pending = Connection_->asyncCallWithArgumentList ("Notify", arguments);
		auto watcher = new QDBusPendingCallWatcher (pending, this);
		Watcher2AD_ [watcher] = ad;
		connect (watcher,
				SIGNAL (finished (QDBusPendingCallWatcher*)),
				this,
				SLOT (handleNotificationCallFinished (QDBusPendingCallWatcher*)));
	}

	void NotificationManager::handleGotServerInfo (QDBusPendingCallWatcher *w)
	{
		w->deleteLater ();

		QDBusPendingReply<QString, QString, QString, QString> reply = *w;
		if (reply.isError ())
		{
			qWarning () << Q_FUNC_INFO
					<< reply.error ().name ()
					<< reply.error ().message ();
			Connection_.reset ();
			return;
		}

		const auto& vendor = reply.argumentAt<1> ();
		qDebug () << Q_FUNC_INFO
				<< "using"
				<< reply.argumentAt<0> ()
				<< vendor
				<< reply.argumentAt<2> ()
				<< reply.argumentAt<3> ();

		if (vendor == "LeechCraft")
			Connection_.reset ();
	}

	void NotificationManager::handleNotificationCallFinished (QDBusPendingCallWatcher *w)
	{
		QDBusPendingReply<uint> reply = *w;
		if (reply.isError ())
		{
			qWarning () << Q_FUNC_INFO
					<< reply.error ().name ()
					<< reply.error ().message ();
			return;
		}
		int id = reply.argumentAt<0> ();
		CallID2AD_ [id] = Watcher2AD_ [w];
		Watcher2AD_.remove (w);

		w->deleteLater ();
	}

	void NotificationManager::handleCapCheckCallFinished (QDBusPendingCallWatcher *w)
	{
		QDBusPendingReply<QStringList> reply = *w;
		if (reply.isError ())
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to handle notification, failed to query caps:"
				<< reply.error ().name ()
				<< reply.error ().message ();
			return;
		}
		QStringList caps = reply.argumentAt<0> ();
		bool hasActions = caps.contains ("actions");
		Entity e = Watcher2CapCheck_.take (w).Entity_;
		DoNotify (e, hasActions);
	}

	void NotificationManager::handleActionInvoked (uint id, QString action)
	{
		const ActionData& ad = CallID2AD_.take (id);
		if (!ad.Handler_)
		{
			qWarning () << Q_FUNC_INFO
					<< "handler already destroyed";
			return;
		}

		int idx = action.toInt ();

		QMetaObject::invokeMethod (ad.Handler_.get (),
				"notificationActionTriggered",
				Qt::QueuedConnection,
				Q_ARG (int, idx));
	}

	void NotificationManager::handleNotificationClosed (uint id)
	{
		CallID2AD_.remove (id);
	}
}
}
