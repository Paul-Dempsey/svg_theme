// VCV Rack-specific helpers for svgtheme.hpp. See that file for details.
// See the end of this file for copyright and license information.

#ifndef SVG_THEME_RACK_HELP
#define SVG_THEME_RACK_HELP
#include <rack.hpp>
#include "svgtheme.hpp"

using namespace rack;
namespace svg_theme {

// This helper recurses the Widget tree from `widget`, finding widgets
// implementing IApplyTheme, calling the interface to apply the theme.
// If modifications to widgets are detected, the DirtyEvent is sent down the
// tree so that affected widgets will be redrawn.
//
bool ApplyChildrenTheme(Widget * widget, SvgThemes& themes, std::shared_ptr<Theme> theme, bool top = true);

// This helper appends a theme menu that offers a "Theme" option submenu with a 
// list of all the themes you've defined.
// Call from your module widget's appendContextMenu override.
// Your IThemeHolder::applyTheme(std::string theme) override should update 
// the themes of visible widgets, and remember the theme.
//
void AppendThemeMenu(Menu* menu, IThemeHolder* holder, SvgThemes& themes);

//  
#ifdef IMPLEMENT_SVG_THEME
bool ApplyChildrenTheme(Widget * widget, SvgThemes& themes, std::shared_ptr<Theme> theme, bool top)
{
    bool modified = false;

    auto change = dynamic_cast<svg_theme::IApplyTheme*>(widget);
    if (change && change->applyTheme(themes, theme)) {
        modified = true;
    }

    for (Widget* child : widget->children) {
        change = dynamic_cast<svg_theme::IApplyTheme*>(child);
        if (change && change->applyTheme(themes, theme)) {
            modified = true;
        }
        if (!child->children.empty()) {
            if (ApplyChildrenTheme(child, themes, theme, false)) {
                modified = true;
            }
        }
    }

    if (top) {
        EventContext cDirty;
        Widget::DirtyEvent eDirty;
        eDirty.context = &cDirty;
        widget->onDirty(eDirty);
    }

    return modified;
}

void AppendThemeMenu(Menu* menu, IThemeHolder* holder, SvgThemes& themes)
{
    auto theme_names = themes.getThemeNames();
    if (theme_names.empty()) return; // no themes

    std::vector<rack::ui::MenuItem*> menus;
    for (auto theme : theme_names) {
        menus.push_back(createCheckMenuItem(
            theme, "", [=]() { return 0 == theme.compare(holder->getTheme()); },
            [=]() { holder->setTheme(theme); }
        ));
    }
    for (auto child: menus) {
        menu->addChild(child);
    }    
}

#endif // #ifdef IMPLEMENT_SVG_THEME

} // namespace svg_theme
#endif //#ifdef SVG_THEME_RACK_HELP

/* Copyright (C) 2023 Paul Chase Dempsey pcdempsey@live.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
*/
