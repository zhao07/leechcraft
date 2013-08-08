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

#ifndef INTERFACES_ISYNCABLE_H
#define INTERFACES_ISYNCABLE_H
#include <QByteArray>
#include <QSet>
#include <QList>
#include <QMetaType>
#include <QtPlugin>

namespace LeechCraft
{
	namespace Sync
	{
		struct Payload
		{
			QByteArray Data_;
		};

		typedef QList<Payload> Payloads_t;

		struct Delta
		{
			quint32 ID_;
			Payload Payload_;
		};

		typedef QList<Delta> Deltas_t;

		typedef QByteArray ChainID_t;

		typedef QSet<ChainID_t> ChainIDs_t;
	}
}

Q_DECLARE_METATYPE (LeechCraft::Sync::ChainID_t);

/** @brief Interface for plugins that have content/data/settings that
 * can be synchronized via other plugins — syncers.
 *
 * To notify about new deltas, the following signal is expected:
 * newDeltasAvailable(const ChainID_t& chain)
 */
class Q_DECL_EXPORT ISyncable
{
public:
	virtual ~ISyncable () {}

	virtual LeechCraft::Sync::ChainIDs_t AvailableChains () const = 0;

	virtual LeechCraft::Sync::Payloads_t GetAllDeltas (const LeechCraft::Sync::ChainID_t& chain) const = 0;

	virtual LeechCraft::Sync::Payloads_t GetNewDeltas (const LeechCraft::Sync::ChainID_t& chain) const = 0;

	virtual void PurgeNewDeltas (const LeechCraft::Sync::ChainID_t& chain, quint32 numToPurge) = 0;

	virtual void ApplyDeltas (const LeechCraft::Sync::Payloads_t& deltas,
			const LeechCraft::Sync::ChainID_t& chain) = 0;

	virtual void newDeltasAvailable (const LeechCraft::Sync::ChainID_t& chain) = 0;
};

Q_DECLARE_INTERFACE (ISyncable, "org.Deviant.LeechCraft.Sync.ISyncable/1.0");

#endif
