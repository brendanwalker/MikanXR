// Define a typemap macro used to bind a C# delegate to a C STDCALL callback function type
%define %cs_callback(TYPE, CSTYPE)
    %typemap(ctype) TYPE, TYPE& "void *"
    %typemap(in) TYPE  %{ $1 = ($1_type)$input; %}
    %typemap(in) TYPE& %{ $1 = ($1_type)&$input; %}
    %typemap(imtype, inattributes="[global::System.Runtime.InteropServices.MarshalAs(global::System.Runtime.InteropServices.UnmanagedType.FunctionPtr)]", out="IntPtr") TYPE, TYPE& "CSTYPE"
    %typemap(cstype, out="IntPtr") TYPE, TYPE& "CSTYPE"
    %typemap(csin) TYPE, TYPE& "$csinput"
%enddef

// Insert the C# delegate type into the MikanClient class
%pragma(csharp) modulecode=%{
  public delegate void MikanLogDelegate(int log_level, string message);

  [global::System.Runtime.InteropServices.UnmanagedFunctionPointer(global::System.Runtime.InteropServices.CallingConvention.Cdecl)]
  public delegate void MikanLogCallback(
    int log_level,
    [MarshalAs(UnmanagedType.LPStr)]
    string log_message); 
  
  public static MikanResult Mikan_Initialize(MikanLogLevel min_log_level, MikanLogCallback log_delegate) {
    MikanLogCallback callback = (log_level, message) => {
      log_delegate(log_level, message);
    };
   
    MikanResult ret = (MikanResult)MikanClientPINVOKE.Mikan_Initialize(min_log_level, callback);
    return ret;
  }  
%}

// Register function pointer types in the typemap
%cs_callback(MikanLogCallback, MikanClient.MikanLogCallback)