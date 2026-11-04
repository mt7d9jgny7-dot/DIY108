const NUM_LAYERS = 2;
const NUM_ROWS = 8;
const NUM_COLS = 18;
const MAX_MACROS = 10;
const MAX_MACRO_STEPS = 50;

// 总数据大小 = 2层 * 8行 * 18列 * 2字节(键位) + 10宏 * 50步 * 3字节(动作) = 2076
const TOTAL_BYTES = 2076; 