#pragma once

#include "halley/data_structures/vector.h"
#include <memory>
#include "halley/text/halleystring.h"
#include <set>
#include <gsl/span>

#include "halley/data_structures/config_node.h"
#include "halley/time/halleytime.h"

namespace Halley
{
	class NetworkService;
	class IConnection;
	class MessageQueue;

	namespace DevCon {
		constexpr static int devConPort = 12500;
		class LogMsg;
		class ReloadAssetsMsg;
		class NotifyInterestMsg;
	}

	class DevConServer;

	class DevConServerConnection
	{
	public:
		DevConServerConnection(DevConServer& parent, size_t id, std::shared_ptr<IConnection> connection);
		
		void update(Time t);
		bool isAlive() const;
		size_t getId() const;
		
		void reloadAssets(Vector<String> assetIds, Vector<String> packIds);

		void registerInterest(const String& id, const ConfigNode& params, uint32_t handle);
		void updateInterest(uint32_t handle, const ConfigNode& params);
		void unregisterInterest(uint32_t handle);

	private:
		DevConServer& parent;
		size_t id;
		std::shared_ptr<IConnection> connection;
		std::shared_ptr<MessageQueue> queue;

		void onReceiveLogMsg(DevCon::LogMsg& msg);
		void onReceiveNotifyInterestMsg(DevCon::NotifyInterestMsg& msg);
	};

	class DevConServer
	{
		friend class DevConServerConnection;

	public:
		using InterestCallback = std::function<void(size_t, ConfigNode)>;
		using InterestHandle = uint32_t;

		DevConServer(std::unique_ptr<NetworkService> service, int port = DevCon::devConPort);
		
		void update(Time t);

		void reloadAssets(Vector<String> assetIds, Vector<String> packIds);

		InterestHandle registerInterest(String id, ConfigNode params, InterestCallback callback);
		void updateInterest(InterestHandle handle, ConfigNode params);
		void unregisterInterest(InterestHandle handle);
		const ConfigNode& getInterestParams(InterestHandle handle) const;

	protected:
		void onReceiveNotifyInterestMsg(const DevConServerConnection& connection, DevCon::NotifyInterestMsg& msg);

	private:
		struct Interest {
			String id;
			ConfigNode config;
			InterestCallback callback;
			Vector<size_t> hadResult;
		};

		std::unique_ptr<NetworkService> service;
		Vector<std::shared_ptr<DevConServerConnection>> connections;
		size_t connId = 0;

		HashMap<InterestHandle, Interest> interest;
		InterestHandle interestId = 0;

		void initConnection(DevConServerConnection& conn);
		void terminateConnection(DevConServerConnection& conn);
	};
}
