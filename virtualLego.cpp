////////////////////////////////////////////////////////////////////////////////
//
// File: virtualLego.cpp
//
// Original Author: 박창현 Chang-hyeon Park, 
// Modified by Bong-Soo Sohn and Dong-Jun Kim
// 
// Originally programmed for Virtual LEGO. 
// Modified later to program for Virtual Billiard.
//        
////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <vector>

#include "d3dUtility.h"
using namespace std;

#define M_HEIGHT 0.01
#define M_RADIUS 0.21
#define PI 3.14159265

#define cntBall 5
#define cntWall 3

IDirect3DDevice9* Device = NULL;

// Window Size
const int Width  = 1024;
const int Height = 768;
int remainBallCnt = cntBall;
int remainLifeCnt = 4;

const float spherePos[cntBall][2] = {
	{3.3f, -2.0f},
	{3.3f, -1.0f},
	{3.3f, 0},
	{3.3f, 1.0f},
	{3.3f, 2.0f}
};
const D3DXCOLOR sphereColor[cntBall] = {
	d3d::YELLOW,
	d3d::YELLOW,
	d3d::YELLOW,
	d3d::YELLOW,
	d3d::YELLOW
};

// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld;
D3DXMATRIX g_mView;
D3DXMATRIX g_mProj;

// -----------------------------------------------------------------------------
// CSphere class definition
// -----------------------------------------------------------------------------
class CSphere{
private:
	bool isRemovable = true;
	float center_x, center_y, center_z;
    float m_radius;
	float m_velocity_x;
	float m_velocity_z;

	D3DXMATRIX m_mLocal;
	D3DMATERIAL9 m_mtrl;
	ID3DXMesh* m_pSphereMesh;

public:
    CSphere(void){
		D3DXMatrixIdentity(&m_mLocal);
		ZeroMemory(&m_mtrl, sizeof(m_mtrl));
		m_radius = 0;
		m_velocity_x = 0;
		m_velocity_z = 0;
		m_pSphereMesh = NULL;
    }

    ~CSphere(void){}

    bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE){
		if(pDevice == NULL){
			return false;
		}
		
        m_mtrl.Ambient  = color;
        m_mtrl.Diffuse  = color;
		m_mtrl.Emissive = d3d::BLACK;
		m_mtrl.Power = 5.0f;
        m_mtrl.Specular = color;
		
		if(FAILED(D3DXCreateSphere(pDevice, getRadius(), 50, 50, &m_pSphereMesh, NULL))){
			return false;
		}

        return true;
    }
	
    void destroy(void){
        if(m_pSphereMesh != NULL){
            m_pSphereMesh->Release();
            m_pSphereMesh = NULL;
        }
    }

    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld){
		if(pDevice == NULL){
			return;
		}

        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);

		m_pSphereMesh->DrawSubset(0);
    }
	
    bool hasIntersected(CSphere& ball){
		// Insert your code here.
		float curBallX = this->getCenter().x;
		float curBallY = this->getCenter().y;
		float curBallZ = this->getCenter().z;
		float curBallR = this->getRadius();

		float targetBallX = ball.getCenter().x;
		float targetBallY = ball.getCenter().y;
		float targetBallZ = ball.getCenter().z;
		float targetBallR = ball.getRadius();

		float distance = (curBallX - targetBallX) * (curBallX - targetBallX) + (curBallY - targetBallY) * (curBallY - targetBallY) + (curBallZ - targetBallZ) * (curBallZ - targetBallZ);

		if(distance <= curBallR + targetBallR){
			return true;
		}

		return false;
	}
	
	void hitBy(CSphere& ball){
		// Insert your code here.
		if(hasIntersected(ball)){
			float dx = ball.getCenter().x - this->getCenter().x;
			float dz = ball.getCenter().z - this->getCenter().z;

			float dist = sqrt(dx * dx + dz * dz);
			float oldVel = sqrt(ball.getVelocity_X() * ball.getVelocity_X() + ball.getVelocity_Z() * ball.getVelocity_Z());

			ball.setPower(oldVel * dx / dist, oldVel * dz / dist);

			if(this->isRemovable){
				remainBallCnt--;
				this->setCenter(4.0f + (this->getRadius() + 0.5f) * (remainBallCnt - cntBall), this->getCenter().y, -4.0f);
			}
		}
	}

	void ballUpdate(float timeDiff){
		const float TIME_SCALE = 3.3;
	
		D3DXVECTOR3 cord = this->getCenter();

		double vx = abs(this->getVelocity_X());
		double vz = abs(this->getVelocity_Z());
		if(vx > 0.01 || vz > 0.01){
			float tX = cord.x + TIME_SCALE * timeDiff * m_velocity_x;
			float tZ = cord.z + TIME_SCALE * timeDiff *m_velocity_z;

			//correction of position of ball
			// Please uncomment this part because this correction of ball position is necessary when a ball collides with a wall
			/*if(tX >= (4.5 - M_RADIUS))
				tX = 4.5 - M_RADIUS;
			else if(tX <=(-4.5 + M_RADIUS))
				tX = -4.5 + M_RADIUS;
			else if(tZ <= (-3 + M_RADIUS))
				tZ = -3 + M_RADIUS;
			else if(tZ >= (3 - M_RADIUS))
				tZ = 3 - M_RADIUS;*/
			
			this->setCenter(tX, cord.y, tZ);
		}else{
			this->setPower(0,0);
		}

		//this->setPower(this->getVelocity_X() * DECREASE_RATE, this->getVelocity_Z() * DECREASE_RATE);

		this->setPower(getVelocity_X(), getVelocity_Z());
	}

	D3DXVECTOR3 getCenter(void) const {
		D3DXVECTOR3 org(center_x, center_y, center_z);
		return org;
	}

	const D3DXMATRIX& getLocalTransform(void) const {
		return m_mLocal;
	}

	float getRadius(void)  const {
		return (float)(M_RADIUS);
	}

	double getVelocity_X(){
		return this->m_velocity_x;
	}

	double getVelocity_Z(){
		return this->m_velocity_z;
	}

	bool getRemovable() {
		return this->isRemovable;
	}

	void setCenter(float x, float y, float z){
		D3DXMATRIX m;
		center_x = x;
		center_y = y;
		center_z = z;

		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}

	void setLocalTransform(const D3DXMATRIX& mLocal) {
		m_mLocal = mLocal;
	}

	void setPower(double vx, double vz) {
		this->m_velocity_x = vx;
		this->m_velocity_z = vz;
	}

	void setRemovable(bool isRemovable) {
		this->isRemovable = isRemovable;
	}
};

// -----------------------------------------------------------------------------
// CWall class definition
// -----------------------------------------------------------------------------

class CWall{
private:
    float m_x;
	float m_z;
	float m_width;
    float m_depth;
	float m_height;

	D3DXMATRIX m_mLocal;
	D3DMATERIAL9 m_mtrl;
	ID3DXMesh* m_pBoundMesh;

	void setLocalTransform(const D3DXMATRIX& mLocal){
		m_mLocal = mLocal;
	}
	
public:
    CWall(void){
        D3DXMatrixIdentity(&m_mLocal);
        ZeroMemory(&m_mtrl, sizeof(m_mtrl));
        m_width = 0;
        m_depth = 0;
        m_pBoundMesh = NULL;
    }

    ~CWall(void){}

    bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE){
		if(pDevice == NULL){
			return false;
		}
		
        m_mtrl.Ambient = color;
        m_mtrl.Diffuse = color;
		m_mtrl.Emissive = d3d::BLACK;
        m_mtrl.Power = 5.0f;
		m_mtrl.Specular = color;
		
        m_width = iwidth;
        m_depth = idepth;
		
		if(FAILED(D3DXCreateBox(pDevice, iwidth, iheight, idepth, &m_pBoundMesh, NULL))){
			return false;
		}

        return true;
    }

    void destroy(void){
        if(m_pBoundMesh != NULL){
            m_pBoundMesh->Release();
            m_pBoundMesh = NULL;
        }
    }

    void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld){
		if (pDevice == NULL) {
			return;
		}

        pDevice->SetTransform(D3DTS_WORLD, &mWorld);
        pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal);
        pDevice->SetMaterial(&m_mtrl);

 		m_pBoundMesh->DrawSubset(0);
    }
	
	bool hasIntersected(CSphere& ball){
		// Insert your code here.

		float ballX = ball.getCenter().x;
		float ballZ = ball.getCenter().z;
		float ballR = ball.getRadius();

		float wallXmin = this->m_x - (this->getWidth() / 2 + ballR);
		float wallXmax = this->m_x + (this->getWidth() / 2 + ballR);
		float wallZmin = this->m_z - (this->getDepth() / 2 + ballR);
		float wallZmax = this->m_z + (this->getDepth() / 2 + ballR);

		if((wallXmin <= ballX && ballX <= wallXmax) && (wallZmin <= ballZ && ballZ <= wallZmax)){
			return true;
		}

		return false;
	}

	void hitBy(CSphere& ball){
		// Insert your code here.

		if(hasIntersected(ball)){
			float ballX = ball.getCenter().x;
			float ballY = ball.getCenter().y;
			float ballZ = ball.getCenter().z;
			float ballR = ball.getRadius();

			float wallXmin = this->m_x - (this->getWidth() / 2);
			float wallXmax = this->m_x + (this->getWidth() / 2);
			float wallZmin = this->m_z - (this->getDepth() / 2);
			float wallZmax = this->m_z + (this->getDepth() / 2);

			if((wallXmin <= ballX && ballX <= wallXmax) && !(wallZmin <= ballZ && ballZ <= wallZmax)){
				if(wallZmin - ballR <= ballZ && ballZ <= this->m_z){
					ballZ = wallZmin - ballR - 0.01f;
				}else{
					ballZ = wallZmax + ballR + 0.01f;
				}

				ball.setPower(ball.getVelocity_X(), -ball.getVelocity_Z());
			}

			if(!(wallXmin <= ballX && ballX <= wallXmax) && (wallZmin <= ballZ && ballZ <= wallZmax)){
				if (wallXmin - ballR <= ballX && ballX <= this->m_x) {
					ballX = wallXmin - ballR - 0.01f;
				}
				else {
					ballX = wallXmax + ballR + 0.01f;
				}

				ball.setPower(-ball.getVelocity_X(), ball.getVelocity_Z());
			}

			ball.setCenter(ballX, ballY, ballZ);
		}

	}    
	
	void setPosition(float x, float y, float z){
		D3DXMATRIX m;
		this->m_x = x;
		this->m_z = z;

		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}

	float getCenterX(void) const {
		return m_x;
	}

	float getCenterZ(void) const {
		return m_z;
	}

	float getDepth(void) const {
		return m_depth;
	};
	
    float getHeight(void) const{
		return M_HEIGHT;
	}

	float getWidth(void) const {
		return m_width;
	}
};

// -----------------------------------------------------------------------------
// CLight class definition
// -----------------------------------------------------------------------------

class CLight{
private:
	DWORD m_index;
	D3DLIGHT9 m_lit;
	D3DXMATRIX m_mLocal;
	ID3DXMesh* m_pMesh;
	d3d::BoundingSphere m_bound;
public:
    CLight(void){
        static DWORD i = 0;
        m_index = i++;

        D3DXMatrixIdentity(&m_mLocal);
        ::ZeroMemory(&m_lit, sizeof(m_lit));

        m_pMesh = NULL;
        m_bound._center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        m_bound._radius = 0.0f;
    }

    ~CLight(void){}

    bool create(IDirect3DDevice9* pDevice, const D3DLIGHT9& lit, float radius = 0.1f){
		if(pDevice == NULL){
			return false;
		}

		if(FAILED(D3DXCreateSphere(pDevice, radius, 10, 10, &m_pMesh, NULL))){
			return false;
		}
		
        m_bound._center = lit.Position;
        m_bound._radius = radius;

		m_lit.Ambient = lit.Ambient;
		m_lit.Attenuation0 = lit.Attenuation0;
		m_lit.Attenuation1 = lit.Attenuation1;
		m_lit.Attenuation2 = lit.Attenuation2;
		m_lit.Diffuse = lit.Diffuse;
		m_lit.Direction = lit.Direction;
		m_lit.Falloff = lit.Falloff;
		m_lit.Phi = lit.Phi;
		m_lit.Position = lit.Position;
		m_lit.Range = lit.Range;
		m_lit.Specular = lit.Specular;
		m_lit.Theta = lit.Theta;
        m_lit.Type = lit.Type;

        return true;
    }

    void destroy(void){
        if (m_pMesh != NULL) {
            m_pMesh->Release();
            m_pMesh = NULL;
        }
    }

    bool setLight(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld){
		if(pDevice == NULL){
			return false;
		}
		
        D3DXVECTOR3 pos(m_bound._center);
        D3DXVec3TransformCoord(&pos, &pos, &m_mLocal);
        D3DXVec3TransformCoord(&pos, &pos, &mWorld);

        m_lit.Position = pos;
		
        pDevice->SetLight(m_index, &m_lit);
        pDevice->LightEnable(m_index, TRUE);

        return true;
    }

    void draw(IDirect3DDevice9* pDevice){
		if(pDevice == NULL){
			return;
		}

        D3DXMATRIX m;
        D3DXMatrixTranslation(&m, m_lit.Position.x, m_lit.Position.y, m_lit.Position.z);

        pDevice->SetTransform(D3DTS_WORLD, &m);
        pDevice->SetMaterial(&d3d::WHITE_MTRL);

        m_pMesh->DrawSubset(0);
    }

    D3DXVECTOR3 getPosition(void) const{
		return D3DXVECTOR3(m_lit.Position);
	}    
};

// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
CWall g_boardBackground;
CWall	 g_boardWall[cntWall];
CSphere	g_sphere[cntBall];
CSphere g_sphereMoving;
CSphere	g_sphereControl;
CLight	g_light;

bool isGameStart = false;
double g_camera_pos[3] = {0.0, 5.0, -8.0};

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------

// Initialization
bool InitSphere() {
	//Create Balls and Set Position
	for (int i = 0; i < cntBall; i++) {
		if (!g_sphere[i].create(Device, sphereColor[i])) {
			return false;
		}

		g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]);
		g_sphere[i].setPower(0, 0);
	}

	if (!g_sphereMoving.create(Device, d3d::RED)) {
		return false;
	}

	g_sphereMoving.setCenter(-4.0f, (float)M_RADIUS, .0f);
	g_sphereMoving.setPower(0, 0);
	g_sphereMoving.setRemovable(false);

	// Create Blue Ball and Set Position
	if (!g_sphereControl.create(Device, d3d::BLUE)) {
		return false;
	}

	g_sphereControl.setCenter(-4.5f, (float)M_RADIUS, .0f);
	g_sphereControl.setRemovable(false);

	remainBallCnt = cntBall;

	return true;
}

bool Setup(){
	D3DXMatrixIdentity(&g_mProj);
	D3DXMatrixIdentity(&g_mView);
    D3DXMatrixIdentity(&g_mWorld);
		
	// Create Plane and Set Position
	if(!g_boardBackground.create(Device, -1, -1, 9, 0.03f, 6, d3d::GREEN)){
		return false;
	}

    g_boardBackground.setPosition(0.0f, -0.0006f / 5, 0.0f);
	
	// Create Walls and Set Position.
	if(!g_boardWall[0].create(Device, -1, -1, 9, 0.3f, 0.12f, d3d::DARKRED)){
		return false;
	}
	g_boardWall[0].setPosition(0.0f, 0.12f, 3.06f);

	if(!g_boardWall[1].create(Device, -1, -1, 9, 0.3f, 0.12f, d3d::DARKRED)){
		return false;
	}
	g_boardWall[1].setPosition(0.0f, 0.12f, -3.06f);

	if(!g_boardWall[2].create(Device, -1, -1, 0.12f, 0.3f, 6.24f, d3d::DARKRED)){
		return false;
	}
	g_boardWall[2].setPosition(4.56f, 0.12f, 0.0f);

	if(!InitSphere()){
		return false;
	}
	
	// Setup UI Light
    D3DLIGHT9 lit;
    ::ZeroMemory(&lit, sizeof(lit));
	lit.Ambient = d3d::WHITE * 0.9f;
	lit.Attenuation0 = 0.0f;
	lit.Attenuation1 = 0.9f;
	lit.Attenuation2 = 0.0f;
	lit.Diffuse = d3d::WHITE;
	lit.Position = D3DXVECTOR3(0.0f, 3.0f, 0.0f);
	lit.Range = 100.0f;
	lit.Specular = d3d::WHITE * 0.9f;
    lit.Type = D3DLIGHT_POINT;

	if(!g_light.create(Device, lit)){
		return false;
	}

	// Position and Aim Camera.
	D3DXVECTOR3 pos(-7.5f, 10.0f, 0.0f);
	D3DXVECTOR3 target(5.0f, -10.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 5.0f, 0.0f);
	D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
	Device->SetTransform(D3DTS_VIEW, &g_mView);
	
	// Set the Projection Matrix.
	D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI / 4, (float)Width / (float)Height, 1.0f, 100.0f);
	Device->SetTransform(D3DTS_PROJECTION, &g_mProj);
	
    // Set Render States.
    Device->SetRenderState(D3DRS_LIGHTING, TRUE);
    Device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
    Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	
	g_light.setLight(Device, g_mWorld);

	return true;
}

void Restart(void){
	g_sphereMoving.setCenter(-4.0f, (float)M_RADIUS, .0f);
	g_sphereMoving.setPower(0, 0);
	g_sphereMoving.setRemovable(false);
	isGameStart = false;
}

void Reset(void) {
	InitSphere();
}

void Cleanup(void){
    g_boardBackground.destroy();

	for(int i = 0 ; i < cntWall; i++){
		g_boardWall[i].destroy();
	}

    g_light.destroy();
}


// timeDelta represents the time between the current image frame and the last image frame.
// the distance of moving balls should be "velocity * timeDelta"
bool Display(float timeDelta){
	if(Device){
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0);
		Device->BeginScene();
		
		// Update the Position of Each Ball. During update, check whether Each Ball hit by Walls.
		for(int i = 0; i < cntBall; i++){
			g_sphere[i].ballUpdate(timeDelta);
		}

		g_sphereMoving.ballUpdate(timeDelta);
		for (int i = 0; i < cntWall; i++) {
			g_boardWall[i].hitBy(g_sphereMoving);
		}

		for (int i = 0; i < cntBall; i++) {
			g_sphere[i].hitBy(g_sphereMoving);
		}

		g_sphereControl.hitBy(g_sphereMoving);

		if (g_sphereMoving.getCenter().x < -4.5f) {
			string msgContent = "Ball Out!!남은 기회는 ";
			msgContent += to_string(remainLifeCnt);
			msgContent += "회 입니다.";
			MessageBox(NULL, (LPCSTR)msgContent.c_str(), (LPCSTR)"Game Clear!!", MB_OK | MB_ICONQUESTION);

			Restart();
		}

		// Draw Board and Balls
		g_boardBackground.draw(Device, g_mWorld);
		for(int i =0; i < cntWall; i++){
			g_boardWall[i].draw(Device, g_mWorld);
		}
		for(int i = 0; i < cntBall; i++){
			g_sphere[i].draw(Device, g_mWorld);
		}
		g_sphereMoving.draw(Device, g_mWorld);
		g_sphereControl.draw(Device, g_mWorld);
        g_light.draw(Device);
		
		Device->EndScene();
		Device->Present(0, 0, 0, 0);
		Device->SetTexture( 0, NULL );

		if(!remainBallCnt){
			if(MessageBox(NULL, (LPCSTR)"Game Clear!! 게임을 종료하시겠습니까?", (LPCSTR)"Game Clear!!", MB_YESNO | MB_ICONQUESTION) == IDYES){
				exit(0);
			}else{
				Reset();
				isGameStart = false;
			}
		}
	}
	return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	static bool wire = false;
	static bool isReset = true;
    static float oldZ = -1;
    static enum{
		WORLD_MOVE,
		LIGHT_MOVE,
		BLOCK_MOVE
	}move = WORLD_MOVE;
	
	switch(msg){
		case WM_DESTROY:
			::PostQuitMessage(0);
			break;
		case WM_KEYDOWN:
			switch (wParam){
				case VK_ESCAPE:
					::DestroyWindow(hwnd);
					break;
				case VK_RETURN:
					if(Device != NULL){
						wire = !wire;
						Device->SetRenderState(D3DRS_FILLMODE, (wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID));
					}
					break;
				case VK_SPACE:
					if (!isGameStart) {
						isGameStart = true;
						g_sphereMoving.setPower(5.0f, 0.0);
					}
					break;
				case VK_LEFT:
				case 0x41:
					for (int i = 0; i < 10; i++) {
						g_sphereControl.setCenter(g_sphereControl.getCenter().x, g_sphereControl.getCenter().y, g_sphereControl.getCenter().z + 0.01f);
					}
					break;
				case VK_RIGHT:
				case 0x44:
					for (int i = 0; i < 10; i++) {
						g_sphereControl.setCenter(g_sphereControl.getCenter().x, g_sphereControl.getCenter().y, g_sphereControl.getCenter().z - 0.01f);
					}
					break;
			}
			break;
		case WM_MOUSEMOVE:
			if (LOWORD(wParam)) {
				float newZ = LOWORD(lParam);

				if (oldZ < 0) {
					oldZ = newZ;
				}

				float dZ = oldZ - newZ;

				D3DXVECTOR3 curSphereControl = g_sphereControl.getCenter();

				float boardLeft = g_boardWall[1].getCenterZ() + g_boardWall[1].getDepth() / 2 + g_sphereControl.getRadius();
				float boardRight = g_boardWall[0].getCenterZ() - g_boardWall[0].getDepth() / 2 - g_sphereControl.getRadius();
				float tempZ = curSphereControl.z + dZ * (0.01f);
				if (tempZ > boardRight) {
					tempZ = boardRight;
				}

				if (tempZ < boardLeft) {
					tempZ = boardLeft;
				}

				g_sphereControl.setCenter(curSphereControl.x, curSphereControl.y, tempZ);
				oldZ = newZ;

				move = WORLD_MOVE;
			}

			break;
	}
	
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd){
    srand(static_cast<unsigned int>(time(NULL)));
	
	if(!d3d::InitD3D(hinstance, Width, Height, true, D3DDEVTYPE_HAL, &Device)){
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}
	
	if(!Setup()){
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}
	
	d3d::EnterMsgLoop( Display );
	
	Cleanup();
	
	Device->Release();
	
	return 0;
}