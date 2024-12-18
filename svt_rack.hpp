// VCV Rack-specific helpers for svgtheme.hpp. See that file for details.
// See the end of this file for copyright and license information.

#ifndef SVG_THEME_RACK_HELP
#define SVG_THEME_RACK_HELP
#include <rack.hpp>
#include "svgtheme.hpp"

using namespace rack;
namespace svg_theme {

// Walks the Widget tree from `widget`, finding widgets
// implementing IApplyTheme, and applies the theme.
// If modifications to widgets are detected, the DirtyEvent is sent down the
// tree so that affected widgets will be redrawn.
//
bool ApplyChildrenTheme(Widget * widget, SvgThemeEngine& themes, std::shared_ptr<SvgTheme> theme, bool top = true);

// Appends a theme menu that offers a "Theme" option submenu with a 
// list of all the themes you've defined.
// Call from your module widget's appendContextMenu override.
// Your IThemeHolder::applyTheme(std::string theme) override should update 
// the themes of visible widgets, and remember the theme.
//
void AppendThemeMenu(Menu* menu, IThemeHolder* holder, SvgThemeEngine& themes);

// ---------------------------------------------------------------------------
// Themed Widget creation
// These templates assume a themed widget has a constructor of the form WidgetT(svg_theme::SvgThemeEngine & engine, svg_theme::SvgTheme theme)
//

// Center any widget
// We use this instead of Rack's style of defining a lot of create<widget>Centered templates
template <class TWidget>
TWidget* Center(TWidget * widget) {
	widget->box.pos = widget->box.pos.minus(widget->box.size.div(2));
	return widget;
}

template <class TWidget>
TWidget* createThemedWidget(math::Vec pos, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme)
{
	TWidget* o = new TWidget();
	o->box.pos = pos;
	o->applyTheme(engine, theme);
	return o;
}

template <class TPanel = app::SvgPanel>
TPanel* createThemedPanel(std::string svgPath, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) {
	TPanel* panel = new TPanel;
	panel->setBackground(engine.loadSvg(svgPath, theme));
	return panel;
}

template <class TParamWidget>
TParamWidget* createThemedParam(math::Vec pos, engine::Module* module, int paramId, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) {
	TParamWidget* o = new TParamWidget();
	o->box.pos = pos;
	o->app::ParamWidget::module = module;
	o->app::ParamWidget::paramId = paramId;
	o->initParamQuantity();
	o->applyTheme(engine, theme);
	return o;
}

template <class TPortWidget>
TPortWidget* createThemedInput(math::Vec pos, engine::Module* module, int inputId, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) {
	TPortWidget* o = new TPortWidget();
	o->box.pos = pos;
	o->app::PortWidget::module = module;
	o->app::PortWidget::type = engine::Port::INPUT;
	o->app::PortWidget::portId = inputId;
	o->applyTheme(engine, theme);
	return o;
}


template <class TPortWidget>
TPortWidget* createThemedOutput(math::Vec pos, engine::Module* module, int outputId, SvgThemeEngine& engine, std::shared_ptr<SvgTheme> theme) {
	TPortWidget* o = new TPortWidget();
	o->box.pos = pos;
	o->app::PortWidget::module = module;
	o->app::PortWidget::type = engine::Port::OUTPUT;
	o->app::PortWidget::portId = outputId;
	o->applyTheme(engine, theme);
	return o;
}

#ifdef IMPLEMENT_SVG_THEME
bool ApplyChildrenTheme(Widget * widget, SvgThemeEngine& themes, std::shared_ptr<SvgTheme> theme, bool top)
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

    if (top && modified) {
        EventContext cDirty;
        Widget::DirtyEvent eDirty;
        eDirty.context = &cDirty;
        widget->onDirty(eDirty);
    }

    return modified;
}

void AppendThemeMenu(Menu* menu, IThemeHolder* holder, SvgThemeEngine& themes)
{
    auto theme_names = themes.getThemeNames();
    if (theme_names.empty()) return; // no themes

    std::vector<rack::ui::MenuItem*> menus;
    for (auto theme : theme_names) {
        menus.push_back(createCheckMenuItem(
            theme, "", [=]() { return 0 == theme.compare(holder->getThemeName()); },
            [=]() { holder->setThemeName(theme); }
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
