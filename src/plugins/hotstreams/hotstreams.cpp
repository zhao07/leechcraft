/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "hotstreams.h"
#include <QIcon>
#include <QStandardItem>
#include <QTimer>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include "somafmlistfetcher.h"
#include "stealkilllistfetcher.h"
#include "icecastfetcher.h"

#ifdef HAVE_QJSON
#include "audioaddictstreamfetcher.h"
#include "rockradiolistfetcher.h"
#endif

#include "radiostation.h"
#include "roles.h"
#include "stringlistradiostation.h"

Q_DECLARE_METATYPE (QList<QUrl>);

namespace LeechCraft
{
namespace HotStreams
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("hotstreams");

		Proxy_ = proxy;

		auto nam = Proxy_->GetNetworkAccessManager ();

#ifdef HAVE_QJSON
		auto di = new QStandardItem ("Digitally Imported");
		di->setData (Media::RadioType::None, Media::RadioItemRole::ItemType);
		di->setEditable (false);
		di->setIcon (QIcon (":/hotstreams/resources/images/di.png"));
		Roots_ ["di"] = di;
		Root2Fetcher_ [di] = [nam, this] (QStandardItem *di)
			{
				new AudioAddictStreamFetcher (AudioAddictStreamFetcher::Service::DI,
						di, nam, this);
			};

		auto sky = new QStandardItem ("SkyFM");
		sky->setData (Media::RadioType::None, Media::RadioItemRole::ItemType);
		sky->setEditable (false);
		sky->setIcon (QIcon (":/hotstreams/resources/images/skyfm.png"));
		Roots_ ["sky"] = sky;
		Root2Fetcher_ [sky] = [nam, this] (QStandardItem *sky)
			{
				new AudioAddictStreamFetcher (AudioAddictStreamFetcher::Service::SkyFM,
						sky, nam, this);
			};

		auto rr = new QStandardItem ("RockRadio");
		rr->setData (Media::RadioType::None, Media::RadioItemRole::ItemType);
		rr->setEditable (false);
		rr->setIcon (QIcon (":/hotstreams/resources/images/rockradio.png"));
		Roots_ ["rr"] = rr;
		Root2Fetcher_ [rr] = [nam, this] (QStandardItem *rr)
				{ new RockRadioListFetcher (rr, nam, this); };
#endif

		auto somafm = new QStandardItem ("SomaFM");
		somafm->setData (Media::RadioType::None, Media::RadioItemRole::ItemType);
		somafm->setEditable (false);
		somafm->setIcon (QIcon (":/hotstreams/resources/images/somafm.png"));
		Roots_ ["somafm"] = somafm;
		Root2Fetcher_ [somafm] = [nam, this] (QStandardItem *somafm)
				{ new SomaFMListFetcher (somafm, nam, this); };

		auto stealkill = new QStandardItem ("42fm");
		stealkill->setData (Media::RadioType::None, Media::RadioItemRole::ItemType);
		stealkill->setEditable (false);
		stealkill->setIcon (QIcon (":/hotstreams/resources/images/radio.png"));
		Roots_ ["42fm"] = stealkill;
		Root2Fetcher_ [stealkill] = [nam, this] (QStandardItem *stealkill)
				{ new StealKillListFetcher (stealkill, nam, this); };

		auto icecast = new QStandardItem ("Icecast");
		icecast->setData (Media::RadioType::None, Media::RadioItemRole::ItemType);
		icecast->setEditable (false);
		icecast->setIcon (QIcon (":/hotstreams/resources/images/radio.png"));
		Roots_ ["icecast"] = icecast;
		Root2Fetcher_ [icecast] = [nam, this] (QStandardItem *icecast)
			{
				auto icecastFetcher = new IcecastFetcher (icecast, nam, this);
				connect (icecastFetcher,
						SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)),
						this,
						SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)));
			};
	}

	void Plugin::SecondInit ()
	{
		QTimer::singleShot (5000,
				this,
				SLOT (refreshRadios ()));
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.HotStreams";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "HotStreams";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides some radio streams like Digitally Imported and SomaFM to other plugins.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon ("lcicons:/hotstreams/resources/images/hotstreams.svg");
		return icon;
	}

	QList<QStandardItem*> Plugin::GetRadioListItems () const
	{
		return Roots_.values ();
	}

	Media::IRadioStation_ptr Plugin::GetRadioStation (QStandardItem *item, const QString&)
	{
		const auto& name = item->data (StreamItemRoles::PristineName).toString ();
		const auto& format = item->data (StreamItemRoles::PlaylistFormat).toString ();
		if (format != "urllist")
		{
			auto nam = Proxy_->GetNetworkAccessManager ();
			const auto& url = item->data (Media::RadioItemRole::RadioID).toUrl ();
			return Media::IRadioStation_ptr (new RadioStation (url, name, nam, format));
		}
		else
		{
			const auto& urlList = item->data (Media::RadioItemRole::RadioID).value<QList<QUrl>> ();
			return Media::IRadioStation_ptr (new StringListRadioStation (urlList, name));
		}
	}

	void Plugin::RefreshItems (const QList<QStandardItem*>& items)
	{
		auto clearRoot = [] (QStandardItem *item)
		{
			while (item->rowCount ())
				item->removeRow (0);
		};

		for (auto item : items)
		{
			if (!Root2Fetcher_.contains (item))
				continue;

			clearRoot (item);
			Root2Fetcher_ [item] (item);
		}
	}

	void Plugin::refreshRadios ()
	{
		RefreshItems (GetRadioListItems ());
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_hotstreams, LeechCraft::HotStreams::Plugin);

