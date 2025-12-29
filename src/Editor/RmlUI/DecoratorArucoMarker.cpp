#include "DecoratorArucoMarker.h"

#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Property.h>
#include <RmlUi/Core/Texture.h>
#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/Geometry.h>
#include <RmlUi/Core/GeometryUtilities.h>
#include <RmlUi/Core/Vertex.h>

#include "opencv2/opencv.hpp"
#include "opencv2/objdetect/aruco_detector.hpp"

#include "Logger.h"

DecoratorArucoMarker::DecoratorArucoMarker()
	: m_markerId(0)
	, m_markerSize(200)
{
}

DecoratorArucoMarker::~DecoratorArucoMarker()
{
}

bool DecoratorArucoMarker::Initialise(int markerId, int markerSize)
{
	m_markerId = markerId;
	m_markerSize = markerSize;

	// Initialize the ArUco dictionary (6x6, 250 markers)
	// TODO: Pass in the dictionary type as a parameter
	m_dictionary = std::make_shared<cv::aruco::Dictionary>(cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250));

	if (!m_dictionary)
	{
		MIKAN_LOG_ERROR("DecoratorArucoMarker::Initialise") << "Failed to create ArUco dictionary";
		return false;
	}

	// Generate the marker texture during initialization
	return GenerateMarkerTexture(m_markerId, m_markerSize);
}


Rml::DecoratorDataHandle DecoratorArucoMarker::GenerateElementData(Rml::Element* element) const
{
	Rml::Geometry* geometry = new Rml::Geometry(element);
	geometry->SetTexture(GetTexture());

	GenerateGeometry(
		geometry->GetVertices(),
		geometry->GetIndices(),
		element);

	return reinterpret_cast<Rml::DecoratorDataHandle>(geometry);
}

// Generates geometry to render this tile across a surface.
void DecoratorArucoMarker::GenerateGeometry(
	Rml::Vector< Rml::Vertex >& vertices,
	Rml::Vector< int >& indices, 
	Rml::Element* element) const
{
	const Rml::Vector2f surface_dimensions = element->GetBox().GetSize(Rml::Box::PADDING);
	if (surface_dimensions.x <= 0 || surface_dimensions.y <= 0)
		return;

	const auto& computed = element->GetComputedValues();
	float opacity = computed.opacity();
	Rml::Colourb quad_colour = computed.image_color();

	// Apply opacity
	quad_colour.alpha = (Rml::byte)(opacity * (float)quad_colour.alpha);

	// Resize the vertex and index arrays to fit the new geometry.
	int index_offset = (int)vertices.size();
	vertices.resize(vertices.size() + 4);
	Rml::Vertex* new_vertices = &vertices[0] + index_offset;

	size_t num_indices = indices.size();
	indices.resize(indices.size() + 6);
	int* new_indices = &indices[0] + num_indices;

	// Generate the vertices for the tiled surface.
	Rml::Vector2f tile_position = Rml::Vector2f(0, 0);
	Rml::Vector2f tile_dimensions = surface_dimensions;
	Rml::Math::SnapToPixelGrid(tile_position, tile_dimensions);

	Rml::GeometryUtilities::GenerateQuad(
		new_vertices, new_indices, 
		tile_position, tile_dimensions, 
		quad_colour, 
		Rml::Vector2f(0, 0), Rml::Vector2f(1, 1),
		index_offset);
}

void DecoratorArucoMarker::ReleaseElementData(Rml::DecoratorDataHandle element_data) const
{
	Rml::Geometry* geometry = reinterpret_cast<Rml::Geometry*>(element_data);
	delete geometry;
}

void DecoratorArucoMarker::RenderElement(Rml::Element* element, Rml::DecoratorDataHandle element_data) const
{
	Rml::Geometry* geometry = reinterpret_cast<Rml::Geometry*>(element_data);
	geometry->Render(element->GetAbsoluteOffset(Rml::Box::PADDING).Round());
}

bool DecoratorArucoMarker::GenerateMarkerTexture(int markerId, int markerSize)
{
	if (!m_dictionary)
	{
		MIKAN_LOG_ERROR("DecoratorArucoMarker::GenerateMarkerTexture") << "ArUco dictionary not initialized";
		return false;
	}

	// Validate marker ID is within dictionary range
	if (markerId < 0 || markerId >= m_dictionary->bytesList.rows)
	{
		MIKAN_LOG_WARNING("DecoratorArucoMarker::GenerateMarkerTexture")
			<< "Marker ID " << markerId << " is out of range for dictionary (max: "
			<< (m_dictionary->bytesList.rows - 1) << ")";
		markerId = 0; // Fall back to marker 0
	}

	// Create texture callback that generates the ArUco marker
	Rml::TextureCallback texture_callback = [this, markerId, markerSize](const Rml::String& /*name*/, Rml::UniquePtr<const Rml::byte[]>& data, Rml::Vector2i& dimensions) -> bool {
		return CreateTextureFromMat(markerId, markerSize, data, dimensions);
	};

	// Create texture with callback
	Rml::Texture texture;
	texture.Set("aruco-marker-texture", texture_callback);
	AddTexture(texture);

	return true;
}

bool DecoratorArucoMarker::CreateTextureFromMat(int markerId, int markerSize, Rml::UniquePtr<const Rml::byte[]>& data, Rml::Vector2i& dimensions)
{
	try
	{
		// Generate the ArUco marker
		cv::Mat markerImage;
		cv::aruco::generateImageMarker(*m_dictionary, markerId, markerSize, markerImage, 1);

		// Convert from grayscale to RGBA for RmlUI
		cv::Mat rgba_image;
		cv::cvtColor(markerImage, rgba_image, cv::COLOR_GRAY2RGBA);

		// Set dimensions
		dimensions.x = rgba_image.cols;
		dimensions.y = rgba_image.rows;

		// Allocate and copy texture data
		const size_t data_size = rgba_image.total() * rgba_image.elemSize();
		Rml::byte* texture_data = new Rml::byte[data_size];
		std::memcpy(texture_data, rgba_image.data, data_size);

		// Transfer ownership to RmlUI
		data.reset(texture_data);

		return true;
	}
	catch (const cv::Exception& e)
	{
		MIKAN_LOG_ERROR("DecoratorArucoMarker::CreateTextureFromMat")
			<< "OpenCV exception generating marker: " << e.what();
		return false;
	}
}