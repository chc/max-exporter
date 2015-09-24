#include "SceneExp.h"

CHCScnExp::CHCScnExp() {
}
CHCScnExp::~CHCScnExp() {
}
int CHCScnExp::ExtCount() {
	return 1;
}
const TCHAR *	CHCScnExp::Ext(int n) {
	return NULL;
}
const TCHAR *	CHCScnExp::LongDesc() {
	 return _T("CHC Engine Exporter");
}
const TCHAR *	CHCScnExp::ShortDesc() {
	 return _T("CHCEngExp");
}
const TCHAR *	CHCScnExp::AuthorName() {
	 return _T("CHC");
}
const TCHAR *	CHCScnExp::CopyrightMessage() {
	 return _T("BSD");
}
const TCHAR *	CHCScnExp::OtherMessage1() {
	 return _T("Msg 1");
}
const TCHAR *	CHCScnExp::OtherMessage2() {
	 return _T("Msg 2");
}
unsigned int	CHCScnExp::Version() {
	return 0;
}
void			CHCScnExp::ShowAbout(HWND hWnd) {
}
int				CHCScnExp::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options) {
	return 0;
}
BOOL			CHCScnExp::SupportsOptions(int ext, DWORD options) {
	return FALSE;
}