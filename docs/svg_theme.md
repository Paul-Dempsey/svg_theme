# svg_theme

**svg_theme** is lean, in-memory SVG theming for VCV Rack plugins.
This allows you to maintain a single SVG file for your module or widget.

This is a single-header-file implementation.
One and only one source file of your plugin must contain:

```cpp
#define IMPLEMENT_SVG_THEME
#include "svgtheme.hpp"    // SVG theming for nanosvg
#include "svt_rack.hpp" // VCV Rack-specific helpers
```

In the Demo module, you can find the example of this in [`src/svg_theme_impl.cpp`](../src/svg_theme_impl.cpp) (with path adjusted for whereever the header file exists for you)..

You define your themes in a json file included with your plugin's resources.
You can have as many themes as you like.
A theme is a collection of styles, similar to a very simplified CSS.
Styles can address a limited number of attributes to reflect the theme.

The styles can be applied to any `NSVGimage*`, including the ones that underly Rack's `Svg` class, used by it's SVG-based UI widgets.

## SVG element ids and style tags

The SVG element id is used to target an element for modification.
Inkscape preserves ids across edits of the SVG, so the mechanism is robust to editing.
The element id to be targeted can use a suffix beginning with two dashes, followed by a tag that names a style.
For example, the following element is tagged with `theme-bezel`:

```xml
<circle id="outside_ring--theme-bezel" cx="7.5" cy="7.5" r="6.2" fill="#999999" />
```

In this example, all the tags begin with `theme-`, but this prefix isn't required.
It's just a handy way to notate that the element is a theme-able element.
A tag can group related objects that use the same style.
At this writing, only a single tag (style) can be defined for a given element.

You can edit an object's id in Inkscape: Right click and choose **Object Properties...**. The id is separate from Inkscape's object label which is kept in a separate `inkscape:label` attribute. Inkscape uses it's labels for what it shows you in **Layers and Objects**.

Nano restricts ids to 63 characters.

> Note: Unfortunately, the current Rack SVGs in the component library do not contain ids, so they cannot be themed.
If you want to target Rack-style SVG widgets, you must subclass the Rack widget and supply your own SVG(s(.)

## svg_theme JSON format

The JSON is an array of theme objects.
A theme objects has a `"name"` string and a `"theme"` object containing tag-named members that each provide an object
with the definition of the attributes will be modified when the theme is applied.

A limited set of attributes can be modified by theming:

- Opacity of the element.

- Stroke and fill color. Unlike SVG, colors include opacity.
Targeting only the opacity of a stroke or fill color is not supported -- only the full color as one unit.

- Stroke width

- Color and offset of an existing gradient stop.
To target a gradient, the element in the master SVG must also define a gradient with that stop.
This is necessary because the scheme does not have a complete definition of a gradient,
and we don't have the ability to create and destroy gradients in the SVG.

- Gradients can't be removed.
A color fill or stroke can be changed to 'none', but setting a gradient fill or stroke to a color or 'none' would cause a memory leak.

- Stroke dashes are not supported, but there is no barriers to implementing such supprt.

## Example theme

```json
[
    {
        "name": "Light",
        "theme": {
            "theme_background": {
                "fill": {
                    "gradient": [            
                        {
                            "index":0,
                            "color":"#ee2e63",
                            "offset":0.0
                        }
                    ]
                },
                "stroke": { "color":"#808080", "width":1.5 }
            },
            "theme_no-fill": {
                "fill":"none",
            },
            "theme_no-stroke": {
                "stroke":"none",
            },
            "theme_bezel": {
                "fill": "#808080",
                "stroke": "#323232",
            },
            "theme_ouzel": {
                "opacity": 0.8,
                "fill": {
                    "gradient" [
                        {
                            "index":0,
                            "color":"#ec8d49",
                            "offset":0.0
                        },
                        {
                            "index":1,
                            "color":"#ec8d4900",
                            "offset":0.0
                        }
                    ]
                }
            }
        }
    },
    {
        "name": "Dark",
        "theme": {
        //...
        }
    }
]
```

Each style can have **opacity**, **fill**, and **stroke**.

### opacity

Sets the opacity of the entire element. This is a floating-point value between 0 and 1.0.

### fill

The **fill** is a hex color string value, **none**, or an object containing a **gradient**:

```json
    "strokeless-purple": {
        "fill" : "#952a9ccc",
        "stroke": "none"
    }
```

Color or **none** cannot be applied to an object with a gradient,
and a gradient cannot be applied to an element that does not have one.

Here is an example of a full gradient between two opaque colors:

```json
    "budgie": {
        "fill": {
            "gradient" [
                {
                    "index":0,
                    "color":"#3f7837",
                    "offset":0.0
                },
                {
                    "index":1,
                    "color":"#a01466",
                    "offset":0.0
                }
            ]
        }
    }
```

In a gradient, **index** is required to know whether to change gradient stop 0 or stop 1.
**color** and **offset** are optional, and need to be present only if you're changing it.

### stroke

A **stroke** is a hex color string value, **none**, or an object containing optional **color** string, **width** number, and **gradient** object.

The **gradient** object for stroke is identical to a gradient for a fill.
Note that at this writing, nanosvg doesn't appear to implement stroke gradients or stroke or fill radial gradients reliably,
so while these are supported by this library, you may not get the visual results you're after.

## Creating a theme

- Start with a design that will be one of your themes.
This will be your default theme and if you follow the Rack model, it would be your 'Light' theme.

  For the rest of this discussion we'll refer to this as the "Light" theme.

- Decide which elements will change depending on the theme.

- For each theme-able element, define a style in your Light theme in the JSON to address the things that will be changed.
This is necessary so that user can get back to the Light theme after changing it to something else.

- Copy the Light theme, rename it, and change the colors to suit.

- To prevent leftovers from one theme appearing when another is chosen, each theme must provide all the same styles, setting the same attributes (but with different values).
At this time, the theme engine does not provide a way to verify that themese are parallel, so care must be taken.

## Using the API

The best way to understand the API and see how it works in action is to read the headers (they're well commented),
and to read, build, and run the Demo (also thoroughly commented).

| File | Description |
|--|--|
| [`svgtheme.hpp`](../svgtheme.hpp) | The main implementation of SVG theming. |
| [`svt_rack.hpp`](../svt_rack.hpp) | VCV-Rack-specific helpers for the theme of the widget tree, and appending a Theme menu to your context menu. |
| [`src/Demo.cpp](../src/Demo.cpp) | Demo VCV Rack module |
| [`src/widgets.hpp`](../src/widgets.hpp) | Themeable widgets implementing IApplyTheme. As of this writing, only a themed Screw widget that looks exactly like the standard Rack Silver and Black screws. |
| [`src/svg_theme_impl.cpp`](../src/svg_theme_impl.cpp) | cpp file where the theming code implementation lives. |
| [`res/Demo.svg`](../res/Demo.svg) | Panel SVG for the Demo module. |
| [`res/Screw.svg`](../res/Screw.svg) | SVG for ThemeScrew. |
| [`res/Demo-themes.json`](../res/Demo-themes.json) | Theme definition JSON for the Demo project. |
