#include "SceneExp.h"
#include <stdint.h>
#include <stdmat.h>
#define CHCMESH_VERSION 2
enum ECHCMeshFlags {
	ECHCMeshFlag_ColAsInt = (1<<1),
	ECHCMeshFlag_HasNormals = (1<<2),
	ECHCMeshFlag_HasCol = (1<<3),
	ECHCMeshFlag_HasUVs = (1<<4),
};
typedef struct {
	uint32_t version;
	uint32_t num_verts;
	uint32_t num_indices;
	uint8_t flags;
} CHCMeshHead;

CHCScnExp::CHCScnExp() {
}
CHCScnExp::~CHCScnExp() {
}
int CHCScnExp::ExtCount() {
	return 1;
}
const TCHAR *	CHCScnExp::Ext(int n) {
	switch(n) {
	case 0:
		return _T("MESH");
	}
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
void CHCScnExp::ExportMaterial(Mtl *mtl) {
	OutputDebugStringW(mtl->GetName());
	OutputDebugStringW(_T("\n"));
	for(int j=0;j<mtl->NumSubTexmaps();j++) {
		Texmap *texmap = mtl->GetSubTexmap(j);
		if(texmap != NULL) {
			OutputDebugStringW(texmap->GetName());
			OutputDebugStringW(_T("!!\n"));
			if (texmap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00)) {
				TSTR mapName = ((BitmapTex *)texmap)->GetMapName();
				OutputDebugStringW(mapName);
				OutputDebugStringW(_T("\n"));
				StdUVGen* uvGen = ((BitmapTex *)tex)->GetUVGen();
				if(uvGen) {
					float u_tiling = uvGen->GetUScl(0);
					float v_tiling = uvGen->GetVScl(0);
					float u_offset = uvGen->GetUOffs(0);
					float v_offset = uvGen->GetVOffs(0);
				}
			}
		}
	}
	for(int i=0;i<mtl->NumSubMtls();i++) {
		Mtl *mtl2 = mtl->GetSubMtl(i);
		if(mtl) {
			ExportMaterial(mtl2);
		}
	}
	
}
void CHCScnExp::ExportMesh(INode *node) {
	TimeValue t = 0;
	Mtl* nodeMtl = node->GetMtl();
	Matrix3 tm = node->GetObjTMAfterWSM(t);
	BOOL negScale = TMNegParity(tm);
	int vx1, vx2, vx3;

	// Order of the vertices. Get 'em counter clockwise if the objects is
	// negatively scaled.
	if (negScale) {
		vx1 = 2;
		vx2 = 1;
		vx3 = 0;
	}
	else {
		vx1 = 0;
		vx2 = 1;
		vx3 = 2;
	}
	BOOL needDel;
	TriObject* tri = GetTriObjectFromNode(node, t, needDel);
	if (!tri) {
		return;
	}
	
	Mesh* mesh = &tri->GetMesh();
	
	mesh->buildNormals();

	CHCMeshHead head;
	memset(&head,0,sizeof(head));
	head.num_verts = mesh->getNumVerts();
	head.num_indices = mesh->getNumFaces();

	uint32_t uvcount = 0;
	uint32_t numTVx = mesh->getNumTVerts();
	
	if(numTVx > 0) {
		uvcount = 1;
		head.flags |= ECHCMeshFlag_HasUVs;
	}

	head.version = CHCMESH_VERSION;
	fwrite(&head,sizeof(head),1,fd);
	fwrite(&uvcount,sizeof(uint32_t),1,fd);
	for(int i=0;i<uvcount;i++) {
		fwrite(&numTVx,sizeof(uint32_t),1,fd);
	}
	
	/*
	//get UV count
	for (int mp = 2; mp < MAX_MESHMAPS-1; mp++) {
		if (mesh->mapSupport(mp)) {
			uvcount++;
		}
	}

	if(uvcount> 0) {
		head.flags |= ECHCMeshFlag_HasUVs;
	}
	head.version = CHCMESH_VERSION;
	fwrite(&head,sizeof(head),1,fd);


	fwrite(&uvcount,sizeof(uint32_t),1,fd);
	
	//write individual UV map count
	for (int mp = 2; mp < MAX_MESHMAPS-1; mp++) {
		if (mesh->mapSupport(mp)) {
			uint32_t numTVx = mesh->getNumMapVerts(mp);
			fwrite(&numTVx,sizeof(uint32_t),1,fd);
		}
	}
		*/
	float verts[3];
	for (int i=0; i<head.num_verts; i++) {
		Point3 v = tm * mesh->verts[i];
		verts[0] = v.x;
		verts[1] = v.y;
		verts[2] = v.z;
		fwrite(&verts,sizeof(float),3,fd);
	}

	if(head.flags & ECHCMeshFlag_HasUVs) {
		for(int i=0;i<uvcount;i++) {
			for(int j=0;j<numTVx;j++) {
				UVVert tv = mesh->tVerts[j];
				fwrite(&tv.x,sizeof(float),1,fd);
				fwrite(&tv.y,sizeof(float),1,fd);
				fwrite(&tv.z,sizeof(float),1,fd);
			}
		
		}
	}

	uint32_t indices[3];
	for (int i=0; i<head.num_indices; i++) {
		indices[0] = mesh->faces[i].v[vx1];
		indices[1] = mesh->faces[i].v[vx2];
		indices[2] = mesh->faces[i].v[vx3];
		fwrite(&indices,sizeof(uint32_t),3,fd);
	}

	ExportMaterial(node->GetMtl());
}
void			CHCScnExp::ExportGeomObject(INode *node) {
	ObjectState os = node->EvalWorldState(0);
	if (!os.obj)
		return;
	
	// Targets are actually geomobjects, but we will export them
	// from the camera and light objects, so we skip them here.
	if (os.obj->ClassID() == Class_ID(TARGET_CLASS_ID, 0))
		return;

	ExportMesh(node);
	
}
void CHCScnExp::ProcessNode(INode *node) {
		// The ObjectState is a 'thing' that flows down the pipeline containing
		// all information about the object. By calling EvalWorldState() we tell
		// max to eveluate the object at end of the pipeline.
		TimeValue t = 0;
		ObjectState os = node->EvalWorldState(t); 
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
		fd = (FILE *)fopen("scene.mesh","wb");
		ProcessNode(snode);
		OutputDebugStringA("node loop\n");
		fclose(fd);
	}

	return IMPEXP_SUCCESS;
}
BOOL			CHCScnExp::SupportsOptions(int ext, DWORD options) {
	return FALSE;
}
// Determine is the node has negative scaling.
// This is used for mirrored objects for example. They have a negative scale factor
// so when calculating the normal we should take the vertices counter clockwise.
// If we don't compensate for this the objects will be 'inverted'.
BOOL CHCScnExp::TMNegParity(Matrix3 &m)
{
	return (DotProd(CrossProd(m.GetRow(0),m.GetRow(1)),m.GetRow(2))<0.0)?1:0;
}
// Return a pointer to a TriObject given an INode or return NULL
// if the node cannot be converted to a TriObject
TriObject* CHCScnExp::GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt)
{
	deleteIt = FALSE;
	Object *obj = node->EvalWorldState(t).obj;
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) { 
		TriObject *tri = (TriObject *) obj->ConvertToType(t, 
			Class_ID(TRIOBJ_CLASS_ID, 0));
		// Note that the TriObject should only be deleted
		// if the pointer to it is not equal to the object
		// pointer that called ConvertToType()
		if (obj != tri) deleteIt = TRUE;
		return tri;
	}
	else {
		return NULL;
	}
}