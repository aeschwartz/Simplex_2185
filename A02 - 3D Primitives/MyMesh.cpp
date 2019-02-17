#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}

void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();


	float angle = (PI * 2) / a_nSubdivisions;
	vector3 centerCircle = vector3(0.0f, -a_fHeight/2.0f, 0.0f);
	vector3 coneApex = vector3(0.0f, a_fHeight/2.0f, 0.0f);
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		vector3 bottomLeft = vector3(a_fRadius * cos(angle * i), -a_fHeight / 2.0f, a_fRadius * sin(angle * i));
		vector3 bottomRight = vector3(a_fRadius * cos(angle * (i + 1)), -a_fHeight / 2.0f, a_fRadius * sin(angle * (i + 1)));
		AddTri(bottomLeft, bottomRight, centerCircle); // creates the base
		AddTri(coneApex, bottomRight, bottomLeft); // creates a non-base face
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();


	float angle = (PI * 2) / a_nSubdivisions;
	vector3 centerBottom = vector3(0.0f, -a_fHeight / 2.0f, 0.0f);
	vector3 centerTop = vector3(0.0f, a_fHeight / 2.0f, 0.0f);
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		vector3 bottomLeft = vector3(a_fRadius * cos(angle * i), -a_fHeight / 2.0f, a_fRadius * sin(angle * i));
		vector3 bottomRight = vector3(a_fRadius * cos(angle * (i + 1)), -a_fHeight / 2.0f, a_fRadius * sin(angle * (i + 1)));
		vector3 topLeft = bottomLeft + vector3(0, a_fHeight, 0);
		vector3 topRight = bottomRight + vector3(0, a_fHeight, 0);
		AddTri(bottomLeft, bottomRight, centerBottom); // creates bottom base
		AddTri(centerTop, topRight, topLeft); // creates top base
		AddQuad(topLeft, topRight, bottomLeft, bottomRight); // creates non-base face (side)
	}


	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	float angle = (PI * 2) / a_nSubdivisions;
	vector3 centerBottom = vector3(0.0f, -a_fHeight / 2.0f, 0.0f);
	vector3 centerTop = vector3(0.0f, a_fHeight / 2.0f, 0.0f);
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		vector3 bottomLeftOuter = vector3(a_fOuterRadius * cos(angle * i), -a_fHeight / 2.0f, a_fOuterRadius * sin(angle * i));
		vector3 bottomRightOuter = vector3(a_fOuterRadius * cos(angle * (i + 1)), -a_fHeight / 2.0f, a_fOuterRadius * sin(angle * (i + 1)));

		vector3 bottomLeftInner = vector3(a_fInnerRadius * cos(angle * i), -a_fHeight / 2.0f, a_fInnerRadius * sin(angle * i));
		vector3 bottomRightInner = vector3(a_fInnerRadius * cos(angle * (i + 1)), -a_fHeight / 2.0f, a_fInnerRadius * sin(angle * (i + 1)));

		vector3 topLeftOuter = bottomLeftOuter + vector3(0, a_fHeight, 0);
		vector3 topRightOuter = bottomRightOuter + vector3(0, a_fHeight, 0);

		vector3 topLeftInner = bottomLeftInner + vector3(0, a_fHeight, 0);
		vector3 topRightInner = bottomRightInner + vector3(0, a_fHeight, 0);

		AddQuad(bottomLeftOuter, bottomRightOuter, bottomLeftInner, bottomRightInner); // creates bottom face of tube
		AddQuad(topLeftInner, topRightInner, topLeftOuter, topRightOuter); // creates top face of tube
		AddQuad(topLeftOuter, topRightOuter, bottomLeftOuter, bottomRightOuter); // outside "wall" face of the tube
		AddQuad(bottomLeftInner, bottomRightInner, topLeftInner, topRightInner); // inside "wall" face of the tube
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsHeight, int a_nSubdivisionsCircumference, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsHeight < 3)
		a_nSubdivisionsHeight = 3;
	if (a_nSubdivisionsHeight > 360)
		a_nSubdivisionsHeight = 360;

	if (a_nSubdivisionsCircumference < 3)
		a_nSubdivisionsCircumference = 3;
	if (a_nSubdivisionsCircumference > 360)
		a_nSubdivisionsCircumference = 360;

	Release();
	Init();

	float fTubeRadius = (a_fOuterRadius - a_fInnerRadius) / 2.0f; // gets radius of the "tube" part of the torus

	std::vector<std::vector<vector3>> torusPoints; // stores all the points necessary for creating the torus

	// each of these calculates angle per iteration, based on the amount of subdivisions, in order to go in a full circle
	float angleA = (PI * 2) / a_nSubdivisionsHeight;
	float angleB = (PI * 2) / a_nSubdivisionsCircumference;

	// loops through and creates all the points that are part of the torus
	for (int b = 0; b < a_nSubdivisionsCircumference; b++) // loops around the circumference of the torus
	{
		torusPoints.push_back(std::vector<vector3>()); // adds a set of points (a circle) per iteration
		for (int a = 0; a < a_nSubdivisionsHeight; a++)
		{
			// creates point on vertical circle
			torusPoints[b].push_back(
				vector3(
					a_fInnerRadius + fTubeRadius + fTubeRadius * cos(angleA * a),
					fTubeRadius * sin(angleA * a),
					0
				)
			);
			// rotates it to where it is on the torus
			torusPoints[b][a] = vector3(glm::rotate(angleB * b, vector3(0, 1.0f, 0)) * vector4(torusPoints[b][a], 0));
		}
	}

	// loop creates quads between points on one vertical circle to points on the next vertical circle in the vector of points
	for (int i = 0; i < a_nSubdivisionsCircumference; i++)
		for (int j = 0; j < a_nSubdivisionsHeight; j++)
			AddQuad(
				torusPoints[i][j],
				torusPoints[(i + 1) % a_nSubdivisionsCircumference][j],
				torusPoints[i][(j + 1) % a_nSubdivisionsHeight],
				torusPoints[(i + 1) % a_nSubdivisionsCircumference][(j + 1) % a_nSubdivisionsHeight]
			);


	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 6)
		a_nSubdivisions = 6;

	Release();
	Init();


	float fValue = a_fRadius;
	float fLength = a_fRadius * 2;
	//3--2
	//|  |
	//0--1

	int nSubPoints = a_nSubdivisions - 1;

	// I based this method on the GenerateCube method in the code above, because it involves making a cube.

	vector3 point0(-fValue, -fValue, fValue); //0
	vector3 point1(fValue, -fValue, fValue); //1
	vector3 point2(fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue, -fValue, -fValue); //4
	vector3 point5(fValue, -fValue, -fValue); //5
	vector3 point6(fValue, fValue, -fValue); //6
	vector3 point7(-fValue, fValue, -fValue); //7

	// the following code creates a cube that is subdivided by a_nSubdivisions on each axis
	// it essentially creates a_nSubdivisions^2 faces per axis

	//F
	for (int i = 0; i <= nSubPoints; i++) // x
		for (int j = 0; j <= nSubPoints; j++) // y
			AddQuad(
				point0 + vector3(i * fLength / a_nSubdivisions, j * fLength / a_nSubdivisions, 0),
				point1 + vector3(-(nSubPoints - i) * fLength / a_nSubdivisions, j * fLength / a_nSubdivisions, 0),
				point3 + vector3(i * fLength / a_nSubdivisions, -(nSubPoints - j) * fLength / a_nSubdivisions, 0),
				point2 + vector3(-(nSubPoints - i) * fLength / a_nSubdivisions, -(nSubPoints - j) * fLength / a_nSubdivisions, 0)
			);

	//B
	for (int i = 0; i <= nSubPoints; i++) // x
		for (int j = 0; j <= nSubPoints; j++) // y
			AddQuad(
				point5 + vector3(-(nSubPoints - i) * fLength / a_nSubdivisions, j * fLength / a_nSubdivisions, 0),
				point4 + vector3(i * fLength / a_nSubdivisions, j * fLength / a_nSubdivisions, 0),
				point6 + vector3(-(nSubPoints - i) * fLength / a_nSubdivisions, -(nSubPoints - j) * fLength / a_nSubdivisions, 0),
				point7 + vector3(i * fLength / a_nSubdivisions, -(nSubPoints - j) * fLength / a_nSubdivisions, 0)
			);

	//L
	for (int i = 0; i <= nSubPoints; i++) // z
		for (int j = 0; j <= nSubPoints; j++) // y
			AddQuad(
				point4 + vector3(0, j * fLength / a_nSubdivisions, i * fLength / a_nSubdivisions),
				point0 + vector3(0, j * fLength / a_nSubdivisions, -(nSubPoints - i) * fLength / a_nSubdivisions),
				point7 + vector3(0, -(nSubPoints - j) * fLength / a_nSubdivisions, i * fLength / a_nSubdivisions),
				point3 + vector3(0, -(nSubPoints - j) * fLength / a_nSubdivisions, -(nSubPoints - i) * fLength / a_nSubdivisions)
			);

	//R
	for (int i = 0; i <= nSubPoints; i++) // z
		for (int j = 0; j <= nSubPoints; j++) // y
			AddQuad(
				point1 + vector3(0, j * fLength / a_nSubdivisions, -(nSubPoints - i) * fLength / a_nSubdivisions),
				point5 + vector3(0, j * fLength / a_nSubdivisions, i * fLength / a_nSubdivisions),
				point2 + vector3(0, -(nSubPoints - j) * fLength / a_nSubdivisions, -(nSubPoints - i) * fLength / a_nSubdivisions),
				point6 + vector3(0, -(nSubPoints - j) * fLength / a_nSubdivisions, i * fLength / a_nSubdivisions)
			);

	//U
	for (int i = 0; i <= nSubPoints; i++) // x
		for (int j = 0; j <= nSubPoints; j++) // z
			AddQuad(
				point3 + vector3(i * fLength / a_nSubdivisions, 0, -(nSubPoints - j) * fLength / a_nSubdivisions),
				point2 + vector3(-(nSubPoints - i) * fLength / a_nSubdivisions, 0, -(nSubPoints - j) * fLength / a_nSubdivisions),
				point7 + vector3(i * fLength / a_nSubdivisions, 0, j * fLength / a_nSubdivisions),
				point6 + vector3(-(nSubPoints - i) * fLength / a_nSubdivisions, 0, j * fLength / a_nSubdivisions)
			);


	//D
	for (int i = 0; i <= nSubPoints; i++) // x
		for (int j = 0; j <= nSubPoints; j++) // z
			AddQuad(
				point4 + vector3(i * fLength / a_nSubdivisions, 0, j * fLength / a_nSubdivisions),
				point5 + vector3(-(nSubPoints - i) * fLength / a_nSubdivisions, 0, j * fLength / a_nSubdivisions),
				point0 + vector3(i * fLength / a_nSubdivisions, 0, -(nSubPoints - j) * fLength / a_nSubdivisions),
				point1 + vector3(-(nSubPoints - i) * fLength / a_nSubdivisions, 0, -(nSubPoints - j) * fLength / a_nSubdivisions)
			);

	
	// the next for loop takes every point that has been added to m_lVertexPos from the AddQuad method calls, normalizes it,
	// then multiplies it by the radius. the more subdivisions, the more the resulting "cube" looks like a circle
	// if the following for loop is removed, this method will produce a cube with subdivisions.
	// this isn't necessarily the most efficient way, but it works.

	for (int i = 0; i < m_lVertexPos.size(); i++)
		m_lVertexPos[i] = a_fRadius * glm::normalize(m_lVertexPos[i]);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}