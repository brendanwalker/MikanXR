#pragma once

#include <memory>

class IMkWindowManager;
using IMkWindowManagerPtr = std::shared_ptr<IMkWindowManager>;
using IMkWindowManagerConstPtr = std::shared_ptr<const IMkWindowManager>;
using IMkWindowManagerWeakPtr = std::weak_ptr<IMkWindowManager>;

class IMkWindow;
using IMkWindowPtr = std::shared_ptr<IMkWindow>;
using IMkWindowConstPtr = std::shared_ptr<const IMkWindow>;
using IMkWindowWeakPtr = std::weak_ptr<IMkWindow>;

class MkWindowEvent;
