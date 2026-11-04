let device = null;
const connectBtn = document.getElementById('connectBtn');
const statusText = document.getElementById('statusText');
const readBtn = document.getElementById('readBtn');
const writeBtn = document.getElementById('writeBtn');

async function connectDevice() {
    if (device) {
        await device.close();
        device = null;
        statusText.innerText = "未连接"; statusText.className = "status-badge";
        connectBtn.innerText = "连接设备"; readBtn.disabled = true; writeBtn.disabled = true;
        return;
    }
    try {
        const devices = await navigator.hid.requestDevice({ filters: [{ vendorId: 0xCafe, productId: 0x4000 }] });
        if (devices.length === 0) return;
        device = devices[0];
        if (!device.opened) await device.open();
        
        statusText.innerText = `已连接: ${device.productName}`; statusText.className = "status-badge connected";
        connectBtn.innerText = "断开连接"; readBtn.disabled = false; writeBtn.disabled = false;

        navigator.hid.addEventListener('disconnect', () => {
            device = null; statusText.innerText = "未连接"; statusText.className = "status-badge";
            connectBtn.innerText = "连接设备"; readBtn.disabled = true; writeBtn.disabled = true;
        });
    } catch (err) { alert("连接失败: " + err.message); }
}
if (connectBtn) connectBtn.addEventListener('click', connectDevice);