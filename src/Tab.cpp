// Tab.cpp
#include "Tab.h"

namespace NuggetsInc {

Tab::Tab(String name, DisplayArea area, Arduino_GFX* display)
    : name(name), area(area), style(STYLE_NONE), currentLine(0), gfx(display) {}

void Tab::setStyle(LineStyle newStyle) {
    style = newStyle;
    tabNeedsRefresh = true; // Mark for refresh when style changes
}

void Tab::addLine(String line) {
    lines.push_back(line);
    tabNeedsRefresh = true; // Mark for refresh when a new line is added
}

void Tab::clearTab() {
    // Clear the display area by filling it with red (or any background color)
    gfx->fillRect(area.x, area.y, area.width, area.height, COLOR_RED);
}

int Tab::calculateLineHeight() {
    // Adjust line height based on the style (e.g., numbered or bullets)
    if (style == STYLE_NUMBERED || style == STYLE_BULLETS) {
        return 24;  // Increase line height for numbered/bullet lines
    }
    return 20;  // Default line height
}

std::vector<String> Tab::wrapText(String text, int maxWidth, String prefix) {
    std::vector<String> wrappedLines;
    String currentLine = prefix;
    int16_t x1, y1;
    uint16_t width, height;

    // Split text into words
    std::vector<String> words;
    int start = 0;
    int end = 0;
    while (end <= text.length()) {
        if (end < text.length() && text[end] != ' ') {
            end++;
        }
        else {
            if (end > start) {
                String word = text.substring(start, end);
                words.push_back(word);
            }
            start = end + 1;
            end++;
        }
    }

    for (size_t i = 0; i < words.size(); ++i) {
        String word = words[i];
        String trialLine = currentLine;

        // Determine if a space is needed
        if (currentLine.length() > (prefix.length() > 0 ? prefix.length() : 0)) {
            trialLine += " ";
        }
        trialLine += word;

        gfx->getTextBounds(trialLine, 0, 0, &x1, &y1, &width, &height);

        if (width <= maxWidth) {
            // Add the word to the current line
            if (currentLine.length() > (prefix.length() > 0 ? prefix.length() : 0)) {
                currentLine += " ";
            }
            currentLine += word;
        }
        else {
            if (currentLine.length() > (prefix.length() > 0 ? prefix.length() : 0)) {
                // Push the current line and start a new one
                wrappedLines.push_back(currentLine);
                currentLine = "";
            }

            // Now, try to add the word to the new line
            gfx->getTextBounds(word, 0, 0, &x1, &y1, &width, &height);
            if (width <= maxWidth) {
                currentLine = word;
            }
            else {
                // Split the word with hyphen
                String part = "";
                size_t splitPos = 0;
                for (; splitPos < word.length(); ++splitPos) {
                    String temp = part + word[splitPos];
                    String tempWithHyphen = temp + "-";
                    gfx->getTextBounds(tempWithHyphen, 0, 0, &x1, &y1, &width, &height);
                    if (width > maxWidth) {
                        break;
                    }
                    part += word[splitPos];
                }

                if (splitPos > 0) {
                    part += "-";
                    wrappedLines.push_back(part);
                    // Add the remaining part of the word to the next line
                    String remaining = word.substring(splitPos);
                    // Recursive call to handle the remaining part
                    std::vector<String> remainingWrapped = wrapText(remaining, maxWidth);
                    wrappedLines.insert(wrappedLines.end(), remainingWrapped.begin(), remainingWrapped.end());
                    currentLine = "";
                }
                else {
                    // Cannot split the word, add as is
                    wrappedLines.push_back(word);
                    currentLine = "";
                }
            }
        }
    }

    // Add the last line if it's not empty
    if (currentLine.length() > (prefix.length() > 0 ? prefix.length() : 0)) {
        wrappedLines.push_back(currentLine);
    }

    return wrappedLines;
}

void Tab::refreshTab() {
    if (!tabNeedsRefresh) return;  // Only refresh if the content has changed

    clearTab();  // Clear the tab area before updating content

    int lineHeight = calculateLineHeight();  // Adjust line height based on style
    gfx->setTextColor(COLOR_WHITE);

    // Recompute all wrapped lines
    wrappedLinesCache.clear();

    for (size_t i = 0; i < lines.size(); ++i) {
        // Determine the prefix based on the line style
        String prefix = "";
        if (style == STYLE_NUMBERED) {
            prefix = String(i + 1) + ". ";  // Numbering style
        }
        else if (style == STYLE_BULLETS) {
            prefix = "• ";  // Bullet style
        }

        // Wrap the line with the prefix applied only on the first wrapped line
        std::vector<String> wrappedLines = wrapText(lines[i], area.width, prefix);

        // Add to cache
        for (auto &line : wrappedLines) {
            wrappedLinesCache.push_back(line);
        }
    }

    // Now render the visible lines based on currentLine and visibleLines
    int visibleLines = area.height / lineHeight;  // Number of lines that fit in the area

    for (int i = currentLine; i < currentLine + visibleLines && i < wrappedLinesCache.size(); ++i) {
        int cursorY = area.y + (i - currentLine) * lineHeight;

        gfx->setCursor(area.x, cursorY);
        gfx->println(wrappedLinesCache[i]);
    }

    tabNeedsRefresh = false;  // Mark tab as refreshed
}

void Tab::scrollUp() {
    if (currentLine > 0) {
        --currentLine;
        tabNeedsRefresh = true;  // Mark content as changed, refresh on next update
    }
}

void Tab::scrollDown() {
    int visibleLines = area.height / calculateLineHeight();
    if (currentLine + visibleLines < wrappedLinesCache.size()) {
        ++currentLine;
        tabNeedsRefresh = true;  // Mark content as changed, refresh on next update
    }
}

void Tab::switchTab() {
    // Logic to switch between tabs, to be implemented
}

void Tab::selectTab() {
    // Return the selected tab's name as a string
    Serial.println("Selected Tab: " + name);
}

String Tab::getName() const {
    return name;
}

DisplayArea Tab::getArea() const {
    return area;
}

LineStyle Tab::getStyle() const {
    return style;
}

std::vector<String> Tab::getLines() const {
    return lines;
}

} // namespace NuggetsInc
