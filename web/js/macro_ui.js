let macros = Array.from({ length: MAX_MACROS }, () => []);
let currentMacroId = 0;

function renderMacroSidebar() {
    const list = document.getElementById('macroList');
    if (!list) return;
    list.innerHTML = '';
    for(let i=0; i<MAX_MACROS; i++) {
        const li = document.createElement('li');
        li.innerText = `宏 ${i + 1}`;
        if(i === currentMacroId) li.classList.add('active');
        li.onclick = () => { currentMacroId = i; renderMacroSidebar(); renderMacroEditor(); };
        list.appendChild(li);
    }
}

function renderMacroEditor() {
    document.getElementById('macroTitle').innerText = `编辑 宏 ${currentMacroId + 1}`;
    const list = document.getElementById('actionList');
    if (!list) return;
    list.innerHTML = '';
    const actions = macros[currentMacroId];
    if(actions.length === 0) { list.innerHTML = '<div style="color:#9ca3af; text-align:center; margin-top:20px;">暂无动作</div>'; return; }

    actions.forEach((act, index) => {
        const div = document.createElement('div');
        div.className = 'action-item';
        let typeStr = act.type === 1 ? "按下" : (act.type === 2 ? "抬起" : "延迟");
        div.innerHTML = `<span style="width: 50px; color:#666;">${index + 1}.</span><span style="width: 60px; font-weight:bold; color:var(--primary);">${typeStr}</span>`;

        if(act.type === 3) {
            const input = document.createElement('input');
            input.type = 'number'; input.className = 'action-input'; input.value = act.value;
            input.onchange = (e) => { macros[currentMacroId][index].value = parseInt(e.target.value) || 0; };
            div.appendChild(input); div.appendChild(document.createTextNode(' ms'));
        } else {
            const btn = document.createElement('button');
            btn.className = 'btn-action'; btn.style.padding = '4px 10px';
            btn.innerText = codeToName[act.value] || `0x${act.value.toString(16)}`;
            btn.onclick = () => { openKeyPicker((code) => { macros[currentMacroId][index].value = code; renderMacroEditor(); }); };
            div.appendChild(btn);
        }

        const delBtn = document.createElement('button');
        delBtn.className = 'btn-del'; delBtn.innerText = '✕';
        delBtn.onclick = () => { macros[currentMacroId].splice(index, 1); renderMacroEditor(); };
        div.appendChild(delBtn); list.appendChild(div);
    });
}

function addMacroAction(typeStr) {
    if(macros[currentMacroId].length >= MAX_MACRO_STEPS) return alert("已达到最大步骤限制");
    if(typeStr === 'DELAY') {
        macros[currentMacroId].push({ type: 3, value: 100 });
        renderMacroEditor();
    } else {
        openKeyPicker((code) => {
            macros[currentMacroId].push({ type: typeStr === 'DOWN' ? 1 : 2, value: code });
            renderMacroEditor();
        });
    }
}

// 宏录制逻辑
let isRecording = false;
let lastRecordTime = 0;
let activeRecordKeys = new Set(); // 🔥 记录当前按住未松开的键

function toggleMacroRecording() {
    isRecording = !isRecording;
    const btn = document.getElementById('recordBtn');
    if (!btn) return;
    
    if (isRecording) {
        macros[currentMacroId] = []; 
        activeRecordKeys.clear();
        lastRecordTime = performance.now();
        btn.innerText = "⏹️ 停止录制";
        btn.style.background = "#ef4444";
        window.addEventListener('keydown', onRecordKeyDown);
        window.addEventListener('keyup', onRecordKeyUp);
        renderMacroEditor();
    } else {
        btn.innerText = "🎙️ 开始录制";
        btn.style.background = "";
        window.removeEventListener('keydown', onRecordKeyDown);
        window.removeEventListener('keyup', onRecordKeyUp);
        
        // 🔥 关键修复：如果停止录制时还有键被按住，自动补全抬起动作，防止宏死锁
        if (activeRecordKeys.size > 0) {
            activeRecordKeys.forEach(code => {
                macros[currentMacroId].push({ type: 2, value: code });
            });
            activeRecordKeys.clear();
        }
        renderMacroEditor();
    }
}

function onRecordKeyDown(e) {
    if (!isRecording) return;
    e.preventDefault(); 
    const now = performance.now();
    const delay = Math.round(now - lastRecordTime);
    if (delay > 20) { macros[currentMacroId].push({ type: 3, value: delay }); }
    lastRecordTime = now;

    const hidCode = webCodeToHid[e.code];
    if (hidCode !== undefined) {
        if (e.repeat) return;
        activeRecordKeys.add(hidCode);
        macros[currentMacroId].push({ type: 1, value: hidCode });
        renderMacroEditor();
    }
}

function onRecordKeyUp(e) {
    if (!isRecording) return;
    e.preventDefault();
    const now = performance.now();
    const delay = Math.round(now - lastRecordTime);
    if (delay > 20) { macros[currentMacroId].push({ type: 3, value: delay }); }
    lastRecordTime = now;

    const hidCode = webCodeToHid[e.code];
    if (hidCode !== undefined) {
        activeRecordKeys.delete(hidCode);
        macros[currentMacroId].push({ type: 2, value: hidCode });
        renderMacroEditor();
    }
}