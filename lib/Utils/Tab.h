#ifndef TAB_H
#define TAB_H

#include <Arduino_GFX.h>
#include <Arduino.h>
#include <vector>
#include <String.h>
#include "Colors.h"

namespace NuggetsInc {

enum LineStyle {
    STYLE_NONE,      
    STYLE_NUMBERED, 
    STYLE_BULLETS  
};

struct DisplayArea {
    int x;
    int y;
    int width;
    int height;
};

class Tab {
public:
    Tab(String name, DisplayArea area, Arduino_GFX* display);

    void setStyle(LineStyle newStyle);
    void addLine(String line);
    void RemoveAllLines();
    void clearTab();
    void refreshTab();
    void scrollUp();
    void scrollDown();
    void DrawTabHeaders(const std::vector<Tab>& tabs, int currentIndex);
    void setNeedsRefresh(bool needsRefresh);

private:
    String name;                
    DisplayArea area;           
    std::vector<String> lines;  
    LineStyle style;            
    int scrollOffset;          
    Arduino_GFX* gfx;            
    bool tabNeedsRefresh = true; 
    std::vector<String> wrappedLinesCache; 
    int maxVisibleLines;       

    std::vector<String> wrapText(String text, int maxWidth, String prefix = "");
    int calculateLineHeight();
    void clampScrollOffset();
};

} // namespace NuggetsInc

#endif // TAB_H
