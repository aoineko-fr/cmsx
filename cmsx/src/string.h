//-----------------------------------------------------------------------------
//  █▀▀ █▀▄▀█ █▀ ▀▄▀
//  █▄▄ █ ▀ █ ▄█ █ █ v0.2
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
///
u8 String_GetLength(const c8* str)
{
	u8 ret = 0;
	while(*str++)
		ret++;		
	return ret;
}