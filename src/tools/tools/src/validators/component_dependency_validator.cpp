#include "halley/tools/validators/component_dependency_validator.h"

#include "halley/editor_extensions/entity_validator.h"
#include "halley/entity/entity.h"
#include "halley/entity/world.h"
#include "halley/utils/algorithm.h"

using namespace Halley;

ComponentDependencyValidator::ComponentDependencyValidator(const ECSData* ecsData)
	: ecsData(ecsData)
{

}

Vector<IEntityValidator::Result> ComponentDependencyValidator::validateEntity(EntityValidator& validator, const EntityData& entityData)
{
	Vector<Result> result;

	const auto& componentSchemas = ecsData->getComponents();
	for (auto& component : entityData.getComponents()) {
		const auto& componentSchema = componentSchemas.at(component.first);
		for (auto& dependsOn : componentSchema.componentDependencies) {
			const bool hasDepedency = std_ex::contains_if(entityData.getComponents(), [&](const std::pair<String, ConfigNode>& comp) { return comp.first == dependsOn; });
			if (!hasDepedency) {
				result.emplace_back(Severity::Error, component.first + " depends on " + dependsOn + " which is missing.");
			}
		}
	}

	return result;
}
