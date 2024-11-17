#include "Tab.h"

namespace NuggetsInc
{
    Tab::Tab(String name, DisplayArea area, Arduino_GFX *display)
        : name(name), area(area), style(STYLE_NONE), scrollOffset(0), gfx(display)
    {
        maxVisibleLines = (area.height - 20) / calculateLineHeight(); 
    }

    void Tab::setStyle(LineStyle newStyle)
    {
        style = newStyle;
        tabNeedsRefresh = true;
    }

    void Tab::addLine(String line)
    {
        lines.push_back(line);
        tabNeedsRefresh = true;
    }

    void Tab::RemoveAllLines()
    {
        lines.clear();
        wrappedLinesCache.clear();
        scrollOffset = 0;
        tabNeedsRefresh = true;
    }

    void Tab::clearTab()
    {
        gfx->fillRect(area.x, area.y, area.width, area.height, COLOR_BLACK);
    }

    int Tab::calculateLineHeight()
    {
        return 20;
    }

    std::vector<String> Tab::wrapText(String text, int maxWidth, String prefix)
    {
        std::vector<String> wrappedLines;
        String currentLine = prefix;
        int16_t x1, y1;
        uint16_t w, h;

        for (size_t i = 0; i < text.length(); ++i)
        {
            currentLine += text[i];
            gfx->setTextSize(2);
            gfx->getTextBounds(currentLine, 0, 0, &x1, &y1, &w, &h);

            if (w > maxWidth)
            {
                currentLine.remove(currentLine.length() - 1);
                wrappedLines.push_back(currentLine);

                currentLine = prefix + text[i];
            }
        }

        if (currentLine.length() > 0)
        {
            wrappedLines.push_back(currentLine);
        }

        return wrappedLines;
    }

    void Tab::refreshTab()
    {
        if (!tabNeedsRefresh)
            return;

        clearTab();

        DrawTabHeaders({*this}, 0);

        wrappedLinesCache.clear();
        for (size_t i = 0; i < lines.size(); ++i)
        {
            String prefix = "";
            if (style == STYLE_NUMBERED)
            {
                prefix = String(i + 1) + ". ";
            }
            else if (style == STYLE_BULLETS)
            {
                prefix = "• ";
            }

            std::vector<String> wrapped = wrapText(lines[i], area.width - 10, prefix);
            wrappedLinesCache.insert(wrappedLinesCache.end(), wrapped.begin(), wrapped.end());
        }

        gfx->setTextColor(COLOR_WHITE);
        gfx->setTextSize(2);

        int lineHeight = calculateLineHeight();

        maxVisibleLines = (area.height - 20) / lineHeight; 

        clampScrollOffset();

        int startLine = scrollOffset;
        int endLine = std::min(startLine + maxVisibleLines, static_cast<int>(wrappedLinesCache.size()));

        int cursorY = area.y + 20;

        for (int i = startLine; i < endLine; ++i)
        {
            gfx->setCursor(area.x + 5, cursorY);
            gfx->println(wrappedLinesCache[i]);
            cursorY += lineHeight;
        }

        tabNeedsRefresh = false;
    }

    void Tab::DrawTabHeaders(const std::vector<Tab> &tabs, int currentIndex)
    {
        int tabWidth = area.width / tabs.size();
        int tabHeight = 20;

        for (size_t i = 0; i < tabs.size(); ++i)
        {
            int x = area.x + i * tabWidth;
            uint16_t bgColor = (i == currentIndex) ? COLOR_ORANGE : COLOR_WHEAT_CREAM;
            uint16_t textColor = COLOR_WHITE;

            gfx->fillRect(x, area.y, tabWidth, tabHeight, bgColor);

            int16_t x1, y1;
            uint16_t w, h;
            gfx->setTextSize(2);
            gfx->getTextBounds(tabs[i].name.c_str(), x, 0, &x1, &y1, &w, &h);
            int textX = x + (tabWidth - w) / 2;
            int textY = area.y + (tabHeight - h) / 2;

            gfx->setCursor(textX, textY);
            gfx->setTextColor(textColor);
            gfx->print(tabs[i].name);
        }
    }

    void Tab::scrollUp()
    {
        if (scrollOffset > 0)
        {
            scrollOffset--;
            tabNeedsRefresh = true;
        }
    }

    void Tab::scrollDown()
    {
        int maxScrollOffset = std::max(0, static_cast<int>(wrappedLinesCache.size()) - maxVisibleLines);

        if (scrollOffset < maxScrollOffset)
        {
            scrollOffset++;
            tabNeedsRefresh = true;
        }
    }

    void Tab::clampScrollOffset()
    {
        int maxScrollOffset = std::max(0, static_cast<int>(wrappedLinesCache.size()) - maxVisibleLines);

        if (scrollOffset > maxScrollOffset)
        {
            scrollOffset = maxScrollOffset;
        }

        if (scrollOffset < 0)
        {
            scrollOffset = 0;
        }
    }

    void Tab::setNeedsRefresh(bool needsRefresh)
    {
        tabNeedsRefresh = needsRefresh;
    }

} // namespace NuggetsInc