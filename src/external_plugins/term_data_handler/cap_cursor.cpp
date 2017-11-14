#include "term_buffer.h"
#include "term_window.h"
#include "cap_manager.h"

DEFINE_CAP(cursor_right);
DEFINE_CAP(cursor_left);
DEFINE_CAP(cursor_down);
DEFINE_CAP(cursor_up);
DEFINE_CAP(carriage_return);
DEFINE_CAP(cursor_home);
DEFINE_CAP(cursor_address);
DEFINE_CAP(row_address);
DEFINE_CAP(key_shome);
DEFINE_CAP(next_line);
DEFINE_CAP(line_feed);
DEFINE_CAP(parm_index);
DEFINE_CAP(parm_rindex);
DEFINE_CAP(column_address);
DEFINE_CAP(parm_right_cursor);
DEFINE_CAP(parm_left_cursor);
DEFINE_CAP(parm_down_cursor);
DEFINE_CAP(parm_up_cursor);
DEFINE_CAP(save_cursor);
DEFINE_CAP(restore_cursor);


static
void __parm_move_cursor(term_data_context_s & term_context,
                        const std::vector<int> & params,
                        bool move_down,
                        bool do_refresh,
                        bool do_scroll) {
    auto count = 1;
    if (params.size() > 0)
        count = params[0];

    auto scrolled = term_context.term_buffer->MoveCurRow(count, move_down, do_scroll);

    // if (do_refresh &&  scrolled)
    //     term_context.term_window->Refresh();
    (void)scrolled;
    (void)do_refresh;
}

void cursor_right(term_data_context_s & term_context,
                  const std::vector<int> & params){
    parm_right_cursor(term_context, params);
}

void cursor_left(term_data_context_s & term_context,
                 const std::vector<int> & params){
    parm_left_cursor(term_context, params);
}

void cursor_down(term_data_context_s & term_context,
                 const std::vector<int> & params){
    __parm_move_cursor(term_context, params, true, true, false);
}

void cursor_up(term_data_context_s & term_context,
               const std::vector<int> & params){
    __parm_move_cursor(term_context, params, false, true, false);
}

void carriage_return(term_data_context_s & term_context,
                     const std::vector<int> & params){
    term_context.term_buffer->SetCol(0);
    (void)params;
}

void cursor_home(term_data_context_s & term_context,
                 const std::vector<int> & params){
    (void)params;
    set_cursor(term_context, 0, 0);
}

void cursor_address(term_data_context_s & term_context,
                    const std::vector<int> & params){
    set_cursor(term_context, params[1], params[0]);
}

void row_address(term_data_context_s & term_context,
                 const std::vector<int> & params){
    set_cursor(term_context,
               term_context.term_buffer->GetCol(),
               params[0]);
}

void key_shome(term_data_context_s & term_context,
               const std::vector<int> & params) {
    (void)params;
    set_cursor(term_context,
               1,
               0);
}

void next_line(term_data_context_s & term_context,
               const std::vector<int> & params) {
    term_context.term_buffer->SetCol(0);
    __parm_move_cursor(term_context, params, true, true, true);
}
void line_feed(term_data_context_s & term_context,
               const std::vector<int> & params) {
    __parm_move_cursor(term_context, params, true, true, true);
}

void parm_index(term_data_context_s & term_context,
                const std::vector<int> & params) {
    auto col = term_context.term_buffer->GetCol();
    auto row = term_context.term_buffer->GetRow();
    __parm_move_cursor(term_context, params, true, true, true);
    term_context.term_buffer->SetCol(col);
    term_context.term_buffer->SetRow(row);
}

void parm_rindex(term_data_context_s & term_context,
                 const std::vector<int> & params) {
    (void)params;
    auto col = term_context.term_buffer->GetCol();
    auto row = term_context.term_buffer->GetRow();
    __parm_move_cursor(term_context, params, false, true, true);
    term_context.term_buffer->SetCol(col);
    term_context.term_buffer->SetRow(row);
}

void column_address(term_data_context_s & term_context,
                    const std::vector<int> & params) {
    term_context.term_buffer->SetCol(params[0]);
}

void parm_right_cursor(term_data_context_s & term_context,
                       const std::vector<int> & params) {
    auto col = term_context.term_buffer->GetCol();
    auto cols = term_context.term_buffer->GetCols();

    if (col >= cols)
        col = cols - 1;

    if (params.size() > 0)
        col += params[0];
    else
        col += 1;

    if (col >= cols)
        col = cols - 1;

    term_context.term_buffer->SetCol(col);
}

void parm_left_cursor(term_data_context_s & term_context,
                      const std::vector<int> & params) {
    auto col = term_context.term_buffer->GetCol();
    auto cols = term_context.term_buffer->GetCols();

    if (col >= cols)
        col = cols - 1;

    if (params.size() > 0)
    {
        if (col > (uint32_t)params[0])
            col -= params[0];
        else
            col = 0;
    }
    else if (col > 0) {
        col -= 1;
    }

    if (col >= cols)
        col = cols - 1;

    term_context.term_buffer->SetCol(col);
}

void parm_down_cursor(term_data_context_s & term_context,
                       const std::vector<int> & params) {
    __parm_move_cursor(term_context,
                       params,
                       true,
                       true,
                       true);
}

void parm_up_cursor(term_data_context_s & term_context,
                       const std::vector<int> & params) {
    __parm_move_cursor(term_context,
                       params,
                       false,
                       true,
                       true);
}

void save_cursor(term_data_context_s & term_context,
                 const std::vector<int> & params) {
    (void)params;
    term_context.saved_col = term_context.term_buffer->GetCol();
    term_context.saved_row = term_context.term_buffer->GetRow();

    term_context.saved_charset_modes_translate[0] = term_context.charset_modes_translate[0];
    term_context.saved_charset_modes_translate[1] = term_context.charset_modes_translate[1];
    term_context.saved_charset_mode = term_context.charset_mode;

    term_context.saved_origin_mode = term_context.origin_mode;
}

void restore_cursor(term_data_context_s & term_context,
                    const std::vector<int> & params) {
    (void)params;
    if (term_context.saved_col != (uint32_t)-1)
        term_context.term_buffer->SetCol(term_context.saved_col);
    if (term_context.saved_row != (uint32_t)-1)
        term_context.term_buffer->SetRow(term_context.saved_row);

    term_context.charset_modes_translate[0] = term_context.saved_charset_modes_translate[0];
    term_context.charset_modes_translate[1] = term_context.saved_charset_modes_translate[1];
    term_context.charset_mode = term_context.saved_charset_mode;

    term_context.origin_mode = term_context.saved_origin_mode;
}
/*
    def cursor_invisible(self, context):
        self._cursor_visible = False

    def cursor_normal(self, context):
        self._cursor_visible = True

    def cursor_visible(self, context):
        self.cursor_normal(context)
*/
