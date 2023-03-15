#include "halley/editor_extensions/entity_validator.h"
#include "halley/editor_extensions/standard_entity_validators.h"
#include "halley/entity/entity.h"
#include "halley/support/logger.h"

using namespace Halley;

bool IEntityValidator::Action::operator==(const Action& other) const
{
	return label == other.label && actionData == other.actionData;
}

bool IEntityValidator::Action::operator!=(const Action& other) const
{
	return label != other.label || actionData != other.actionData;
}

bool IEntityValidator::Result::operator==(const Result& other) const
{
	return errorMessage == other.errorMessage && suggestedActions == other.suggestedActions;
}

bool IEntityValidator::Result::operator!=(const Result& other) const
{
	return errorMessage != other.errorMessage || suggestedActions != other.suggestedActions;
}

void ISceneData::reloadEntity(const String& id, const EntityData* data)
{
	return reloadEntities(gsl::span<const String>(&id, 1), gsl::span<const EntityData*>(&data, 1));
}

EntityValidator::EntityValidator(World& world, const ISceneEditorWindow& sceneEditorWindow)
	: world(world)
	, sceneEditorWindow(sceneEditorWindow)
{
}

Vector<IEntityValidator::Result> EntityValidator::validateEntity(const EntityData& entity, bool recursive)
{
	try {
		Vector<IEntityValidator::Result> result;

		curEntity = &entity;
		validateEntity(entity, recursive, result);
		curEntity = nullptr;
		return result;
	} catch (const std::exception& e) {
		Logger::logError("Exception when attempting to validate entity:");
		Logger::logException(e);
		return {};
	} catch (...) {
		Logger::logError("Unknown exception when attempting to validate entity.");
		return {};
	}
}

void EntityValidator::validateEntity(const EntityData& entity, bool recursive, Vector<IEntityValidator::Result>& result)
{
	if (entity.getFlag(EntityData::Flag::Disabled)) {
		return;
	}

	for (const auto& validator: validators) {
		auto r = validator->validateEntity(*this, entity);
		for (auto& v: r) {
			for (auto& a: v.suggestedActions) {
				a.actionData["entity"] = entity.getInstanceUUID().toString();
			}
		}
		result.insert(result.end(), r.begin(), r.end());
	}

	if (recursive) {
		for (const auto& c: entity.getChildren()) {
			validateEntity(c, recursive, result);
		}
	}
}

void EntityValidator::applyAction(IEntityEditor& entityEditor, EntityData& data, const ConfigNode& actionData)
{
	for (const auto& handler: handlers) {
		if (handler->canHandle(actionData)) {
			handler->applyAction(*this, entityEditor, data, actionData);
			return;
		}
	}
	Logger::logError("No handler found for EntityValidator action.");
}

bool EntityValidator::canApplyAction(const IEntityEditor& entityEditor, const EntityData& data, const ConfigNode& actionData)
{
	if (actionData["entity"].asString("") != data.getInstanceUUID().toString()) {
		return false;
	}
	
	for (const auto& handler: handlers) {
		if (handler->canHandle(actionData)) {
			return handler->canApplyAction(*this, entityEditor, data, actionData);
		}
	}
	return false;
}

void EntityValidator::addValidator(std::unique_ptr<IEntityValidator> validator)
{
	validators.push_back(std::move(validator));
}

void EntityValidator::addActionHandler(std::unique_ptr<IEntityValidatorActionHandler> handler)
{
	handlers.push_back(std::move(handler));
}

void EntityValidator::addDefaults()
{
	addValidator(std::make_unique<TransformEntityValidator>());

	addActionHandler(std::make_unique<AddComponentValidatorActionHandler>());
	addActionHandler(std::make_unique<RemoveComponentValidatorActionHandler>());
	addActionHandler(std::make_unique<ModifyFieldsValidatorActionHandler>());
}

World& EntityValidator::getWorld()
{
	return world;
}

Vector<const EntityData*> EntityValidator::getEntityDataStack(const UUID& uuid) const
{
	Logger::logError("TODO: getEntityDataStack() is currently not implemented correctly");
	return {};

	// TODO disabled for now.. does not work as intended
	//if (curEntity && !curEntity->getPrefab().isEmpty()) {
	//	Vector<const EntityData*> entityDataStack;
	//	curEntity->fillEntityDataStack(entityDataStack, uuid);
	//	return entityDataStack;
	//}
	//return sceneEditorWindow.getEntityDataStack(uuid);
}
