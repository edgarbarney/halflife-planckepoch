#pragma once

namespace FranUtils
{

#pragma region Non-ensured Math Funcitons

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

#pragma region Ensured Math Funcitons

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


#pragma endregion

#pragma region String Functions

	inline char* strcharstr(const char* mainstr, const char* substr)
	{
		const char* buffer1 = mainstr;
		const char* buffer2 = substr;
		const char* result = *buffer2 == 0 ? mainstr : 0;

		while (*buffer1 != 0 && *buffer2 != 0)
		{
			if (tolower((unsigned char)*buffer1) == tolower((unsigned char)*buffer2))
			{
				if (result == 0)
				{
					result = buffer1;
				}

				buffer2++;
			}
			else
			{
				buffer2 = substr;
				if (result != 0)
				{
					buffer1 = result + 1;
				}

				if (tolower((unsigned char)*buffer1) == tolower((unsigned char)*buffer2))
				{
					result = buffer1;
					buffer2++;
				}
				else
				{
					result = 0;
				}
			}

			buffer1++;
		}

		return *buffer2 == 0 ? (char*)result : 0;
	}

#pragma endregion

}
