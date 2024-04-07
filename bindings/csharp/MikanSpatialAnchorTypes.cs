using System.Collections.Generic;

namespace MikanXR
{
	public class MikanSpatialAnchorList : MikanResponse
	{
		public List<int> spatial_anchor_id_list { get; set; }

		public MikanSpatialAnchorList() : base(typeof(MikanSpatialAnchorList).Name) {}
	};

	public class MikanSpatialAnchorInfo : MikanResponse
	{
		public int anchor_id { get; set ;}
		public MikanTransform world_transform { get; set ;}
		public string anchor_name { get; set ;}

		public MikanSpatialAnchorInfo() : base(typeof(MikanSpatialAnchorInfo).Name) {}
	};	
}