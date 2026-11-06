document.getElementById('readBtn')?.addEventListener('click', async () => {
    if (!device) return;
    try {
        let fullBuffer = new Uint8Array(0); 
        const numChunks = Math.ceil(TOTAL_BYTES / 64);
        for (let i = 0; i < numChunks; i++) {
            const result = await device.receiveFeatureReport(4);
            let chunk = new Uint8Array(result.buffer);
            if (chunk.length === 65 && chunk[0] === 0x04) chunk = chunk.slice(1);
            const temp = new Uint8Array(fullBuffer.length + chunk.length);
            temp.set(fullBuffer); temp.set(chunk, fullBuffer.length); fullBuffer = temp;
        }
        
        if (fullBuffer.length < TOTAL_BYTES) throw new Error(`数据包不完整`);

        const dataView = new DataView(fullBuffer.buffer); let offset = 0;
        // 解析键位
        for (let l = 0; l < NUM_LAYERS; l++) 
            for (let r = 0; r < NUM_ROWS; r++) 
                for (let c = 0; c < NUM_COLS; c++) {
                    keymap[l][r][c] = dataView.getUint16(offset, true); offset += 2;
                }
        // 解析宏
        for (let m = 0; m < MAX_MACROS; m++) {
            macros[m] = [];
            for (let s = 0; s < MAX_MACRO_STEPS; s++) {
                const type = fullBuffer[offset];
                const value = dataView.getUint16(offset + 1, true);
                offset += 3;
                if (type !== 0) macros[m].push({ type, value });
            }
        }
        // 解析模式
        const mode = fullBuffer[offset];
        document.getElementById('mode6kro').checked = (mode === 0);
        document.getElementById('modeNkro').checked = (mode === 1);

        renderKeyboard(); renderMacroEditor();
        alert("读取成功！");
    } catch (err) { alert("读取失败: " + err.message); }
});

document.getElementById('writeBtn')?.addEventListener('click', async () => {
    if (!device) return;
    try {
        // 🔥 关键修复：写入前强制补全所有按下却没有抬起的按键，防止宏死锁
        for (let m = 0; m < MAX_MACROS; m++) {
            let pendingKeys = new Set();
            for (let s = 0; s < macros[m].length; s++) {
                if (macros[m][s].type === 1) pendingKeys.add(macros[m][s].value);
                if (macros[m][s].type === 2) pendingKeys.delete(macros[m][s].value);
            }
            // 如果有键没抬起，自动追加抬起动作
            pendingKeys.forEach(key => {
                if (macros[m].length < MAX_MACRO_STEPS) {
                    macros[m].push({ type: 2, value: key });
                }
            });
        }

        const buffer = new ArrayBuffer(TOTAL_BYTES); const view = new DataView(buffer); let offset = 0;
        for (let l = 0; l < NUM_LAYERS; l++) 
            for (let r = 0; r < NUM_ROWS; r++) 
                for (let c = 0; c < NUM_COLS; c++) {
                    view.setUint16(offset, keymap[l][r][c], true); offset += 2;
                }
        for (let m = 0; m < MAX_MACROS; m++) {
            for (let s = 0; s < MAX_MACRO_STEPS; s++) {
                if (s < macros[m].length) {
                    view.setUint8(offset, macros[m][s].type);
                    view.setUint16(offset + 1, macros[m][s].value, true);
                } else {
                    view.setUint8(offset, 0); view.setUint16(offset + 1, 0, true);
                }
                offset += 3;
            }
        }
        const mode = document.getElementById('modeNkro').checked ? 1 : 0;
        view.setUint8(offset, mode);

        const fullArray = new Uint8Array(buffer); const numChunks = Math.ceil(TOTAL_BYTES / 64);
        for (let i = 0; i < numChunks; i++) {
            const chunk = new Uint8Array(64); chunk.fill(0); chunk.set(fullArray.slice(i * 64, i * 64 + 64));
            await device.sendFeatureReport(4, chunk);
        }
        alert("写入成功！请拔插键盘生效。");
    } catch (err) { alert("写入失败: " + err.message); }
});

// 固件重启指令
document.getElementById('rebootBtn')?.addEventListener('click', async () => {
    if (!device) return alert("请先连接设备");
    if (!confirm("确定要让键盘进入刷写模式吗？进入后屏幕会弹出 U 盘。")) return;
    try {
        await device.sendFeatureReport(6, new Uint8Array(64));
    } catch (err) {
        alert("进入刷写模式失败，请手动按住 BOOT 键插拔 USB。\n" + err.message);
    }
});