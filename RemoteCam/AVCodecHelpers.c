#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// for libmingwex.a
#if (_MSC_VER < 1400)
int _get_output_format(void)
{
	return 0;
}
#endif