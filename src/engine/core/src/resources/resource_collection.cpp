#include "halley/resources/resource_collection.h"
#include "halley/resources/resource_locator.h"
#include "halley/resources/resources.h"
#include <halley/resources/resource.h>
#include <utility>

#include "halley/graphics/sprite/sprite.h"
#include "halley/support/logger.h"

using namespace Halley;


ResourceCollectionBase::ResourceCollectionBase(Resources& parent, AssetType type)
	: parent(parent)
	, type(type)
{
}

void ResourceCollectionBase::clear()
{
	resources.clear();
}

void ResourceCollectionBase::unload(std::string_view assetId)
{
	resources.erase(assetId);
}

void ResourceCollectionBase::unloadAll(int minDepth)
{
	for (auto iter = resources.begin(); iter != resources.end(); ) {
		auto next = iter;
		++next;

		auto& res = (*iter).second;
		if (res.depth >= minDepth) {
			resources.erase(iter);
		}

		iter = next;
	}
}

void ResourceCollectionBase::reload(std::string_view assetId)
{
	auto res = resources.find(assetId);
	if (res != resources.end()) {
		auto& resWrap = res->second;
		try {
			const auto [newAsset, loaded] = loadAsset(assetId, ResourceLoadPriority::High, false);
			newAsset->setAssetId(assetId);
			newAsset->onLoaded(parent);
			resWrap.res->reloadResource(std::move(*newAsset));
		} catch (std::exception& e) {
			Logger::logError("Error while reloading " + String(assetId) + ": " + e.what());
		} catch (...) {
			Logger::logError("Unknown error while reloading " + String(assetId));
		}
	}
}

void ResourceCollectionBase::purge(std::string_view assetId)
{
	if (!resourceLoader) {
		parent.locator->purge(assetId, type);
	}
}

std::shared_ptr<Resource> ResourceCollectionBase::getUntyped(std::string_view name, ResourceLoadPriority priority)
{
	return doGet(name, priority, true);
}

Vector<String> ResourceCollectionBase::enumerate() const
{
	if (resourceEnumerator) {
		return resourceEnumerator();
	} else {
		return parent.locator->enumerate(type);
	}
}

AssetType ResourceCollectionBase::getAssetType() const
{
	return type;
}

ResourceMemoryUsage ResourceCollectionBase::getMemoryUsage() const
{
	ResourceMemoryUsage usage;
	std::shared_lock lock(mutex);

	for (auto& r: resources) {
		usage += r.second.res->getMemoryUsage();
	}

	return usage;
}

void ResourceCollectionBase::age(float time)
{
	std::shared_lock lock(mutex);

	for (auto& r: resources) {
		const auto& resourcePtr = r.second.res;
		if (resourcePtr.use_count() <= 2) {
			resourcePtr->increaseAge(time);
		} else {
			resourcePtr->resetAge();
		}
	}
}

ResourceMemoryUsage ResourceCollectionBase::clearOldResources(float maxAge)
{
	Vector<decltype(resources)::iterator> toDelete;
	ResourceMemoryUsage usage;

	{
		std::shared_lock lock(mutex);

		for (auto iter = resources.begin(); iter != resources.end(); ) {
			auto next = iter;
			++next;

			auto& resourcePtr = iter->second.res;
			if (resourcePtr.use_count() <= 2 && resourcePtr->getAge() > maxAge) {
				usage += resourcePtr->getMemoryUsage();
				toDelete.push_back(iter);
			}

			iter = next;
		}

		for (auto& a: toDelete) {
			resources.erase(a);
		}
	}

	// Delete out of the lock to avoid stalling resources for too long
	toDelete.clear();

	return usage;
}

ResourceMemoryUsage ResourceCollectionBase::getMemoryUsageAndAge(float time)
{
	ResourceMemoryUsage usage;
	std::shared_lock lock(mutex);

	for (auto& r: resources) {
		auto& resourcePtr = r.second.res;
		if (resourcePtr.use_count() <= 2) {
			resourcePtr->increaseAge(time);
		} else {
			resourcePtr->resetAge();
		}
		usage += resourcePtr->getMemoryUsage();
	}

	return usage;
}

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef max
#undef min

namespace {
	HMODULE getCurrentModuleHandle()
	{
		HMODULE hMod = nullptr;
		GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCWSTR>(&getCurrentModuleHandle), &hMod);
		return hMod;
	}

	bool isRunningFromDLL()
	{
		char name[1024];
		GetModuleFileNameA(getCurrentModuleHandle(), name, sizeof(name));
		auto str = std::string_view(name);
		return str.length() > 4 && str.substr(str.length() - 4, 4) == ".dll";
	}
}
#endif

std::pair<std::shared_ptr<Resource>, bool> ResourceCollectionBase::loadAsset(std::string_view assetId, ResourceLoadPriority priority, bool allowFallback)
{
#ifdef _WIN32
	//assert(!isRunningFromDLL());
#endif

	std::shared_ptr<Resource> newRes;

	if (resourceLoader) {
		// Overriding loader
		newRes = resourceLoader(assetId, priority);
	} else {
		// Normal loading
		auto resLoader = ResourceLoader(*(parent.locator), assetId, type, priority, parent.api, parent);		
		newRes = loadResource(resLoader);
		if (newRes) {
			newRes->setMeta(resLoader.getMeta());
		} else if (resLoader.loaded) {
			throw Exception("Unable to construct resource from data: " + toString(type) + ":" + assetId, HalleyExceptions::Resources);
		}
	}

	if (!newRes) {
		if (allowFallback && !fallback.isEmpty()) {
			Logger::logError("Resource not found: \"" + toString(type) + ":" + assetId + "\"");
			return { loadAsset(fallback, priority, false).first, false };
		}
		
		throw Exception("Resource not found: \"" + toString(type) + ":" + assetId + "\"", HalleyExceptions::Resources);
	}
	
	return std::make_pair(newRes, true);
}

std::shared_ptr<Resource> ResourceCollectionBase::doGet(std::string_view assetId, ResourceLoadPriority priority, bool allowFallback)
{
	// Look in cache and return if it's there
	{
		std::shared_lock lock(mutex);
		const auto res = resources.find(assetId);
		if (res != resources.end()) {
			return res->second.res;
		}
	}

	// Lock mutex, and make sure it's still not there (someone else might have gone through this by now)
	std::unique_lock lockWrite(mutex);
	const auto res = resources.find(assetId);
	if (res != resources.end()) {
		return res->second.res;
	}
	
	// Load resource from disk
	const auto [newRes, loaded] = loadAsset(assetId, priority, allowFallback);

	// Store in cache
	if (loaded) {
		newRes->setAssetId(assetId);
		resources.emplace(assetId, Wrapper(newRes, 0));
		lockWrite.unlock();
		newRes->onLoaded(parent);
	}

	return newRes;
}

bool ResourceCollectionBase::exists(std::string_view assetId) const
{
	// Look in cache
	const auto res = resources.find(assetId);
	if (res != resources.end()) {
		return true;
	}

	return parent.locator->exists(assetId, type);
}

void ResourceCollectionBase::setFallback(std::string_view assetId)
{
	fallback = assetId;
}

void ResourceCollectionBase::setResource(int curDepth, std::string_view name, std::shared_ptr<Resource> resource) {
	resources.emplace(name, Wrapper(std::move(resource), curDepth));
}

void ResourceCollectionBase::setResourceLoader(ResourceLoaderFunc loader)
{
	resourceLoader = std::move(loader);
}

void ResourceCollectionBase::setResourceEnumerator(ResourceEnumeratorFunc enumerator)
{
	resourceEnumerator = std::move(enumerator);
}
