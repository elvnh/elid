#pragma once

DEFINE_MAYBE_TYPE(scan_Value);
DEFINE_MAYBE_TYPE(scan_Comparison);

scan_Value_Maybe      gui_try_parse_search_value(const char *input, scan_ValueType type, u32 base);
