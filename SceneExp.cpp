#include "SceneExp.h"
#include <stdint.h>
#include <stdmat.h>
#include <bmmlib.h>
#include "crc32.h"
#define CHCMESH_VERSION 2
#define MAX_MATERIAL_PASSES 4
enum ECHCMeshFlags {
	ECHCMeshFlag_ColAsInt = (1<<1),
	ECHCMeshFlag_HasNormals = (1<<2),
	ECHCMeshFlag_HasCol = (1<<3),
	ECHCMeshFlag_HasUVs = (1<<4),
};
typedef struct {
	uint32_t version;
	uint32_t num_meshes;
	uint32_t num_materials;
} CHCMeshHead;
typedef struct {
	uint32_t num_verts;
	uint32_t num_indices;
	uint8_t flags;
	uint32_t m_material_checksum;
} CHCMeshItemHead;

typedef struct {
	uint32_t m_checksum;
	bool m_tiling[3]; //UVW
	float m_uv_offset[3];
} CHCMaterialTexInfo;
typedef struct {
	uint32_t m_material_checksum;
	float m_specular_colour[3];
	float m_ambient_colour[3];
	uint8_t m_tex_count;
} CHCMaterialInfo;


typedef struct {
	uint32_t num_textures;
} CHCTexTableHead;
enum EColourType {
	EColourType_8BPP_256Palette,
	EColourType_16BPP,
	EColourType_24BPP,
	EColourType_32BPP,
	EColourType_DXT1,
	EColourType_DXT2,
	EColourType_DXT3,
	EColourType_DXT5,
};
typedef struct {
	uint32_t checksum;
	uint32_t width;
	uint32_t height;
	EColourType colourType;
	uint32_t data_size;
}CHCTexTableItem;
CHCScnExp::CHCScnExp() {
	m_tex_count = 0;
	m_mtl_count = 0;
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
void CHCScnExp::AddTextureToTexTbl(Texmap *texmap, uint32_t checksum) {
	BitmapTex *btex = ((BitmapTex *)texmap);
	BitmapInfo  bi;
	TimeValue t = 0;
	Bitmap *bmap = btex->GetBitmap(t);

	m_tex_count++;
	
	bi.SetWidth ( bmap->Width()   );
	bi.SetHeight( bmap->Height() );
	bi.SetType  ( BMM_TRUE_32   );
	bi.SetFlags ( MAP_HAS_ALPHA );
	bi.SetCustomFlag( 0 );
	Bitmap *bmap_out = TheManager->Create(&bi);
	texmap->RenderBitmap(t,bmap_out);

	CHCTexTableItem item;
	memset(&item,0,sizeof(item));
	item.checksum = checksum;
	item.width = bmap->Width();
	item.height = bmap->Height();
	item.colourType = EColourType_32BPP;
	item.data_size = bi.Width() * bi.Height() * sizeof(uint32_t);
	fwrite(&item,sizeof(item),1,texfd);
	BMM_Color_64 *col_data = (BMM_Color_64*)malloc(bi.Width() * sizeof(BMM_Color_64));
	uint32_t col = 0;
	for(int i=0;i<bi.Height();i++) {
		bmap_out->GetPixels(0,i,bi.Width(),col_data);
		for(int j=0;j<bi.Width();j++) {
			col = 0;
			col |= ((uint8_t)col_data[j].a) << 24;
			col |= ((uint8_t)col_data[j].b) << 16;
			col |= ((uint8_t)col_data[j].g) << 8;
			col |= ((uint8_t)col_data[j].r);
			fwrite(&col,sizeof(col),1,texfd);
		}
	}
	free(col_data);
	TheManager->DelBitmap(bmap_out);
}
uint32_t CHCScnExp::GetChecksum(TSTR str) {
	char ostr[128];
#ifdef _UNICODE
	wcstombs(ostr,str.data(),sizeof(ostr));
#else
	sprintf(ostr,"%s",str.data());
#endif
	return crc32(0,ostr,strlen(ostr));
}
void CHCScnExp::ExportMaterial(Mtl *mtl) {
	m_mtl_count++;
	CHCMaterialInfo mat;
	memset(&mat,0,sizeof(mat));
	mat.m_material_checksum = GetChecksum(mtl->GetName());
	Color specCol = mtl->GetSpecular();
	mat.m_specular_colour[0] = specCol.r;
	mat.m_specular_colour[1] = specCol.g;
	mat.m_specular_colour[2] = specCol.b;
	specCol = mtl->GetAmbient();
	mat.m_ambient_colour[0] = specCol.r;
	mat.m_ambient_colour[1] = specCol.g;
	mat.m_ambient_colour[2] = specCol.b;


	//temporary hack until multiple mats are sorted..
	Texmap *texmap = NULL;
	int tex_count = 0;
	for(int i=0;i<mtl->NumSubTexmaps();i++) {
		Texmap *tmap = mtl->GetSubTexmap(i);
		if(tmap!= NULL) {
			if (tmap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00)) {
				tex_count++;
				texmap = tmap;
			}
		}
	}
	for(int i=0;i<mtl->NumSubMtls();i++) {
		Mtl *mtl2 = mtl->GetSubMtl(i);
		if(mtl2) {
			for(int i=0;i<mtl2->NumSubTexmaps();i++) {
				Texmap *tmap = mtl2->GetSubTexmap(i);
				if(tmap!= NULL) {
					if (tmap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00)) {
						tex_count++;
						texmap = tmap;
					}
				}
			}
		}
	}

	mat.m_tex_count = tex_count;
	fwrite(&mat,sizeof(mat),1,fd);
	if(texmap != NULL) {
		CHCMaterialTexInfo tex;
		memset(&tex,0,sizeof(tex));
		BitmapTex *btex = ((BitmapTex *)texmap);
		TSTR mapName = btex->GetMapName();
		StdUVGen* uvGen = btex->GetUVGen();
		tex.m_checksum = GetChecksum(mapName);
		if(uvGen) {
			tex.m_tiling[0] = uvGen->GetUScl(0);
			tex.m_tiling[1] = uvGen->GetVScl(0);
			tex.m_uv_offset[0] = uvGen->GetUOffs(0);
			tex.m_uv_offset[1] = uvGen->GetVOffs(0);
		}
		AddTextureToTexTbl(texmap,tex.m_checksum);
		fwrite(&tex,sizeof(tex),1,fd);
	}
	/*
	for(int j=0;j<mtl->NumSubTexmaps();j++) {
		CHCMaterialTexInfo tex;
		memset(&tex,0,sizeof(tex));

		Texmap *texmap = mtl->GetSubTexmap(j);
		if(texmap != NULL) {
			if (texmap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0x00)) {
				BitmapTex *btex = ((BitmapTex *)texmap);
				TSTR mapName = btex->GetMapName();
				StdUVGen* uvGen = btex->GetUVGen();
				tex.m_checksum = crc32(0,mapName.data(),mapName.Length());
				char msg[128];
				sprintf(msg,"mat exp checksum is: 0x%08X\n",tex.m_checksum);
				OutputDebugStringA(msg);
				if(uvGen) {
					tex.m_tiling[0] = uvGen->GetUScl(0);
					tex.m_tiling[1] = uvGen->GetVScl(0);
					tex.m_uv_offset[0] = uvGen->GetUOffs(0);
					tex.m_uv_offset[1] = uvGen->GetVOffs(0);
				}
				AddTextureToTexTbl(texmap,tex.m_checksum);
				fwrite(&tex,sizeof(tex),1,fd);
			}
		}
	}
	for(int i=0;i<mtl->NumSubMtls();i++) {
		Mtl *mtl2 = mtl->GetSubMtl(i);
		if(mtl2) {
			ExportMaterial(mtl2);
		}
	}
	*/
	
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

	CHCMeshItemHead head;
	memset(&head,0,sizeof(head));
	head.num_verts = mesh->getNumVerts();
	head.num_indices = mesh->getNumFaces();

	Mtl *mtl = node->GetMtl();
	if(mtl) {
		head.m_material_checksum = GetChecksum(mtl->GetName());
	}
	uint32_t uvcount = 0;
	uint32_t numTVx = mesh->getNumTVerts();
	
	if(numTVx > 0) {
		uvcount = 1;
		head.flags |= ECHCMeshFlag_HasUVs;
	}

	fwrite(&head,sizeof(head),1,fd);
	if(head.flags & ECHCMeshFlag_HasUVs) {
		fwrite(&uvcount,sizeof(uint32_t),1,fd);
		for(int i=0;i<uvcount;i++) {
			fwrite(&numTVx,sizeof(uint32_t),1,fd);
		}
	}

	float verts[3];
	for (int i=0; i<head.num_verts; i++) {
		Point3 v = tm * mesh->verts[i];
		verts[0] = v.x;
		verts[1] = v.y;
		verts[2] = v.z;
		fwrite(&verts,sizeof(float),3,fd);
	}

	//Point2 *tv = new Point2[head.num_verts];
	float ident = 1.0f;
	//GetTVerts(mesh,tv);
	if(head.flags & ECHCMeshFlag_HasUVs) {
		for(int i=0;i<uvcount;i++) {
			for(int j=0;j<numTVx;j++) {
				Point3 v = mesh->tVerts[j];
				fwrite(&v.x,sizeof(float),1,fd);
				fwrite(&v.y,sizeof(float),1,fd);
				fwrite(&v.z,sizeof(float),1,fd);
			}
		
		}
	}
	//delete tv;

	uint32_t indices[3];
	for (int i=0; i<head.num_indices; i++) {
		indices[0] = mesh->faces[i].v[vx1];
		indices[1] = mesh->faces[i].v[vx2];
		indices[2] = mesh->faces[i].v[vx3];
		fwrite(&indices,sizeof(uint32_t),3,fd);
	}
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
void			CHCScnExp::ExportGeomMaterial(INode *node) {
	ObjectState os = node->EvalWorldState(0);
	if (!os.obj)
		return;
	
	// Targets are actually geomobjects, but we will export them
	// from the camera and light objects, so we skip them here.
	if (os.obj->ClassID() == Class_ID(TARGET_CLASS_ID, 0))
		return;
	Mtl *mtl = node->GetMtl();
	if(mtl)
		ExportMaterial(mtl);
}
void CHCScnExp::ProcessMesh(INode *node) {
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
void CHCScnExp::ProcessMaterial(INode *node) {
		TimeValue t = 0;
		ObjectState os = node->EvalWorldState(t); 
		if(os.obj) {
			switch(os.obj->SuperClassID()) {
				case GEOMOBJECT_CLASS_ID: {
					ExportGeomMaterial(node);
					break;
				}
			}
		}
}
int				CHCScnExp::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options) {
	INode *node = i->GetRootNode();
	char out_name[FILENAME_MAX+1];
	char fname[FILENAME_MAX+1];
#ifdef _UNICODE
	wcstombs(fname,name,sizeof(fname));
#else
	sprintf(fname,"%s",name);
#endif
	sprintf(out_name,"%s.mesh",fname);
	fd = (FILE *)fopen(out_name,"wb");
	sprintf(out_name,"%s.tex",fname);
	texfd = (FILE *)fopen(out_name,"wb");
	CHCTexTableHead tex;
	memset(&tex,0,sizeof(tex));
	fwrite(&tex,sizeof(tex),1,texfd);
	int numChildren = node->NumberOfChildren();
	CHCMeshHead head;
	memset(&head,0,sizeof(head));
	head.num_meshes = numChildren;
	head.num_materials = numChildren;
	head.version = CHCMESH_VERSION;
	fwrite(&head,sizeof(head),1,fd);
	
	for(int i=0;i<numChildren;i++) {
		INode *snode = node->GetChildNode(i);
		ProcessMesh(snode);
	}
	for(int i=0;i<numChildren;i++) {
		INode *snode = node->GetChildNode(i);
		ProcessMaterial(snode);
	}
	head.num_materials = m_mtl_count;
	fseek(fd,0,SEEK_SET);
	fwrite(&head,sizeof(head),1,fd);
	fclose(fd);
	tex.num_textures = m_tex_count;
	fseek(texfd,0,SEEK_SET);
	fwrite(&tex,sizeof(tex),1,texfd);
	fclose(texfd);
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
uint32_t CHCScnExp::getTextureChecksum(const char *path) {
	char _path[FILENAME_MAX+1];
	return crc32(0,path,strlen(path));
}

short CHCScnExp::GetTVerts(Mesh* mesh, Point2 *tv) {
	int nv = mesh->getNumVerts();
 	int nf = mesh->getNumFaces();
	short wrap = 0;
	BitArray done(nv);
	for (int j=0; j<nf; j++) {
		Face& face = mesh->faces[j];
		TVFace& tvface = mesh->tvFace[j];
		for (int k=0; k<3; k++)  {
			// get the texture vertex.
			Point3 uvw = mesh->tVerts[tvface.t[k]];
			Point2 v(uvw.x,uvw.y);
			// stuff it into the 3DSr4 vertex
			int vert = face.v[k];
			if (vert>65535) continue;
			if (!done[vert]) {
				tv[vert] = v;
				done.Set(vert,1);
				}
			else {
				if (v.x!=tv[vert].x) {
					//wrap |= UWRAP;
					if (v.x<tv[vert].x) tv[vert].x = v.x;
					}
				if (v.y!=tv[vert].y) {
					//wrap |= VWRAP;
					if (v.y<tv[vert].y) tv[vert].y = v.y;
					}
				}
			}
		}
	return wrap;
	}
