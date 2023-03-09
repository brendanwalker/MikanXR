%include "arrays_csharp.i"
%include "typemaps.i"
%include "swigtype_inout.i"

//------------------------------TypeMaps--------------------------
//Swig typemaps help to either convert a C++ type into a C# type or tell it which types are the same
//Swig typemaps also can modify the postprocessed C++ file as well as the postprocessed C# files
//For example the cscode typemap is used extensively to write C# code into the postprocessed file
//----------------------------------------------------------------

//------------------------------Simple Typemaps--------------------------
// Remap C-style char * -> C# string
CSHARP_ARRAYS(char *,string)

%typemap(ctype)  void* "void *"
%typemap(imtype) void* "System.IntPtr"
%typemap(cstype) void* "System.IntPtr"
%typemap(csin)   void* "$csinput"
%typemap(in)     void* %{ $1 = $input; %}
%typemap(out)    void* %{ $result = $1; %}
%typemap(csout, excode=SWIGEXCODE)  void* { 
    System.IntPtr cPtr = $imcall;$excode
    return cPtr;
    }
%typemap(csvarout, excode=SWIGEXCODE2) void* %{ 
    get {
        System.IntPtr cPtr = $imcall;$excode 
        return cPtr; 
   } 
%}

%typemap(ctype)  unsigned char* "unsigned char *"
%typemap(imtype) unsigned char* "System.IntPtr"
%typemap(cstype) unsigned char* "System.IntPtr"
%typemap(csin)   unsigned char* "$csinput"
%typemap(in)     unsigned char* %{ $1 = $input; %}
%typemap(out)    unsigned char* %{ $result = $1; %}
%typemap(csout, excode=SWIGEXCODE)  unsigned char* { 
    System.IntPtr cPtr = $imcall;$excode
    return cPtr;
    }
%typemap(csvarout, excode=SWIGEXCODE2) unsigned char* %{ 
    get {
        System.IntPtr cPtr = $imcall;$excode 
        return cPtr; 
   } 
%}  

//------------------------------Complex Typemaps--------------------------

/*
  Macro used to create a sizeof helper for the given structure.
  These helper functions are needed by CUSTOM_READONLY_STRUCT_ARRAY_PROPERTY.
  
  STRUCT_TYPE - The C# structure type
*/
%define MAKE_STRUCT_SIZEOF_ACCESSOR(STRUCT_TYPE)
%{
size_t STRUCT_TYPE ## _getSize()
{
  return sizeof(STRUCT_TYPE);
}
%}
size_t STRUCT_TYPE ## _getSize();
%enddef

/*
  Macro to marshall a primitive type array property from C to C#.
  NOTE: This makes a new copy of the managed copy of the primitive type array every time this accessor is called.
  
  ARRAY_PROPERTY - The array property name we want exposed
  LENGTH_PROPERTY - Property used to determine the length of the array
  C_TYPE - The C primitive array type
  CS_TYPE - The corresponding C# primitive array type
  IN_DATA_GET_FUNCTION - The swig-generated p/invoke call that fetches the array pointer (ex: MikanSpatialAnchorList_spatial_anchor_id_list_get)
*/
%define CUSTOM_READONLY_PRIMITIVE_TYPE_ARRAY_PROPERTY(ARRAY_PROPERTY,LENGTH_PROPERTY,C_TYPE,CS_TYPE,IN_DATA_GET_FUNCTION)
  %typemap(cstype, out="CS_TYPE[]") C_TYPE ARRAY_PROPERTY[ANY] "CS_TYPE[]"
  %typemap(csvarout, excode=SWIGEXCODE2) C_TYPE ARRAY_PROPERTY[ANY]
  %{
    get {
      System.IntPtr cPtr = MikanClientPINVOKE.IN_DATA_GET_FUNCTION(swigCPtr);
      int len = (int)LENGTH_PROPERTY;
      if (len<=0)
      {
        return null;
      }
      CS_TYPE[] returnArray = new CS_TYPE[len];
      System.Runtime.InteropServices.Marshal.Copy(cPtr, returnArray, 0, len);
       
      return returnArray;
    }
  %}
%enddef

// Marshall the spatial_anchor_id_list on MikanSpatialAnchorList from unmanaged to managed memory
%immutable spatial_anchor_id_list;
%immutable spatial_anchor_count;
CUSTOM_READONLY_PRIMITIVE_TYPE_ARRAY_PROPERTY(
  spatial_anchor_id_list,
  spatial_anchor_count,
  int,
  int, 
  MikanSpatialAnchorList_spatial_anchor_id_list_get);

// Marshall the spatial_fastener_id_list on MikanSpatialFastenerList from unmanaged to managed memory
%immutable spatial_fastener_id_list;
%immutable spatial_fastener_count;
CUSTOM_READONLY_PRIMITIVE_TYPE_ARRAY_PROPERTY(
  spatial_fastener_id_list,
  spatial_fastener_count,
  int,
  int, 
  MikanSpatialFastenerList_spatial_fastener_id_list_get);

// Marshall the stencil_id_list on MikanStencilList from unmanaged to managed memory
%immutable stencil_id_list;
%immutable stencil_count;
CUSTOM_READONLY_PRIMITIVE_TYPE_ARRAY_PROPERTY(
  stencil_id_list,
  stencil_count,
  int,
  int, 
  MikanStencilList_stencil_id_list_get);

// Marshall the vr_device_id_list on MikanVRDeviceList from unmanaged to managed memory
%immutable vr_device_id_list;
%immutable vr_device_count;
CUSTOM_READONLY_PRIMITIVE_TYPE_ARRAY_PROPERTY(
  vr_device_id_list, 
  vr_device_count,
  int,
  int, 
  MikanVRDeviceList_vr_device_id_list_get);

// Apply a rule for renaming the enum elements to avoid the common prefixes
// which are redundant in C#/Java
%rename("%(regex:/^([A-Z][a-z]+)+_(.*)/\\2/)s", %$isenumitem) ""; // MikanResult_Success -> Success