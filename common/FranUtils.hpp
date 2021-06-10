#pragma once

#include <string>

/*
#if defined (CLIENT_DLL) && !defined (CDLL_DLL_H)
#include <cl_dll.h>
#endif
*/

namespace FranUtils
{

#pragma region Debug Functions

	/**
	* Prints 2 strings with a boolean value inbetween.
	* Can be used as extrapolation, when lerpfactor is outside of the range [0,1].
	*
	* @param firstStr : First string. Unformattable. You can use it to store variable name (Ex: "Foo: ")
	* @param boolSelf : Boolean
	* @param afterStr : Last string. Formattable.
	* @param Args: Standard C printing format args
	*/
	inline void PRINT_BOOL(std::string firstStr, bool boolSelf, std::string afterStr = "\n", ...)
	{
		#ifdef _DEBUG
			va_list args;
			char* tst = const_cast<char*>(afterStr.c_str());
			va_start(args, tst);
			//firstStr = firstStr + ": %s" + afterStr;
			firstStr = firstStr + "%s" + afterStr;
			#ifdef CLIENT_DLL
			gEngfuncs.Con_Printf(firstStr.c_str(), boolSelf ? "true" : "false", args);
			#else
			ALERT(at_console, firstStr.c_str(), boolSelf ? "true" : "false", args);
			#endif
		#else
			return;
		#endif
	}

	inline void PRINT(const char* cStr, ...) 
	{
		#ifdef _DEBUG
			va_list args;
			va_start(args, cStr);
			#ifdef CLIENT_DLL
			gEngfuncs.Con_Printf(cStr, args);
			#else
			ALERT(at_console, cStr, args);
			#endif
			va_end(args);
		#else
			return;
		#endif
	}

	inline void PRINT(std::string cStr, ...)
	{
		#ifdef _DEBUG
			va_list args;
			char* tst = const_cast<char*>(cStr.c_str());
			va_start(args, tst);
			#ifdef CLIENT_DLL
			gEngfuncs.Con_Printf(cStr.c_str(), args);
			#else
			ALERT(at_console, cStr.c_str(), args);
			#endif
			va_end(args);
		#else
			return;
		#endif
	}

#pragma endregion


#pragma region Non-ensured Funcitons

	/**
	* Basic linear interpolation.
	* Can be used as extrapolation, when lerpfactor is outside of the range [0,1].
	*
	* @see FranUtils::Lerp_s
	* @param lerpfactor : Factor of Interpolation
	* @param A : Starting Point
	* @param B : Ending Point
	* @return Interpolated Value
	*/
	inline float Lerp(float lerpfactor, float A, float B) 
	{ 
		return A + lerpfactor * (B - A); 
	}

	/**
	* (C - A) / (B - A) || Calculation of a float in between 2 floats as a decimal in the range [0,1].
	* Can underflow or overflow. return is not clamped.
	*
	* @see FranUtils::WhereInBetween_s
	* @param find : Number to Find
	* @param min : Starting Point (Minimum)
	* @param max : Ending Point (Maximum)
	* @return Found Value
	*/
	inline float WhereInBetween(float find, float min, float max) 
	{ 
		return (find - min) / (max - min); 
	}

	/**
	* ASSEMBLY | For HL Messages - Returns a long that contains float information
	*
	* @see FranUtils::ftol
	* @param x : Float to store
	* @return Long to send over
	*/
	inline long ftol_asm(float x)
	{
		__asm mov eax, x;
	}

	/**
	* For HL Messages - Returns a long that contains float information
	*
	* @see FranUtils::ftol_asm
	* @param x : Float to store
	* @return Long to send over
	*/
	inline long ftol(float x)
	{
		return *(long*)(&x);
	}

#pragma endregion

#pragma region Ensured Funcitons

	/**
	* Ensured linear interpolation.
	* - Ensure : Can't be used as extrapolation. lerpfactor is clamped the range [0,1].
	*
	* @see FranUtils::Lerp
	* @param lerpfactor : Factor of Interpolation
	* @param A : Starting Point
	* @param B : Ending Point
	* @return Interpolated Value
	*/
	inline float Lerp_s(float lerpfactor, float A, float B)
	{
		if (lerpfactor > 1) 
		{
			return A + 1 * (B - A);
		}
		else if (lerpfactor < 0)
		{ 
			return A + 0 * (B - A);
		}
		else
		{
			return A + lerpfactor * (B - A);
		}
	}

	/**
	* (C - A) / (B - A) || Ensured calculation of a float in between 2 floats as a decimal in the range [0,1].
	* - Ensure : Can't underflow or overflow. return is clamped the range [0,1].
	*
	* @see FranUtils::WhereInBetween
	* @param find : Number to Find
	* @param min : Starting Point (Minimum)
	* @param max : Ending Point (Maximum)
	* @return Found Value in the range [0,1]
	*/
	inline float WhereInBetween_s(float find, float min, float max)
	{
		float calc = (find - min) / (max - min);
		if (calc > 1)
			return 1;
		else if (calc < 0)
			return 0;
		else
			return calc;
	}
}

#pragma endregion