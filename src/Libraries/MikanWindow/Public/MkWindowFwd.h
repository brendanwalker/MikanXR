#pragma once

#include <memory>

class IMkWindowContextManager;
using IMkWindowContextManagerPtr= std::shared_ptr<IMkWindowContextManager>;
using IMkWindowContextManagerConstPtr= std::shared_ptr<const IMkWindowContextManager>;
using IMkWindowContextManagerWeakPtr= std::weak_ptr<IMkWindowContextManager>;

class IMkWindowContext;
using IMkWindowContextPtr= std::shared_ptr<IMkWindowContext>;
using IMkWindowContextConstPtr= std::shared_ptr<const IMkWindowContext>;
using IMkWindowContextWeakPtr= std::weak_ptr<IMkWindowContext>;

class MkWindowEvent;
