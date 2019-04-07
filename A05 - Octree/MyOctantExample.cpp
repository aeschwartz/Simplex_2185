#include "MyOctantExample.h"
using namespace Simplex;
//MyOctantExample

// static object definitions
uint Simplex::MyOctant::m_uOctantCount = 0;
uint Simplex::MyOctant::m_uMaxLevel = 0;
uint Simplex::MyOctant::m_uIdealEntityCount = 0;

// Allocations
void Simplex::MyOctant::Init(void)
{
	m_uID = 0;
	m_uLevel = 0;
	m_uChildren = 0;

	m_fSize = 0;

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);
}

void Simplex::MyOctant::Release(void)
{
	m_pMeshMngr = nullptr;
	m_pEntityMngr = nullptr;

	m_pParent = nullptr;
	if (m_uChildren > 0)
		for (uint i = 0; i < 8; ++i)
			delete m_pChild[i];

	m_pRoot = nullptr;
	m_lChild.clear();

}

Simplex::MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	Init();
	m_uOctantCount = 0;
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	++m_uOctantCount;
	m_uLevel = 0;

	m_pRoot = this;
	
	// set initial values
	MyRigidBody* pRigidBody = m_pEntityMngr->GetEntity(0)->GetRigidBody();
	m_v3Max = pRigidBody->GetMaxGlobal();
	m_v3Min = pRigidBody->GetMinGlobal();
	m_lEntityList.push_back(0);
	for (uint i = 1; i < m_pEntityMngr->GetEntityCount(); ++i)
	{
		pRigidBody = m_pEntityMngr->GetEntity(i)->GetRigidBody();
		vector3 v3TempMax = pRigidBody->GetMaxGlobal();
		vector3 v3TempMin = pRigidBody->GetMinGlobal();

		// check for new max
		if (m_v3Max.x < v3TempMax.x)
			m_v3Max.x = v3TempMax.x;
		if (m_v3Max.y < v3TempMax.y)
			m_v3Max.y = v3TempMax.y;
		if (m_v3Max.z < v3TempMax.z)
			m_v3Max.z = v3TempMax.z;

		// check for new min
		if (m_v3Min.x > v3TempMin.x)
			m_v3Min.x = v3TempMin.x;
		if (m_v3Min.y > v3TempMin.y)
			m_v3Min.y = v3TempMin.y;
		if (m_v3Min.z > v3TempMin.z)
			m_v3Min.z = v3TempMin.z;

		// also add every index of entitymanager to entitylist since this is the root
		m_lEntityList.push_back(i);
	}
	
	// calculate center
	m_v3Center = 0.5f * (m_v3Max + m_v3Min);

	// ensure octree is a cube
	vector3 v3MaxDiff = m_v3Max - m_v3Min;
	m_fSize = // calculate size = side length, greatest of the 3 AA distances
		((v3MaxDiff.x > v3MaxDiff.y ? v3MaxDiff.x : v3MaxDiff.y) >
			v3MaxDiff.z ? v3MaxDiff.y : v3MaxDiff.z);
	float fHalfSize = m_fSize / 2.0f;
	m_v3Min = vector3(m_v3Center.x - fHalfSize, m_v3Center.y - fHalfSize, m_v3Center.z - fHalfSize);
	m_v3Max = vector3(m_v3Center.x + fHalfSize, m_v3Center.y + fHalfSize, m_v3Center.z + fHalfSize);

	// calculate to see if:
	// a) entities in this octant are more than ideal amount
	// b) max level of subdivisions has not been reached
	// if so, subdivide
	if (m_lEntityList.size() > m_uIdealEntityCount && m_uLevel < m_uMaxLevel)
		Subdivide();
}

Simplex::MyOctant::MyOctant(MyOctant* a_pParent, vector3 a_v3Center, float a_fSize)
{
	Init();
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;
	
	m_pParent = a_pParent;
	m_uLevel = m_pParent->m_uLevel + 1;
	m_pRoot = m_pParent->m_pRoot;

	m_uID = m_uOctantCount;
	++m_uOctantCount;

	// calculate min and max from center and size
	float fHalfSize = a_fSize / 2.0f;
	vector3 v3HalfWidth(fHalfSize);
	m_v3Min = a_v3Center - v3HalfWidth;
	m_v3Max = a_v3Center + v3HalfWidth;

	// calculate which entities are within this octant
	for (uint i = 0; i < m_pParent->m_lEntityList.size(); ++i)
	{
		// create temp values for each entity's rigidbox
		vector3 v3TempMax = m_pEntityMngr->GetEntity(m_pParent->m_lEntityList[i])->GetRigidBody()->GetMaxGlobal();
		vector3 v3TempMin = m_pEntityMngr->GetEntity(m_pParent->m_lEntityList[i])->GetRigidBody()->GetMinGlobal();

		// check if entity is within the octant using AABB
		if (m_v3Max.x > v3TempMin.x &&
			m_v3Min.x < v3TempMax.x &&
			m_v3Max.y > v3TempMin.y &&
			m_v3Min.y < v3TempMax.y &&
			m_v3Max.z > v3TempMin.z &&
			m_v3Min.z < v3TempMax.z)
			m_lEntityList.push_back(m_pParent->m_lEntityList[i]); // if so add it to the list
	}
}

// rule of 3

Simplex::MyOctant::MyOctant(MyOctant const & a_pOther)
{
	m_uID = a_pOther.m_uID;
	m_uLevel = a_pOther.m_uLevel;
	m_uChildren = a_pOther.m_uChildren;

	m_fSize = a_pOther.m_fSize;

	m_pMeshMngr = a_pOther.m_pMeshMngr;
	m_pEntityMngr = a_pOther.m_pEntityMngr;

	m_v3Center = a_pOther.m_v3Center;
	m_v3Min = a_pOther.m_v3Min;
	m_v3Max = a_pOther.m_v3Max;

	m_pParent = a_pOther.m_pParent;
	
	for (uint i = 0; i < 8; ++i)
		m_pChild[i] = a_pOther.m_pChild[i];

	m_lEntityList = a_pOther.m_lEntityList;

	m_pRoot = a_pOther.m_pRoot;
	m_lChild = a_pOther.m_lChild;
}

MyOctant & Simplex::MyOctant::operator=(MyOctant const & a_pOther)
{
	if (this != &a_pOther)
	{
		Release();
		Init();
		MyOctant temp(a_pOther);
		Swap(temp);
	}
	return *this;
}

Simplex::MyOctant::~MyOctant(void) { Release(); }

void Simplex::MyOctant::Swap(MyOctant & other)
{
	std::swap(m_uID, other.m_uID);
	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_uChildren, other.m_uChildren);

	std::swap(m_fSize, other.m_fSize);

	std::swap(m_pMeshMngr, other.m_pMeshMngr);
	std::swap(m_pEntityMngr, other.m_pEntityMngr);

	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);

	std::swap(m_pParent, other.m_pParent);
	std::swap(m_pChild, other.m_pChild);

	std::swap(m_lEntityList, other.m_lEntityList);

	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_lChild, other.m_lChild);
}

float Simplex::MyOctant::GetSize(void) { return m_fSize; }
vector3 Simplex::MyOctant::GetCenterGlobal(void) { return m_v3Center; }
vector3 Simplex::MyOctant::GetMinGlobal(void) { return m_v3Min; }
vector3 Simplex::MyOctant::GetMaxGlobal(void) { return m_v3Max; }

bool Simplex::MyOctant::IsColliding(uint a_uRBIndex)
{
	if (std::find(m_lEntityList.begin(), m_lEntityList.end(), a_uRBIndex) == m_lEntityList.end())
		return false; //if octant does not contain the entity, return false
	else if (m_uChildren != 0) //if octant does contain the entity and has children, find the leaves containing the entity and run collision checks on them
	{
		bool bCollisionOverall = false;
		for (uint i = 0; i < m_lChild.size(); ++i)
			if (std::find(m_lChild[i]->m_lEntityList.begin(), m_lChild[i]->m_lEntityList.end(), a_uRBIndex) != m_lChild[i]->m_lEntityList.end())
				bCollisionOverall = bCollisionOverall || m_lChild[i]->IsColliding(a_uRBIndex);
		
		return bCollisionOverall;
	}
	else // base case, once we get to the leaves
	{
		bool bCollisionLeaf = false;
		MyEntity* pCCEntity = m_pEntityMngr->GetEntity(a_uRBIndex);
		for (uint i = 0; i < m_lEntityList.size(); ++i)
		{
			bCollisionLeaf = bCollisionLeaf ||
				(a_uRBIndex != m_lEntityList[i] && pCCEntity->IsColliding(m_pEntityMngr->GetEntity(m_lEntityList[i])));
		}
		return bCollisionLeaf;
	}
}

void Simplex::MyOctant::Update()
{
	//Clear all collisions
	for (uint i = 0; i < m_pEntityMngr->GetEntityCount(); i++)
		m_pEntityMngr->GetEntity(i)->ClearCollisionList();

	//check collisions through the octree
	for (uint i = 0; i < m_pEntityMngr->GetEntityCount(); i++)
		IsColliding(i);
}

void Simplex::MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	// go back to normal display (display all octants) if index = -1
	if (a_nIndex == -1)
		Display(a_v3Color);
	else //recursive-ish case
	{
		if (a_nIndex == m_uID)
			m_pMeshMngr->AddWireCubeToRenderList(glm::translate(m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color);
		else
			for (uint i = 0; i < m_uChildren; i++)
					m_pChild[i]->Display(a_nIndex);
	}
}

void Simplex::MyOctant::Display(vector3 a_v3Color)
{
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color);
	// if node has children, diplay them too.
	if (!IsLeaf())
		for (uint i = 0; i < 8; ++i)
			m_pChild[i]->Display();
}

void Simplex::MyOctant::Subdivide(void)
{
	// if there are already suboctants do nothing
	if (m_uChildren == 8)
		return;

	// pre-calculate m_fSize of all child nodes
	float fChildSize = m_fSize / 2.0f;

	// calculate all the corners
	vector3 v3Corners[8] = {
		m_v3Min,
		vector3(m_v3Max.x, m_v3Min.y, m_v3Min.z),
		vector3(m_v3Max.x, m_v3Min.y, m_v3Max.z),
		vector3(m_v3Min.x, m_v3Min.y, m_v3Max.z),
		vector3(m_v3Min.x, m_v3Max.y, m_v3Max.z),
		vector3(m_v3Min.x, m_v3Max.y, m_v3Min.z),
		vector3(m_v3Max.x, m_v3Max.y, m_v3Min.z),
		m_v3Max
	};

	// create all the child octants
	for (uint i = 0; i < 8; ++i)
		m_pChild[i] = new MyOctant(this, m_v3Center - (m_v3Center - v3Corners[i]) / 2.0f, fChildSize);

	// set children to 8 after done
	m_uChildren = 8;

	// so that ids are breadth first, loop through again and subdivide
	for (uint i = 0; i < 8; ++i)
	{
		if (m_pChild[i]->m_lEntityList.size() > 0) 
		{
			//if there are entities in the octant, add it to the list of octants with entities
			m_lChild.push_back(m_pChild[i]);

			// calculate to see if:
			// a) entities in this octant are more than ideal amount
			// b) max level of subdivisions has not been reached
			// if so, subdivide
			if (m_pChild[i]->m_lEntityList.size() > m_uIdealEntityCount && m_pChild[i]->m_uLevel < m_uMaxLevel)
				m_pChild[i]->Subdivide();
		}	
	}
}

MyOctant * Simplex::MyOctant::GetChild(uint a_nChild) { return m_pChild[a_nChild]; }

MyOctant * Simplex::MyOctant::GetParent(void)
{
	return m_pParent;
}

bool Simplex::MyOctant::IsLeaf(void) { return m_uChildren == 0; }

uint Simplex::MyOctant::GetOctantCount(void){ return m_uOctantCount; }


