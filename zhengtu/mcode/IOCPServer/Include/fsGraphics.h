#pragma once


/*
** Hardware color macros
*/
#define ARGB(a,r,g,b)	((DWORD(a)<<24) + (DWORD(r)<<16) + (DWORD(g)<<8) + DWORD(b))
#define GETA(col)		((col)>>24)
#define GETR(col)		(((col)>>16) & 0xFF)
#define GETG(col)		(((col)>>8) & 0xFF)
#define GETB(col)		((col) & 0xFF)
#define SETA(col,a)		(((col) & 0x00FFFFFF) + (DWORD(a)<<24))
#define SETR(col,r)		(((col) & 0xFF00FFFF) + (DWORD(r)<<16))
#define SETG(col,g)		(((col) & 0xFFFF00FF) + (DWORD(g)<<8))
#define SETB(col,b)		(((col) & 0xFFFFFF00) + DWORD(b))


/*
** fs Blending constants
*/
#define	BLEND_COLORADD		1
#define	BLEND_COLORMUL		0
#define	BLEND_ALPHABLEND	2
#define	BLEND_ALPHAADD		0
#define BLEND_SRCCOLOR      3
#define	BLEND_ZWRITE		4
#define	BLEND_NOZWRITE		0

#define BLEND_DEFAULT		(BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_NOZWRITE)
#define BLEND_DEFAULT_Z		(BLEND_COLORMUL | BLEND_ALPHABLEND | BLEND_ZWRITE)

/*
typedef enum _D3DBLEND {
D3DBLEND_ZERO               = 1,
D3DBLEND_ONE                = 2,
D3DBLEND_SRCCOLOR           = 3,
D3DBLEND_INVSRCCOLOR        = 4,
D3DBLEND_SRCALPHA           = 5,
D3DBLEND_INVSRCALPHA        = 6,
D3DBLEND_DESTALPHA          = 7,
D3DBLEND_INVDESTALPHA       = 8,
D3DBLEND_DESTCOLOR          = 9,
D3DBLEND_INVDESTCOLOR       = 10,
D3DBLEND_SRCALPHASAT        = 11,
D3DBLEND_BOTHSRCALPHA       = 12,
D3DBLEND_BOTHINVSRCALPHA    = 13,
D3DBLEND_FORCE_DWORD        = 0x7fffffff, 
} D3DBLEND;	
 */

/*
** fs Primitive type constants
*/
#define FS_PRIM_LINES		2
#define FS_PRIM_TRIPLES		3
#define FS_PRIM_QUADS		4


enum GRAPHICS_TYPE
{
	GRAPHICS_DX3D8 = 0,	//DirectX3D8
	GRAPHICS_DX3D9,		//DirectX3D9(暂时未实现)
	GRAPHICS_OPENGL,	//OpenGL(暂时未实现)
	GRAPHICS_SOFTWARE	//软件实现(暂时未实现)
};


class fsGraphics;
class fsTexture;


/*
** fsVertex structure
*/
class FS_API fsVertex
{
public:
	fsVertex();
	~fsVertex();


	//fsVertex& operator  = (const fsVertex& vertex)
	//{
	//	x = vertex.x;
	//	y = vertex.y;
	//	z = vertex.z;
	//	col = vertex.col;
	//	tx = vertex.tx;
	//	ty = vertex.ty;
	//}

	float			x, y;		// screen position    
	float			z;			// Z-buffer depth 0..1
	DWORD			col;		// color
	float			tx, ty;		// texture coordinates
};


/*
** fsTriple structure
*/
class FS_API fsTriple
{
public:
	fsTriple();
	~fsTriple();


	fsVertex		v[3];
	fsTexture*		tex;
	int				blend;
};


/*
** fsQuad structure
*/
class FS_API fsQuad
{
public:
	fsQuad();
	~fsQuad();


	fsVertex		v[4];
	fsTexture*		tex;
	int				blend;
};

/*
** fsViewPort structure
*/
class FS_API fsViewPort
{
	public:
		fsViewPort();
		~fsViewPort();


		int m_iX;
		int m_iY;
		int m_iWidth;
		int m_iHeight;
};


//=================
// Class fsTexture
//=================
class FS_API fsTexture
{
public:
	virtual ~fsTexture(void){};

	//! Load texture from disk.
	virtual BOOL Load(fsGraphics *pGraphics, const char* filename, DWORD dwColorKey=0xFFFFFFFF, BOOL bMipmap=FALSE) = 0;

	//! Load texture from stream.
	virtual BOOL Load(fsGraphics *pGraphics, void* pData, DWORD dataLen, DWORD dwColorKey=0xFFFFFFFF, BOOL bMipmap=FALSE) = 0;

	//! Create empty texture.
	virtual BOOL Create(fsGraphics *pGraphics, int width, int height) = 0;

	//! Get width.
	virtual int GetWidth(BOOL bOriginal = FALSE) const = 0;

	//! Get height.
	virtual int GetHeight(BOOL bOriginal = FALSE) const = 0;

	//! Lock Texture.
	virtual DWORD* Lock(bool bReadOnly=true, int left=0, int top=0, int width=0, int height=0) = 0;
	
	//! Unlock Texture.
	virtual void Unlock() = 0;
	
	//! Release the texture object
	virtual void Release() = 0;

	/*
	//! Display to backbuffer.
	virtual long Blit( float dX, float dY, DWORD aColor=0xFFFFFFFF, 
		float fAngle = 0.0f, float aScalX=1.0f, float aScalY=1.0f,
		float sX=0, float sY=0, 
		int aWidth=-1, int aHeight=-1 ) = 0;

	//! Display to backbuffer.
	virtual long BlitStretch( float dX, float dY, int dwWidth = -1, int dwHeight = -1, 
		DWORD aColor=0xFFFFFFFF, float fAngle = 0.0f ) = 0;

	//! Set rotation Center.
	virtual void SetCenter(float x, float y, float z=0.0f) = 0;
*/
};



//=================
// Class fsGraphics
//=================
class FS_API fsGraphics
{
public:
	virtual ~fsGraphics(void){};

	//! Init the Graphics and create the device with hWnd.
	virtual BOOL		Init(HWND hWnd, int aWidth, int aHeight, BOOL bWindow = TRUE) = 0;


	virtual void		Resize(int width, int height) = 0;

	//! open or close ZBuffer
	virtual void		EnableZBuffer(BOOL bEnable) = 0;
	virtual BOOL		IsEnableZBuffer() = 0;

	//! open or close TextureFilter
	virtual void		EnableTextureFilter(BOOL bEnable) = 0;
	virtual BOOL		IsEnableTextureFilter() = 0;

	//! 截屏
	virtual void		Snapshot(const char* filename = NULL) = 0;

	virtual BOOL		BeginScene() = 0;
	virtual void		EndScene() = 0;
	virtual void		Clear(DWORD color) = 0;
	virtual void		RenderLine(float x1, float y1, float x2, float y2, DWORD color=0xFFFFFFFF, float z=0.5f) = 0;
	virtual void		RenderTriple(const fsTriple *triple) = 0;
	virtual void		RenderQuad(const fsQuad *quad) = 0;
	//virtual fsVertex*	StartBatch(int prim_type, fsTexture* tex, int blend, int *max_prim) = 0;
	//virtual void		FinishBatch(int nprim) = 0;
	virtual void		SetClipping(int x=0, int y=0, int w=0, int h=0) = 0;
	virtual const fsViewPort&	GetClipping() const = 0;
	virtual void		SetTransform(float x=0, float y=0, float dx=0, float dy=0, float rot=0, float hscale=0, float vscale=0) = 0; 
	virtual GRAPHICS_TYPE	GetDriverType() const = 0;	//Get Driver Type
	//! Lock BackBuffer.
	virtual DWORD* LockBackBuffer() = 0;
	//! Unlock BackBuffer.
	virtual void UnlockBackBuffer() = 0;

	//! Release the device object
	virtual void Release() = 0;
};
