/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Alexander Konovalov
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

#include "ciphertextformat.h"

namespace LeechCraft
{
namespace Plugins
{
namespace SecMan
{
namespace StoragePlugins
{
namespace SecureStorage
{
	CipherTextFormat::CipherTextFormat (void *buffer, int dataLength)
	: Buffer_ (reinterpret_cast<unsigned char*> (buffer))
	, DataLength_ (dataLength)
	{
	}

	unsigned char* CipherTextFormat::Iv () const
	{
		return Buffer_;
	}

	unsigned char* CipherTextFormat::Data () const
	{
		return Buffer_ + IVLength;
	}

	unsigned char* CipherTextFormat::Rnd () const
	{
		return Buffer_ + IVLength + DataLength_;
	}

	unsigned char* CipherTextFormat::Hmac () const
	{
		return Buffer_ + IVLength + DataLength_ + RndLength;
	}

	unsigned char* CipherTextFormat::BufferBegin () const
	{
		return Buffer_;
	}

	unsigned char* CipherTextFormat::BufferEnd () const
	{
		return Buffer_ + IVLength + DataLength_ + RndLength + HMACLength;
	}

	int CipherTextFormat::GetDataLength () const
	{
		return DataLength_;
	}

	namespace CipherTextFormatUtils
	{
		int BufferLengthFor (int dataLength)
		{
			return dataLength + (IVLength + RndLength + HMACLength);
		}

		int DataLengthFor (int bufferLength)
		{
			return bufferLength - (IVLength + RndLength + HMACLength);
		}

		int DecryptBufferLengthFor (int bufferLength)
		{
			return bufferLength - (IVLength + HMACLength);
		}
	}
}
}
}
}
}