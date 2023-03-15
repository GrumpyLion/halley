#pragma once

#include <cstdint>
#include <gsl/gsl>
#include "utils.h"
#include "halley/text/halleystring.h"

struct XXH64_state_s;
typedef struct XXH64_state_s XXH64_state_t;

namespace Halley {
    class Hash {
    public:
        static uint64_t hash(const Bytes& bytes);
        static uint64_t hash(gsl::span<const gsl::byte> bytes);
		
    	template <typename T>
    	static uint64_t hashValue(const T& v)
    	{
    		return hash(gsl::as_bytes(gsl::span<const T>(&v, 1)));
    	}

		static uint32_t compressTo32(uint64_t value);


		class Hasher
		{
		public:
			Hasher();
			~Hasher();

			void feed(const String& string)
			{
				feedBytes(gsl::as_bytes(gsl::span<const char>(string.c_str(), string.length())));
			}

			void feed(std::string_view string)
			{
				feedBytes(gsl::as_bytes(gsl::span<const char>(string.data(), string.size())));
			}

			template<typename T, std::enable_if_t<std::is_trivially_copyable_v<T>, int> = 0>
			void feed(const T& data)
			{
				feedBytes(gsl::as_bytes(gsl::span<const T>(&data, 1)));
			}

			void feedBytes(gsl::span<const gsl::byte> bytes);

			[[nodiscard]] uint64_t digest();
			void reset();

		private:
			bool ready;
			XXH64_state_t* data;
		};
    };
}
