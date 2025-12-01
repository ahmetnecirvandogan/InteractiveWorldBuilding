#include "UIDropdownMenu.h"
#include "UIRenderer.h"
#include <algorithm>
#include <cmath>
#include <iostream>

UIDropdownMenu::UIDropdownMenu(float x, float y, float width, float itemHeight)
    : UIElement(x, y, width, 0.0f), // Height will be calculated dynamically
      m_itemHeight(itemHeight),
      m_itemSpacing(5.0f),
      m_isOpen(false),
      m_animationSpeed(8.0f),
      m_animationProgress(0.0f),
      m_uiRenderer(nullptr),
      m_buttonsInRenderer(false)
{
    // The dropdown starts with zero height when closed
}

void UIDropdownMenu::AddMenuItem(const std::string& text, std::function<void()> callback, const std::string& res_path) {
    // Calculate target position going upward (negative Y offset) to match UpdateItemPositions
    float targetY = m_y - (m_menuItems.size() + 1) * (m_itemHeight + m_itemSpacing);
    
    // Create button at initial position (m_y) - it will animate to targetY when opened
    auto button = std::make_shared<UIButton>(m_x, m_y, m_width, m_itemHeight, text);
   
    // Style the menu item buttons
    button->SetNormalColor(vec3(0.6f, 0.6f, 0.6f)); 
    button->SetHoverColor(vec3(0.8f, 0.8f, 0.8f));    
    button->SetPressedColor(vec3(0.2f, 0.2f, 0.2f));
     if(res_path != ""){
        std::cout << "texture set ni" << std::endl;
        button->SetTexture(res_path);
    }
    
    // Wrap the callback to also close the menu when an item is clicked
    button->SetOnClickCallback([this, callback]() {
        if (callback) {
            callback();
        }
        //SetOpen(false);
    });
    
    MenuItem item;
    item.button = button;
    item.targetY = targetY;
    item.currentY = m_y; // Start at the button position (closed position)
    
    m_menuItems.push_back(item);
    
    // Update the total height of the dropdown when open
    UpdateItemPositions();
}

void UIDropdownMenu::ClearMenuItems() {
    m_menuItems.clear();
    UpdateItemPositions();
}

void UIDropdownMenu::SetOpen(bool open) {
    std::cout << "UIDropdownMenu::SetOpen(" << (open ? "true" : "false") << ")" << std::endl;
    
    if (open && !m_isOpen) {
        // Opening: Add buttons to UIRenderer
        UpdateItemVisibility();
    } else if (!open && m_isOpen) {
        // Closing: Don't remove buttons immediately, let animation handle it
    }
    
    m_isOpen = open;
    // Animation will handle the transition
}

void UIDropdownMenu::Update(float deltaTime) {
    UpdateAnimation(deltaTime);
    
    // Update button positions and visibility based on animation progress
    bool anyVisible = false;
    for (auto& item : m_menuItems) {
        float animatedY = m_y + (item.targetY - m_y) * m_animationProgress;
        item.currentY = animatedY;
        item.button->SetPosition(m_x,animatedY);
        
        // Update button visibility based on animation progress
        // Only show buttons when there's significant animation progress
        bool shouldBeVisible = m_animationProgress > 0.1f;
        item.button->SetVisible(shouldBeVisible);
        
        if (shouldBeVisible) anyVisible = true;
    }
    
    // Debug output
    static int debugCounter = 0;
    if (debugCounter++ % 60 == 0 && m_isOpen) { // Print every 60 frames when open
        //std::cout << "Animation progress: " << m_animationProgress << ", any buttons visible: " << anyVisible << std::endl;
    }
    
    // Remove buttons from UIRenderer when animation is complete and dropdown is closed
    if (!m_isOpen && m_animationProgress <= 0.01f && m_uiRenderer && m_buttonsInRenderer) {
        std::cout << "Animation complete, removing buttons from UIRenderer" << std::endl;
        for (auto& item : m_menuItems) {
            m_uiRenderer->RemoveUIElement(item.button);
        }
        m_buttonsInRenderer = false;
    }
}

void UIDropdownMenu::UpdateAnimation(float deltaTime) {
    float targetProgress = m_isOpen ? 1.0f : 0.0f;
    
    if (std::abs(m_animationProgress - targetProgress) > 0.001f) {
        float direction = (targetProgress > m_animationProgress) ? 1.0f : -1.0f;
        m_animationProgress += direction * m_animationSpeed * deltaTime;
        
        // Clamp to target
        if (direction > 0.0f) {
            m_animationProgress = std::min(m_animationProgress, targetProgress);
        } else {
            m_animationProgress = std::max(m_animationProgress, targetProgress);
        }
        
        // Debug output
        static int debugCounter = 0;
        if (debugCounter++ % 30 == 0) { // Print every 30 frames
            std::cout << "Animation progress: " << m_animationProgress << " (target: " << targetProgress << ")" << std::endl;
        }
        
        // Apply easing
        float easedProgress = EaseOutCubic(m_animationProgress);
        
        // Update the effective height of the dropdown
        float maxHeight = m_menuItems.size() * (m_itemHeight + m_itemSpacing);
        m_height = maxHeight * easedProgress;
    }
}

float UIDropdownMenu::EaseOutCubic(float t) const {
    return 1.0f - std::pow(1.0f - t, 3.0f);
}

bool UIDropdownMenu::OnClick(float x, float y) {
    if (!m_isOpen) {
        return false; // Don't handle clicks when closed
    }
    
    // Check if any menu item was clicked
    for (auto& item : m_menuItems) {
        if (item.button->OnClick(x, y)) {
            return true; // Item handled the click
        }
    }
    
    return false;
}

bool UIDropdownMenu::OnMouseMove(float x, float y) {
    if (!m_isOpen) {
        return false; // Don't handle mouse move when closed
    }
    
    bool handled = false;
    for (auto& item : m_menuItems) {
        if (item.button->OnMouseMove(x, y)) {
            handled = true;
        }
    }
    
    return handled;
}

void UIDropdownMenu::Render() {
    // UIDropdownMenu doesn't render itself directly
    // Individual buttons will be rendered by the UIRenderer
    // This method is mainly for debugging
    std::cout << "UIDropdownMenu::Render() called - isOpen: " << m_isOpen << ", animProgress: " << m_animationProgress << std::endl;
}

bool UIDropdownMenu::IsPointInMenuArea(float x, float y) const {
    if (!m_isOpen) {
        return false;
    }
    
    // Check if point is within the dropdown area
    float menuBottom = m_y + m_height;
    return (x >= m_x && x <= m_x + m_width && 
            y >= m_y && y <= menuBottom);
}

void UIDropdownMenu::UpdateItemPositions() {
    for (size_t i = 0; i < m_menuItems.size(); ++i) {
        float itemY = m_y - (i + 1) * (m_itemHeight + m_itemSpacing);
        m_menuItems[i].targetY = itemY;
    }
}

void UIDropdownMenu::UpdateItemVisibility() {
    if (!m_uiRenderer || m_buttonsInRenderer) return;
    
    std::cout << "UpdateItemVisibility: Adding menu items to UIRenderer" << std::endl;
    
    // Only add buttons to UIRenderer, don't remove them here
    for (auto& item : m_menuItems) {
        m_uiRenderer->AddUIElement(item.button);
        item.button->SetVisible(false); // Start invisible, animation will show them
    }
    m_buttonsInRenderer = true;
}
