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

#include "notificationruleswidget.h"
#include <algorithm>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <interfaces/an/ianemitter.h>
#include <interfaces/an/constants.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <util/resourceloader.h>
#include <util/util.h>
#include <util/xpc/stdanfields.h>
#include "xmlsettingsmanager.h"
#include "matchconfigdialog.h"
#include "typedmatchers.h"
#include "core.h"
#include "rulesmanager.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	NotificationRulesWidget::NotificationRulesWidget (RulesManager *rm, QWidget *parent)
	: QWidget (parent)
	, RM_ (rm)
	, MatchesModel_ (new QStandardItemModel (this))
	{
		Cat2Types_ [AN::CatIM] << AN::TypeIMAttention
				<< AN::TypeIMIncFile
				<< AN::TypeIMIncMsg
				<< AN::TypeIMMUCHighlight
				<< AN::TypeIMMUCInvite
				<< AN::TypeIMMUCMsg
				<< AN::TypeIMStatusChange
				<< AN::TypeIMSubscrGrant
				<< AN::TypeIMSubscrRequest
				<< AN::TypeIMSubscrRevoke
				<< AN::TypeIMSubscrSub
				<< AN::TypeIMSubscrUnsub;

		Cat2Types_ [AN::CatOrganizer] << AN::TypeOrganizerEventDue;

		Cat2Types_ [AN::CatDownloads] << AN::TypeDownloadError
				<< AN::TypeDownloadFinished;

		Cat2Types_ [AN::CatPackageManager] << AN::TypePackageUpdated;

		Cat2Types_ [AN::CatMediaPlayer] << AN::TypeMediaPlaybackStatus;

		Cat2Types_ [AN::CatGeneric] << AN::TypeGeneric;

		Ui_.setupUi (this);
		Ui_.RulesTree_->setModel (RM_->GetRulesModel ());
		Ui_.MatchesTree_->setModel (MatchesModel_);

		connect (Ui_.RulesTree_->selectionModel (),
				SIGNAL (currentChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleItemSelected (QModelIndex, QModelIndex)));

		const auto& cat2hr = RM_->GetCategory2HR ();
		for (const QString& cat : cat2hr.keys ())
			Ui_.EventCat_->addItem (cat2hr [cat], cat);
		on_EventCat__currentIndexChanged (0);

		XmlSettingsManager::Instance ().RegisterObject ("AudioTheme",
				this, "resetAudioFileBox");
		resetAudioFileBox ();
	}

	void NotificationRulesWidget::ResetMatchesModel ()
	{
		MatchesModel_->clear ();
		MatchesModel_->setHorizontalHeaderLabels (QStringList (tr ("Field name"))
				<< tr ("Rule description"));
	}

	QString NotificationRulesWidget::GetCurrentCat () const
	{
		const auto idx = Ui_.EventCat_->currentIndex ();
		return Ui_.EventCat_->itemData (idx).toString ();
	}

	QStringList NotificationRulesWidget::GetSelectedTypes () const
	{
		QStringList types;
		for (int i = 0; i < Ui_.EventTypes_->topLevelItemCount (); ++i)
		{
			auto item = Ui_.EventTypes_->topLevelItem (i);
			if (item->checkState (0) == Qt::Checked)
				types << item->data (0, Qt::UserRole).toString ();
		}
		if (types.isEmpty ())
			types = Cat2Types_ [GetCurrentCat ()];
		return types;
	}

	NotificationRule NotificationRulesWidget::GetRuleFromUI () const
	{
		const QStringList& types = GetSelectedTypes ();

		if (types.isEmpty () ||
				Ui_.RuleName_->text ().isEmpty ())
			return NotificationRule ();

		NotificationRule rule (Ui_.RuleName_->text (),
				Ui_.EventCat_->itemData (Ui_.EventCat_->currentIndex ()).toString (),
				types);

		NotificationMethods methods = NMNone;
		if (Ui_.NotifyVisual_->checkState () == Qt::Checked)
			methods |= NMVisual;
		if (Ui_.NotifySysTray_->checkState () == Qt::Checked)
			methods |= NMTray;
		if (Ui_.NotifyAudio_->checkState () == Qt::Checked)
			methods |= NMAudio;
		if (Ui_.NotifyCmd_->checkState () == Qt::Checked)
			methods |= NMCommand;
		if (Ui_.NotifyUrgent_->checkState () == Qt::Checked)
			methods |= NMUrgentHint;
		rule.SetMethods (methods);

		rule.SetFieldMatches (Matches_);

		const int audioIdx = Ui_.AudioFile_->currentIndex ();
		const QString& audioFile = audioIdx >= 0 ?
				Ui_.AudioFile_->itemText (audioIdx) :
				QString ();
		rule.SetAudioParams (AudioParams (audioFile));

		QStringList cmdArgs;
		for (int i = 0; i < Ui_.CommandArgsTree_->topLevelItemCount (); ++i)
			cmdArgs << Ui_.CommandArgsTree_->topLevelItem (i)->text (0);
		rule.SetCmdParams (CmdParams (Ui_.CommandLineEdit_->text ().simplified (), cmdArgs));

		const QModelIndex& curIdx = Ui_.RulesTree_->currentIndex ();
		rule.SetEnabled (curIdx.sibling (curIdx.row (), 0).data (Qt::CheckStateRole) == Qt::Checked);
		rule.SetSingleShot (Ui_.RuleSingleShot_->checkState () == Qt::Checked);

		return rule;
	}

	namespace
	{
		QList<ANFieldData> GetPluginFields (const QByteArray& pluginId)
		{
			QList<ANFieldData> fields;
			if (pluginId.isEmpty ())
				return Util::GetStdANFields ({});
			else
			{
				auto pObj = Core::Instance ().GetProxy ()->
						GetPluginsManager ()->GetPluginByID (pluginId);
				auto iae = qobject_cast<IANEmitter*> (pObj);
				if (!iae)
					qWarning () << Q_FUNC_INFO
							<< pObj
							<< "doesn't implement IANEmitter";
				else
					return iae->GetANFields ();
			}

			return {};
		}
	}

	QList<QStandardItem*> NotificationRulesWidget::MatchToRow (const FieldMatch& match) const
	{
		auto fieldName = match.GetFieldName ();

		const auto& fields = GetPluginFields (match.GetPluginID ().toUtf8 ());

		const auto pos = std::find_if (fields.begin (), fields.end (),
				[&fieldName] (decltype (fields.front ()) field) { return field.ID_ == fieldName; });
		if (pos != fields.end ())
			fieldName = pos->Name_;
		else
			qWarning () << Q_FUNC_INFO
					<< "unable to find field"
					<< fieldName;

		QList<QStandardItem*> items;
		items << new QStandardItem (fieldName);
		items << new QStandardItem (match.GetMatcher () ?
				match.GetMatcher ()->GetHRDescription () :
				tr ("<empty matcher>"));
		return items;
	}

	QMap<QObject*, QList<ANFieldData>> NotificationRulesWidget::GetRelevantANFieldsWPlugins () const
	{
		QMap<QObject*, QList<ANFieldData>> result;
		result [nullptr] += Util::GetStdANFields (GetCurrentCat ());
		for (const auto& type : GetSelectedTypes ())
			result [nullptr] += Util::GetStdANFields (type);

		const auto& emitters = Core::Instance ().GetProxy ()->
				GetPluginsManager ()->GetAllCastableRoots<IANEmitter*> ();
		for (auto emitterObj : emitters)
		{
			auto emitter = qobject_cast<IANEmitter*> (emitterObj);
			for (const auto& field : emitter->GetANFields ())
				if (!GetSelectedTypes ().toSet ().intersect (field.EventTypes_.toSet ()).isEmpty ())
					result [emitterObj] << field;
		}

		return result;
	}

	QList<ANFieldData> NotificationRulesWidget::GetRelevantANFields () const
	{
		QList<ANFieldData> result;
		for (const auto& sublist : GetRelevantANFieldsWPlugins ())
			result += sublist;
		return result;
	}

	QString NotificationRulesWidget::GetArgumentText ()
	{
		const auto& fields = GetRelevantANFields ();

		if (fields.isEmpty ())
			return QInputDialog::getText (this,
					"LeechCraft",
					tr ("Please enter the argument:"));

		QStringList items;
		for (const auto& field : fields)
			items << tr ("Custom field %1 (%2)")
					.arg (field.Name_)
					.arg (field.Description_);

		bool ok = false;
		const auto& value = QInputDialog::getItem (this,
				"LeechCraft",
				tr ("Please enter the argument:"),
				items,
				0,
				true,
				&ok);

		if (value.isEmpty () || !ok)
			return {};

		const auto idx = items.indexOf (value);
		return idx >= 0 ?
				('$' + fields.value (idx).ID_) :
				value;
	}

	void NotificationRulesWidget::handleItemSelected (const QModelIndex& index, const QModelIndex& prevIndex)
	{
		if (prevIndex.isValid ())
		{
			const auto& prevRule = RM_->GetRulesList ().value (prevIndex.row ());
			const auto& uiRule = GetRuleFromUI ();
			if (uiRule != prevRule &&
					QMessageBox::question (this,
							"LeechCraft",
							tr ("The rule has been changed. Do you want to save it?"),
							QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
				RM_->UpdateRule (prevIndex, uiRule);
		}

		resetAudioFileBox ();
		Ui_.CommandArgsTree_->clear ();
		Ui_.CommandLineEdit_->setText (QString ());

		const auto& rule = RM_->GetRulesList ().value (index.row ());

		const int catIdx = Ui_.EventCat_->findData (rule.GetCategory ());
		Ui_.EventCat_->setCurrentIndex (std::max (catIdx, 0));

		const auto& types = rule.GetTypes ();
		for (int i = 0; i < Ui_.EventTypes_->topLevelItemCount (); ++i)
		{
			QTreeWidgetItem *item = Ui_.EventTypes_->topLevelItem (i);
			const bool cont = types.contains (item->data (0, Qt::UserRole).toString ());
			item->setCheckState (0, cont ? Qt::Checked : Qt::Unchecked);
		}

		Ui_.RuleName_->setText (rule.GetName ());

		const NotificationMethods methods = rule.GetMethods ();
		Ui_.NotifyVisual_->setCheckState ((methods & NMVisual) ?
				Qt::Checked : Qt::Unchecked);
		Ui_.NotifySysTray_->setCheckState ((methods & NMTray) ?
				Qt::Checked : Qt::Unchecked);
		Ui_.NotifyAudio_->setCheckState ((methods & NMAudio) ?
				Qt::Checked : Qt::Unchecked);
		Ui_.NotifyCmd_->setCheckState ((methods & NMCommand) ?
				Qt::Checked : Qt::Unchecked);
		Ui_.NotifyUrgent_->setCheckState ((methods & NMUrgentHint) ?
				Qt::Checked : Qt::Unchecked);

		ResetMatchesModel ();
		Matches_ = rule.GetFieldMatches ();
		for (const auto& m : Matches_)
			MatchesModel_->appendRow (MatchToRow (m));

		const AudioParams& params = rule.GetAudioParams ();
		if (params.Filename_.isEmpty ())
			Ui_.AudioFile_->setCurrentIndex (-1);
		else
		{
			int idx = Ui_.AudioFile_->findData (params.Filename_);
			if (idx == -1)
				idx = Ui_.AudioFile_->findText (params.Filename_);

			if (idx == -1)
			{
				Ui_.AudioFile_->insertItem (0, params.Filename_, params.Filename_);
				idx = 0;
			}

			Ui_.AudioFile_->setCurrentIndex (idx);
		}

		const CmdParams& cmdParams = rule.GetCmdParams ();
		if (!cmdParams.Cmd_.isEmpty ())
		{
			Ui_.CommandLineEdit_->setText (cmdParams.Cmd_);

			for (const auto& arg : cmdParams.Args_)
				new QTreeWidgetItem (Ui_.CommandArgsTree_, QStringList (arg));
		}

		Ui_.RuleSingleShot_->setChecked (rule.IsSingleShot () ?
					Qt::Checked :
					Qt::Unchecked);
	}

	void NotificationRulesWidget::on_AddRule__released ()
	{
		RM_->prependRule ();
	}

	void NotificationRulesWidget::on_UpdateRule__released ()
	{
		const auto& index = Ui_.RulesTree_->currentIndex ();
		if (!index.isValid ())
			return;

		RM_->UpdateRule (index, GetRuleFromUI ());
		Ui_.RulesTree_->setCurrentIndex (index);
	}

	void NotificationRulesWidget::on_MoveRuleUp__released ()
	{
		RM_->moveUp (Ui_.RulesTree_->currentIndex ());
	}

	void NotificationRulesWidget::on_MoveRuleDown__released ()
	{
		RM_->moveDown (Ui_.RulesTree_->currentIndex ());
	}

	void NotificationRulesWidget::on_RemoveRule__released ()
	{
		const QModelIndex& index = Ui_.RulesTree_->currentIndex ();
		if (!index.isValid ())
			return;

		RM_->removeRule (index);
	}

	void NotificationRulesWidget::on_DefaultRules__released ()
	{
		if (QMessageBox::question (this,
					"LeechCraft",
					tr ("Are you sure you want to replace all rules with "
						"the default set?"),
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		RM_->reset ();
	}

	void NotificationRulesWidget::on_AddMatch__released ()
	{
		MatchConfigDialog dia (GetCurrentCat (), GetSelectedTypes (), this);
		if (dia.exec () != QDialog::Accepted)
			return;

		const FieldMatch& match = dia.GetFieldMatch ();
		Matches_ << match;
		MatchesModel_->appendRow (MatchToRow (match));
	}

	void NotificationRulesWidget::on_ModifyMatch__released ()
	{
		const QModelIndex& index = Ui_.MatchesTree_->currentIndex ();
		if (!index.isValid ())
			return;

		MatchConfigDialog dia (GetCurrentCat (), GetSelectedTypes (), this);
		if (dia.exec () != QDialog::Accepted)
			return;

		const int row = index.row ();

		const FieldMatch& match = dia.GetFieldMatch ();
		Matches_ [row] = match;
		int i = 0;
		for (QStandardItem *item : MatchToRow (match))
			MatchesModel_->setItem (row, i++, item);
	}

	void NotificationRulesWidget::on_RemoveMatch__released ()
	{
		const QModelIndex& index = Ui_.MatchesTree_->currentIndex ();
		if (!index.isValid ())
			return;

		Matches_.removeAt (index.row ());
		MatchesModel_->removeRow (index.row ());
	}

	void NotificationRulesWidget::on_EventCat__currentIndexChanged (int)
	{
		const auto& catId = GetCurrentCat ();
		Ui_.EventTypes_->clear ();

		for (const QString& type : Cat2Types_ [catId])
		{
			const QString& hr = RM_->GetType2HR () [type];
			QTreeWidgetItem *item = new QTreeWidgetItem (QStringList (hr));
			item->setData (0, Qt::UserRole, type);
			item->setCheckState (0, Qt::Unchecked);
			Ui_.EventTypes_->addTopLevelItem (item);
		}
	}

	void NotificationRulesWidget::on_NotifyVisual__stateChanged (int)
	{
	}

	void NotificationRulesWidget::on_NotifySysTray__stateChanged (int)
	{
	}

	void NotificationRulesWidget::on_NotifyAudio__stateChanged (int state)
	{
		Ui_.PageAudio_->setEnabled (state == Qt::Checked);
	}

	void NotificationRulesWidget::on_NotifyCmd__stateChanged (int state)
	{
		Ui_.PageCommand_->setEnabled (state == Qt::Checked);
	}

	void NotificationRulesWidget::on_BrowseAudioFile__released ()
	{
		const QString& fname = QFileDialog::getOpenFileName (this,
				tr ("Select audio file"),
				QDir::homePath (),
				tr ("Audio files (*.ogg *.wav *.flac *.mp3);;All files (*.*)"));
		if (fname.isEmpty ())
			return;

		const bool shouldReplace = Ui_.AudioFile_->count () &&
				Ui_.AudioFile_->itemText (0) == Ui_.AudioFile_->itemData (0);
		if (shouldReplace)
		{
			Ui_.AudioFile_->setItemText (0, fname);
			Ui_.AudioFile_->setItemData (0, fname);
		}
		else
			Ui_.AudioFile_->insertItem (0, fname, fname);

		Ui_.AudioFile_->setCurrentIndex (0);
	}

	void NotificationRulesWidget::on_TestAudio__released ()
	{
		const int idx = Ui_.AudioFile_->currentIndex ();
		if (idx == -1)
			return;

		const QString& path = Ui_.AudioFile_->itemData (idx).toString ();
		if (path.isEmpty ())
			return;

		const Entity& e = Util::MakeEntity (path, QString (), Internal);
		Core::Instance ().SendEntity (e);
	}

	void NotificationRulesWidget::on_AddArgument__released()
	{
		const auto& text = GetArgumentText ();
		if (text.isEmpty ())
			return;

		new QTreeWidgetItem (Ui_.CommandArgsTree_, QStringList (text));
	}

	void NotificationRulesWidget::on_ModifyArgument__released()
	{
		QTreeWidgetItem *item = Ui_.CommandArgsTree_->currentItem ();
		if (!item)
			return;

		const auto& text = GetArgumentText ();
		if (text.isEmpty ())
			return;

		item->setText (0, text);
	}

	void NotificationRulesWidget::on_RemoveArgument__released()
	{
		const QModelIndex& index = Ui_.CommandArgsTree_->currentIndex ();
		if (!index.isValid ())
			return;

		delete Ui_.CommandArgsTree_->takeTopLevelItem (index.row ());
	}

	void NotificationRulesWidget::resetAudioFileBox ()
	{
		Ui_.AudioFile_->clear ();

		const QString& theme = XmlSettingsManager::Instance ().property ("AudioTheme").toString ();
		const QStringList filters = QStringList ("*.ogg")
				<< "*.wav"
				<< "*.flac"
				<< "*.mp3";

		const QFileInfoList& files = Core::Instance ()
				.GetAudioThemeLoader ()->List (theme,
						filters, QDir::Files | QDir::Readable);
		Q_FOREACH (const QFileInfo& file, files)
			Ui_.AudioFile_->addItem (file.baseName (), file.absoluteFilePath ());
	}
}
}
