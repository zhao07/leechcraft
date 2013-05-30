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

#include "protocol.h"
#include <QIcon>
#include <QFormLayout>
#include <QtDebug>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/structures.h>
#include "account.h"
#include "accregfirstpage.h"

namespace LeechCraft
{
namespace Azoth
{
namespace VelvetBird
{
	Protocol::Protocol (PurplePlugin *plug, ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, PPlug_ (plug)
	{
	}

	void Protocol::Release ()
	{
		for (auto acc : Accounts_)
		{
			acc->Release ();
			emit accountRemoved (acc);
		}
	}

	QObject* Protocol::GetQObject()
	{
		return this;
	}

	IProtocol::ProtocolFeatures Protocol::GetFeatures () const
	{
		return PFNone;
	}

	QList<QObject*> Protocol::GetRegisteredAccounts ()
	{
		QList<QObject*> result;
		for (auto acc : Accounts_)
			result << acc;
		return result;
	}

	QObject* Protocol::GetParentProtocolPlugin () const
	{
		return parent ();
	}

	QString Protocol::GetProtocolName () const
	{
		return QString::fromUtf8 (purple_plugin_get_name (PPlug_)) + " (by libpurple)";
	}

	QIcon Protocol::GetProtocolIcon () const
	{
		auto id = GetPurpleID ();
		if (id.startsWith ("prpl-"))
			id.remove (0, 5);

		QIcon result = QIcon::fromTheme (QString::fromUtf8 ("im-" + id));
		if (result.isNull ())
			result = QIcon ("lcicons:/azoth/velvetbird/resources/images/velvetbird.svg");
		return result;
	}

	QByteArray Protocol::GetProtocolID () const
	{
		return "VelvetBird." + GetPurpleID ();
	}

	QList<QWidget*> Protocol::GetAccountRegistrationWidgets (IProtocol::AccountAddOptions opts)
	{
		auto nameWidget = new AccRegFirstPage ();
		return { nameWidget };
	}

	void Protocol::RegisterAccount (const QString& name, const QList<QWidget*>& widgets)
	{
		auto nameWidget = dynamic_cast<AccRegFirstPage*> (widgets.value (0));
		if (!nameWidget)
		{
			qWarning () << Q_FUNC_INFO
					<< "incorrect widgets"
					<< widgets;
			return;
		}

		auto pacc = purple_account_new (nameWidget->GetName ().toUtf8 ().constData (),
				GetPurpleID ().constData ());
		purple_account_set_alias (pacc, nameWidget->GetNick ().toUtf8 ().constData ());
		purple_account_set_string (pacc, "AccountName", name.toUtf8 ().constData ());
		purple_accounts_add (pacc);

		PushAccount (pacc);
	}

	QWidget* Protocol::GetMUCJoinWidget ()
	{
		return 0;
	}

	void Protocol::RemoveAccount (QObject *accObj)
	{
		auto acc = qobject_cast<Account*> (accObj);
		emit accountRemoved (accObj);

		purple_accounts_delete (acc->GetPurpleAcc ());
		delete acc;
	}

	QByteArray Protocol::GetPurpleID () const
	{
		return purple_plugin_get_id (PPlug_);
	}

	void Protocol::PushAccount (PurpleAccount *pacc)
	{
		auto account = new Account (pacc, this);
		Accounts_ << account;
		emit accountAdded (account);

		pacc->ui_data = account;
	}

	ICoreProxy_ptr Protocol::GetCoreProxy () const
	{
		return Proxy_;
	}
}
}
}
