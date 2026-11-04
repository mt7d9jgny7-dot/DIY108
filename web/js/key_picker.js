const modal = document.getElementById('keyModal');
let pickerCallback = null;

function openKeyPicker(cb) {
    pickerCallback = cb;
    if(!modal) return;
    modal.style.display = 'block';
    document.getElementById('keySearch').value = '';
    renderModalList();
    document.getElementById('keySearch').focus();
}

function renderModalList() {
    const keyGrid = document.getElementById('keyGrid');
    if(!keyGrid) return;
    keyGrid.innerHTML = '';
    const keyword = document.getElementById('keySearch').value.toLowerCase();
    Object.entries(keycodeMap).forEach(([codeStr, name]) => {
        if (name.toLowerCase().includes(keyword) || codeStr.includes(keyword)) {
            const btn = document.createElement('div');
            btn.className = 'key-btn'; btn.innerText = name;
            btn.onclick = () => { if(pickerCallback) pickerCallback(parseInt(codeStr)); modal.style.display = 'none'; };
            keyGrid.appendChild(btn);
        }
    });
}
if(document.getElementById('keySearch')) {
    document.getElementById('keySearch').addEventListener('input', renderModalList);
    window.onclick = (event) => { if (event.target == modal) modal.style.display = 'none'; };
}