#include "plugin_base.h"

#include "scintilla_editor_cell.h"
#include <vector>
#include <bitset>
#include <iostream>
#include <stdio.h>
#include <iomanip>

#include <vector>
#include <iostream>

#include <cstddef>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cmath>

#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <cassert>
#include <functional>
#include <locale>
#include <codecvt>

#include "Platform.h"

#include "ILoader.h"
#include "ILexer.h"
#include "Scintilla.h"

#include "StringCopy.h"
#include "Position.h"
#include "UniqueString.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "ContractionState.h"
#include "CellBuffer.h"
#include "KeyMap.h"
#include "Indicator.h"
#include "XPM.h"
#include "LineMarker.h"
#include "Style.h"
#include "ViewStyle.h"
#include "CharClassify.h"
#include "Decoration.h"
#include "CaseFolder.h"
#include "Document.h"
#include "UniConversion.h"
#include "Selection.h"
#include "PositionCache.h"
#include "EditModel.h"
#include "MarginView.h"
#include "EditView.h"
#include "Editor.h"
#include "AutoComplete.h"
#include "CallTip.h"
#include "ScintillaBase.h"

#include "scintilla_editor.h"

static
std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wcharconv;

class ScintillaEditorCell : public virtual PluginBase, public virtual TermCell {
public:
    ScintillaEditorCell(ScintillaEditor * pEditor, uint32_t row, uint32_t col) :
        PluginBase("scintilla_editor_cell", "default cell plugin for scintilla editor", 1)
        , m_pEditor(pEditor)
        , m_Row(row)
        , m_Col(col)
    {
        SetModified(false);
    }

    wchar_t GetChar() const {
        int length = m_pEditor->WndProc(SCI_GETTEXTLENGTH, 0, 0);
        uint32_t line_count = m_pEditor->WndProc(SCI_GETLINECOUNT, 0, 0);
        uint32_t line_length = m_pEditor->WndProc(SCI_LINELENGTH, m_Row, 0);

        if (m_Row >= line_count || m_Col >= line_length)
            return L' ';

        auto pos = CursorToDocPos(m_pEditor, m_Row, m_Col, false);

        if (pos >= length)
            return L' ';

        wchar_t ch = m_pEditor->WndProc(SCI_GETCHARAT, pos , 0);

        if (ch == L'\n')
            return L' ';

        return ch;
    }

    void SetChar(wchar_t c) override {
        if (!m_pEditor)
            return;

        auto pos = CursorToDocPos(m_pEditor, m_Row, m_Col);

        std::string bytes = wcharconv.to_bytes((wchar_t)c);

        m_pEditor->WndProc(SCI_SETSEL, pos, pos + 1);
        m_pEditor->WndProc(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(bytes.c_str()));
    }

    uint16_t GetForeColorIndex() const override {
        return TermCell::DefaultForeColorIndex;
    }
    void SetForeColorIndex(uint16_t idx) override {
        (void)idx;
    }
    uint16_t GetBackColorIndex() const override {
        return TermCell::DefaultBackColorIndex;
    }
    void SetBackColorIndex(uint16_t idx) override {
        (void)idx;
    }

    uint16_t GetMode() const override {
        return 0;
    }

    void SetMode(uint16_t m) override {
        (void)m;
    }

    void AddMode(uint16_t m) override {
        (void)m;
    }

    void RemoveMode(uint16_t m) override {
        (void)m;
    }

    void Reset(TermCellPtr cell) override {
        (void)cell;
    }

    bool IsWideChar() const override {
        return false;
    }

    void SetWideChar(bool wide_char) override {
        (void)wide_char;
    }

    bool IsModified() const override {
        return false;
    }

    void SetModified(bool modified) override {
        (void)modified;
    }
private:
    ScintillaEditor * m_pEditor;
    uint32_t m_Row;
    uint32_t m_Col;
};

TermCellPtr CreateDefaultTermCell(ScintillaEditor * pEditor, uint32_t row, uint32_t col)
{
    return TermCellPtr { new ScintillaEditorCell(pEditor, row, col) };
}
