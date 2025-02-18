#include "script_branching.h"
using namespace Halley;

gsl::span<const IScriptNodeType::PinType> ScriptBranch::getPinConfiguration(const ScriptGraphNode& node) const
{
	using ET = ScriptNodeElementType;
	using PD = GraphNodePinDirection;
	const static auto data = std::array<PinType, 4>{ PinType{ ET::FlowPin, PD::Input }, PinType{ ET::ReadDataPin, PD::Input }, PinType{ ET::FlowPin, PD::Output }, PinType{ ET::FlowPin, PD::Output } };
	return data;
}

std::pair<String, Vector<ColourOverride>> ScriptBranch::getNodeDescription(const ScriptGraphNode& node, const World* world, const ScriptGraph& graph) const
{
	auto str = ColourStringBuilder(true);
	str.append("If ");
	str.append(getConnectedNodeName(world, node, graph, 1), parameterColour);
	return str.moveResults();
}

IScriptNodeType::Result ScriptBranch::doUpdate(ScriptEnvironment& environment, Time time, const ScriptGraphNode& node) const
{
	const bool value = readDataPin(environment, node, 1).asBool(false);
	return Result(ScriptNodeExecutionState::Done, 0, value ? 1 : 2);
}

String ScriptBranch::getPinDescription(const ScriptGraphNode& node, PinType elementType, uint8_t elementIdx) const
{
	if (elementIdx >= 1) {
		if (elementIdx == 1) {
			return "Condition";
		} else {
			return "Flow Output if " + String(elementIdx == 2 ? "true" : "false");
		}
	} else {
		return IScriptNodeType::getPinDescription(node, elementType, elementIdx);
	}	
}



gsl::span<const IScriptNodeType::PinType> ScriptFork::getPinConfiguration(const ScriptGraphNode& node) const
{
	using ET = ScriptNodeElementType;
	using PD = GraphNodePinDirection;
	const static auto data = std::array<PinType, 4>{ PinType{ ET::FlowPin, PD::Input }, PinType{ ET::FlowPin, PD::Output }, PinType{ ET::FlowPin, PD::Output }, PinType{ ET::FlowPin, PD::Output } };
	return data;
}

std::pair<String, Vector<ColourOverride>> ScriptFork::getNodeDescription(const ScriptGraphNode& node, const World* world, const ScriptGraph& graph) const
{
	auto str = ColourStringBuilder(true);
	str.append("Fork execution.");
	return str.moveResults();
}

IScriptNodeType::Result ScriptFork::doUpdate(ScriptEnvironment& environment, Time time, const ScriptGraphNode& node) const
{
	return Result(ScriptNodeExecutionState::Done, 0, 1 | 2 | 4);
}



gsl::span<const IScriptNodeType::PinType> ScriptMergeAny::getPinConfiguration(const ScriptGraphNode& node) const
{
	using ET = ScriptNodeElementType;
	using PD = GraphNodePinDirection;
	const static auto data = std::array<PinType, 3>{ PinType{ ET::FlowPin, PD::Input }, PinType{ ET::FlowPin, PD::Input }, PinType{ ET::FlowPin, PD::Output } };
	return data;
}

std::pair<String, Vector<ColourOverride>> ScriptMergeAny::getNodeDescription(const ScriptGraphNode& node, const World* world, const ScriptGraph& graph) const
{
	auto str = ColourStringBuilder(true);
	str.append("Allows ");
	str.append("any", Colour4f(1, 0, 0));
	str.append(" flow threads to continue forward.");
	return str.moveResults();
}

IScriptNodeType::Result ScriptMergeAny::doUpdate(ScriptEnvironment& environment, Time time, const ScriptGraphNode& node) const
{
	return Result(ScriptNodeExecutionState::Done);
}



gsl::span<const IScriptNodeType::PinType> ScriptMergeAll::getPinConfiguration(const ScriptGraphNode& node) const
{
	using ET = ScriptNodeElementType;
	using PD = GraphNodePinDirection;
	const static auto data = std::array<PinType, 2>{ PinType{ ET::FlowPin, PD::Input }, PinType{ ET::FlowPin, PD::Output } };
	return data;
}

std::pair<String, Vector<ColourOverride>> ScriptMergeAll::getNodeDescription(const ScriptGraphNode& node, const World* world, const ScriptGraph& graph) const
{
	auto str = ColourStringBuilder(true);
	str.append("Proceeds with execution only when ");
	str.append("all", Colour4f(1, 0, 0));
	str.append(" connected flows arrive.");
	return str.moveResults();
}

IScriptNodeType::Result ScriptMergeAll::doUpdate(ScriptEnvironment& environment, Time time, const ScriptGraphNode& node) const
{
	auto& counter = environment.getNodeCounter(node.getId());

	size_t expected = 0;
	const auto& pinConfigs = getPinConfiguration(node);
	const auto& pins = node.getPins();
	for (size_t i = 0; i < pins.size(); ++i) {
		if (pinConfigs[i].type == GraphElementType(ScriptNodeElementType::FlowPin) && pinConfigs[i].direction == GraphNodePinDirection::Input) {
			for (const auto& conn: pins[i].connections) {
				if (conn.dstNode) {
					++expected;
				}
			}			
		}
	}

	if (++counter == expected) {
		counter = 0;
		return Result(ScriptNodeExecutionState::MergeAndContinue);
	} else {
		return Result(ScriptNodeExecutionState::MergeAndWait);
	}
}
