// 菜单切换逻辑 (左侧边栏)
function switchMenu(viewId) {
    document.querySelectorAll('.view-container').forEach(el => el.classList.remove('active'));
    document.querySelectorAll('.menu-item').forEach(el => el.classList.remove('active'));

    document.getElementById(viewId).classList.add('active');
    document.querySelector(`.menu-item[onclick="switchMenu('${viewId}')"]`).classList.add('active');
}

// 顶层 Tab 切换逻辑 (自定义按键 / 宏设置)
function switchTab(tabId) {
    document.querySelectorAll('.sub-view').forEach(el => el.classList.remove('active'));
    document.querySelectorAll('.tab-item').forEach(el => el.classList.remove('active'));

    document.getElementById(tabId).classList.add('active');
    document.getElementById(tabId + '-title').classList.add('active');

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