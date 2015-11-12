#include "SceneExp.h"
#include <stdint.h>
#include <stdmat.h>
#include <bmmlib.h>
#include <ilayer.h>
#include <ILayerProperties.h>


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


void CHCScnExp::ExportMaterial(Mtl *mtl, pugi::xml_node *xmlnode) {

	pugi::xml_node xnode = m_materials_xml.append_child();
    xnode.set_name("material");
#ifdef _UNICODE
	xnode.append_attribute("name") = mtl->GetName().ToCStr();
#else
	xnode.append_attribute("name") = mtl->GetName().data();
#endif

	Color specCol = mtl->GetSpecular();
	
	pugi::xml_node param = xnode.append_child();
	param.set_name("specular_colour");
	param.append_attribute("r") = specCol.r;
	param.append_attribute("g") = specCol.g;
	param.append_attribute("b") = specCol.b;


	param = xnode.append_child();
	param.set_name("ambient_colour");
	param.append_attribute("r") = specCol.r;
	param.append_attribute("g") = specCol.g;
	param.append_attribute("b") = specCol.b;

	param = xnode.append_child();
	param.set_name("shine");
	param.append_attribute("shine") = mtl->GetShininess();
	param.append_attribute("shine_strength") = mtl->GetShinStr();


	param = xnode.append_child();
	param.set_name("transparency");
	param.append_attribute("transparency") = mtl->GetXParency();

	StdMat* std = (StdMat*)mtl;

	if(std->GetTwoSided()) {
		param = xnode.append_child();
		param.set_name("twosided");
		param.append_attribute("value") = 1;
	}
	if(std->GetWire()) {
		param = xnode.append_child();
		param.set_name("wireframe");
		param.append_attribute("value") = 1;
	}




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



	if(texmap != NULL) {

		BitmapTex *btex = ((BitmapTex *)texmap);
		TSTR mapName = btex->GetMapName();
		StdUVGen* uvGen = btex->GetUVGen();
		param = xnode.append_child();
		param.set_name("texture");
		param.append_attribute("tile_u") = uvGen->GetUScl(0);
		param.append_attribute("tile_v") = uvGen->GetVScl(0);
		param.append_attribute("u_offset") = uvGen->GetUOffs(0);
		param.append_attribute("v_offset") = uvGen->GetVOffs(0);

#ifndef _UNICODE
		param.append_attribute("path") = mapName.data();
#else
		param.append_attribute("path") = mapName.ToCStr();
#endif
	}

	for(int i=0;i<mtl->NumSubMtls();i++) {
		Mtl *mtl2 = mtl->GetSubMtl(i);
		if(mtl2) {
			ExportMaterial(mtl2);
		}
	}

	
}

void	CHCScnExp::ExportCollision(INode *node, pugi::xml_node *xmlnode) {
	if(xmlnode == NULL) {
		xmlnode = &m_collision_xml;
	}

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
	
	pugi::xml_node main_node = xmlnode->append_child();
	
	main_node.set_name("mesh");

#ifdef _UNICODE
	char fname[FILENAME_MAX+1];
	wcstombs(fname,node->GetName(),sizeof(fname));
	main_node.append_attribute("name") = fname;
#else
	main_node.append_attribute("name") = node->GetName();
#endif

	main_node.append_attribute("type") = "bbox";
	
	Box3 bbox = mesh->getBoundingBox();
	pugi::xml_node xnode = main_node.append_child();
	xnode.set_name("bounds");

	xnode.append_attribute("minx") = bbox.Min().x;
	xnode.append_attribute("miny") = bbox.Min().y;
	xnode.append_attribute("minz") = bbox.Min().z;

	xnode.append_attribute("maxx") = bbox.Max().x;
	xnode.append_attribute("maxy") = bbox.Max().y;
	xnode.append_attribute("maxz") = bbox.Max().z;

}

void CHCScnExp::ExportMesh(INode *node, pugi::xml_node *xmlnode) {
	if(xmlnode == NULL) {
		xmlnode = &m_mesh_xml;
	}
	
	ILayer* layer = (ILayer*)node->GetReference(NODE_LAYER_REF);
	ILayerProperties* layer_props = (ILayerProperties*)layer->GetInterface(LAYERPROPERTIES_INTERFACE);

	if(_wcsicmp(layer_props->getName(), L"collision") == 0) {
		ExportCollision(node);
		return;
	}

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

	uint32_t num_verts = mesh->getNumVerts();
	uint32_t num_indices = mesh->getNumFaces();


	Mtl *mtl = node->GetMtl();

	uint32_t uvcount = 0;
	uint32_t numTVx = mesh->getNumTVerts();
	
	if(numTVx > 0) {
		uvcount = 1;
	}

	pugi::xml_node main_node = xmlnode->append_child();
	
	main_node.set_name("mesh");

#ifdef _UNICODE
	char fname[FILENAME_MAX+1];
	wcstombs(fname,node->GetName(),sizeof(fname));
	main_node.append_attribute("name") = fname;
#else
	main_node.append_attribute("name") = node->GetName();
#endif

	if(mtl) {
#ifdef _UNICODE
		wcstombs(fname,mtl->GetName(),sizeof(fname));
		main_node.append_attribute("material_name") = fname;
#else
		main_node.append_attribute("name") = mtl->GetName();
#endif

	}
	pugi::xml_node xnode = main_node.append_child();
    xnode.set_name("verticies");
	float verts[3];
	for (int i=0; i<num_verts; i++) {
		Point3 v = tm * mesh->verts[i];
		verts[0] = v.x;
		verts[1] = v.y;
		verts[2] = v.z;
		pugi::xml_node param = xnode.append_child();
		param.set_name("point");

		// add attributes to param node
		param.append_attribute("x") = v.x;
		param.append_attribute("y") = v.y;
		param.append_attribute("z") = v.z;
	}
	xnode = main_node.append_child();
    xnode.set_name("normals");
	if(mesh->normalCount > 0) {
		for(int i=0;i<num_verts;i++) {
			Point3 n = mesh->getNormal(i);
			pugi::xml_node param = xnode.append_child();
			param.set_name("point");

			// add attributes to param node
			param.append_attribute("x") = n.x;
			param.append_attribute("y") = n.y;
			param.append_attribute("z") = n.z;
		}
	}

	//Point2 *tv = new Point2[head.num_verts];
	float ident = 1.0f;
	//GetTVerts(mesh,tv);
	xnode = main_node.append_child();
    xnode.set_name("uvws");
	if(uvcount > 0) {
		for(int i=0;i<uvcount;i++) {
			for(int j=0;j<numTVx;j++) {
				Point3 v = mesh->tVerts[j];
				pugi::xml_node param = xnode.append_child();
				param.set_name("point");

				// add attributes to param node
				param.append_attribute("x") = v.x;
				param.append_attribute("y") = v.y;
				param.append_attribute("z") = v.z;
			}
		
		}
	}
	//delete tv;


	xnode = main_node.append_child();
    xnode.set_name("indices");
	uint32_t indices[3];
	for (int i=0; i<num_indices; i++) {
		indices[0] = mesh->faces[i].v[vx1];
		indices[1] = mesh->faces[i].v[vx2];
		indices[2] = mesh->faces[i].v[vx3];
		pugi::xml_node param = xnode.append_child();
		param.set_name("point");

		// add attributes to param node
		param.append_attribute("x") = indices[0];
		param.append_attribute("y") = indices[1];
		param.append_attribute("z") = indices[2];
	}
}
void			CHCScnExp::ExportGeomObject(INode *node, pugi::xml_node *xmlnode) {
	ObjectState os = node->EvalWorldState(0);
	if (!os.obj)
		return;
	
	// Targets are actually geomobjects, but we will export them
	// from the camera and light objects, so we skip them here.
	if (os.obj->ClassID() == Class_ID(TARGET_CLASS_ID, 0))
		return;

	ExportMesh(node, xmlnode);
	
}
void			CHCScnExp::ExportGeomMaterial(INode *node, pugi::xml_node *xmlnode) {
	ObjectState os = node->EvalWorldState(0);
	if (!os.obj)
		return;
	
	// Targets are actually geomobjects, but we will export them
	// from the camera and light objects, so we skip them here.
	if (os.obj->ClassID() == Class_ID(TARGET_CLASS_ID, 0))
		return;
	Mtl *mtl = node->GetMtl();
	if(mtl)
		ExportMaterial(mtl, xmlnode);
}
void			CHCScnExp::ExportLight(INode *node) {
}
void CHCScnExp::ProcessGroups(INode *node) {
		// The ObjectState is a 'thing' that flows down the pipeline containing
		// all information about the object. By calling EvalWorldState() we tell
		// max to eveluate the object at end of the pipeline.
		TimeValue t = 0;
		ObjectState os = node->EvalWorldState(t); 
		if(os.obj) {
			switch(os.obj->SuperClassID()) {
				case HELPER_CLASS_ID: {
					if(node->IsGroupHead()) {
						pugi::xml_node xnode = m_mesh_xml.append_child();
						xnode.set_name("group");
						#ifdef _UNICODE
							char fname[FILENAME_MAX+1];
							wcstombs(fname,node->GetName(),sizeof(fname));
							xnode.append_attribute("name") = fname;
						#else
							xnode.append_attribute("name") = node->GetName();
						#endif
						for(int i=0;i<node->NumChildren();i++) {
							INode *cnode = node->GetChildNode(i);
							ExportGeomObject(cnode, &xnode);
						}
					}
					break;
				}
			}
		}
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
					ExportGeomObject(node, NULL);
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
					ExportGeomMaterial(node, NULL);
					break;
				}
			}
		}
}
void CHCScnExp::ProcessLights(INode *node) {
		TimeValue t = 0;
		ObjectState os = node->EvalWorldState(t); 
		if(os.obj) {
			switch(os.obj->SuperClassID()) {
				case LIGHT_CLASS_ID: {
					ExportLight(node);
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
	strcpy(fname,name);
#endif
	
	int numChildren = node->NumberOfChildren();

	
	for(int i=0;i<numChildren;i++) {
		INode *snode = node->GetChildNode(i);
		ProcessGroups(snode);
	}
	for(int i=0;i<numChildren;i++) {
		INode *snode = node->GetChildNode(i);
		ProcessMesh(snode);
	}
	for(int i=0;i<numChildren;i++) {
		INode *snode = node->GetChildNode(i);
		ProcessMaterial(snode);
	}
	for(int i=0;i<numChildren;i++) {
		INode *snode = node->GetChildNode(i);
		ProcessLights(snode);
	}

	sprintf(out_name,"%s.mesh.xml",fname);
	std::ofstream mesh_xml_out;
	mesh_xml_out.open(out_name);
	m_mesh_xml.save(mesh_xml_out);
	mesh_xml_out.close();

	sprintf(out_name,"%s.col.xml",fname);
	std::ofstream col_xml_out;
	col_xml_out.open(out_name);
	m_collision_xml.save(col_xml_out);
	col_xml_out.close();
	
	sprintf(out_name,"%s.mat.xml",fname);
	mesh_xml_out.open(out_name);
	m_materials_xml.save(mesh_xml_out);
	mesh_xml_out.close();
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
