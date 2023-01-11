#pragma once

//-- includes -----
#include "MikanClientTypes.h"
#include "DeviceInterface.h"

#include <string>

#include "glm/ext/quaternion_double.hpp"
#include "glm/ext/vector_double3.hpp"

#ifdef _MSC_VER
    #pragma warning (push)
    #pragma warning (disable: 4996) // This function or variable may be unsafe
    #pragma warning (disable: 4244) // 'return': conversion from 'const int64_t' to 'float', possible loss of data
    #pragma warning (disable: 4715) // configuru::Config::operator[]': not all control paths return a value
#endif
#include <configuru.hpp>
#ifdef _MSC_VER
    #pragma warning (pop)
#endif

//-- definitions -----
/*
    Note that CommonConfig is an abstract class because it has 2 pure virtual functions.
    Child classes must add public member variables that store the config data,
    as well as implement writeToJSON and readFromJSON that use pt[key]= value and
    pt.get_or<type>(), respectively, to convert between member variables and the
    property tree. 
*/
class CommonConfig 
{
public:
    CommonConfig(const std::string &fnamebase = std::string("CommonConfig"));

	const std::string getConfigPath() const;
    void save();
	void save(const std::string &path);
    bool load();
	bool load(const std::string &path);
    
    std::string ConfigFileBase;

    virtual const configuru::Config writeToJSON() = 0;  // Implement by each device class' own Config
    virtual void readFromJSON(const configuru::Config &pt) = 0;  // Implement by each device class' own Config
    
	static void writeMonoTrackerIntrinsics(
		configuru::Config& pt,
		const MikanMonoIntrinsics& tracker_intrinsics);
	static void readMonoTrackerIntrinsics(
		const configuru::Config& pt,
		MikanMonoIntrinsics& tracker_intrinsics);

	static void writeStereoTrackerIntrinsics(
		configuru::Config& pt,
		const MikanStereoIntrinsics& tracker_intrinsics);
	static void readStereoTrackerIntrinsics(
		const configuru::Config& pt,
        MikanStereoIntrinsics& tracker_intrinsics);

    static void writeDistortionCoefficients(
        configuru::Config& pt,
        const char* coefficients_name,
        const MikanDistortionCoefficients* coefficients);
    static void readDistortionCoefficients(
        const configuru::Config& pt,
        const char* coefficients_name,
        MikanDistortionCoefficients* outCoefficients,
        const MikanDistortionCoefficients* defaultCoefficients);

    static void writeMatrix3d(
        configuru::Config &pt,
        const char *matrix_name,
        const MikanMatrix3d& matrix);
    static void readMatrix3d(
        const configuru::Config &pt,
        const char *matrix_name,
        MikanMatrix3d& outMatrix);

    static void writeMatrix43d(
        configuru::Config& pt,
        const char* matrix_name,
        const MikanMatrix4x3d& matrix);
    static void readMatrix43d(
        const configuru::Config& pt,
        const char* matrix_name,
        MikanMatrix4x3d& outMatrix);

    static void writeMatrix4d(
        configuru::Config& pt,
        const char* matrix_name,
        const MikanMatrix4d& matrix);
    static void readMatrix4d(
        const configuru::Config& pt,
        const char* matrix_name,
        MikanMatrix4d& outMatrix);

	static void writeMatrix4f(
		configuru::Config& pt,
		const char* matrix_name,
		const MikanMatrix4f& matrix);
	static void readMatrix4f(
		const configuru::Config& pt,
		const char* matrix_name,
		MikanMatrix4f& outMatrix);

	static void writeQuaderntiond(
		configuru::Config& pt,
		const char* quat_name,
		const MikanQuatd& quat);
	static void readQuaterniond(
		const configuru::Config& pt,
		const char* quat_name,
        MikanQuatd& outQuat);

    static void writeVector3d(
        configuru::Config& pt,
        const char* vector_name,
        const MikanVector3d& vector);
    static void readVector3d(
        const configuru::Config& pt,
        const char* vector_name,
        MikanVector3d& outVector);

	static void writeVector3f(
		configuru::Config& pt,
		const char* rotator_name,
		const MikanVector3f& rotator);
	static void readVector3f(
		const configuru::Config& pt,
		const char* rotator_name,
		MikanVector3f& outVector);

	static void writeRotator3f(
		configuru::Config& pt,
		const char* vector_name,
		const MikanRotator3f& vector);
	static void readRotator3f(
		const configuru::Config& pt,
		const char* vector_name,
		MikanRotator3f& outVector);

	static void writeDeviceType(
		configuru::Config& pt,
		const char* fieldName,
		const eDeviceType deviceType);
	static void readDeviceType(
		const configuru::Config& pt,
		const char* fieldName,
        eDeviceType& outDeviceType);
};