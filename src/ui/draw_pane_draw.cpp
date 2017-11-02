#include <pybind11/embed.h>

#include <iostream>

#include "term_window.h"
#include "draw_pane.h"
#include <wx/dcbuffer.h>

#include "term_context.h"
#include "term_buffer.h"
#include "term_line.h"
#include "term_cell.h"
#include "term_network.h"
#include "color_theme.h"
#include "scope_locker.h"

#include <bitset>

extern
wxCoord PADDING;

class __DCAttributesChanger {
public:
    __DCAttributesChanger(wxDC * pdc) :
        m_pDC(pdc)
        , back(pdc->GetBackground())
        , txt_fore(pdc->GetTextForeground())
        , txt_back(pdc->GetTextBackground())
    {
    }

    ~__DCAttributesChanger() {
        m_pDC->SetTextForeground(txt_fore);
        m_pDC->SetTextBackground(txt_back);
        m_pDC->SetBackground(back);
    }

    wxDC * m_pDC;
    const wxBrush & back;
    const wxColour & txt_fore;
    const wxColour & txt_back;
};

void DrawPane::DrawContent(wxDC &dc,
                           wxString & content,
                           uint16_t & last_fore_color,
                           uint16_t & last_back_color,
                           uint16_t & last_mode,
                           uint16_t fore_color,
                           uint16_t back_color,
                           uint16_t mode,
                           wxCoord & last_x,
                           wxCoord & last_y,
                           wxCoord y)
{
    wxSize content_size = dc.GetTextExtent(content);
    bool multi_line = content.Find('\n', true) > 0;

    wxSize content_last_line_size {0, 0};
    wxSize content_before_last_line_size {0, 0};

    std::bitset<16> m(last_mode);

    if (multi_line)
    {
        content_last_line_size = dc.GetTextExtent(content.AfterLast('\n'));
        content_before_last_line_size.SetHeight(content_size.GetHeight() - content_last_line_size.GetHeight());
        content_before_last_line_size.SetWidth(content_size.GetWidth());
    }

    uint16_t back_color_use = last_back_color;
    uint16_t fore_color_use = last_fore_color;

    if (m.test(TermCell::Cursor))
    {
        back_color_use = TermCell::DefaultCursorColorIndex;
        fore_color_use = last_back_color;
    }

    if (m.test(TermCell::Reverse))
    {
        uint16_t tmp = back_color_use;
        back_color_use = fore_color_use;
        fore_color_use = tmp;
    }

    if (back_color_use != TermCell::DefaultBackColorIndex)
    {
        wxBrush brush(m_ColorTable[back_color_use]);
        dc.SetBrush(brush);
        dc.SetPen(wxPen(m_ColorTable[back_color_use]));

        if (multi_line)
        {
            dc.DrawRectangle(wxPoint(last_x, last_y),
                             content_before_last_line_size);
            dc.DrawRectangle(wxPoint(PADDING, last_y +
                                     content_before_last_line_size.GetHeight()),
                             content_last_line_size);
        }
        else
        {
            dc.DrawRectangle(wxPoint(last_x, last_y), content_size);
        }
    }

    dc.SetTextForeground(m_ColorTable[fore_color_use]);
    dc.SetTextBackground(m_ColorTable[back_color_use]);
    dc.DrawText(content, last_x, last_y);

    if (multi_line)
    {
        last_x = PADDING + content_last_line_size.GetWidth();
    } else {
        last_x += content_size.GetWidth();
    }

    last_y = y;
    content.Clear();
    last_fore_color = fore_color;
    last_back_color = back_color;
    last_mode = mode;
}

void DrawPane::CalculateClipRegion(wxRegion & clipRegion, TermBufferPtr buffer)
{
    wxRect clientSize = GetClientSize();

    clipRegion.Union(clientSize);

    auto rows = buffer->GetRows();

    for (auto row = 0u; row < rows; row++) {
        auto line = buffer->GetLine(row);

        if (!clipRegion.IsEmpty()
            && row == line->GetLastRenderLineIndex()
            && !line->IsModified())
        {
            wxRect rowRect(0, PADDING + row * m_LineHeight, clientSize.GetWidth(), m_LineHeight);
            clipRegion.Subtract(rowRect);
        }
    }
}

void DrawPane::DoPaint(wxDC & dc, TermBufferPtr buffer, bool full_paint)
{
    __DCAttributesChanger changer(&dc);

    wxBrush backgroundBrush(m_ColorTable[TermCell::DefaultBackColorIndex]);

    wxString content {""};

    wxDCFontChanger fontChanger(dc, *GetFont());

    auto rows = buffer->GetRows();
    auto cols = buffer->GetCols();

    auto y = PADDING;

    uint16_t last_fore_color = TermCell::DefaultForeColorIndex;
    uint16_t last_back_color = TermCell::DefaultBackColorIndex;
    wxCoord last_y = PADDING;
    wxCoord last_x = PADDING;
    uint16_t last_mode = 0;

    dc.SetBackground( backgroundBrush );
    dc.Clear();

    for (auto row = 0u; row < rows; row++) {
        auto line = buffer->GetLine(row);

        if (!full_paint &&
            row == line->GetLastRenderLineIndex()
            && !line->IsModified())
        {
            y += m_LineHeight;

            if (content.Length() > 0)
            {
                DrawContent(dc, content,
                            last_fore_color,
                            last_back_color,
                            last_mode,
                            TermCell::DefaultForeColorIndex,
                            TermCell::DefaultBackColorIndex,
                            0,
                            last_x,
                            last_y,
                            y);
            }

            last_x = PADDING;
            last_y = y;

            continue;
        }

        line->SetLastRenderLineIndex(row);
        line->SetModified(false);

        for (auto col = 0u; col < cols; col++) {
            auto cell = line->GetCell(col);

            wchar_t ch = 0;
            uint16_t fore_color = TermCell::DefaultForeColorIndex;
            uint16_t back_color = TermCell::DefaultBackColorIndex;
            uint16_t mode = 0;

            if (cell && cell->GetChar() != 0) {
                fore_color = cell->GetForeColorIndex();
                back_color = cell->GetBackColorIndex();
                mode = cell->GetMode();
                ch = cell->GetChar();
            } else if (!cell) {
                ch = ' ';
            }

            if (ch != 0)
            {
                if (last_fore_color != fore_color
                    || last_back_color != back_color
                    || last_mode != mode)
                {
                    DrawContent(dc, content,
                                last_fore_color,
                                last_back_color,
                                last_mode,
                                fore_color,
                                back_color,
                                mode,
                                last_x,
                                last_y,
                                y);

                }

                content.append(ch);
            }

        }

        y += m_LineHeight;

        if (last_x == PADDING)
        {
            if (row != rows - 1)
                content.append("\n");
        }
        else if (content.Length() > 0)
        {
            DrawContent(dc, content,
                        last_fore_color,
                        last_back_color,
                        last_mode,
                        TermCell::DefaultForeColorIndex,
                        TermCell::DefaultBackColorIndex,
                        0,
                        last_x,
                        last_y,
                        y);
            last_x = PADDING;
            last_y = y;
        }
    }

    if (content.Length() > 0)
    {
        dc.SetTextForeground(m_ColorTable[last_fore_color]);
        dc.SetTextBackground(m_ColorTable[last_back_color]);

        dc.DrawText(content, last_x, last_y);
    }
}

void DrawPane::PaintOnDemand()
{
   TermContextPtr context = std::dynamic_pointer_cast<TermContext>(m_TermWindow->GetPluginContext());

    if (!context)
        return;

    TermBufferPtr buffer = context->GetTermBuffer();

    if (!buffer)
        return;

    int refreshNow = 0;

    {
        wxCriticalSectionLocker locker(m_RefreshLock);
        refreshNow = m_RefreshNow;
        if (refreshNow)
            std::cout << "refresh:" << refreshNow << std::endl;
    }

    if (refreshNow)
    {
        wxClientDC dc(this);

        wxRect clientSize = GetClientSize();

        wxRegion clipRegion(clientSize);

        wxBufferedDC bDC(&dc,
                         GetClientSize());

        __ScopeLocker locker(buffer);

        bool paintChanged = true;

        TermCellPtr cell = buffer->GetCurCell();

        if (cell)
            cell->AddMode(TermCell::Cursor);
        else {
            TermLinePtr line = buffer->GetCurLine();

            if (line)
                line->SetModified(true);
        }

        if (paintChanged)
        {
            CalculateClipRegion(clipRegion, buffer);

            dc.DestroyClippingRegion();
            dc.SetDeviceClippingRegion(clipRegion);
        }

        DoPaint(bDC, buffer, !paintChanged);

        if (cell)
            cell->RemoveMode(TermCell::Cursor);
    }

    {
        wxCriticalSectionLocker locker(m_RefreshLock);
        if (refreshNow)
            std::cout << "end refresh:" << m_RefreshNow << "," << refreshNow << std::endl;
        m_RefreshNow -= refreshNow;
    }
}
