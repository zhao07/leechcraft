/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCACCOUNT_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCACCOUNT_H

#include <memory>
#include <QObject>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/ihaveconsole.h>
#include <interfaces/azoth/isupportbookmarks.h>
#include "core.h"
#include "localtypes.h"

namespace LeechCraft
{
namespace Azoth
{

class IProtocol;

namespace Acetamide
{

	class ClientConnection;
	class IrcProtocol;
	class IrcAccountConfigurationWidget;

	class IrcAccount : public QObject
						, public IAccount
						, public IHaveConsole
						, public ISupportBookmarks
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IAccount
				LeechCraft::Azoth::IHaveConsole
				LeechCraft::Azoth::ISupportBookmarks)

		QString AccountName_;
		IrcProtocol *ParentProtocol_;
		QByteArray AccountID_;

		QString RealName_;
		QString UserName_;
		QStringList NickNames_;
		QString DefaultServer_;
		int DefaultPort_;
		QString DefaultEncoding_;
		QString DefaultChannel_;
		State IrcAccountState_;

		std::shared_ptr<ClientConnection> ClientConnection_;
		bool IsFirstStart_;
		QList<IrcBookmark> ActiveChannels_;
	public:
		IrcAccount (const QString&, QObject*);
		void Init ();

		QObject* GetQObject ();
		QObject* GetParentProtocol () const;
		AccountFeatures GetAccountFeatures () const;
		QList<QObject*> GetCLEntries ();
		void QueryInfo (const QString&);

		QString GetAccountName () const;
		QString GetOurNick () const;
		QString GetUserName () const;
		QString GetRealName () const;
		QStringList GetNickNames () const;

		std::shared_ptr<ClientConnection> GetClientConnection () const;

		void RenameAccount (const QString&);

		QByteArray GetAccountID () const;
		void SetAccountID (const QString&);

		QList<QAction*> GetActions () const;

		void OpenConfigurationDialog ();
		void FillSettings (IrcAccountConfigurationWidget*);

		void JoinServer (ServerOptions, ChannelOptions, bool = false);

		void SetBookmarks (const QList<IrcBookmark>&);
		QList<IrcBookmark> GetBookmarks () const;

		QWidget* GetMUCBookmarkEditorWidget ();
		QVariantList GetBookmarkedMUCs () const;
		void SetBookmarkedMUCs (const QVariantList&);

		EntryStatus GetState () const;
		void ChangeState (const EntryStatus&);
		void SetState (const EntryStatus& status);
		void Authorize (QObject*);
		void DenyAuth (QObject*);
		void RequestAuth (const QString&, const QString&,
				const QString&, const QStringList&);
		void RemoveEntry (QObject*);
		QObject* GetTransferManager () const;

		PacketFormat GetPacketFormat () const;
		void SetConsoleEnabled (bool);
		QByteArray Serialize () const;
		static IrcAccount* Deserialize (const QByteArray&, QObject*);
	private:
		void SaveActiveChannels ();
	public slots:
		void handleEntryRemoved (QObject*);
		void handleGotRosterItems (const QList<QObject*>&);
	private slots:
		void handleDestroyClient ();
		void joinFromBookmarks ();
	signals:
		void gotCLItems (const QList<QObject*>&);
		void removedCLItems (const QList<QObject*>&);
		void accountRenamed (const QString&);
		void authorizationRequested (QObject*, const QString&);
		void itemSubscribed (QObject*, const QString&);
		void itemUnsubscribed (QObject*, const QString&);
		void itemUnsubscribed (const QString&, const QString&);
		void itemCancelledSubscription (QObject*, const QString&);
		void itemGrantedSubscription (QObject*, const QString&);
		void statusChanged (const EntryStatus&);
		void mucInvitationReceived (const QVariantMap&,
				const QString&, const QString&);

		void accountSettingsChanged ();

		void scheduleClientDestruction ();

		void gotConsolePacket (const QByteArray&, IHaveConsole::PacketDirection, const QString&);

		void bookmarksChanged ();
	};

	typedef std::shared_ptr<IrcAccount> IrcAccount_ptr;
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCACCOUNT_H
