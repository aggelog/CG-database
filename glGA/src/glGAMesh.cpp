#include <iostream>
#include <cassert>

#include "glGAMesh.h"


Mesh::Mesh()
{

}

Mesh::~Mesh()
{
    clear();
}

void Mesh::clear()
{
    for (unsigned int i=0; i<m_Textures.size(); ++i) {
        SAFE_DELETE(m_Textures[i]);
    }
    
}//end clear()

bool Mesh::loadMesh(const std::string& filename )
{
    //release a previously loaded mesh (if it exists)
    numIndices = 0;
	numVertices = 0;
    
    
    bool Ret = false;
    //create an instance of the importer class
    Assimp::Importer importer;
    
    Assimp::DefaultLogger::create( ASSIMP_DEFAULT_LOG_NAME,
                                  Assimp::Logger::VERBOSE,
                                  aiDefaultLogStream_DEBUGGER|aiDefaultLogStream_FILE,
                                   NULL );
    
    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll 
    // propably to request more postprocessing than we do in this example:
    // aiProcess_CalcTangentSpace       | 
    // aiProcess_Triangulate            |
    // aiProcess_JoinIdenticalVertices  |
    // aiProcess_SortByPType |
    // aiProcess_FlipUVs
    const aiScene* pScene = importer.ReadFile( filename.c_str(), 
                aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenNormals | aiProcess_FlipUVs |
				aiProcess_CalcTangentSpace);

    if (pScene) {
        Ret = initFromScene(pScene, filename);
    }
    else
    {
        std::cout<<"Error parsing "<< filename.c_str() <<": "<<importer.GetErrorString() <<std::endl;
    }

    //make sure the VAO is not changed from outside
    //glBindVertexArray(0);
    
    return Ret;
}//end loadMesh()

bool Mesh::initFromScene(const aiScene* pScene, const std::string& filename)
{
    m_Entries.resize(pScene->mNumMeshes);
    m_Textures.resize(pScene->mNumMaterials);
    
    // count the num of vertices and indices
    for (unsigned int i=0; i<m_Entries.size(); ++i) 
    {
        m_Entries[i].materialIndex  = pScene->mMeshes[i]->mMaterialIndex;
        m_Entries[i].numIndices     = pScene->mMeshes[i]->mNumFaces *3;
        m_Entries[i].baseVertex     = numVertices;
        m_Entries[i].baseIndex      = numIndices;
        
        numVertices += pScene->mMeshes[i]->mNumVertices;
        numIndices  += m_Entries[i].numIndices;
    }
    
    // Reserve space in the vectors for the vertex attributes and indixes
    Positions.reserve(numVertices);
    Normals.reserve(numVertices);
    TexCoords.reserve(numVertices);
    Indices.reserve(numIndices);
	Tangents.reserve(numVertices);
    
    // initialize the scene meshes one by one
    for (unsigned int i=0; i<m_Entries.size(); ++i) {
        const aiMesh* paiMesh = pScene->mMeshes[i];
		initMesh(paiMesh, Positions, Normals, TexCoords, Indices, Tangents);
    }
    if (!initMaterials(pScene, filename)) {
        return false;
    }
    
    // @@@GPTODO: Colors are missing
    
    return  GLCheckError();
    
}//end initFromScene()

void Mesh::initMesh(const aiMesh* paiMesh,
              std::vector<glm::vec3>& positions,
              std::vector<glm::vec3>& normals,
              std::vector<glm::vec2>& texCoords,
              std::vector<unsigned int>& indices,
			  std::vector<glm::vec3>& tangents
              )
{
    const aiVector3D    zero3D(0.0f,0.0f,0.0f);
    
    // Populate the vertex attribute vectors
    for (unsigned int i=0; i < paiMesh->mNumVertices; ++i) 
    {
        const aiVector3D* pPos      = &(paiMesh->mVertices[i]);
        const aiVector3D* pNormal   = &(paiMesh->mNormals[i]);
        const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &zero3D;
		const aiVector3D* pTangent  = &(paiMesh->mTangents[i]);
        
        positions.push_back(glm::vec3(pPos->x, pPos->y, pPos->z));
        if(pNormal!=NULL)
            normals.push_back(glm::vec3(pNormal->x,pNormal->y,pNormal->z));
        texCoords.push_back(glm::vec2(pTexCoord->x,pTexCoord->y));
		tangents.push_back(glm::vec3(pTangent->x, pTangent->y, pTangent->z));
    }
    
    // Populate the index buffer
    for (unsigned int i=0; i < paiMesh->mNumFaces; ++i)
    {
        const aiFace& face = paiMesh->mFaces[i];
        assert(face.mNumIndices == 3);
        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }
    
}//end initFromScene()

bool Mesh::initMaterials(const aiScene* pScene, const std::string& filename)
{
    // Extract the directory part from the file name
    std::string::size_type  slashIndex = filename.find_last_of("/");
    std::string             dir;
    
    if (slashIndex == std::string::npos) {
        dir = ".";
    }
    else if (slashIndex == 0)
    {   
        dir = "/";
    }
    else
    {
        dir = filename.substr(0, slashIndex);
    }
    
    bool Ret = true;
    
    // initialize the materials
    for (unsigned int i=0; i < pScene->mNumMaterials; ++i) 
    {
        const aiMaterial* pMaterial = pScene->mMaterials[i];
        
        m_Textures[i] = NULL;
        
        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) >0) 
        {
            aiString path;
            
            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS ) 
            {
                std::string fullPath = dir + "/" + path.data;
                m_Textures[i] = new Texture(GL_TEXTURE_2D, fullPath.c_str());
                
            
                if (!m_Textures[i]->loadTexture()) {
                    std::cout<<"Error Loading texture: "<<fullPath.c_str() <<std::endl;
                    delete m_Textures[i];
                    m_Textures[i] = NULL;
                    Ret = false;
                }
                else 
                {
                    std::cout<<"Loaded texture: "<<fullPath.c_str() <<std::endl;
                }
            }//end if pMaterial->getTexture
        }//end if pMaterial->GetTextureCount
    }//end for()
    
    return Ret;
    
}//end initMaterials()

void Mesh::render()
{
    
    for (unsigned int i=0; i < m_Entries.size(); ++i) 
    {
        const unsigned int materialIndex = m_Entries[i].materialIndex;
        
        assert(materialIndex < m_Textures.size());
        
        if (m_Textures[materialIndex]) {
            m_Textures[materialIndex]->bindTexture(GL_TEXTURE0);
        }
        
        // render primitives from array data with a per-element offset
        /*
         glDrawElementsBaseVertex behaves identically to glDrawElements except that the ith element transferred by the corresponding draw call will be taken from element indices[i] + basevertex of each enabled array:
         
         void glDrawElementsBaseVertex(	GLenum  	mode,
         GLsizei  	count,
         GLenum  	type,
         GLvoid * 	indices,
         GLint  	basevertex);
         */
        glDrawElementsBaseVertex(GL_TRIANGLES, 
                                 m_Entries[i].numIndices,
                                 GL_UNSIGNED_INT,
                                 (void*)(sizeof(unsigned int) * m_Entries[i].baseIndex),
                                 m_Entries[i].baseVertex);
    }//end for
    
}//end render()






































