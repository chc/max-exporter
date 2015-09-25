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
void			CHCScnExp::ExportGeomObject(INode *node) {
	ObjectState os = node->EvalWorldState(0);
	if (!os.obj)
		return;
	
	// Targets are actually geomobjects, but we will export them
	// from the camera and light objects, so we skip them here.
	if (os.obj->ClassID() == Class_ID(TARGET_CLASS_ID, 0))
		return;
	
}
void CHCScnExp::ProcessNode(INode *node) {
		// The ObjectState is a 'thing' that flows down the pipeline containing
		// all information about the object. By calling EvalWorldState() we tell
		// max to eveluate the object at end of the pipeline.
		ObjectState os = node->EvalWorldState(0); 
		if(os.obj) {
			switch(os.obj->SuperClassID()) {
				case GEOMOBJECT_CLASS_ID: {
					ExportGeomObject(node);
					break;
				}
			}
		}
}
int				CHCScnExp::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options) {
	INode *node = i->GetRootNode();
	int numChildren = node->NumberOfChildren();
	for(int i=0;i<numChildren;i++) {
		INode *snode = node->GetChildNode(i);
		ProcessNode(snode);
	}
	return 0;
}
BOOL			CHCScnExp::SupportsOptions(int ext, DWORD options) {
	return FALSE;
}