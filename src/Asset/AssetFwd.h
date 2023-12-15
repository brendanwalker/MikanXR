#pragma once

#include <memory>
#include <vector>

class AssetReference;
using AssetReferencePtr = std::shared_ptr<AssetReference>;
using AssetReferenceConstPtr = std::shared_ptr<const AssetReference>;

class AssetReferenceFactory;
using AssetReferenceFactoryPtr = std::shared_ptr<AssetReferenceFactory>;
using AssetReferenceFactoryConstPtr = std::shared_ptr<const AssetReferenceFactory>;

class MaterialAssetReference;
using MaterialAssetReferencePtr = std::shared_ptr<MaterialAssetReference>;
using MaterialAssetReferenceConstPtr = std::shared_ptr<const MaterialAssetReference>;

class ModelAssetReference;
using ModelAssetReferencePtr = std::shared_ptr<ModelAssetReference>;
using ModelAssetReferenceConstPtr = std::shared_ptr<const ModelAssetReference>;

class TextureAssetReference;
using TextureAssetReferencePtr = std::shared_ptr<TextureAssetReference>;
using TextureAssetReferenceConstPtr = std::shared_ptr<const TextureAssetReference>;