//macros borrowed from GLUT to get rid of win32 dependent junk in gl headers
#ifdef _WIN32
# ifndef APIENTRY
#  define VGUI_APIENTRY_DEFINED
#  if (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#   define APIENTRY    __stdcall
#  else
#   define APIENTRY
#  endif
# endif
# ifndef CALLBACK
#  define VGUI_CALLBACK_DEFINED
#  if (defined(_M_MRX000) || defined(_M_IX86) || defined(_M_ALPHA) || defined(_M_PPC)) && !defined(MIDL_PASS)
#   define CALLBACK __stdcall
#  else
#   define CALLBACK
#  endif
# endif
# ifndef WINGDIAPI
#  define VGUI_WINGDIAPI_DEFINED
#  define WINGDIAPI __declspec(dllimport)
# endif
# ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#  define _WCHAR_T_DEFINED
# endif
# pragma comment(lib,"opengl32.lib")
#endif

#include <GL/gl.h>