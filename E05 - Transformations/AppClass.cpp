#include "AppClass.h"
void Application::InitVariables(void)
{
	// write model matrices
	// row 1
	m_lMatrices.push_back(glm::translate(vector3(-3.0f, 4.0f, 0.0f)));
	m_lMatrices.push_back(glm::translate(vector3(3.0f, 4.0f, 0.0f)));

	// row 2
	m_lMatrices.push_back(glm::translate(vector3(-2.0f, 3.0f, 0.0f)));
	m_lMatrices.push_back(glm::translate(vector3(2.0f, 3.0f, 0.0f)));

	// row 3
	for (float a = -3.0f; a <= 3.0f; a++)
		m_lMatrices.push_back(glm::translate(vector3(a, 2.0f, 0.0f)));

	// row 4
	m_lMatrices.push_back(glm::translate(vector3(-4.0f, 1.0f, 0.0f)));
	m_lMatrices.push_back(glm::translate(vector3(-3.0f, 1.0f, 0.0f)));

	m_lMatrices.push_back(glm::translate(vector3(-1.0f, 1.0f, 0.0f)));
	m_lMatrices.push_back(glm::translate(vector3(0.0f, 1.0f, 0.0f)));
	m_lMatrices.push_back(glm::translate(vector3(1.0f, 1.0f, 0.0f)));

	m_lMatrices.push_back(glm::translate(vector3(3.0f, 1.0f, 0.0f)));
	m_lMatrices.push_back(glm::translate(vector3(4.0f, 1.0f, 0.0f)));

	// row 5
	for (float b = -5.0f; b <= 5.0f; b++)
		m_lMatrices.push_back(glm::translate(vector3(b, 0.0f, 0.0f)));

	// row 6
	m_lMatrices.push_back(glm::translate(vector3(-5.0f, -1.0f, 0.0f)));

	for (float c = -3.0f; c <= 3.0f; c++)
		m_lMatrices.push_back(glm::translate(vector3(c, -1.0f, 0.0f)));

	m_lMatrices.push_back(glm::translate(vector3(5.0f, -1.0f, 0.0f)));

	// row 7
	m_lMatrices.push_back(glm::translate(vector3(-5.0f, -2.0f, 0.0f)));
	m_lMatrices.push_back(glm::translate(vector3(-3.0f, -2.0f, 0.0f)));
	m_lMatrices.push_back(glm::translate(vector3(3.0f, -2.0f, 0.0f)));
	m_lMatrices.push_back(glm::translate(vector3(5.0f, -2.0f, 0.0f)));

	// row 8
	m_lMatrices.push_back(glm::translate(vector3(-2.0f, -3.0f, 0.0f)));
	m_lMatrices.push_back(glm::translate(vector3(-1.0f, -3.0f, 0.0f)));
	m_lMatrices.push_back(glm::translate(vector3(1.0f, -3.0f, 0.0f)));
	m_lMatrices.push_back(glm::translate(vector3(2.0f, -3.0f, 0.0f)));

	// init meshes
	for (int i = 0; i < m_lMatrices.size(); i++)
	{
		m_lMeshes.push_back(new MyMesh());
		m_lMeshes[i]->GenerateCube(1.0f, C_BLACK);
	}
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	matrix4 m4View = m_pCameraMngr->GetViewMatrix();
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix();
	
	//matrix4 m4Scale = glm::scale(IDENTITY_M4, vector3(2.0f,2.0f,2.0f));
	static float value = 0.0f;
	matrix4 m4Translate = glm::translate(vector3(value, 0.0f, 0.0f));
	value += 0.01f;

	//matrix4 m4Model = m4Translate * m4Scale;

	for (int i = 0; i < m_lMatrices.size(); i++)
	{
		m_lMeshes[i]->Render(m4Projection, m4View, m4Translate * m_lMatrices[i]);
	}

	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	for (int i = 0; i < m_lMatrices.size(); i++)
	{
		SafeDelete(m_lMeshes[i]);
	}
	m_lMeshes.clear();
	m_lMatrices.clear();

	//release GUI
	ShutdownGUI();
}