/*
 *  MeshBatch.h
 *  moFloTest
 *
 *  Created by Scott Downie on 10/12/2010.
 *  Copyright 2010 Tag Games. All rights reserved.
 *
 */

#ifndef _MO_FLO_RENDERING_MESH_BATCH_H_
#define _MO_FLO_RENDERING_MESH_BATCH_H_

#include <ChilliSource/ChilliSource.h>
#include <ChilliSource/Rendering/Base/VertexLayouts.h>

namespace ChilliSource
{
	namespace Rendering
	{
		//--------------------------------------------------------------
		/// Vertex format for mesh batches. Mesh batches must be in this
		/// format.
		//--------------------------------------------------------------
		struct MeshBatchVertex
		{
			MeshBatchVertex(){}
			MeshBatchVertex(const Core::CVector4 &invPos, const Core::CVector3 &invNorm, const Core::CVector2 &invTexCoord)
			: Pos(invPos), Norm(invNorm), Tex(invTexCoord)
            {
				
            }
			Core::CVector4 Pos;
			Core::CVector3 Norm;
			Core::CVector2 Tex;
		};
		
		
		class MeshBatch
		{
		public:
			MeshBatch();
			~MeshBatch();
			//------------------------------------------------------
			/// Add Mesh
			///
			/// Insert a mesh into this batch.
			///
			/// @param Static mesh component
			/// @param Transform
			//------------------------------------------------------
			void AddMesh(const StaticMeshComponentSPtr &inpSprite, const Core::CMatrix4x4& inmatTransform);
			//------------------------------------------------------
			/// Set Material
			///
			/// @param Material
			//------------------------------------------------------
			void SetMaterial(MaterialSPtr inpMaterial);
			//------------------------------------------------------
			/// Get Material
			///
			/// @return Material
			//------------------------------------------------------
			const MaterialSPtr& GetMaterial() const;
			//------------------------------------------------------
			/// Render
			///
			/// Draw the contents of the mesh buffer
			///
			/// @param Active render system
			//------------------------------------------------------
			void Render(RenderSystem* inpRenderSystem) const;
			//------------------------------------------------------
			/// Get Tag
			///
			/// Associated data usually used to determine layer
			//------------------------------------------------------
			s32 GetTag() const;
			//------------------------------------------------------
			/// Set Tag
			///
			/// Associated data usually used to determine layer
			//------------------------------------------------------
			void SetTag(s32 indwValue);
			//------------------------------------------------------
			/// Create Static Buffer
			///
			/// Get the render system to build a static mesh
			/// buffer.
			///
			/// @param Render system
			//------------------------------------------------------
			void CreateStaticBuffer(RenderSystem* inpRenderSystem);
			//------------------------------------------------------
			/// Build
			///
			/// Finalise the batch. Nothing can be changed once
			/// the batch is builtı
			//------------------------------------------------------
			void Build();

		private:
			
			s32 mdwTag;
			
			u32 mudwVertexCount;
			u32 mudwIndexCount;
			
			//The shared material
			MaterialSPtr mpMaterial;
			
			//---Render Buffer
			MeshBuffer* mpMeshBuffer;
			
			//List of sprites in the batch
			typedef std::unordered_map<StaticMeshComponentSPtr, Core::CMatrix4x4> MapMeshToTransform;
			MapMeshToTransform mmapMeshCache;
		};
	}
}

#endif