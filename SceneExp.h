#include <Windows.h>
#include <Max.h>
#include <list>
#include <stdint.h>
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
	void			ExportGeomObject(INode *node);
	void			ExportGeomMaterial(INode *node);					
	void			ExportMesh(INode *node);
	void			ExportMaterial(Mtl *mtl);
	BOOL			TMNegParity(Matrix3 &m);
	TriObject*		GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt);
	uint32_t		getTextureChecksum(const char *path);
	void			AddTextureToTexTbl(Texmap *texmap, uint32_t checksum);
	uint32_t		GetChecksum(TSTR str);
	short			GetTVerts(Mesh* mesh, Point2 *tv);
	Point3			GetVertexNormal(Mesh* mesh, int faceNo, RVertex* rv);
	void			ProcessLights(INode *node);
private:
	FILE *fd;
	FILE *texfd;
	std::list<uint32_t> importedTextures;
	int m_tex_count;
	int m_mtl_count;
};