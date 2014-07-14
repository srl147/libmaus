/*	
	author : Sachin Garg (Copyright 2006, see http://www.sachingarg.com/compression/entropy_coding/64bit)
		
	Further modifications by German Tischler

	License:

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	3. The name of the author may not be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#if ! defined(ARITHENCODER_HPP)
#define ARITHENCODER_HPP

#include <libmaus/arithmetic/ArithCoder.hpp>
// #include <iostream>

namespace libmaus
{
	namespace arithmetic
	{
		template<typename _bitstream_type>
		struct ArithmeticEncoder : public ArithmeticCoder
		{
			typedef _bitstream_type bitstream_type;
		
			private:
			bool Flushed;
			bitstream_type & Output;
			uint64_t UnderflowCount;

			public:
			ArithmeticEncoder(bitstream_type & BitOStream) : Flushed(false), Output(BitOStream), UnderflowCount(0) {}

			~ArithmeticEncoder() { if(!Flushed) flush(); }

			template<typename model_type>
			void encode(model_type const & model, unsigned int const sym)
			{
				EncodeRange(model.getLow(sym),model.getHigh(sym),model.getTotal());
			}

			template<typename model_type>
			void encodeUpdate(model_type & model, unsigned int const sym)
			{
				encode(model,sym);
				model.update(sym);
			}
			
			void EncodeRange(uint32_t const SymbolLow,uint32_t const SymbolHigh,uint32_t const TotalRange)
			{
				uint64_t const TempRange=(High-Low)+1;
				High = Low + ((TempRange*(uint64_t)SymbolHigh)/TotalRange)-1;
				Low  = Low + ((TempRange*(uint64_t)SymbolLow )/TotalRange);

				while(true)
				{
					if((High & 0x80000000)==(Low & 0x80000000))
					{
						Output.writeBit(High>>31);
						while(UnderflowCount)
						{
							Output.writeBit((High>>31)^1);
							UnderflowCount--;
						}
					}
					else
					{
						if (
							(Low & 0x40000000ul) 
							&& 
							!(High & 0x40000000ul)
						)
						{
							UnderflowCount++;

							Low  &= 0x3FFFFFFFul;
							High |=	0x40000000ul;
						}
						else
							return;
					}

					Low = (Low<<1) & 0xFFFFFFFFul;
					High=((High<<1)|1) & 0xFFFFFFFFul;
				}
			}
			
			void writeEndMarker()
			{
				/* write termination marker */
				for ( unsigned int i = 0; i < 16; ++i )
				{
					// 8 one bits
					for ( unsigned int j = 0; j < 8; ++j )
						Output.writeBit(1);
	
					// write 16-i-1 using 8 bits
					uint8_t const v = 16-i-1;
					for ( unsigned int j = 0; j < 8; ++j )
					{
						uint8_t const mask = (1u << (8-j-1));
						Output.writeBit( (v & mask) != 0 );
					}
				}
				Output.flush();
			}

			void flush(bool useEndMarker = false)
			{
				if(!Flushed)
				{
					Output.writeBit((Low>>30)&1);
					UnderflowCount++;

					while(UnderflowCount)
					{
						Output.writeBit(((Low>>30)^1)&1);
						UnderflowCount--;
					}

					Output.flush();
					Flushed=true;
			
					if ( useEndMarker )
						writeEndMarker();
				}
			}

		};
	}
}
#endif


