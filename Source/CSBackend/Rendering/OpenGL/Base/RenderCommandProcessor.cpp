//
//  The MIT License (MIT)
//
//  Copyright (c) 2016 Tag Games Limited
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//

#include <CSBackend/Rendering/OpenGL/Base/RenderCommandProcessor.h>

#include <CSBackend/Rendering/OpenGL/Base/GLError.h>
#include <CSBackend/Rendering/OpenGL/Lighting/GLAmbientLight.h>
#include <CSBackend/Rendering/OpenGL/Lighting/GLDirectionalLight.h>
#include <CSBackend/Rendering/OpenGL/Lighting/GLPointLight.h>
#include <CSBackend/Rendering/OpenGL/Material/GLMaterial.h>
#include <CSBackend/Rendering/OpenGL/Model/GLMesh.h>
#include <CSBackend/Rendering/OpenGL/Model/GLSkinnedAnimation.h>
#include <CSBackend/Rendering/OpenGL/Shader/GLShader.h>
#include <CSBackend/Rendering/OpenGL/Target/GLTargetGroup.h>
#include <CSBackend/Rendering/OpenGL/Texture/GLTexture.h>

#include <ChilliSource/Rendering/Model/IndexFormat.h>
#include <ChilliSource/Rendering/Model/PolygonType.h>
#include <ChilliSource/Rendering/Model/RenderDynamicMesh.h>
#include <ChilliSource/Rendering/RenderCommand/RenderCommandList.h>
#include <ChilliSource/Rendering/RenderCommand/RenderCommandBuffer.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/ApplyAmbientLightRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/ApplyDirectionalLightRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/ApplyCameraRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/ApplyDynamicMeshRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/ApplyMaterialRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/ApplyMeshRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/ApplyPointLightRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/ApplySkinnedAnimationRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/BeginRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/BeginWithTargetGroupRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/EndRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/LoadMaterialGroupRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/LoadMeshRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/LoadShaderRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/LoadTargetGroupRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/LoadTextureRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/RenderInstanceRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/UnloadMaterialGroupRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/UnloadMeshRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/UnloadShaderRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/UnloadTargetGroupRenderCommand.h>
#include <ChilliSource/Rendering/RenderCommand/Commands/UnloadTextureRenderCommand.h>

#ifdef CS_TARGETPLATFORM_IOS
#   import <CSBackend/Platform/iOS/Core/Base/CSAppDelegate.h>
#   import <CSBackend/Platform/iOS/Core/Base/CSGLViewController.h>
#endif

namespace CSBackend
{
    namespace OpenGL
    {
        namespace
        {
            const std::string k_uniformWVPMat = "u_wvpMat";
            const std::string k_uniformWorldMat = "u_worldMat";
            const std::string k_uniformNormalMat = "u_normalMat";
            
            /// Converts from a ChilliSource polygon type to a OpenGL polygon type.
            ///
            /// @param blendMode
            ///     The ChilliSource polygon type.
            ///
            /// @return The OpenGL polygon type.
            ///
            GLenum ToGLPolygonType(ChilliSource::PolygonType polygonType) noexcept
            {
                switch(polygonType)
                {
                    case ChilliSource::PolygonType::k_triangle:
                        return GL_TRIANGLES;
                    case ChilliSource::PolygonType::k_triangleStrip:
                        return GL_TRIANGLE_STRIP;
                    case ChilliSource::PolygonType::k_line:
                        return GL_LINES;
                    default:
                        CS_LOG_FATAL("Invalid polygon type.");
                        return GL_TRIANGLES;
                };
            }
            
            /// Converts from a ChilliSource IndexFormat to an OpenGL type.
            ///
            /// @param indexFormat
            ///     The index format to convert.
            ///
            /// @return The OpenGL type.
            ///
            GLenum ToGLIndexType(ChilliSource::IndexFormat indexFormat) noexcept
            {
                switch (indexFormat)
                {
                    case ChilliSource::IndexFormat::k_short:
                        return GL_UNSIGNED_SHORT;
                    default:
                        CS_LOG_FATAL("Invalid index format.");
                        return GL_UNSIGNED_SHORT;
                }
            }
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::Process(const ChilliSource::RenderCommandBuffer* renderCommandBuffer) noexcept
        {
            if (m_initRequired)
            {
                m_initRequired = false;
                Init();
            }
            
            for (const auto& renderCommandList : renderCommandBuffer->GetQueue())
            {
                for (const auto& renderCommand : renderCommandList->GetOrderedList())
                {
                    switch (renderCommand->GetType())
                    {
                        case ChilliSource::RenderCommand::Type::k_loadShader:
                            LoadShader(static_cast<const ChilliSource::LoadShaderRenderCommand*>(renderCommand));
                            break;
                        case ChilliSource::RenderCommand::Type::k_loadTexture:
                            LoadTexture(static_cast<const ChilliSource::LoadTextureRenderCommand*>(renderCommand));
                            break;
                        case ChilliSource::RenderCommand::Type::k_loadMaterialGroup:
                            // Do nothing in OpenGL 2.0 / ES 2.0
                            break;
                        case ChilliSource::RenderCommand::Type::k_loadMesh:
                            LoadMesh(static_cast<const ChilliSource::LoadMeshRenderCommand*>(renderCommand));
                            break;
                        case ChilliSource::RenderCommand::Type::k_loadTargetGroup:
                            LoadTargetGroup(static_cast<const ChilliSource::LoadTargetGroupRenderCommand*>(renderCommand));
                            break;
                        case ChilliSource::RenderCommand::Type::k_begin:
                            Begin(static_cast<const ChilliSource::BeginRenderCommand*>(renderCommand));
                            break;
                        case ChilliSource::RenderCommand::Type::k_beginWithTargetGroup:
                            BeginWithTargetGroup(static_cast<const ChilliSource::BeginWithTargetGroupRenderCommand*>(renderCommand));
                            break;
                        case ChilliSource::RenderCommand::Type::k_applyCamera:
                            ApplyCamera(static_cast<const ChilliSource::ApplyCameraRenderCommand*>(renderCommand));
                            break;
                        case ChilliSource::RenderCommand::Type::k_applyAmbientLight:
                            ApplyAmbientLight(static_cast<const ChilliSource::ApplyAmbientLightRenderCommand*>(renderCommand));
                            break;
                        case ChilliSource::RenderCommand::Type::k_applyDirectionalLight:
                            ApplyDirectionalLight(static_cast<const ChilliSource::ApplyDirectionalLightRenderCommand*>(renderCommand));
                            break;
                        case ChilliSource::RenderCommand::Type::k_applyPointLight:
                            ApplyPointLight(static_cast<const ChilliSource::ApplyPointLightRenderCommand*>(renderCommand));
                            break;
                        case ChilliSource::RenderCommand::Type::k_applyMaterial:
                            ApplyMaterial(static_cast<const ChilliSource::ApplyMaterialRenderCommand*>(renderCommand));
                            break;
                        case ChilliSource::RenderCommand::Type::k_applyMesh:
                            ApplyMesh(static_cast<const ChilliSource::ApplyMeshRenderCommand*>(renderCommand));
                            break;
                        case ChilliSource::RenderCommand::Type::k_applyDynamicMesh:
                            ApplyDynamicMesh(static_cast<const ChilliSource::ApplyDynamicMeshRenderCommand*>(renderCommand));
                            break;
                        case ChilliSource::RenderCommand::Type::k_applySkinnedAnimation:
                            ApplySkinnedAnimation(static_cast<const ChilliSource::ApplySkinnedAnimationRenderCommand*>(renderCommand));
                            break;
                        case ChilliSource::RenderCommand::Type::k_renderInstance:
                            RenderInstance(static_cast<const ChilliSource::RenderInstanceRenderCommand*>(renderCommand));
                            break;
                        case ChilliSource::RenderCommand::Type::k_end:
                            End();
                            break;
                        case ChilliSource::RenderCommand::Type::k_unloadShader:
                            UnloadShader(static_cast<const ChilliSource::UnloadShaderRenderCommand*>(renderCommand));
                            break;
                        case ChilliSource::RenderCommand::Type::k_unloadTexture:
                            UnloadTexture(static_cast<const ChilliSource::UnloadTextureRenderCommand*>(renderCommand));
                            break;
                        case ChilliSource::RenderCommand::Type::k_unloadMaterialGroup:
                            // Do nothing in OpenGL 2.0 / ES 2.0
                            break;
                        case ChilliSource::RenderCommand::Type::k_unloadMesh:
                            UnloadMesh(static_cast<const ChilliSource::UnloadMeshRenderCommand*>(renderCommand));
                            break;
                        case ChilliSource::RenderCommand::Type::k_unloadTargetGroup:
                            UnloadTargetGroup(static_cast<const ChilliSource::UnloadTargetGroupRenderCommand*>(renderCommand));
                            break;
                        default:
                            CS_LOG_FATAL("Unknown render command.");
                            break;
                    }
                }
            }
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::Init() noexcept
        {
            m_textureUnitManager = GLTextureUnitManagerUPtr(new GLTextureUnitManager());
            m_glDynamicMesh = GLDynamicMeshUPtr(new GLDynamicMesh(ChilliSource::RenderDynamicMesh::k_maxVertexDataSize, ChilliSource::RenderDynamicMesh::k_maxIndexDataSize));
            
            ResetCache();
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::LoadShader(const ChilliSource::LoadShaderRenderCommand* renderCommand) noexcept
        {
            ResetCache();
            
            auto renderShader = renderCommand->GetRenderShader();
            
            //TODO: Should be pooled.
            auto glShader = new GLShader(renderCommand->GetVertexShader(), renderCommand->GetFragmentShader());
            
            renderShader->SetExtraData(glShader);
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::LoadTexture(const ChilliSource::LoadTextureRenderCommand* renderCommand) noexcept
        {
            ResetCache();
            
            auto renderTexture = renderCommand->GetRenderTexture();
            
            //TODO: Should be pooled.
            auto glTexture = new GLTexture(renderCommand->GetTextureData(), renderCommand->GetTextureDataSize(), renderTexture->GetDimensions(), renderTexture->GetImageFormat(),
                                           renderTexture->GetImageCompression(), renderTexture->GetFilterMode(), renderTexture->GetWrapModeS(), renderTexture->GetWrapModeT(),
                                           renderTexture->IsMipmapped());
            
            renderTexture->SetExtraData(glTexture);
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::LoadMesh(const ChilliSource::LoadMeshRenderCommand* renderCommand) noexcept
        {
            ResetCache();
            
            auto renderMesh = renderCommand->GetRenderMesh();
            
            //TODO: Should be pooled.
            auto glMesh = new GLMesh(renderMesh->GetVertexFormat(), renderCommand->GetVertexData(), renderCommand->GetVertexDataSize(), renderCommand->GetIndexData(), renderCommand->GetIndexDataSize());
            
            renderMesh->SetExtraData(glMesh);
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::LoadTargetGroup(const ChilliSource::LoadTargetGroupRenderCommand* renderCommand) noexcept
        {
            ResetCache();
            
            auto renderTargetGroup = renderCommand->GetRenderTargetGroup();
            
            //TODO: Should be pooled.
            auto glTargetGroup = new GLTargetGroup(renderTargetGroup);
            
            renderTargetGroup->SetExtraData(glTargetGroup);
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::Begin(const ChilliSource::BeginRenderCommand* renderCommand) noexcept
        {
            ResetCache();
     
            // iOS doesn't have a default frame buffer bound to 0, instead the view controllers frame buffer must
            // be bound manually. Other platforms can just bind 0.
#ifdef CS_TARGETPLATFORM_IOS
            GLKView* glView = (GLKView*)[CSAppDelegate sharedInstance].viewController.view;
            [glView bindDrawable];
#else
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
            
            glViewport(0, 0, renderCommand->GetResolution().x, renderCommand->GetResolution().y);
            
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDepthMask(GL_TRUE);
            
            glClearColor(renderCommand->GetClearColour().r, renderCommand->GetClearColour().g, renderCommand->GetClearColour().b, renderCommand->GetClearColour().a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            glBlendEquation(GL_FUNC_ADD);
            glDepthFunc(GL_LEQUAL);
            
            CS_ASSERT_NOGLERROR("An OpenGL error occurred while beginning rendering.");
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::BeginWithTargetGroup(const ChilliSource::BeginWithTargetGroupRenderCommand* renderCommand) noexcept
        {
            ResetCache();
            
            m_currentRenderTargetGroup = renderCommand->GetRenderTargetGroup();
            CS_ASSERT(m_currentRenderTargetGroup, "Cannot render with a null render target group.");
            
            auto glTargetGroup = reinterpret_cast<GLTargetGroup*>(m_currentRenderTargetGroup->GetExtraData());
            CS_ASSERT(glTargetGroup, "Cannot render with a render target group which hasn't been loaded.");
            
            glTargetGroup->Bind();
            
            glViewport(0, 0, m_currentRenderTargetGroup->GetResolution().x, m_currentRenderTargetGroup->GetResolution().y);
            
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDepthMask(GL_TRUE);
            
            glClearColor(renderCommand->GetClearColour().r, renderCommand->GetClearColour().g, renderCommand->GetClearColour().b, renderCommand->GetClearColour().a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            glBlendEquation(GL_FUNC_ADD);
            glDepthFunc(GL_LEQUAL);
            
            CS_ASSERT_NOGLERROR("An OpenGL error occurred while beginning rendering with a target group.");
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::ApplyCamera(const ChilliSource::ApplyCameraRenderCommand* renderCommand) noexcept
        {
            m_currentMaterial = nullptr;
            
            m_currentCamera = GLCamera(renderCommand->GetPosition(), renderCommand->GetViewProjectionMatrix());
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::ApplyAmbientLight(const ChilliSource::ApplyAmbientLightRenderCommand* renderCommand) noexcept
        {
            m_currentMaterial = nullptr;
            
            m_currentLight = GLLightUPtr(new GLAmbientLight(renderCommand->GetColour()));
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::ApplyDirectionalLight(const ChilliSource::ApplyDirectionalLightRenderCommand* renderCommand) noexcept
        {
            m_currentMaterial = nullptr;
            
            m_currentLight = GLLightUPtr(new GLDirectionalLight(renderCommand->GetColour(), renderCommand->GetDirection(), renderCommand->GetLightViewProjection(),
                                                                renderCommand->GetShadowTolerance(), renderCommand->GetShadowMapRenderTexture()));
        }

        //------------------------------------------------------------------------------
        void RenderCommandProcessor::ApplyPointLight(const ChilliSource::ApplyPointLightRenderCommand* renderCommand) noexcept
        {
            m_currentMaterial = nullptr;
            
            m_currentLight = GLLightUPtr(new GLPointLight(renderCommand->GetColour(), renderCommand->GetPosition(), renderCommand->GetAttenuation()));
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::ApplyMaterial(const ChilliSource::ApplyMaterialRenderCommand* renderCommand) noexcept
        {
            auto renderMaterial = renderCommand->GetRenderMaterial();
            if (m_currentMaterial != renderMaterial)
            {
                m_currentMaterial = renderMaterial;
                
                auto renderShader = m_currentMaterial->GetRenderShader();
                GLShader* glShader = static_cast<GLShader*>(renderShader->GetExtraData());
                if (m_currentShader != renderShader)
                {
                    m_currentMesh = nullptr;
                    m_currentDynamicMesh = nullptr;
                    m_currentSkinnedAnimation = nullptr;
                    m_currentShader = renderShader;
                    
                    glShader->Bind();
                }
                
                m_textureUnitManager->Bind(m_currentMaterial->GetRenderTextures());
                
                m_currentCamera.Apply(glShader);
                
                GLMaterial::Apply(renderMaterial, glShader);
                
                if (m_currentLight)
                {
                    // The light may bind additional textures, meaning it must be applied after the material is applied.
                    m_currentLight->Apply(glShader, m_textureUnitManager.get());
                }
            }
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::ApplyMesh(const ChilliSource::ApplyMeshRenderCommand* renderCommand) noexcept
        {
            CS_ASSERT(m_currentMaterial, "A material must be applied before applying mesh.");
            CS_ASSERT(m_currentShader, "A shader must be applied before applying mesh.");
            
            auto renderMesh = renderCommand->GetRenderMesh();
            if (m_currentMesh != renderMesh)
            {
                m_currentMesh = renderMesh;
                m_currentDynamicMesh = nullptr;
                m_currentSkinnedAnimation = nullptr;
                
                auto glMesh = reinterpret_cast<GLMesh*>(m_currentMesh->GetExtraData());
                auto glShader = reinterpret_cast<GLShader*>(m_currentShader->GetExtraData());
                glMesh->Bind(glShader);
            }
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::ApplyDynamicMesh(const ChilliSource::ApplyDynamicMeshRenderCommand* renderCommand) noexcept
        {
            CS_ASSERT(m_currentMaterial, "A material must be applied before applying mesh.");
            CS_ASSERT(m_currentShader, "A shader must be applied before applying mesh.");
            
            auto renderDynamicMesh = renderCommand->GetRenderDynamicMesh();
            if (m_currentDynamicMesh != renderDynamicMesh)
            {
                m_currentMesh = nullptr;
                m_currentDynamicMesh = renderDynamicMesh;
                m_currentSkinnedAnimation = nullptr;
                
                auto glShader = reinterpret_cast<GLShader*>(m_currentShader->GetExtraData());
                const auto& vertexFormat = m_currentDynamicMesh->GetVertexFormat();
                auto vertexData = m_currentDynamicMesh->GetVertexData();
                auto vertexDataSize = m_currentDynamicMesh->GetVertexDataSize();
                auto indexData = m_currentDynamicMesh->GetIndexData();
                auto indexDataSize = m_currentDynamicMesh->GetIndexDataSize();
                
                m_glDynamicMesh->Bind(glShader, vertexFormat, vertexData, vertexDataSize, indexData, indexDataSize);
            }
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::ApplySkinnedAnimation(const ChilliSource::ApplySkinnedAnimationRenderCommand* renderCommand) noexcept
        {
            CS_ASSERT(m_currentMaterial, "A material must be applied before applying skinned animation.");
            CS_ASSERT(m_currentShader, "A shader must be applied before applying skinned animation.");
            CS_ASSERT(m_currentMesh || m_currentDynamicMesh, "A mesh must be applied before applying skinned animation.");
            
            if (m_currentSkinnedAnimation != renderCommand->GetRenderSkinnedAnimation())
            {
                m_currentSkinnedAnimation = renderCommand->GetRenderSkinnedAnimation();
                
                auto glShader = reinterpret_cast<GLShader*>(m_currentShader->GetExtraData());
                
                GLSkinnedAnimation::Apply(m_currentSkinnedAnimation, glShader);
            }
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::RenderInstance(const ChilliSource::RenderInstanceRenderCommand* renderCommand) noexcept
        {
            CS_ASSERT(m_currentMaterial, "A material must be applied before rendering a mesh.");
            CS_ASSERT(m_currentShader, "A shader must be applied before rendering a mesh.");
            CS_ASSERT(m_currentMesh || m_currentDynamicMesh, "A mesh must be applied before rendering.");
            CS_ASSERT(!m_currentMesh != !m_currentDynamicMesh, "Both mesh types are currently bound, this shouldn't be possible.");
            
            auto glShader = static_cast<GLShader*>(m_currentShader->GetExtraData());
            glShader->SetUniform(k_uniformWorldMat, renderCommand->GetWorldMatrix(), GLShader::FailurePolicy::k_silent);
            glShader->SetUniform(k_uniformWVPMat, renderCommand->GetWorldMatrix() * m_currentCamera.GetViewProjectionMatrix(), GLShader::FailurePolicy::k_silent);
            glShader->SetUniform(k_uniformNormalMat, ChilliSource::Matrix4::Transpose(ChilliSource::Matrix4::Inverse(renderCommand->GetWorldMatrix())), GLShader::FailurePolicy::k_silent);
            
            if (m_currentMesh)
            {
                if (m_currentMesh->GetNumIndices() > 0)
                {
                    glDrawElements(ToGLPolygonType(m_currentMesh->GetPolygonType()), m_currentMesh->GetNumIndices(), ToGLIndexType(m_currentMesh->GetIndexFormat()), 0);
                }
                else
                {
                    glDrawArrays(ToGLPolygonType(m_currentMesh->GetPolygonType()), 0, m_currentMesh->GetNumVertices());
                }
            }
            else
            {
                if (m_currentDynamicMesh->GetNumIndices() > 0)
                {
                    glDrawElements(ToGLPolygonType(m_currentDynamicMesh->GetPolygonType()), m_currentDynamicMesh->GetNumIndices(), ToGLIndexType(m_currentDynamicMesh->GetIndexFormat()), 0);
                }
                else
                {
                    glDrawArrays(ToGLPolygonType(m_currentDynamicMesh->GetPolygonType()), 0, m_currentDynamicMesh->GetNumVertices());
                }
            }
            
            CS_ASSERT_NOGLERROR("An OpenGL error occurred while rendering an instance.");
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::End() noexcept
        {
            if (m_currentRenderTargetGroup)
            {
                //TODO: Update texture data for bound target group, when render to texture is implemented.
            }
            
            ResetCache();
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::UnloadShader(const ChilliSource::UnloadShaderRenderCommand* renderCommand) noexcept
        {
            ResetCache();
            
            auto renderShader = renderCommand->GetRenderShader();
            auto glShader = reinterpret_cast<GLShader*>(renderShader->GetExtraData());
            
            CS_SAFEDELETE(glShader);
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::UnloadTexture(const ChilliSource::UnloadTextureRenderCommand* renderCommand) noexcept
        {
            ResetCache();
            
            auto renderTexture = renderCommand->GetRenderTexture();
            auto glTexture = reinterpret_cast<GLTexture*>(renderTexture->GetExtraData());
            
            CS_SAFEDELETE(glTexture);
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::UnloadMesh(const ChilliSource::UnloadMeshRenderCommand* renderCommand) noexcept
        {
            ResetCache();
            
            auto renderMesh = renderCommand->GetRenderMesh();
            auto glMesh = reinterpret_cast<GLMesh*>(renderMesh->GetExtraData());
            
            CS_SAFEDELETE(glMesh);
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::UnloadTargetGroup(const ChilliSource::UnloadTargetGroupRenderCommand* renderCommand) noexcept
        {
            ResetCache();
            
            auto renderTargetGroup = renderCommand->GetRenderTargetGroup();
            auto glTargetGroup = reinterpret_cast<GLTargetGroup*>(renderTargetGroup->GetExtraData());
            
            CS_SAFEDELETE(glTargetGroup);
        }
        
        //------------------------------------------------------------------------------
        void RenderCommandProcessor::ResetCache() noexcept
        {
            m_textureUnitManager->Reset();
            m_currentCamera = GLCamera();
            m_currentLight.reset();
            m_currentRenderTargetGroup = nullptr;
            m_currentMesh = nullptr;
            m_currentDynamicMesh = nullptr;
            m_currentShader = nullptr;
            m_currentMaterial = nullptr;
            m_currentSkinnedAnimation = nullptr;
        }
    }
}