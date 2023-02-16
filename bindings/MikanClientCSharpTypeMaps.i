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

%typemap(ctype)  void** "void **"
%typemap(imtype) void** "ref System.IntPtr"
%typemap(cstype) void** "ref System.IntPtr"
%typemap(csin)   void** "$csinput"
%typemap(in)     void** %{ $1 = $input; %}
%typemap(out)    void** %{ $result = $1; %}
%typemap(csout, excode=SWIGEXCODE)  void** { 
    System.IntPtr cPtr = $imcall;$excode
    return cPtr;
    }
%typemap(csvarout, excode=SWIGEXCODE2) void** %{ 
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
  
  PROPERTY_NAME - The array property name we want exposed
  C_TYPE - The C primitive array type
  CS_TYPE - The corresponding C# primitive array type
  IN_DATA_GET_FUNCTION - The swig-generated p/invoke call that fetches the array pointer (ex: PSMVirtualController_axisStates_get)
  LENGTH_EXPRESSION - Expression used to determine the length of the array
*/
%define CUSTOM_READONLY_PRIMITIVE_TYPE_ARRAY_PROPERTY(PROPERTY_NAME,C_TYPE,CS_TYPE,IN_DATA_GET_FUNCTION,LENGTH_EXPRESSION)
  %typemap(cstype, out="CS_TYPE[]") C_TYPE PROPERTY_NAME[ANY] "CS_TYPE[]"
  %typemap(csvarout, excode=SWIGEXCODE2) C_TYPE PROPERTY_NAME[ANY]
  %{
    get {
      System.IntPtr cPtr = MikanClientPINVOKE.IN_DATA_GET_FUNCTION(swigCPtr);
      int len = (int)LENGTH_EXPRESSION;
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

/*
  Macro to marshall a enum type array property from C to C#.
  NOTE: This makes a new copy of the managed copy of the enum type array every time this accessor is called.
  
  PROPERTY_NAME - The array property name we want exposed
  ENUM_TYPE - The corresponding C# primitive array type
  IN_DATA_GET_FUNCTION - The swig-generated p/invoke call that fetches the array pointer (ex: PSMVirtualController_axisStates_get)
  LENGTH_EXPRESSION - Expression used to determine the length of the array
*/
%define CUSTOM_READONLY_ENUM_TYPE_ARRAY_PROPERTY(PROPERTY_NAME,ENUM_TYPE,IN_DATA_GET_FUNCTION,LENGTH_EXPRESSION)
  %typemap(cstype, out="ENUM_TYPE[]") ENUM_TYPE PROPERTY_NAME[ANY] "ENUM_TYPE[]"
  %typemap(csvarout, excode=SWIGEXCODE2) ENUM_TYPE PROPERTY_NAME[ANY]
  %{
    get {
      System.IntPtr cPtr = MikanClientPINVOKE.IN_DATA_GET_FUNCTION(swigCPtr);
      int len = (int)LENGTH_EXPRESSION;
      if (len<=0)
      {
        return null;
      }
      ENUM_TYPE[] returnArray = new ENUM_TYPE[len];
      byte[] intermediateArray = new byte[len];
      System.Runtime.InteropServices.Marshal.Copy(cPtr, intermediateArray, 0, len);
      for (int i = 0; i < len; ++i)
      {
        returnArray[i] = (ENUM_TYPE)intermediateArray[i];
      }
       
      return returnArray;
    }
  %}
%enddef

/*
  Macro to marshall a primitive type buffer property from C to C#.
  NOTE: This makes a new copy of the managed copy of the primitive type buffer every time this accessor is called.
  
  PROPERTY_NAME - The array property name we want exposed
  C_TYPE - The C primitive array type
  CS_TYPE - The corresponding C# primitive array type
  IN_DATA_GET_FUNCTION - The swig-generated p/invoke call that fetches the array pointer (ex: PSMVirtualController_axisStates_get)
  LENGTH_EXPRESSION - Expression used to determine the length of the array
*/
%define CUSTOM_READONLY_PRIMITIVE_TYPE_BUFFER_PROPERTY(PROPERTY_NAME,C_TYPE,CS_TYPE,IN_DATA_GET_FUNCTION,LENGTH_EXPRESSION)
  %typemap(cstype, out="CS_TYPE[]") C_TYPE* PROPERTY_NAME "CS_TYPE[]"
  %typemap(csvarout, excode=SWIGEXCODE2) C_TYPE* PROPERTY_NAME
  %{
    get {
      System.IntPtr cPtr = MikanClientPINVOKE.IN_DATA_GET_FUNCTION(swigCPtr);
      int len = (int)LENGTH_EXPRESSION;
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

/*
  Macro to marshall a struct array property from C to C#.
  NOTE: This makes a new copy of the managed copy of the struct array every time this accessor is called.
  
  PROPERTY_NAME - The array property name we want exposed
  STRUCT_TYPE - The C# structure type
  IN_DATA_GET_FUNCTION - The swig-generated p/invoke call that fetches the array pointer (ex: PSMTrackerList_trackers_get)
  LENGTH_EXPRESSION - Expression used to determine the length of the array
*/
%define CUSTOM_READONLY_STRUCT_ARRAY_PROPERTY(PROPERTY_NAME,STRUCT_TYPE,IN_DATA_GET_FUNCTION,LENGTH_EXPRESSION) 
  %typemap(cstype, out="STRUCT_TYPE[]") STRUCT_TYPE PROPERTY_NAME[ANY] "STRUCT_TYPE[]"
  %typemap(csvarout, excode=SWIGEXCODE2) STRUCT_TYPE PROPERTY_NAME[ANY]
  %{
    get {
      STRUCT_TYPE[] returnArray;
      
      int structSize = (int)MikanClientPINVOKE. ## STRUCT_TYPE ## _getSize();
      System.IntPtr cPtr = MikanClientPINVOKE.IN_DATA_GET_FUNCTION(swigCPtr);
      int len = LENGTH_EXPRESSION;
      if (len<=0)
      {
        return null;
      }
      returnArray = new STRUCT_TYPE[len];
      for (int i = 0; i < len; ++i)
      {
          System.IntPtr data = new System.IntPtr(cPtr.ToInt64() + structSize * i);
          returnArray[i] = new STRUCT_TYPE(data, false);        
      }

      return returnArray;
    }
  %}
%enddef

// Marshall the spatial_anchor_id_list on MikanSpatialAnchorList from unmanaged to managed memory
CUSTOM_READONLY_PRIMITIVE_TYPE_ARRAY_PROPERTY(
  spatial_anchor_id_list, 
  int,
  int, 
  MikanSpatialAnchorList_spatial_anchor_id_list_get,
  this.spatial_anchor_count);

// Marshall the stencil_id_list on MikanStencilList from unmanaged to managed memory
CUSTOM_READONLY_PRIMITIVE_TYPE_ARRAY_PROPERTY(
  stencil_id_list, 
  int,
  int, 
  MikanStencilList_stencil_id_list_get,
  this.stencil_count);

// Marshall the vr_device_id_list on MikanVRDeviceList from unmanaged to managed memory
CUSTOM_READONLY_PRIMITIVE_TYPE_ARRAY_PROPERTY(
  vr_device_id_list, 
  int,
  int, 
  MikanVRDeviceList_vr_device_id_list_get,
  this.vr_device_count);

// Apply a rule for renaming the enum elements to avoid the common prefixes
// which are redundant in C#/Java
%rename("%(regex:/^([A-Z][a-z]+)+_(.*)/\\2/)s", %$isenumitem) ""; // MikanResult_Success -> Success