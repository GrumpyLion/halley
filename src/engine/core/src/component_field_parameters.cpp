#include <utility>


#include "halley/editor_extensions/component_field_parameters.h"
#include "halley/file_formats/config_file.h"
using namespace Halley;

ComponentDataRetriever::ComponentDataRetriever(ConfigNode& componentData, String fieldName, String labelName)
	: componentData(componentData)
	, labelName(std::move(labelName))
	, name(fieldName)
{
	retriever = [&componentData, fieldName] (RetrievalType type) -> ConfigNode&
	{
		if (type == RetrievalType::ReadParent) {
			return componentData;
		} else if (type == RetrievalType::Write) {
			return componentData[fieldName];
		} else {
			return componentData.at(fieldName);
		}
	};
}

ComponentDataRetriever::ComponentDataRetriever(ConfigNode& componentData, String fieldName, String labelName, Retriever retriever)
	: componentData(componentData)
	, labelName(std::move(labelName))
	, name(std::move(fieldName))
	, retriever(std::move(retriever))
{}

ComponentDataRetriever ComponentDataRetriever::getSubIndex(size_t index) const
{
	auto r = retriever;
	return ComponentDataRetriever(componentData, name + "[" + toString(index) + "]", labelName, [retriever = std::move(r), index] (RetrievalType type) -> ConfigNode&
	{
		if (type == RetrievalType::ReadParent) {
			return retriever(RetrievalType::Read);
		}

		ConfigNode& node = retriever(type);
		if (node.getType() == ConfigNodeType::Sequence) {
			return node[index];
		} else if (index == 0 && node.getType() != ConfigNodeType::Map) {
			return node;
		} else {
			throw Exception("ConfigNode is not a sequence", HalleyExceptions::Entity);
		}
	});
}

ComponentDataRetriever ComponentDataRetriever::getSubKey(const String& key) const
{
	auto r = retriever;
	return ComponentDataRetriever(componentData, name + "[\"" + key + "\"]", labelName, [retriever = std::move(r), key] (RetrievalType type) -> ConfigNode&
	{
		if (type == RetrievalType::ReadParent) {
			return retriever(RetrievalType::Read);
		}
		return retriever(type)[key];
	});
}

const ConfigNode& ComponentDataRetriever::getParentFieldData() const
{
	return retriever(RetrievalType::ReadParent);
}

const ConfigNode& ComponentDataRetriever::getFieldData() const
{
	return retriever(RetrievalType::Read);
}

ConfigNode& ComponentDataRetriever::getWriteableFieldData() const
{
	return retriever(RetrievalType::Write);
}

const String& ComponentDataRetriever::getName() const
{
	return name;
}

const String& ComponentDataRetriever::getLabelName() const
{
	return labelName.isEmpty() ? name : labelName;
}

ComponentFieldParameters::ComponentFieldParameters(String componentName, ComponentDataRetriever data, Vector<String> defaultValue, Vector<String> typeParameters, ConfigNode options)
	: componentName(std::move(componentName))
	, data(std::move(data))
	, defaultValue(std::move(defaultValue))
	, typeParameters(std::move(typeParameters))
	, options(std::move(options))
{}

ComponentFieldParameters ComponentFieldParameters::withSubIndex(size_t index, Vector<String> defaultValue, Vector<String> typeParameters) const
{
	return ComponentFieldParameters(componentName, data.getSubIndex(index), std::move(defaultValue), std::move(typeParameters), ConfigNode(options));
}

ComponentFieldParameters ComponentFieldParameters::withSubKey(const String& key, Vector<String> defaultValue, Vector<String> typeParameters) const
{
	return ComponentFieldParameters(componentName, data.getSubKey(key), std::move(defaultValue), std::move(typeParameters), ConfigNode(options));
}

ComponentFieldParameters ComponentFieldParameters::withSubIndex(size_t index, String defaultValue, Vector<String> typeParameters) const
{
	return withSubIndex(index, defaultValue.isEmpty() ? Vector<String>() : Vector<String>{std::move(defaultValue)}, std::move(typeParameters));
}

ComponentFieldParameters ComponentFieldParameters::withSubKey(const String& key, String defaultValue, Vector<String> typeParameters) const
{
	return withSubKey(key, defaultValue.isEmpty() ? Vector<String>() : Vector<String>{std::move(defaultValue)}, std::move(typeParameters));
}

ComponentFieldParameters ComponentFieldParameters::withOptions(ConfigNode options) const
{
	return ComponentFieldParameters(componentName, data, defaultValue, typeParameters, std::move(options));
}

String ComponentFieldParameters::getStringDefaultParameter(size_t n) const
{
	if (n < defaultValue.size()) {
		return defaultValue[n];
	}
	return "";
}

bool ComponentFieldParameters::getBoolDefaultParameter(size_t n) const
{
	const auto& str = getStringDefaultParameter(n);
	if (str == "true") {
		return true;
	}
	return false;
}

int ComponentFieldParameters::getIntDefaultParameter(size_t n) const
{
	const auto& str = getStringDefaultParameter(n);
	if (str.isInteger()) {
		return str.toInteger();
	}
	return 0;
}

float ComponentFieldParameters::getFloatDefaultParameter(size_t n) const
{
	const auto& str = getStringDefaultParameter(n);
	if (str.isNumber()) {
		return str.toFloat();
	}
	return 0.0f;
}
