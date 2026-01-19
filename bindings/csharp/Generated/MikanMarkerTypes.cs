// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public enum MikanMarkerDictionaryType
	{
		DICT_4x4= 0,
		DICT_5x5= 1,
		DICT_6x6= 2,
		DICT_7x7= 3,
	};

	public class MikanMarkerComponentValues : MikanComponentValues
	{
		public static new readonly long classId= 2808287499176319284;

		public int aruco_id;
		public float length_mm;
	};

	public class MikanMarkerSystemValues
	{
		public static readonly long classId= 133061264921636720;

		public int aruco_id_list;
		public MikanMarkerDictionaryType aruco_dictionary_type;
		public int charuco_rows;
		public int charuco_cols;
		public float charuco_square_length_mm;
		public float charuco_marker_length_mm;
		public MikanMarkerDictionaryType charuco_dictionary_type;
	};

}
