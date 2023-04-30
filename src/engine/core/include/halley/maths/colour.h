/*****************************************************************\
           __
          / /
		 / /                     __  __
		/ /______    _______    / / / / ________   __       __
	   / ______  \  /_____  \  / / / / / _____  | / /      / /
	  / /      | / _______| / / / / / / /____/ / / /      / /
	 / /      / / / _____  / / / / / / _______/ / /      / /
	/ /      / / / /____/ / / / / / / |______  / |______/ /
   /_/      /_/ |________/ / / / /  \_______/  \_______  /
                          /_/ /_/                     / /
			                                         / /
		       High Level Game Framework            /_/

  ---------------------------------------------------------------

  Copyright (c) 2007-2011 - Rodrigo Braz Monteiro.
  This file is subject to the terms of halley_license.txt.

\*****************************************************************/

#pragma once

#include <iomanip>
#include <sstream>
#include <cmath>
#include "halley/text/halleystring.h"
#include <gsl/gsl_assert>
#include <cstdint>

#include "halley/data_structures/config_node.h"
#include "halley/utils/utils.h"
#include "halley/maths/vector3.h"
#include "halley/maths/vector4.h"
#include "halley/text/encode.h"
#include "halley/bytes/config_node_serializer_base.h"

namespace Halley {
	// This whole class is TERRIBLE
	// There should be a type distinction between linear and gamma space, with conversions,
	// and the default Colour type should be Colour4c
	
	template <typename T>
	constexpr T colMinValue()
	{
		return 0;
	}

	template <typename T>
	constexpr T colMaxValue()
	{
		return 255;
	}

	template <>
	constexpr float colMaxValue<float>()
	{
		return 1.0f;
	}

	template <typename T, typename U>
	constexpr U convertColour(T x)
	{
		if constexpr (std::is_integral_v<U>) {
			return U(std::round(static_cast<float>(x) * colMaxValue<U>()) / colMaxValue<T>());
		} else {
			return U(static_cast<float>(x) * colMaxValue<U>() / colMaxValue<T>());
		}
	}

	template <>
	constexpr uint8_t convertColour(int x)
	{
		return static_cast<uint8_t>(x);
	}

	template <typename T>
	class alignas(4 * sizeof(T)) Colour4 {
	public:
		T r = colMinValue<T>();
		T g = colMinValue<T>();
		T b = colMinValue<T>();
		T a = colMaxValue<T>();

		constexpr Colour4() = default;

		explicit constexpr Colour4(T luma)
			: r(luma)
			, g(luma)
			, b(luma)
		{}
		
		constexpr Colour4(T r, T g, T b, T a=colMaxValue<T>())
			: r(r)
			, g(g)
			, b(b)
			, a(a)
		{}

		constexpr Colour4(Vector4D<T> v)
			: r(v.x)
			, g(v.y)
			, b(v.z)
			, a(v.w)
		{}

		Colour4(const String& str)
		{
			*this = fromString(str);
		}

		Colour4(const ConfigNode& node)
		{
			if (node.getType() == ConfigNodeType::Map) {
				*this = fromString(node["colour"].asString());
			} else {
				*this = fromString(node.asString("#000000"));
			}
		}

		template <typename U>
		Colour4(const Colour4<U> c)
		{
			r = convertColour<U, T>(c.r);
			g = convertColour<U, T>(c.g);
			b = convertColour<U, T>(c.b);
			a = convertColour<U, T>(c.a);
		}

		[[nodiscard]] String toString() const
		{
			std::stringstream ss;
			ss << "#" << std::hex;
			writeByte(ss, r);
			writeByte(ss, g);
			writeByte(ss, b);
			if (byteRep(a) != 255) writeByte(ss, a);
			ss.flush();
			return ss.str();
		}

		[[nodiscard]] ConfigNode toConfigNode() const
		{
			return ConfigNode(toString());
		}

		[[nodiscard]] static Colour4 fromString(std::string_view str)
		{
			Colour4 col;
			size_t len = str.length();
			if (len >= 1 && str[0] == '#') {
				if (len >= 3) {
					col.r = parseHex(str.substr(1, 2));
				}
				if (len >= 5) {
					col.g = parseHex(str.substr(3, 2));
				}
				if (len >= 7) {
					col.b = parseHex(str.substr(5, 2));
				}
				if (len >= 9) {
					col.a = parseHex(str.substr(7, 2));
				}
			}
			return col;
		}

		[[nodiscard]] static Colour4 fromHSV(Vector3f hsv, float a)
		{
			return fromHSV(hsv.x, hsv.y, hsv.z, a);
		}

		[[nodiscard]] static Colour4 fromHSV(float h, float s, float v, float a)
		{
			float r = 0;
			float g = 0;
			float b = 0;
			if (s == 0) {
				r = clamp(v, 0.0f, 1.0f);
				g = b = r;
			} else {
				h = float(std::fmod(h, 1.0f));
				int hi = int(h * 6.0f);
				float f = h*6 - hi;
				float p = v*(1-s);
				float q = v*(1-f*s);
				float t = v*(1-(1-f)*s);

				switch (hi) {
				case 0:	r = v; g = t; b = p; break;
				case 1:	r = q; g = v; b = p; break;
				case 2:	r = p; g = v; b = t; break;
				case 3:	r = p; g = q; b = v; break;
				case 4:	r = t; g = p; b = v; break;
				case 5:	r = v; g = p; b = q; break;
				}
			}

			return Colour4<T>(Colour4<float>(r, g, b, a));
		}

		[[nodiscard]] Vector3f toHSV(float defaultHue = 0, float defaultSaturation = 0) const
		{
			const float ma = std::max(r, std::max(g, b));
			const float mi = std::min(r, std::min(g, b));
			const float c = ma - mi;
			float hp = defaultHue * 6;
			if (c > 0.000001f) {
				if (std::abs(ma - r) < 0.00001f) {
					hp = floatModulo((g - b) / c, 6.0f);
				} else if (std::abs(ma - g) < 0.00001f) {
					hp = (b - r) / c + 2;
				} else {
					hp = (r - g) / c + 4;
				}
			}
			const float h = hp / 6.0f;
			const float v = ma;
			const float s = v > 0.00001f ? c / v : defaultSaturation;
			return Vector3f(h, s, v);
		}

		[[nodiscard]] constexpr Colour4 multiplyLuma(float t) const
		{
			return Colour4(r*t, g*t, b*t, a);
		}

		[[nodiscard]] constexpr Colour4 inverseMultiplyLuma(float t) const
		{
			return Colour4(1.0f - ((1.0f - r) * t), 1.0f - ((1.0f - g) * t), 1.0f - ((1.0f - b) * t), a);
		}

		[[nodiscard]] constexpr Colour4 multiplyAlpha(float t) const
		{
			return Colour4(r, g, b, a * t);
		}

		[[nodiscard]] constexpr Colour4 withAlpha(float newA) const
		{
			return Colour4(r, g, b, newA);
		}

		[[nodiscard]] constexpr Colour4 toMaxBrightness() const
		{
			float m = std::max(r, std::max(g, b));
			if (m > 0.001f) {
				return Colour4(r / m, g / m, b / m, a);
			} else {
				return Colour4(1, 1, 1, a);
			}
		}

		[[nodiscard]] constexpr float getLuma() const
		{
			return 0.212f * r + 0.701f * g + 0.087f * b;
		}

		[[nodiscard]] constexpr Colour4 operator+(const Colour4& c) const
		{
			return Colour4(r+c.r, g+c.g, b+c.b, a+c.a);
		}

		[[nodiscard]] constexpr Colour4 operator*(const Colour4& c) const
		{
			return Colour4(r * c.r, g * c.g, b * c.b, a * c.a);
		}

		[[nodiscard]] constexpr Colour4 operator*(float t) const
		{
			return Colour4(r*t, g*t, b*t, a*t);
		}

		[[nodiscard]] constexpr bool operator==(const Colour4& c) const
		{
			return r == c.r && g == c.g && b == c.b && a == c.a;
		}

		[[nodiscard]] constexpr bool operator!=(const Colour4& c) const
		{
			return r != c.r || g != c.g || b != c.b || a != c.a;
		}

		[[nodiscard]] constexpr Vector4D<T> toVector4() const
		{
			return Vector4D<T>(r, g, b, a);
		}

	private:
		uint8_t byteRep(T v) const
		{
			return convertColour<T, uint8_t>(v);
		}

		static T parseHex(std::string_view str)
		{
			auto decode = [] (char character) -> uint8_t
			{
				if (character >= '0' && character <= '9') {
					return character - '0';
				} else if (character >= 'A' && character <= 'F') {
					return character - 'A' + 10;
				} else if (character >= 'a' && character <= 'f') {
					return character - 'a' + 10;
				} else {
					return 0;
				}
			};

			const uint8_t out = static_cast<uint8_t>(decode(str[0]) << 4) | decode(str[1]);
			return convertColour<uint8_t, T>(out);
		}

		template <typename U>
		void writeByte(U& os, T component) const
		{
			unsigned int value = byteRep(component);
			if (value < 16) os << '0';
			os << value;
		}
	};

	typedef Colour4<uint8_t> Colour4c;
	typedef Colour4<float> Colour4f;
	typedef Colour4f Colour;
}
