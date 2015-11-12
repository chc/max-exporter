#include <Windows.h>
#include <Max.h>
#include <list>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <pugixml.hpp>

class CHCScnExp : public SceneExport {
	friend INT_PTR CALLBACK ExportOptionsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

public:
	static	int		layersFrom;					// Derive layers from...
					CHCScnExp();
					~CHCScnExp();
	int				ExtCount();					// Number of extensions supported
	const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
	const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
	const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
	const TCHAR *	AuthorName();				// ASCII Author name
	const TCHAR *	CopyrightMessage();			// ASCII Copyright message
	const TCHAR *	OtherMessage1();			// Other message #1
	const TCHAR *	OtherMessage2();			// Other message #2
	unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
	void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box
	int				DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);	// Export file
	BOOL			SupportsOptions(int ext, DWORD options);

	void			ProcessMesh(INode *node);
	void			ProcessMaterial(INode *node);
	void			ProcessGroups(INode *node);
	void			ExportGeomObject(INode *node, pugi::xml_node *xmlnode = NULL);
	void			ExportGeomMaterial(INode *node, pugi::xml_node *xmlnode = NULL);					
	void			ExportMesh(INode *node, pugi::xml_node *xmlnode = NULL);
	void			ExportMaterial(Mtl *mtl, pugi::xml_node *xmlnode = NULL);
	void			ExportLight(INode *node);
	void			ExportCollision(INode *node, pugi::xml_node *xmlnode = NULL);
	BOOL			TMNegParity(Matrix3 &m);
	TriObject*		GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt);
	short			GetTVerts(Mesh* mesh, Point2 *tv);
	Point3			GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv);
	void			ProcessLights(INode *node);
private:

	std::list<uint32_t> importedTextures;


	pugi::xml_document m_mesh_xml;
	pugi::xml_document m_materials_xml;
	pugi::xml_document m_textures_xml;
	pugi::xml_document m_collision_xml;
};