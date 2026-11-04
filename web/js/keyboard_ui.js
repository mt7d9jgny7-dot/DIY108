let keymap = JSON.parse(JSON.stringify(defaultKeymapData));
let currentLayer = 0;
let selectedKey = null;
const keyUnit = 50, gap = 4;

function renderKeyboard() {
    const board = document.getElementById('keyboardBoard');
    if (!board) return;
    board.innerHTML = '';
    let maxX = 0, maxY = 0;
    customLayout.forEach(k => { maxX = Math.max(maxX, k.x + k.w); maxY = Math.max(maxY, k.y + k.h); });
    board.style.width = (maxX * keyUnit) + 'px'; board.style.height = (maxY * keyUnit) + 'px';

    customLayout.forEach(k => {
        const el = document.createElement('div');
        el.className = 'key-node';
        el.style.left = (k.x * keyUnit) + 'px'; el.style.top = (k.y * keyUnit) + 'px';
        el.style.width = (k.w * keyUnit - gap) + 'px'; el.style.height = (k.h * keyUnit - gap) + 'px';
        
        const hidCode = keymap[currentLayer][k.row][k.col];
        let displayName = codeToName[hidCode] || `0x${hidCode.toString(16).toUpperCase()}`;
        if (hidCode === 0x0000) displayName = "";

        el.innerHTML = `<div class="key-label-top"><span>${k.label}</span><span>${k.row},${k.col}</span></div><div class="key-label-main">${displayName}</div>`;
        if (selectedKey && selectedKey.row === k.row && selectedKey.col === k.col) el.classList.add('selected');

        el.onclick = () => {
            document.querySelectorAll('.key-node').forEach(n => n.classList.remove('selected'));
            el.classList.add('selected'); selectedKey = k;
            openKeyPicker((code) => {
                keymap[currentLayer][k.row][k.col] = code;
                renderKeyboard();
            });
        };
        board.appendChild(el);
    });
}

document.querySelectorAll('.layer-btn').forEach(btn => {
    btn.onclick = () => {
        document.querySelectorAll('.layer-btn').forEach(b => b.classList.remove('active'));
        btn.classList.add('active'); currentLayer = parseInt(btn.dataset.layer); renderKeyboard();
    };
});