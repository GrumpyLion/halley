#pragma once
#include "scripting/script_environment.h"

namespace Halley {
	class ScriptStart final : public ScriptNodeTypeBase<void> {
	public:
		String getId() const override { return "start"; }
		String getName() const override { return "Start"; }
		String getIconName(const ScriptGraphNode& node) const override { return "script_icons/start.png"; }
		ScriptNodeClassification getClassification() const override { return ScriptNodeClassification::Terminator; }
		bool canAdd() const override { return false; }
		bool canDelete() const override { return false; }

		Vector<SettingType> getSettingTypes() const override;
		std::pair<String, Vector<ColourOverride>> getNodeDescription(const ScriptGraphNode& node, const World* world, const ScriptGraph& graph) const override;
		gsl::span<const PinType> getPinConfiguration(const ScriptGraphNode& node) const override;
		String getPinDescription(const ScriptGraphNode& node, PinType elementType, GraphPinId elementIdx) const override;
		String getShortDescription(const World* world, const ScriptGraphNode& node, const ScriptGraph& graph, GraphPinId elementIdx) const override;
		
		Result doUpdate(ScriptEnvironment& environment, Time time, const ScriptGraphNode& node) const override;
		ConfigNode doGetData(ScriptEnvironment& environment, const ScriptGraphNode& node, size_t pinN) const override;
		EntityId doGetEntityId(ScriptEnvironment& environment, const ScriptGraphNode& node, GraphPinId pinN) const override;

	private:
		std::optional<std::pair<GraphNodeId, GraphPinId>> getOtherPin(ScriptEnvironment& environment, const ScriptGraphNode& node, size_t pinN) const;
	};

	class ScriptDestructor final : public ScriptNodeTypeBase<void> {
	public:
		String getId() const override { return "destructor"; }
		String getName() const override { return "Destructor"; }
		String getIconName(const ScriptGraphNode& node) const override { return "script_icons/destructor.png"; }
		ScriptNodeClassification getClassification() const override { return ScriptNodeClassification::Terminator; }

		std::pair<String, Vector<ColourOverride>> getNodeDescription(const ScriptGraphNode& node, const World* world, const ScriptGraph& graph) const override;
		gsl::span<const PinType> getPinConfiguration(const ScriptGraphNode& node) const override;
		Result doUpdate(ScriptEnvironment& environment, Time time, const ScriptGraphNode& node) const override;
	};

	class ScriptRestart final : public ScriptNodeTypeBase<void> {
	public:
		String getId() const override { return "restart"; }
		String getName() const override { return "Restart"; }
		String getIconName(const ScriptGraphNode& node) const override { return "script_icons/restart.png"; }
		gsl::span<const PinType> getPinConfiguration(const ScriptGraphNode& node) const override;
		std::pair<String, Vector<ColourOverride>> getNodeDescription(const ScriptGraphNode& node, const World* world, const ScriptGraph& graph) const override;
		ScriptNodeClassification getClassification() const override { return ScriptNodeClassification::Terminator; }
		Result doUpdate(ScriptEnvironment& environment, Time time, const ScriptGraphNode& node) const override;
	};
	
	class ScriptStop final : public ScriptNodeTypeBase<void> {
	public:
		String getId() const override { return "stop"; }
		String getName() const override { return "Stop"; }
		String getIconName(const ScriptGraphNode& node) const override { return "script_icons/stop.png"; }
		gsl::span<const PinType> getPinConfiguration(const ScriptGraphNode& node) const override;
		std::pair<String, Vector<ColourOverride>> getNodeDescription(const ScriptGraphNode& node, const World* world, const ScriptGraph& graph) const override;
		ScriptNodeClassification getClassification() const override { return ScriptNodeClassification::Terminator; }
		Result doUpdate(ScriptEnvironment& environment, Time time, const ScriptGraphNode& node) const override;
	};
	
	class ScriptSpinwait final : public ScriptNodeTypeBase<void> {
	public:
		String getId() const override { return "spinwait"; }
		String getName() const override { return "Spinwait"; }
		String getIconName(const ScriptGraphNode& node) const override { return "script_icons/spinwait.png"; }
		gsl::span<const PinType> getPinConfiguration(const ScriptGraphNode& node) const override;
		std::pair<String, Vector<ColourOverride>> getNodeDescription(const ScriptGraphNode& node, const World* world, const ScriptGraph& graph) const override;
		ScriptNodeClassification getClassification() const override { return ScriptNodeClassification::Terminator; }
		Result doUpdate(ScriptEnvironment& environment, Time time, const ScriptGraphNode& node) const override;
	};

	class ScriptStartScript final : public ScriptNodeTypeBase<void> {
	public:
		String getId() const override { return "startScript"; }
		String getName() const override { return "Start Script"; }
		String getIconName(const ScriptGraphNode& node) const override { return "script_icons/start.png"; }
		ScriptNodeClassification getClassification() const override { return ScriptNodeClassification::Action; }

		Vector<SettingType> getSettingTypes() const override;
		void updateSettings(ScriptGraphNode& node, const ScriptGraph& graph, Resources& resources) const override;

		gsl::span<const PinType> getPinConfiguration(const ScriptGraphNode& node) const override;
		String getPinDescription(const ScriptGraphNode& node, PinType elementType, GraphPinId elementIdx) const override;

		std::pair<String, Vector<ColourOverride>> getNodeDescription(const ScriptGraphNode& node, const World* world, const ScriptGraph& graph) const override;
		Result doUpdate(ScriptEnvironment& environment, Time time, const ScriptGraphNode& node) const override;
	};

	class ScriptStartScriptName final : public ScriptNodeTypeBase<void> {
	public:
		String getId() const override { return "startScriptName"; }
		String getName() const override { return "Start Script Name"; }
		String getIconName(const ScriptGraphNode& node) const override { return "script_icons/start.png"; }
		ScriptNodeClassification getClassification() const override { return ScriptNodeClassification::Action; }

		Vector<SettingType> getSettingTypes() const override;
		std::pair<String, Vector<ColourOverride>> getNodeDescription(const ScriptGraphNode& node, const World* world, const ScriptGraph& graph) const override;
		gsl::span<const PinType> getPinConfiguration(const ScriptGraphNode& node) const override;
		Result doUpdate(ScriptEnvironment& environment, Time time, const ScriptGraphNode& node) const override;
	};

	class ScriptStopScript final : public ScriptNodeTypeBase<void> {
	public:
		String getId() const override { return "stopScript"; }
		String getName() const override { return "Stop Script"; }
		String getIconName(const ScriptGraphNode& node) const override { return "script_icons/stop.png"; }
		ScriptNodeClassification getClassification() const override { return ScriptNodeClassification::Action; }

		Vector<SettingType> getSettingTypes() const override;
		std::pair<String, Vector<ColourOverride>> getNodeDescription(const ScriptGraphNode& node, const World* world, const ScriptGraph& graph) const override;
		gsl::span<const PinType> getPinConfiguration(const ScriptGraphNode& node) const override;
		Result doUpdate(ScriptEnvironment& environment, Time time, const ScriptGraphNode& node) const override;
	};

	class ScriptStopTag final : public ScriptNodeTypeBase<void> {
	public:
		String getId() const override { return "stopTag"; }
		String getName() const override { return "Stop Tag"; }
		String getIconName(const ScriptGraphNode& node) const override { return "script_icons/stop_tag.png"; }
		ScriptNodeClassification getClassification() const override { return ScriptNodeClassification::Action; }

		Vector<SettingType> getSettingTypes() const override;
		std::pair<String, Vector<ColourOverride>> getNodeDescription(const ScriptGraphNode& node, const World* world, const ScriptGraph& graph) const override;
		gsl::span<const PinType> getPinConfiguration(const ScriptGraphNode& node) const override;
		Result doUpdate(ScriptEnvironment& environment, Time time, const ScriptGraphNode& node) const override;
	};
}
