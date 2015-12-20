// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include "Renderer.h"

namespace ouzel
{
    class RendererOGL: public Renderer
    {
    public:
        RendererOGL(Engine* engine);
        
        bool initOpenGL(uint32_t width, uint32_t height);
        
        virtual void resize(const Size2& size) override;
        
        virtual void clear() override;
        virtual void flush() override;
        
        virtual void activateTexture(Texture* texture, uint32_t layer) override;
        virtual void activateShader(Shader* shader) override;
        
        virtual void drawLine(const Vector2& start, const Vector2& finish, const Vector3& color, const Matrix4& transform = Matrix4()) override;
        virtual void drawRectangle(const Rectangle& rectangle, const Vector3& color, const Matrix4& transform = Matrix4()) override;
        virtual void drawQuad(const Rectangle& rectangle, const Vector3& color, const Matrix4& transform = Matrix4()) override;
    };
}
