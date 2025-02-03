#pragma once

#include <memory>

class CommonConfig;
using CommonConfigPtr = std::shared_ptr<CommonConfig>;
using CommonConfigConstPtr = std::shared_ptr<const CommonConfig>;
using CommonConfigWeakPtr = std::weak_ptr<CommonConfig>;