function switchTab(tabId) {
    document.querySelectorAll('.view-container').forEach(el => el.classList.remove('active'));
    document.querySelectorAll('.tab-item').forEach(el => el.classList.remove('active'));
    document.querySelectorAll('.menu-item').forEach(el => el.classList.remove('active'));

    document.getElementById(tabId).classList.add('active');
    document.getElementById(tabId + '-title').classList.add('active');
    document.querySelector(`.menu-item[onclick="switchTab('${tabId}')"]`).classList.add('active');

    if(tabId === 'macro-tab') {
        document.getElementById('layerSwitch').style.visibility = 'hidden';
    } else {
        document.getElementById('layerSwitch').style.visibility = 'visible';
    }
}

window.onload = () => {
    renderKeyboard();
    renderMacroSidebar();
    renderMacroEditor();
};