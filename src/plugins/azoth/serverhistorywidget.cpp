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

#include "serverhistorywidget.h"
#include <QtDebug>
#include <QSortFilterProxyModel>
#include <util/gui/clearlineeditaddon.h>
#include <interfaces/azoth/ihaveserverhistory.h>
#include "proxyobject.h"
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	namespace
	{
		const auto MaxMsgCount = 25;
	}

	ServerHistoryWidget::ServerHistoryWidget (QObject *account, QWidget *parent)
	: QWidget { parent }
	, Toolbar_ { new QToolBar { this } }
	, AccObj_ { account }
	, IHSH_ { qobject_cast<IHaveServerHistory*> (account) }
	, ContactsFilter_ { new QSortFilterProxyModel { this } }
	{
		Ui_.setupUi (this);

		if (!IHSH_)
		{
			qWarning () << Q_FUNC_INFO
					<< "account doesn't implement IHaveServerHistory"
					<< account;
			return;
		}

		new Util::ClearLineEditAddon (Core::Instance ().GetProxy (), Ui_.ContactsFilter_);

		ContactsFilter_->setFilterCaseSensitivity (Qt::CaseInsensitive);

		const auto& sortParams = IHSH_->GetSortParams ();
		ContactsFilter_->setSortRole (sortParams.Role_);
		ContactsFilter_->setSortCaseSensitivity (Qt::CaseInsensitive);

		ContactsFilter_->setDynamicSortFilter (true);
		ContactsFilter_->setSourceModel (IHSH_->GetServerContactsModel ());

		Ui_.ContactsView_->setModel (ContactsFilter_);
		Ui_.ContactsView_->sortByColumn (sortParams.Column_, sortParams.Order_);

		connect (AccObj_,
				SIGNAL (serverHistoryFetched (QModelIndex, QByteArray, SrvHistMessages_t)),
				this,
				SLOT (handleFetched (QModelIndex, QByteArray, SrvHistMessages_t)));

		connect (Ui_.ContactsFilter_,
				SIGNAL (textChanged (QString)),
				ContactsFilter_,
				SLOT (setFilterFixedString (QString)));

		auto prevAct = Toolbar_->addAction (tr ("Previous page"),
				this, SLOT (navigatePrevious ()));
		prevAct->setProperty ("ActionIcon", "go-previous");

		auto nextAct = Toolbar_->addAction (tr ("Next page"),
				this, SLOT (navigateNext ()));
		nextAct->setProperty ("ActionIcon", "go-next");
	}

	void ServerHistoryWidget::SetTabInfo (QObject *plugin, const TabClassInfo& tc)
	{
		PluginObj_ = plugin;
		TC_ = tc;
	}

	TabClassInfo ServerHistoryWidget::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* ServerHistoryWidget::ParentMultiTabs ()
	{
		return PluginObj_;
	}

	void ServerHistoryWidget::Remove ()
	{
		emit removeTab (this);
	}

	QToolBar* ServerHistoryWidget::GetToolBar () const
	{
		return Toolbar_;
	}

	int ServerHistoryWidget::GetReqMsgCount () const
	{
		return std::max (20, FirstMsgCount_);
	}

	void ServerHistoryWidget::handleFetched (const QModelIndex& index,
			const QByteArray& startId, const SrvHistMessages_t& messages)
	{
		if (index.row () != ContactsFilter_->mapToSource (Ui_.ContactsView_->currentIndex ()).row ())
			return;

		if (FirstMsgCount_ == -1)
			FirstMsgCount_ = messages.size ();

		CurrentID_ = startId;

		Ui_.MessagesView_->clear ();

		const auto& bgColor = palette ().color (QPalette::Base);
		const auto& colors = Core::Instance ().GenerateColors ("hash", bgColor);

		QString preNick = XmlSettingsManager::Instance ().property ("PreNickText").toString ();
		QString postNick = XmlSettingsManager::Instance ().property ("PostNickText").toString ();
		preNick.replace ('<', "&lt;");
		postNick.replace ('<', "&lt;");

		for (const auto& message : messages)
		{
			const auto& color = Core::Instance ().GetNickColor (message.Nick_, colors);

			auto msgText = message.RichBody_;
			if (msgText.isEmpty ())
			{
				msgText = Qt::escape (message.Body_);
				ProxyObject {}.FormatLinks (msgText);
				msgText.replace ('\n', "<br/>");
			}

			QString html = "[" + message.TS_.toString () + "] " + preNick;
			html += "<font color='" + color + "'>" + message.Nick_ + "</font> ";
			html += postNick + ' ' + msgText;

			html.prepend (QString ("<font color='#") +
					(message.Dir_ == IMessage::DIn ? "0000dd" : "dd0000") +
					"'>");
			html += "</font>";

			Ui_.MessagesView_->append (html);
		}

		MaxID_ = messages.value (0).ID_;
	}

	void ServerHistoryWidget::on_ContactsView__activated (const QModelIndex& index)
	{
		CurrentID_ = "-1";
		MaxID_ = "-1";
		FirstMsgCount_ = -1;
		IHSH_->FetchServerHistory (ContactsFilter_->mapToSource (index), CurrentID_, MaxMsgCount);
	}

	void ServerHistoryWidget::on_MessagesView__anchorClicked (const QUrl& url)
	{
		const auto& current = Ui_.ContactsView_->currentIndex ();
		const auto entryObj = current.data (ServerHistoryRole::CLEntry).value<QObject*> ();
		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		Core::Instance ().HandleURLGeneric (url, true, entry);
	}

	void ServerHistoryWidget::navigatePrevious ()
	{
		const auto& index = ContactsFilter_->mapToSource (Ui_.ContactsView_->currentIndex ());
		IHSH_->FetchServerHistory (index,
				MaxID_, GetReqMsgCount ());
	}

	void ServerHistoryWidget::navigateNext ()
	{
		const auto& index = ContactsFilter_->mapToSource (Ui_.ContactsView_->currentIndex ());
		IHSH_->FetchServerHistory (index,
				CurrentID_, -GetReqMsgCount ());
	}
}
}
