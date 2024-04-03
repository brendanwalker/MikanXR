using System.Collections;

namespace MikanXR
{
	public struct MikanSpatialAnchorList : MikanResponse
	{
		public List<int> spatial_anchor_id_list { get; set; }

		public MikanSpatialAnchorList() : MikanResponse(typeof(MikanSpatialAnchorList).Name) {}
	};

	public struct MikanSpatialAnchorInfo : MikanResponse
	{
		public int anchor_id { get; set ;}
		public MikanTransform world_transform { get; set ;}
		public string anchor_name { get; set ;}

		public MikanSpatialAnchorList() : MikanResponse(typeof(MikanSpatialAnchorInfo).Name) {}
	};	
}