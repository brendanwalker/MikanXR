#include "CommonConfig.h"
#include "Logger.h"
#include "MikanVideoSourceTypes.h"
#include "PathUtils.h"
#include "StringUtils.h"

#include <iostream>
#include <string>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4996) // This function or variable may be unsafe
#pragma warning (disable: 4244) // 'return': conversion from 'const int64_t' to 'float', possible loss of data
#pragma warning (disable: 4715) // configuru::Config::operator[]': not all control paths return a value
#endif
#define CONFIGURU_IMPLEMENTATION 1
#include "configuru.hpp"
#ifdef _MSC_VER
#pragma warning (pop)
#endif

// -- ConfigPropertyChangeSet -----
ConfigPropertyChangeSet& ConfigPropertyChangeSet::addPropertyName(const std::string& propertyName)
{
    m_changedProperties.insert(propertyName);
    return *this;
}

bool ConfigPropertyChangeSet::hasPropertyName(const std::string& propertyName) const
{
    return m_changedProperties.find(propertyName) != m_changedProperties.end();
}

// -- CommonConfig -----
CommonConfig::CommonConfig(const std::string &fnamebase)
    : m_configName(fnamebase)
{
}

void CommonConfig::onChildConfigMarkedDirty(
    CommonConfigPtr configPtr,
    const ConfigPropertyChangeSet& changedPropertySet) 
{ 
	m_bIsDirty = true;
	if (OnMarkedDirty)
		OnMarkedDirty(configPtr, changedPropertySet);
}

void CommonConfig::addChildConfig(std::shared_ptr<CommonConfig> childConfig)
{
	childConfig->OnMarkedDirty += MakeDelegate(this, &CommonConfig::onChildConfigMarkedDirty);
	m_childConfigs.push_back(childConfig);
}

void CommonConfig::removeChildConfig(std::shared_ptr<CommonConfig> childConfig)
{
    auto it= std::find(m_childConfigs.begin(), m_childConfigs.end(), childConfig);
    if (it != m_childConfigs.end())
    {
        m_childConfigs.erase(it);
    }
}

bool CommonConfig::isMarkedDirty() const 
{ 
    return m_bIsDirty; 
}

void CommonConfig::markDirty(const ConfigPropertyChangeSet& changedPropertySet) 
{ 
	m_bIsDirty = true;

	if (OnMarkedDirty)
		OnMarkedDirty(shared_from_this(), changedPropertySet);
}

void CommonConfig::clearDirty()
{
    m_bIsDirty= false;
    for (CommonConfigPtr childConfigPtr : m_childConfigs)
    {
        childConfigPtr->clearDirty();
    }
}

const std::filesystem::path CommonConfig::getDefaultConfigPath() const
{
    const std::filesystem::path home_dir= PathUtils::getHomeDirectory();  
    const std::filesystem::path config_path = home_dir / "Mikan";
    
    if (!std::filesystem::exists(config_path))
    {
		if (!std::filesystem::create_directory(config_path))
		{
			MIKAN_LOG_ERROR("CommonConfig::getConfigPath") << "Failed to create config directory: " << config_path;
		}
    }

    const std::string configFilename= m_configName + ".json";
    const std::filesystem::path config_filepath = config_path / configFilename;

    return config_filepath;
}

void CommonConfig::save()
{
    save(getDefaultConfigPath());
}

void CommonConfig::save(const std::filesystem::path& path)
{
    m_configFullFilePath= path;

	configuru::dump_file(path.string(), writeToJSON(), configuru::JSON);
    clearDirty();
}

bool CommonConfig::load()
{
    return load(getDefaultConfigPath());
}

bool CommonConfig::load(const std::filesystem::path& path)
{
    bool bLoadedOk = false;
    
    if (std::filesystem::exists(path))
    {
        m_configFullFilePath= path;

        try
        {
			configuru::Config cfg = configuru::parse_file(path.string(), configuru::JSON);
			readFromJSON(cfg);
			clearDirty();
			bLoadedOk = true;
        }
        catch (std::exception& e)
        {
            MIKAN_LOG_ERROR("CommonConfig::load") << "Failed to load config file: " << path << " - " << e.what();
        }
    }

    return bLoadedOk;
}

configuru::Config CommonConfig::writeToJSON()
{
    return configuru::Config::object();
}

void CommonConfig::readFromJSON(const configuru::Config& pt)
{
}

void CommonConfig::writeMonoTrackerIntrinsics(
    configuru::Config& pt,
    const MikanMonoIntrinsics& tracker_intrinsics)
{
    pt["frame_width"]= tracker_intrinsics.pixel_width;
    pt["frame_height"]= tracker_intrinsics.pixel_height;
    pt["hfov"]= tracker_intrinsics.hfov;
    pt["vfov"]= tracker_intrinsics.vfov;
    pt["aspect_ratio"]= tracker_intrinsics.aspect_ratio;
    pt["zNear"]= tracker_intrinsics.znear;
    pt["zFar"]= tracker_intrinsics.zfar;

    writeMatrix3d(pt, "undistorted_camera_matrix", tracker_intrinsics.undistorted_camera_matrix);
    writeMatrix3d(pt, "distorted_camera_matrix", tracker_intrinsics.distorted_camera_matrix);
    writeDistortionCoefficients(pt, "distortion", &tracker_intrinsics.distortion_coefficients);
}

void CommonConfig::readMonoTrackerIntrinsics(
	const configuru::Config& pt,
	MikanMonoIntrinsics& tracker_intrinsics)
{

	tracker_intrinsics.pixel_width = pt.get_or<double>("frame_width", 640.0);
	tracker_intrinsics.pixel_height = pt.get_or<double>("frame_height", 480.0);
    tracker_intrinsics.hfov = pt.get_or<double>("hfov", 60.0);
    tracker_intrinsics.vfov = pt.get_or<double>("vfov", 45.0);
    tracker_intrinsics.aspect_ratio = pt.get_or<double>("aspect_ratio", 1.0);
    tracker_intrinsics.znear = pt.get_or<double>("zNear", 0.1);
    tracker_intrinsics.zfar = pt.get_or<double>("zFar", 20.0);

	MikanDistortionCoefficients default_distortion_coefficients = {};

    readMatrix3d(pt, "undistorted_camera_matrix", tracker_intrinsics.undistorted_camera_matrix);
    readMatrix3d(pt, "distorted_camera_matrix", tracker_intrinsics.distorted_camera_matrix);
    readDistortionCoefficients(pt, "distortion", 
        &tracker_intrinsics.distortion_coefficients, 
        &default_distortion_coefficients);
}

void CommonConfig::writeStereoTrackerIntrinsics(
    configuru::Config &pt,
    const MikanStereoIntrinsics& tracker_intrinsics)
{
    pt["frame_width"]= tracker_intrinsics.pixel_width;
    pt["frame_height"]= tracker_intrinsics.pixel_height;
    pt["hfov"]= tracker_intrinsics.hfov;
    pt["vfov"]= tracker_intrinsics.vfov;
    pt["aspect_ratio"]= tracker_intrinsics.aspect_ratio;
    pt["zNear"]= tracker_intrinsics.znear;
    pt["zFar"]= tracker_intrinsics.zfar;

    writeMatrix3d(pt, "left_camera_matrix", tracker_intrinsics.left_camera_matrix);
    writeMatrix3d(pt, "right_camera_matrix", tracker_intrinsics.right_camera_matrix);

    writeDistortionCoefficients(pt, "left_distortion_cofficients", &tracker_intrinsics.left_distortion_coefficients);
    writeDistortionCoefficients(pt, "right_distortion_cofficients", &tracker_intrinsics.right_distortion_coefficients);

    writeMatrix3d(pt, "left_rectification_rotation", tracker_intrinsics.left_rectification_rotation);
    writeMatrix3d(pt, "right_rectification_rotation", tracker_intrinsics.right_rectification_rotation);

	writeMatrix43d(pt, "left_rectification_projection", tracker_intrinsics.left_rectification_projection);
	writeMatrix43d(pt, "right_rectification_projection", tracker_intrinsics.right_rectification_projection);

    writeMatrix3d(pt, "rotation_between_cameras", tracker_intrinsics.rotation_between_cameras);
    writeVector3d(pt, "translation_between_cameras", tracker_intrinsics.translation_between_cameras);
    writeMatrix3d(pt, "essential_matrix", tracker_intrinsics.essential_matrix);
    writeMatrix3d(pt, "fundamental_matrix", tracker_intrinsics.fundamental_matrix);
    writeMatrix4d(pt, "reprojection_matrix", tracker_intrinsics.reprojection_matrix);
}

void CommonConfig::readStereoTrackerIntrinsics(
	const configuru::Config& pt,
	MikanStereoIntrinsics& tracker_intrinsics)
{
	tracker_intrinsics.pixel_width = pt.get_or<double>("frame_width", 640.0);
	tracker_intrinsics.pixel_height = pt.get_or<double>("frame_height", 480.0);
    tracker_intrinsics.hfov = pt.get_or<double>("hfov", 60.0);
    tracker_intrinsics.vfov = pt.get_or<double>("vfov", 45.0);
    tracker_intrinsics.aspect_ratio = pt.get_or<double>("aspect_ratio", 1.0);
    tracker_intrinsics.znear = pt.get_or<double>("zNear", 0.1);
    tracker_intrinsics.zfar = pt.get_or<double>("zFar", 20.0);

    MikanDistortionCoefficients default_distortion_coefficients = {};

    readMatrix3d(pt, "left_camera_matrix", tracker_intrinsics.left_camera_matrix);
    readMatrix3d(pt, "right_camera_matrix", tracker_intrinsics.right_camera_matrix);

    readDistortionCoefficients(pt, "left_distortion_cofficients", 
        &tracker_intrinsics.left_distortion_coefficients, 
        &default_distortion_coefficients);
    readDistortionCoefficients(pt, "right_distortion_cofficients", 
        &tracker_intrinsics.right_distortion_coefficients, 
        &default_distortion_coefficients);

    readMatrix3d(pt, "left_rectification_rotation", tracker_intrinsics.left_rectification_rotation);
    readMatrix3d(pt, "right_rectification_rotation", tracker_intrinsics.right_rectification_rotation);

	readMatrix43d(pt, "left_rectification_projection", tracker_intrinsics.left_rectification_projection);
	readMatrix43d(pt, "right_rectification_projection", tracker_intrinsics.right_rectification_projection);

    readMatrix3d(pt, "rotation_between_cameras", tracker_intrinsics.rotation_between_cameras);
    readVector3d(pt, "translation_between_cameras", tracker_intrinsics.translation_between_cameras);
    readMatrix3d(pt, "essential_matrix", tracker_intrinsics.essential_matrix);
    readMatrix3d(pt, "fundamental_matrix", tracker_intrinsics.fundamental_matrix);
    readMatrix4d(pt, "reprojection_matrix", tracker_intrinsics.reprojection_matrix);
}

void CommonConfig::writeDistortionCoefficients(
    configuru::Config& pt,
    const char* coefficients_name,
    const MikanDistortionCoefficients* coefficients)
{
    char full_property_name[256];

    StringUtils::formatString(full_property_name, sizeof(full_property_name), "%s.k1", coefficients_name);
    pt[full_property_name]= coefficients->k1;
    StringUtils::formatString(full_property_name, sizeof(full_property_name), "%s.k2", coefficients_name);
    pt[full_property_name]= coefficients->k2;
    StringUtils::formatString(full_property_name, sizeof(full_property_name), "%s.k3", coefficients_name);
    pt[full_property_name]= coefficients->k3;
    StringUtils::formatString(full_property_name, sizeof(full_property_name), "%s.k4", coefficients_name);
    pt[full_property_name]= coefficients->k4;
    StringUtils::formatString(full_property_name, sizeof(full_property_name), "%s.k5", coefficients_name);
    pt[full_property_name]= coefficients->k5;
    StringUtils::formatString(full_property_name, sizeof(full_property_name), "%s.k6", coefficients_name);
    pt[full_property_name]= coefficients->k6;
    StringUtils::formatString(full_property_name, sizeof(full_property_name), "%s.p1", coefficients_name);
    pt[full_property_name]= coefficients->p1;
    StringUtils::formatString(full_property_name, sizeof(full_property_name), "%s.p2", coefficients_name);
    pt[full_property_name]= coefficients->p2;
}

void CommonConfig::readDistortionCoefficients(
    const configuru::Config& pt,
    const char* coefficients_name,
    MikanDistortionCoefficients* outCoefficients,
    const MikanDistortionCoefficients* defaultCoefficients)
{
    char full_property_name[256];

    StringUtils::formatString(full_property_name, sizeof(full_property_name), "%s.k1", coefficients_name);
    outCoefficients->k1= pt.get_or<double>(full_property_name, defaultCoefficients->k1);
    StringUtils::formatString(full_property_name, sizeof(full_property_name), "%s.k2", coefficients_name);
    outCoefficients->k2= pt.get_or<double>(full_property_name, defaultCoefficients->k2);
    StringUtils::formatString(full_property_name, sizeof(full_property_name), "%s.k3", coefficients_name);
    outCoefficients->k3= pt.get_or<double>(full_property_name, defaultCoefficients->k3);
    StringUtils::formatString(full_property_name, sizeof(full_property_name), "%s.k4", coefficients_name);
    outCoefficients->k4= pt.get_or<double>(full_property_name, defaultCoefficients->k4);
    StringUtils::formatString(full_property_name, sizeof(full_property_name), "%s.k5", coefficients_name);
    outCoefficients->k5= pt.get_or<double>(full_property_name, defaultCoefficients->k5);
    StringUtils::formatString(full_property_name, sizeof(full_property_name), "%s.k6", coefficients_name);
    outCoefficients->k6= pt.get_or<double>(full_property_name, defaultCoefficients->k6);
    StringUtils::formatString(full_property_name, sizeof(full_property_name), "%s.p1", coefficients_name);
    outCoefficients->p1= pt.get_or<double>(full_property_name, defaultCoefficients->p1);
    StringUtils::formatString(full_property_name, sizeof(full_property_name), "%s.p2", coefficients_name);
    outCoefficients->p2= pt.get_or<double>(full_property_name, defaultCoefficients->p2);
}

void CommonConfig::writeMatrix3d(
    configuru::Config &pt,
    const char *matrix_name,
    const MikanMatrix3d& mat)
{
    // Write out 3 columns (3 entries per column)
    auto m = reinterpret_cast<const double(*)[3][3]>(&mat);
    pt[matrix_name]= configuru::Config::array({
        (*m)[0][0], (*m)[0][1], (*m)[0][2],
        (*m)[1][0], (*m)[1][1], (*m)[1][2],
		(*m)[2][0], (*m)[2][1], (*m)[2][2]});
}

void CommonConfig::readMatrix3d(
    const configuru::Config &pt,
    const char *matrix_name,
    MikanMatrix3d& outMatrix)
{
    if (pt[matrix_name].is_array())
    {
        int row= 0;
        int col= 0;
        auto m = reinterpret_cast<double(*)[3][3]>(&outMatrix);
        for (const configuru::Config& element : pt[matrix_name].as_array()) 
        {
            (*m)[col][row]= element.as_double();

            ++row;
            if (row >= 3)
            {
                row=0;
                ++col;
            }
            if (col >= 3)
                break;
        }
    }
}

void CommonConfig::writeMatrix43d(
    configuru::Config &pt,
    const char *matrix_name,
    const MikanMatrix4x3d& mat)
{
    auto m = reinterpret_cast<const double(*)[4][3]>(&mat);

    // Write out 4 columns (3 entries per column)
    pt[matrix_name]= configuru::Config::array({
		(*m)[0][0], (*m)[0][1], (*m)[0][2],
		(*m)[1][0], (*m)[1][1], (*m)[1][2],
		(*m)[2][0], (*m)[2][1], (*m)[2][2],
		(*m)[3][0], (*m)[3][1], (*m)[3][2]});
}

void CommonConfig::readMatrix43d(
    const configuru::Config &pt,
    const char *matrix_name,
    MikanMatrix4x3d& outMatrix)
{
    if (pt[matrix_name].is_array())
    {
        int row= 0;
        int col= 0;
        auto m = reinterpret_cast<double(*)[4][3]>(&outMatrix);

        for (const configuru::Config& element : pt[matrix_name].as_array()) 
        {
            (*m)[col][row] = element.as_double();

            ++row;
            if (row >= 3)
            {
                row=0;
                ++col;
            }
            if (col >= 4)
                break;
        }
    }
}

void CommonConfig::writeMatrix4d(
    configuru::Config &pt,
    const char *matrix_name,
    const MikanMatrix4d& mat)
{
    auto m = reinterpret_cast<const double(*)[4][4]>(&mat);

    pt[matrix_name]= configuru::Config::array({
		(*m)[0][0], (*m)[0][1], (*m)[0][2], (*m)[0][3],
		(*m)[1][0], (*m)[1][1], (*m)[1][2], (*m)[1][3],
		(*m)[2][0], (*m)[2][1], (*m)[2][2], (*m)[2][3],
		(*m)[3][0], (*m)[3][1], (*m)[3][2], (*m)[3][3]});
}

void CommonConfig::readMatrix4d(
    const configuru::Config &pt,
    const char *matrix_name,
    MikanMatrix4d& outMatrix)
{
    if (pt[matrix_name].is_array())
    {
        int row= 0;
        int col= 0;
        auto m = reinterpret_cast<double(*)[4][4]>(&outMatrix);
        for (const configuru::Config& element : pt[matrix_name].as_array()) 
        {
            (*m)[col][row] = element.as_double();

            ++row;
            if (row >= 4)
            {
                row=0;
                ++col;
            }
            if (col >= 4)
                break;
        }
    }
}

void CommonConfig::writeMatrix4f(
	configuru::Config& pt,
	const char* matrix_name,
	const MikanMatrix4f& mat)
{
    auto m = reinterpret_cast<const float(*)[4][4]>(&mat);

	pt[matrix_name] = configuru::Config::array({
		(*m)[0][0], (*m)[0][1], (*m)[0][2], (*m)[0][3],
		(*m)[1][0], (*m)[1][1], (*m)[1][2], (*m)[1][3],
		(*m)[2][0], (*m)[2][1], (*m)[2][2], (*m)[2][3],
		(*m)[3][0], (*m)[3][1], (*m)[3][2], (*m)[3][3] });
}

void CommonConfig::readMatrix4f(
	const configuru::Config& pt,
	const char* matrix_name,
	MikanMatrix4f& outMatrix)
{
    auto m = reinterpret_cast<float(*)[4][4]>(&outMatrix);

	if (pt[matrix_name].is_array())
	{
		int row = 0;
		int col = 0;
		for (const configuru::Config& element : pt[matrix_name].as_array())
		{
			(*m)[col][row] = element.as_float();

			++row;
			if (row >= 4)
			{
				row = 0;
				++col;
			}
			if (col >= 4)
				break;
		}
	}
}

void CommonConfig::writeQuaderntiond(
    configuru::Config& pt,
    const char* quat_name,
    const MikanQuatd& quat)
{
    pt[quat_name] = configuru::Config::array({ quat.w, quat.x, quat.y, quat.z });
}

void CommonConfig::readQuaterniond(
    const configuru::Config& pt,
    const char* quat_name,
    MikanQuatd& outQuat)
{
	if (pt.has_key(quat_name) && pt[quat_name].is_array())
	{
        outQuat.w = pt[quat_name][0].as_double();
		outQuat.x = pt[quat_name][1].as_double();
		outQuat.y = pt[quat_name][2].as_double();
		outQuat.z = pt[quat_name][3].as_double();
	}
    else
    {
		outQuat.w = 1.0;
		outQuat.x = 0.0;
		outQuat.y = 0.0;
		outQuat.z = 0.0;
    }
}

void CommonConfig::writeVector3d(
    configuru::Config &pt,
    const char *vector_name,
    const MikanVector3d& v)
{
    pt[vector_name]= configuru::Config::array({v.x, v.y, v.z});
}

void CommonConfig::readVector3d(
    const configuru::Config &pt,
    const char *vector_name,
    MikanVector3d& outVector)
{
    if (pt.has_key(vector_name) && pt[vector_name].is_array())
    {
        outVector.x= pt[vector_name][0].as_double();
        outVector.y= pt[vector_name][1].as_double();
        outVector.z= pt[vector_name][2].as_double();
    }
    else
    {
		outVector.x = 0.0;
		outVector.y = 0.0;
		outVector.z = 0.0;
    }
}

void CommonConfig::writeVector3f(
	configuru::Config& pt,
	const char* vector_name,
	const MikanVector3f& v)
{
	pt[vector_name] = configuru::Config::array({ v.x, v.y, v.z });
}

void CommonConfig::readVector3f(
	const configuru::Config& pt,
	const char* vector_name,
	MikanVector3f& outVector)
{
	if (pt.has_key(vector_name) && pt[vector_name].is_array())
	{
		outVector.x = pt[vector_name][0].as_float();
		outVector.y = pt[vector_name][1].as_float();
		outVector.z = pt[vector_name][2].as_float();
	}
	else
	{
		outVector.x = 0.f;
		outVector.y = 0.f;
		outVector.z = 0.f;
	}
}

void CommonConfig::writeVector2f(
	configuru::Config& pt,
	const char* vector_name,
	const MikanVector2f& v)
{
	pt[vector_name] = configuru::Config::array({v.x, v.y});
}

void CommonConfig::readVector2f(
	const configuru::Config& pt,
	const char* vector_name,
	MikanVector2f& outVector)
{
	if (pt.has_key(vector_name) && pt[vector_name].is_array())
	{
		outVector.x = pt[vector_name][0].as_float();
		outVector.y = pt[vector_name][1].as_float();
	}
	else
	{
		outVector.x = 0.f;
		outVector.y = 0.f;
	}
}

void CommonConfig::writeRotator3f(
    configuru::Config& pt,
    const char* rotator_name,
    const MikanRotator3f& r)
{
	pt[rotator_name] = configuru::Config::array({ r.x_angle, r.y_angle, r.z_angle });
}

void CommonConfig::readRotator3f(
    const configuru::Config& pt,
    const char* rotator_name,
    MikanRotator3f& outRotator)
{
	if (pt.has_key(rotator_name) && pt[rotator_name].is_array())
	{
		outRotator.x_angle = pt[rotator_name][0].as_float();
		outRotator.y_angle = pt[rotator_name][1].as_float();
		outRotator.z_angle = pt[rotator_name][2].as_float();
	}
	else
	{
		outRotator.x_angle = 0.f;
		outRotator.y_angle = 0.f;
		outRotator.z_angle = 0.f;
	}
}

void CommonConfig::writeQuatf(
    configuru::Config& pt,
    const char* quat_name,
    const MikanQuatf& quat)
{
	pt[quat_name] = configuru::Config::array({ quat.w, quat.x, quat.y, quat.z });
}

void CommonConfig::readQuatf(
    const configuru::Config& pt,
    const char* quat_name,
    MikanQuatf& outQuat)
{
	if (pt.has_key(quat_name) && pt[quat_name].is_array())
	{
        outQuat.w = pt[quat_name][0].as_float();
		outQuat.x = pt[quat_name][1].as_float();
		outQuat.y = pt[quat_name][2].as_float();
		outQuat.z = pt[quat_name][3].as_float();
	}
	else
	{
        outQuat.w = 1.f;
		outQuat.x = 0.f;
		outQuat.y = 0.f;
		outQuat.z = 0.f;
	}
}

void CommonConfig::writeDeviceType(
    configuru::Config& pt,
    const char* fieldName,
    const eDeviceType deviceType)
{
    pt[fieldName] = k_deviceTypeStrings[(int)deviceType];
}

void CommonConfig::readDeviceType(
    const configuru::Config& pt,
    const char* fieldName,
    eDeviceType& outDeviceType)
{
    std::string enumStringValue= pt[fieldName].as_string();
    outDeviceType= StringUtils::FindEnumValue<eDeviceType>(enumStringValue, k_deviceTypeStrings);
}