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

#ifndef _CHILLISOURCE_RENDERING_LIGHTING_RENDERDIRECTIONALLIGHT_H_
#define _CHILLISOURCE_RENDERING_LIGHTING_RENDERDIRECTIONALLIGHT_H_

#include <ChilliSource/ChilliSource.h>
#include <ChilliSource/Core/Base/Colour.h>
#include <ChilliSource/Core/Math/Vector3.h>

namespace ChilliSource
{
    /// A standard-layout container for data the renderer needs which pertains to a single
    /// direction light, such as the colour and direction.
    ///
    class RenderDirectionalLight final
    {
    public:
        
        /// Creates a new instance of the container with default black colour and v-down direction
        /// with no shadow map.
        ///
        RenderDirectionalLight() noexcept;
        
        /// Creates a new instance of the container with the given light colour and direction
        /// with no shadow map.
        ///
        /// @param colour
        ///     The colour of the light.
        /// @param direction
        ///     The direction of the light.
        ///
        RenderDirectionalLight(const Colour& colour, const Vector3& direction) noexcept;
        
        /// Creates a new instance of the container with the given light colour and direction
        /// and shadow map data.
        ///
        /// @param colour
        ///     The colour of the light.
        /// @param direction
        ///     The direction of the light.
        /// @param lightMatrix
        ///     The view projection matrix of the light which is used as the camera when rendering
        ///     the shadow map.
        /// @param shadowTolerance
        ///     The tolerence used to judge if an object is in shadow.
        /// @param shadowMapRenderTexture
        ///     The render texture which should be used for the shadow map.
        ///
        RenderDirectionalLight(const Colour& colour, const Vector3& direction, const Matrix4& lightMatrix, f32 shadowTolerance/*, const RenderTexture* shadowMapRenderTexture*/) noexcept;
        
        /// @return The colour of the light.
        ///
        const Colour& GetColour() const noexcept { return m_colour; }
        
        /// @return The direction of the light.
        ///
        const Vector3& GetDirection() const noexcept { return m_direction; }
        
        /// @return The view projection matrix of the light which is used as the camera when rendering
        ///     the shadow map.
        ///
        const Matrix4& GetLightMatrix() const noexcept { return m_lightMatrix; }
        
        /// @return The tolerence used to judge if an object is in shadow.
        ///
        f32 GetShadowTolerance() const noexcept { return m_shadowTolerance; }
        
        /// @return The render texture which should be used for the shadow map.
        ///
        //const RenderTexture* GetShadowMapRenderTexture() const noexcept { return m_shadowMapRenderTexture; }
        
    private:
        Colour m_colour;
        Vector3 m_direction;
        Matrix4 m_lightMatrix;
        f32 m_shadowTolerance = 0.0f;
        //const RenderTexture* m_shadowMapRenderTexture;
    };
}

#endif