#ifndef TAB_H
#define TAB_H

#include <Arduino_GFX.h>
#include <Arduino.h>
#include <vector>
#include <String.h>
#include "Colors.h"

namespace NuggetsInc {

enum LineStyle {
    STYLE_NONE,      // Just display lines as they are
    STYLE_NUMBERED,  // 1, 2, 3, ...
    STYLE_BULLETS    // •, •, •, ...
};

struct DisplayArea {
    int x;
    int y;
    int width;
    int height;
};

class Tab {
public:
    // Constructor with gfx injection
    Tab(String name, DisplayArea area, Arduino_GFX* display);

    // Methods to handle tab content and interactions
    void setStyle(LineStyle newStyle);
    void addLine(String line);
    void clearTab();
    void refreshTab();
    void scrollUp();
    void scrollDown();
    void switchTab();
    void selectTab();  // Simply returns the selected tab's name
    
    // Getters
    String getName() const;
    DisplayArea getArea() const;
    LineStyle getStyle() const;
    std::vector<String> getLines() const;

private:
    String name;                 // Tab name
    DisplayArea area;            // Area on the screen where this tab is displayed
    std::vector<String> lines;   // Lines of data/content in this tab
    LineStyle style;             // Line style for the tab (none, numbered, bullets)
    int currentLine;             // Current visible line for scrolling
    Arduino_GFX* gfx;            // Graphics display pointer
};

} // namespace NuggetsInc

#endif // TAB_H
