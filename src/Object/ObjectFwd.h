#pragma once

#include <memory>

class MikanObject;
using MikanObjectPtr = std::shared_ptr<MikanObject>;
using MikanObjectConstPtr = std::shared_ptr<const MikanObject>;
using MikanObjectWeakPtr = std::weak_ptr<MikanObject>;

class MikanObjectSystem;
using MikanObjectSystemPtr = std::shared_ptr<MikanObjectSystem>;
using MikanObjectSystemConstPtr = std::shared_ptr<const MikanObjectSystem>;
using MikanObjectSystemWeakPtr = std::weak_ptr<MikanObjectSystem>;