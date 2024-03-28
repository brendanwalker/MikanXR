#pragma once

#include <memory>
#include <future>
#include <functional>
#include <string>

using MikanResponsePtr = std::shared_ptr<struct MikanResponse>;
using MikanResponsePromise = std::promise<MikanResponsePtr>;
using MikanResponseFuture = std::future<MikanResponsePtr>;
using MikanResponseFactory = std::function<MikanResponsePtr(const std::string& responseType)>;

using MikanEventPtr = std::shared_ptr<struct MikanEvent>;
using MikanEventFactory = std::function<MikanEventPtr(const std::string& eventType)>;
