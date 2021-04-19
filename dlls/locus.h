bool		TryCalcLocus_Position	( CBaseEntity *pEntity, CBaseEntity *pLocus, const char *szText, Vector* OUTresult );
bool		TryCalcLocus_Velocity	( CBaseEntity *pEntity, CBaseEntity *pLocus, const char *szText, Vector* OUTresult, Vector* swizzleBasis = nullptr );
bool		TryCalcLocus_PYR	( CBaseEntity *pEntity, CBaseEntity *pLocus, const char *szText, Vector* OUTresult, Vector* swizzleBasis = nullptr ); //LRC 1.8
bool		TryCalcLocus_Number		( CBaseEntity *pLocus, const char *szText, float* OUTresult, Vector* swizzleBasis = nullptr, bool isPYR = false );

inline Vector CalcLocus_Position	( CBaseEntity *pEntity, CBaseEntity *pLocus, const char *szText )
{
	Vector result;
	if ( TryCalcLocus_Position( pEntity, pLocus, szText, &result ) )
		return result;
	else
		return g_vecZero;
}
inline Vector CalcLocus_Velocity	( CBaseEntity *pEntity, CBaseEntity *pLocus, const char *szText, Vector* swizzleBasis = nullptr )
{
	Vector result;
	if ( TryCalcLocus_Velocity( pEntity, pLocus, szText, &result, swizzleBasis ) )
		return result;
	else
		return g_vecZero;
}
inline Vector CalcLocus_PYR	( CBaseEntity *pEntity, CBaseEntity *pLocus, const char *szText, Vector* swizzleBasis = nullptr )
{
	Vector result;
	if ( TryCalcLocus_PYR( pEntity, pLocus, szText, &result, swizzleBasis ) )
		return result;
	else
		return g_vecZero;
}
inline float CalcLocus_Number ( CBaseEntity *pLocus, const char *szText, Vector* swizzleBasis = nullptr, bool isPYR = false )
{
	float result;
	if ( TryCalcLocus_Number( pLocus, szText, &result, swizzleBasis, isPYR ) )
		return result;
	else
		return 0;
}

bool		TryCalcLocus_NumberNonRandom( CBaseEntity *pLocus, const char *szText, float* OUTresult, Vector* swizzleBasis = nullptr, bool isPYR = false ); //LRC 1.8
bool		TryCalcLocus_NumberSimple( CBaseEntity *pLocus, const char *szText, float* OUTresult, Vector* swizzleBasis = nullptr, bool isPYR = false ); //LRC 1.8

CBaseEntity* CalcLocusParameter( CBaseEntity *pLocus, const char* szParam, Vector* swizzleBasis = nullptr, bool isPYR = false); //LRC 1.8
