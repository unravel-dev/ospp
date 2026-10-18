#pragma once

#include "../../bundle.h"
#include "config.hpp"

#if defined(SDL_PLATFORM_ANDROID)
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>
#endif

namespace os
{
namespace bundle
{
namespace detail
{
namespace sdl
{
#if defined(SDL_PLATFORM_ANDROID)

/// The APK's asset manager. It stays valid while its Java AssetManager lives, so that object is held
/// by a global reference for the life of the process.
inline auto get_asset_manager() -> AAssetManager*
{
	static AAssetManager* manager = []() -> AAssetManager*
	{
		auto* env = static_cast<JNIEnv*>(SDL_GetAndroidJNIEnv());
		auto activity = static_cast<jobject>(SDL_GetAndroidActivity());
		if(env == nullptr || activity == nullptr)
		{
			return nullptr;
		}
		jclass activity_class = env->GetObjectClass(activity);
		jmethodID get_assets = env->GetMethodID(activity_class, "getAssets", "()Landroid/content/res/AssetManager;");
		jobject assets = get_assets == nullptr ? nullptr : env->CallObjectMethod(activity, get_assets);
		AAssetManager* result = nullptr;
		if(assets != nullptr)
		{
			result = AAssetManager_fromJava(env, env->NewGlobalRef(assets));
			env->DeleteLocalRef(assets);
		}
		env->DeleteLocalRef(activity_class);
		env->DeleteLocalRef(activity);
		return result;
	}();
	return manager;
}

inline auto is_supported() -> bool
{
	return get_asset_manager() != nullptr;
}

inline auto open_region(const std::string& name) -> file_region
{
	file_region region;
	AAssetManager* manager = get_asset_manager();
	AAsset* asset = manager == nullptr ? nullptr : AAssetManager_open(manager, name.c_str(), AASSET_MODE_UNKNOWN);
	if(asset == nullptr)
	{
		return region;
	}
	off64_t start = 0;
	off64_t length = 0;
	// Only an asset stored uncompressed has a descriptor: the APK file itself, open at its offset.
	const int descriptor = AAsset_openFileDescriptor64(asset, &start, &length);
	AAsset_close(asset);
	if(descriptor >= 0)
	{
		region.descriptor = descriptor;
		region.offset = static_cast<std::uint64_t>(start);
		region.size = static_cast<std::uint64_t>(length);
	}
	return region;
}

inline auto read_file(const std::string& name, std::string& content) -> bool
{
	AAssetManager* manager = get_asset_manager();
	AAsset* asset = manager == nullptr ? nullptr : AAssetManager_open(manager, name.c_str(), AASSET_MODE_STREAMING);
	if(asset == nullptr)
	{
		return false;
	}
	content.resize(static_cast<std::size_t>(AAsset_getLength64(asset)));
	std::size_t total = 0;
	while(total < content.size())
	{
		const int read = AAsset_read(asset, &content[total], content.size() - total);
		if(read <= 0)
		{
			break;
		}
		total += static_cast<std::size_t>(read);
	}
	AAsset_close(asset);
	return total == content.size();
}

#else

inline auto is_supported() -> bool
{
	return false;
}

inline auto open_region(const std::string&) -> file_region
{
	return {};
}

inline auto read_file(const std::string&, std::string&) -> bool
{
	return false;
}

#endif
} // namespace sdl
} // namespace detail
} // namespace bundle
} // namespace os
