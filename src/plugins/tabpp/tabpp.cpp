/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#include "tabpp.h"
#include <QIcon>
#include <QAction>
#include <QMainWindow>
#include "core.h"
#include "tabppwidget.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace TabPP
		{
			void Plugin::Init (ICoreProxy_ptr proxy)
			{
				XmlSettingsDialog_.reset (new LeechCraft::Util::XmlSettingsDialog ());
				XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
						"tabppsettings.xml");

				Core::Instance ().SetProxy (proxy);

				Dock_ = new TabPPWidget (tr ("Tab++"), proxy->GetMainWindow ());
			}

			void Plugin::SecondInit ()
			{
			}

			void Plugin::Release ()
			{
				Dock_->Release ();
			}

			QString Plugin::GetName () const
			{
				return "Tab++";
			}

			QString Plugin::GetInfo () const
			{
				return tr ("Enhances user experience with tabs.");
			}

			QIcon Plugin::GetIcon () const
			{
				return QIcon ();
			}

			QStringList Plugin::Provides () const
			{
				return QStringList ();
			}

			QStringList Plugin::Needs () const
			{
				return QStringList ();
			}

			QStringList Plugin::Uses () const
			{
				return QStringList ();
			}

			void Plugin::SetProvider (QObject*, const QString&)
			{
			}

			QList<QAction*> Plugin::GetActions () const
			{
				QList<QAction*> result;
				result << Dock_->GetActivatorAction ();
				return result;
			}

			boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> Plugin::GetSettingsDialog () const
			{
				return XmlSettingsDialog_;
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_tabpp, LeechCraft::Plugins::TabPP::Plugin);

