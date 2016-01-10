// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include <memory>
#include <set>
#include "NodeContainer.h"
#include "Matrix4.h"
#include "Vector2.h"
#include "Rectangle.h"

namespace ouzel
{
    class Camera;
    class Scene;
    
    class Layer: public NodeContainer, public std::enable_shared_from_this<Layer>
    {
        friend Scene;
    public:
        Layer();
        virtual ~Layer();
        
        virtual void update(float delta);
        virtual void draw();
        
        virtual bool addChild(std::shared_ptr<Node> const& node) override;
        virtual bool removeChild(std::shared_ptr<Node> const& node) override;
        
        void addNode(std::shared_ptr<Node> const& node);
        void removeNode(std::shared_ptr<Node> const& node);
        void reorderNodes();
        
        std::shared_ptr<Camera> const& getCamera() const { return _camera; }
        void setCamera(std::shared_ptr<Camera> const& camera);
        
        std::shared_ptr<Node> pickNode(const Vector2& position);
        std::set<std::shared_ptr<Node>> pickNodes(const Rectangle& rectangle);
        
        Vector2 screenToWorldLocation(const Vector2& position);
        Vector2 worldToScreenLocation(const Vector2& position);
        
        const Matrix4& getProjection() const { return _projection; }
        virtual void recalculateProjection();
        
        int32_t getOrder() const { return _order; }
        void setOrder(int32_t order);
        
        std::shared_ptr<Scene> getScene() const { return _scene.lock(); }
        
    protected:
        virtual void addToScene(std::shared_ptr<Scene> const& scene);
        virtual void removeFromScene();
        
        void lock();
        void unlock();
        
        std::shared_ptr<Camera> _camera;
        std::vector<std::shared_ptr<Node>> _nodes;
        bool _reorderNodes = false;
        
        Matrix4 _projection;
        Matrix4 _inverseProjection;
        
        std::weak_ptr<Scene> _scene;
        int32_t _order = 0;
        
        std::set<std::shared_ptr<Node>> _nodeDeleteList;
        bool _locked = false;
    };
}
