#pragma once

//-- includes -----
#include "CommonConfigFwd.h"
#include "MikanClientTypes.h"
#include "DeviceInterface.h"
#include "MulticastDelegate.h"

#include <memory>
#include <filesystem>
#include <set>
#include <string>
#include <vector>

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
class ConfigPropertyChangeSet
{
public:
	ConfigPropertyChangeSet& addPropertyName(const std::string& propertyName);
	bool hasPropertyName(const std::string& propertyName) const;

private:
	std::set<std::string> m_changedProperties;
};

class CommonConfig : public std::enable_shared_from_this<CommonConfig> 
{
public:
    CommonConfig(const std::string &configName = std::string("CommonConfig"));

	void addChildConfig(std::shared_ptr<CommonConfig> childConfig);
	void removeChildConfig(std::shared_ptr<CommonConfig> childConfig);
	bool isMarkedDirty() const;
	void markDirty(const ConfigPropertyChangeSet& changedPropertySet);
	MulticastDelegate<void(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet)> OnMarkedDirty;

	const std::string& getConfigName() const { return m_configName; }
	const std::filesystem::path getDefaultConfigPath() const;
    const std::filesystem::path& getLoadedConfigPath() const { return m_configFullFilePath; }
    void save();
	void save(const std::filesystem::path& path);
    bool load();
	bool load(const std::filesystem::path& path);

    virtual configuru::Config writeToJSON();  // Implement by each device class' own Config
    virtual void readFromJSON(const configuru::Config &pt);  // Implement by each device class' own Config
    
	template<typename t_value_type>
	static void writeStdVector(
		configuru::Config& pt,
		const std::string& arrayName,
		const std::vector<t_value_type>& vector)
	{
		auto configArray = configuru::Config::array();

		for (auto it = vector.begin(); it != vector.end(); it++)
		{
			configArray.push_back(*it);
		}

		pt[arrayName] = configArray;
	}
	template<typename t_value_type>
	static void readStdVector(
		const configuru::Config& pt,
		const std::string& arrayName,
		std::vector<t_value_type>& vector)
	{
		const auto& configArray = pt[arrayName].as_array();

		vector.clear();
		for (auto it = configArray.begin(); it != configArray.end(); it++)
		{
			vector.push_back(it->get<t_value_type>());
		}
	}

    template<typename t_value_type>
    static void writeStdMap(
        configuru::Config& pt, 
        const std::string& mapName,
        const std::map<std::string, t_value_type>& nameValueMap)
    {
        pt[mapName]= configuru::Config::object();

        for (auto it = nameValueMap.begin(); it != nameValueMap.end(); ++it)
        {
            const std::string& name= it->first;
            const t_value_type& value = it->second;

            pt[mapName][name] = value;
        }
    }
	template<typename t_value_type>
	static void readStdMap(
		const configuru::Config& pt,
		const std::string& mapName,
		std::map<std::string, t_value_type>& nameValueMap)
	{
		const configuru::Config::ConfigObject& configObject= pt[mapName].as_object();

        nameValueMap.clear();
		for (configuru::Config::ConfigObject::const_iterator it = configObject.begin(); it != configObject.end(); ++it)
		{
			const std::string& name = it.key();
			const configuru::Config& config = it.value();
            const t_value_type& value= config.get<t_value_type>();

            nameValueMap.insert({name, value});
		}
	}

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

protected:
	std::vector<CommonConfigPtr> m_childConfigs;
	void onChildConfigMarkedDirty(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);
	void clearDirty();

	bool m_bIsDirty= false;
	std::string m_configName;
	std::filesystem::path m_configFullFilePath;
};