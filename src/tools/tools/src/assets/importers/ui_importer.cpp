#include "ui_importer.h"
#include "halley/tools/assets/import_assets_database.h"
#include "halley/file_formats/config_file.h"
#include "halley/file_formats/yaml_convert.h"
#include "halley/ui/ui_definition.h"

using namespace Halley;

void UIImporter::import(const ImportingAsset& asset, IAssetCollector& collector)
{
	ConfigFile config = YAMLConvert::parseConfig(gsl::as_bytes(gsl::span<const Byte>(asset.inputFiles.at(0).data)));
	
	Metadata meta = asset.inputFiles.at(0).metadata;
	meta.set("asset_compression", "lz4");

	auto ui = UIDefinition(std::move(config));

	collector.output(Path(asset.assetId).replaceExtension("").string(), AssetType::UIDefinition, Serializer::toBytes(ui), meta);
}
