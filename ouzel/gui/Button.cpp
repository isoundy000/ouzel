// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include <functional>
#include "Button.hpp"
#include "core/Engine.hpp"
#include "events/EventHandler.hpp"
#include "events/EventDispatcher.hpp"
#include "scene/Layer.hpp"
#include "scene/Camera.hpp"
#include "Label.hpp"

namespace ouzel
{
    namespace gui
    {
        Button::Button()
        {
            eventHandler.uiHandler = std::bind(&Button::handleUI, this, std::placeholders::_1, std::placeholders::_2);
            engine->getEventDispatcher()->addEventHandler(&eventHandler);

            pickable = true;
        }

        Button::Button(const std::string& normalImage,
                       const std::string& selectedImage,
                       const std::string& pressedImage,
                       const std::string& disabledImage,
                       const std::string& label,
                       const std::string& font,
                       float fontSize,
                       const Color& aLabelColor,
                       const Color& aLabelSelectedColor,
                       const Color& aLabelPressedColor,
                       const Color& aLabelDisabledColor):
            eventHandler(EventHandler::PRIORITY_MAX + 1),
            labelColor(aLabelColor),
            labelSelectedColor(aLabelSelectedColor),
            labelPressedColor(aLabelPressedColor),
            labelDisabledColor(aLabelDisabledColor)
        {
            eventHandler.uiHandler = std::bind(&Button::handleUI, this, std::placeholders::_1, std::placeholders::_2);
            engine->getEventDispatcher()->addEventHandler(&eventHandler);

            if (!normalImage.empty())
            {
                normalSprite.reset(new scene::Sprite());
                if (normalSprite->init(normalImage, false))
                {
                    addComponent(normalSprite.get());
                }
            }

            if (!selectedImage.empty())
            {
                selectedSprite.reset(new scene::Sprite());
                if (selectedSprite->init(selectedImage, false))
                {
                    addComponent(selectedSprite.get());
                }
            }

            if (!pressedImage.empty())
            {
                pressedSprite.reset(new scene::Sprite());
                if (pressedSprite->init(pressedImage, false))
                {
                    addComponent(pressedSprite.get());
                }
            }

            if (!disabledImage.empty())
            {
                disabledSprite.reset(new scene::Sprite());
                if (disabledSprite->init(disabledImage, false))
                {
                    addComponent(disabledSprite.get());
                }
            }

            if (!label.empty())
            {
                labelDrawable.reset(new scene::TextRenderer(font, true, fontSize, label));
                labelDrawable->setColor(labelColor);
                addComponent(labelDrawable.get());
            }

            pickable = true;

            updateSprite();
        }

        void Button::setEnabled(bool newEnabled)
        {
            Widget::setEnabled(newEnabled);

            selected = false;
            pointerOver = false;
            pressed = false;

            updateSprite();
        }

        void Button::setSelected(bool newSelected)
        {
            Widget::setSelected(newSelected);

            updateSprite();
        }

        bool Button::handleUI(Event::Type type, const UIEvent& event)
        {
            if (!enabled) return true;

            if (event.actor == this)
            {
                if (type == Event::Type::ACTOR_ENTER)
                {
                    pointerOver = true;
                    updateSprite();
                }
                else if (type == Event::Type::ACTOR_LEAVE)
                {
                    pointerOver = false;
                    updateSprite();
                }
                else if (type == Event::Type::ACTOR_PRESS)
                {
                    pressed = true;
                    updateSprite();
                }
                else if (type == Event::Type::ACTOR_RELEASE)
                {
                    if (pressed)
                    {
                        pressed = false;
                        updateSprite();
                    }
                }
                else if (type == Event::Type::ACTOR_CLICK)
                {
                    if (pressed)
                    {
                        pressed = false;
                        updateSprite();
                    }
                }
            }

            return true;
        }

        void Button::updateSprite()
        {
            if (normalSprite) normalSprite->setHidden(true);
            if (selectedSprite) selectedSprite->setHidden(true);
            if (pressedSprite) pressedSprite->setHidden(true);
            if (disabledSprite) disabledSprite->setHidden(true);

            if (enabled)
            {
                if (pressed && pointerOver && pressedSprite)
                {
                    pressedSprite->setHidden(false);
                }
                else if (selected && selectedSprite)
                {
                    selectedSprite->setHidden(false);
                }
                else if (normalSprite)
                {
                    normalSprite->setHidden(false);
                }

                if (labelDrawable)
                {
                    if (pressed && pointerOver)
                    {
                        labelDrawable->setColor(labelPressedColor);
                    }
                    else if (selected)
                    {
                        labelDrawable->setColor(labelSelectedColor);
                    }
                    else
                    {
                        labelDrawable->setColor(labelColor);
                    }
                }
            }
            else // disabled
            {
                if (disabledSprite)
                {
                    disabledSprite->setHidden(false);
                }
                else if (normalSprite)
                {
                    normalSprite->setHidden(false);
                }

                if (labelDrawable)
                {
                    labelDrawable->setColor(labelDisabledColor);
                }
            }
        }
    } // namespace gui
} // namespace ouzel
