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

#pragma once

#include <QObject>
#include "notificationrule.h"

class QAbstractItemModel;
class QStandardItemModel;
class QStandardItem;
class QModelIndex;

namespace LeechCraft
{
struct Entity;

namespace AdvancedNotifications
{
	class RulesManager : public QObject
	{
		Q_OBJECT

		QList<NotificationRule> Rules_;
		QStandardItemModel *RulesModel_;

		QMap<QString, QString> Cat2HR_;
		QMap<QString, QString> Type2HR_;
	public:
		RulesManager (QObject* = 0);

		QAbstractItemModel* GetRulesModel () const;
		QList<NotificationRule> GetRulesList () const;

		const QMap<QString, QString>& GetCategory2HR () const;
		const QMap<QString, QString>& GetType2HR () const;

		void SetRuleEnabled (const NotificationRule&, bool);
		void UpdateRule (const QModelIndex&, const NotificationRule&);

		void HandleEntity (const Entity&);
	private:
		void LoadDefaultRules (int = -1);
		void LoadSettings ();
		void ResetModel ();
		void SaveSettings () const;

		QList<QStandardItem*> RuleToRow (const NotificationRule&) const;
	public slots:
		void prependRule ();
		void removeRule (const QModelIndex&);
		void moveUp (const QModelIndex&);
		void moveDown (const QModelIndex&);
		void setRuleEnabled (int index, bool enabled);
		void reset ();

		QVariant getRulesModel () const;
	private slots:
		void handleItemChanged (QStandardItem*);
	};
}
}
