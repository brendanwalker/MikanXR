#pragma once

namespace vr
{
typedef uint32_t TrackedDeviceIndex_t;
struct TrackedDevicePose_t;
class IVRSystem;
}; // namespace vr

class MikanSteamVRDeviceSocket;
using MikanSteamVRDeviceSocketPtr= std::shared_ptr<MikanSteamVRDeviceSocket>;

class MikanSteamVRDeviceMesh;
using MikanSteamVRDeviceMeshPtr= std::shared_ptr<MikanSteamVRDeviceMesh>;

class MikanSteamVRDevice;
using MikanSteamVRDevicePtr= std::shared_ptr<MikanSteamVRDevice>;