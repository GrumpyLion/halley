#pragma once

#include <new>
#include <cstddef>
#include <memory>
#include <functional>
#include "message.h"
#include "halley/utils/utils.h"
#include "halley/text/enum_names.h"

namespace Halley
{
	enum class SystemMessageDestination {
		Local,
		Host,
		AllClients,
		RemoteClients
	};

	template <>
	struct EnumNames<SystemMessageDestination> {
		constexpr std::array<const char*, 4> operator()() const {
			return {{
				"local",
				"host",
				"allClients",
				"remoteClients"
			}};
		}
	};
	
	class SystemMessage : public Message
	{
	public:
		virtual SystemMessageDestination getMessageDestination() const = 0;
	};

	using SystemMessageCallback = std::function<void(std::byte*, Bytes)>;

	struct SystemMessageContext {
		int msgId;
		bool remote;
		std::unique_ptr<SystemMessage> msg;
		SystemMessageCallback callback;

		SystemMessageContext(std::unique_ptr<SystemMessage> msg = {}, SystemMessageCallback callback = {}, bool remote = false)
			: msgId(msg ? msg->getId() : 0)
			, remote(remote)
			, msg(std::move(msg))
			, callback(std::move(callback))
		{}
	};
}
