#pragma once

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>ESP32-CAM ONVIF PRO</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="theme-color" content="#0f172a">
    <link rel="icon" href="data:,">
    <style>
        :root{
            --bg: #0f172a;
            --surface: #1e293b;
            --surface-hover: #334155;
            --primary: #3b82f6;
            --primary-hover: #2563eb;
            --accent: #8b5cf6;
            --text: #f1f5f9;
            --text-mut: #94a3b8;
            --border: #334155;
            --success: #10b981;
            --danger: #ef4444;
            --rad: 12px;
            --shadow: 0 4px 6px -1px rgba(0,0,0,0.1), 0 2px 4px -1px rgba(0,0,0,0.06);
            --font: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
        }
        *{box-sizing:border-box}
        body{
            font-family: var(--font);
            background: var(--bg);
            color: var(--text);
            margin: 0;
            min-height: 100vh;
            display: flex;
            flex-direction: column;
        }
        /* Top Navigation */
        header{
            background: rgba(15, 23, 42, 0.8);
            backdrop-filter: blur(12px);
            border-bottom: 1px solid var(--border);
            padding: 0.8rem 1.2rem;
            position: sticky;
            top: 0;
            z-index: 50;
            display: flex;
            align-items: center;
            justify-content: space-between;
        }
        .brand{
            font-size: 1.25rem;
            font-weight: 700;
            background: linear-gradient(135deg, var(--primary), var(--accent));
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }
        .status-badge{
            font-size: 0.75rem;
            padding: 0.25rem 0.75rem;
            border-radius: 99px;
            background: var(--surface);
            border: 1px solid var(--border);
            color: var(--text-mut);
            font-weight: 500;
            display: flex;
            align-items: center;
            gap: 6px;
        }
        .status-dot{width:8px;height:8px;border-radius:50%;background:var(--text-mut)}
        .status-dot.online{background:var(--success);box-shadow:0 0 8px var(--success)}

        /* Layout */
        main{
            flex: 1;
            width: 100%;
            max-width: 1200px;
            margin: 0 auto;
            padding: 1.5rem;
        }
        
        /* Tabs */
        .tabs{
            display: flex;
            gap: 0.5rem;
            overflow-x: auto;
            padding-bottom: 0.5rem;
            margin-bottom: 1.5rem;
            -webkit-overflow-scrolling: touch;
        }
        .tab-btn{
            background: transparent;
            color: var(--text-mut);
            border: none;
            padding: 0.75rem 1.25rem;
            border-radius: var(--rad);
            font-weight: 600;
            font-size: 0.95rem;
            cursor: pointer;
            transition: all 0.2s ease;
            white-space: nowrap;
        }
        .tab-btn:hover{background: var(--surface-hover); color: var(--text)}
        .tab-btn.active{
            background: var(--surface);
            color: var(--primary);
            box-shadow: var(--shadow);
            border: 1px solid var(--border);
        }

        /* Panels */
        .panel{display:none; animation: fadeUp 0.3s ease-out}
        .panel.active{display:block}

        /* Video Feed */
        .video-wrapper{
            background: #000;
            border-radius: var(--rad);
            overflow: hidden;
            aspect-ratio: 16/9;
            position: relative;
            box-shadow: var(--shadow);
            border: 1px solid var(--border);
            max-width: 800px;
            margin: 0 auto 1.5rem auto;
        }
        .video-feed{
            width: 100%;
            height: 100%;
            object-fit: contain;
            display: block;
        }
        .video-overlay{
            position: absolute;
            bottom: 0; left: 0; right: 0;
            padding: 1rem;
            background: linear-gradient(to top, rgba(0,0,0,0.8), transparent);
            display: flex;
            justify-content: center;
            gap: 0.5rem;
            opacity: 0;
            transition: opacity 0.3s;
        }
        .video-wrapper:hover .video-overlay{opacity: 1}

        /* Controls Grid */
        .grid{
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));
            gap: 1.5rem;
        }
        .card{
            background: var(--surface);
            border: 1px solid var(--border);
            border-radius: var(--rad);
            padding: 1.5rem;
            box-shadow: var(--shadow);
        }
        .card h3{
            margin: 0 0 1rem 0;
            font-size: 1.1rem;
            color: var(--text);
            display: flex;
            justify-content: space-between;
            align-items: center;
        }

        /* Forms */
        .form-group{margin-bottom: 1rem}
        .form-label{display:block; font-size: 0.85rem; color: var(--text-mut); margin-bottom: 0.4rem}
        .form-control{
            width: 100%;
            background: var(--bg);
            border: 1px solid var(--border);
            color: var(--text);
            padding: 0.6rem;
            border-radius: 6px;
            font-size: 0.95rem;
            transition: border-color 0.2s;
        }
        .form-control:focus{outline:none; border-color: var(--primary)}
        
        /* Buttons */
        .btn{
            background: var(--primary);
            color: white;
            border: none;
            padding: 0.6rem 1.2rem;
            border-radius: 6px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.2s;
            display: inline-flex;
            align-items: center;
            justify-content: center;
            gap: 6px;
        }
        .btn:hover{background: var(--primary-hover); transform: translateY(-1px)}
        .btn:disabled{opacity: 0.6; cursor: not-allowed}
        .btn.outline{background: transparent; border: 1px solid var(--border); color: var(--text)}
        .btn.outline:hover{background: var(--surface-hover)}
        .btn.danger{background: var(--danger)}
        .btn.sm{padding: 0.4rem 0.8rem; font-size: 0.85rem}
        .btn.w-full{width: 100%}

        /* Range Slider */
        input[type=range] {
            width: 100%;
            height: 6px;
            background: var(--bg);
            border-radius: 3px;
            outline: none;
            -webkit-appearance: none;
        }
        input[type=range]::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 18px;
            height: 18px;
            border-radius: 50%;
            background: var(--primary);
            cursor: pointer;
            border: 2px solid var(--surface);
        }

        /* Utility */
        .text-sm{font-size: 0.85rem}
        .text-mut{color: var(--text-mut)}
        .flex-row{display: flex; justify-content: space-between; align-items: center; margin-bottom: 0.5rem}
        .badge{padding: 2px 8px; border-radius: 4px; font-size: 0.75rem; background: var(--bg)}
        
        /* WiFi List */
        .wifi-list{display:flex; flex-direction: column; gap: 0.5rem}
        .wifi-item{
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 0.8rem;
            background: var(--bg);
            border-radius: 8px;
            border: 1px solid var(--border);
        }
        
        @keyframes fadeUp{from{opacity:0;transform:translateY(10px)}to{opacity:1;transform:translateY(0)}}
        @media(max-width: 600px){
            main{padding: 1rem}
            .grid{grid-template-columns: 1fr}
        }
    </style>
</head>
<body>

<header>
    <div class="brand">ESP32-CAM ONVIF</div>
    <div class="status-badge" id="connStatus">
        <div class="status-dot"></div> <span id="statusText">Offline</span>
    </div>
</header>

<main>
    <nav class="tabs">
        <button class="tab-btn active" onclick="setTab('dashboard')">📊 Dashboard</button>
        <button class="tab-btn" onclick="setTab('camera')">⚙️ Camera</button>
        <button class="tab-btn" onclick="setTab('wifi')">📡 WiFi</button>
        <button class="tab-btn" onclick="setTab('system')">🔧 System</button>
    </nav>

    <!-- DASHBOARD -->
    <div id="tab-dashboard" class="panel active">
        <div class="video-wrapper">
            <img id="stream" class="video-feed" src="" alt="Stream Paused">
            <div class="video-overlay">
                <button class="btn sm" onclick="toggleStream()">⏯ Play/Pause</button>
                <button class="btn sm" onclick="downloadSnapshot()">📸 Download Snap</button>
                <button class="btn sm" id="btn-flash" onclick="toggleFlash()">⚡ Flash</button>
            </div>
        </div>

        <div class="grid">
            <div class="card">
                <h3>Quick Stats</h3>
                <div class="flex-row"><span>Status</span> <span class="badge" id="d-status">-</span></div>
                <div class="flex-row"><span>Uptime</span> <span class="badge" id="d-uptime">-</span></div>
                <div class="flex-row"><span>Heap Free</span> <span class="badge" id="d-heap">-</span></div>
                <div class="flex-row"><span>Motion</span> <span class="badge" id="d-motion">-</span></div>
                <div class="flex-row"><span>Flash Auto</span> <span class="badge" id="d-autoflash">-</span></div>
            </div>
            <div class="card">
                <h3>Stream Info</h3>
                <div class="form-group">
                    <span class="form-label">RTSP URL</span>
                    <input type="text" readonly class="form-control text-sm" id="rtsp-url">
                </div>
                <div class="form-group">
                    <span class="form-label">ONVIF URL</span>
                    <input type="text" readonly class="form-control text-sm" id="onvif-url">
                </div>
            </div>
        </div>
    </div>

    <!-- CAMERA SETTINGS -->
    <div id="tab-camera" class="panel">
        <div class="grid">
            <div class="card">
                <h3>Image Adjustments</h3>
                <div class="form-group">
                    <div class="flex-row"><span>Resolution</span> <span class="text-mut text-sm">Force VGA for Reliability</span></div>
                    <select class="form-control" onchange="cfg('resolution', this.value)">
                        <option value="FRAMESIZE_UXGA">UXGA (1600x1200)</option>
                        <option value="FRAMESIZE_SXGA">SXGA (1280x1024)</option>
                        <option value="FRAMESIZE_HD">HD (1280x720)</option>
                        <option value="FRAMESIZE_SVGA">SVGA (800x600)</option>
                        <option value="FRAMESIZE_VGA" selected>VGA (640x480)</option>
                        <option value="FRAMESIZE_CIF">CIF (400x296)</option>
                    </select>
                </div>
                <div class="form-group">
                    <div class="flex-row"><span>Quality</span> <span id="val-quality" class="text-mut">12</span></div>
                    <input type="range" min="4" max="63" value="12" oninput="el('val-quality').innerText=this.value" onchange="cfg('quality', this.value)">
                </div>
                <div class="form-group">
                    <div class="flex-row"><span>Brightness</span></div>
                    <input type="range" min="-2" max="2" value="0" onchange="cfg('brightness', this.value)">
                </div>
                <div class="form-group">
                    <div class="flex-row"><span>Contrast</span></div>
                    <input type="range" min="-2" max="2" value="0" onchange="cfg('contrast', this.value)">
                </div>
            </div>
            
            <div class="card">
                <h3>Advanced Features</h3>
                <div class="flex-row">
                    <span>Auto Flash (Night Mode)</span>
                    <input type="checkbox" id="chk-autoflash" onchange="setAutoFlash(this.checked)">
                </div>
                <div class="flex-row">
                    <span>Vertical Flip</span>
                    <input type="checkbox" onchange="cfg('vflip', this.checked ? 1 : 0)">
                </div>
                 <div class="flex-row">
                    <span>Horizontal Mirror</span>
                    <input type="checkbox" onchange="cfg('hmirror', this.checked ? 1 : 0)">
                </div>
                <div class="flex-row">
                    <span>Auto Exposure (AEC)</span>
                    <input type="checkbox" checked onchange="cfg('aec', this.checked ? 1 : 0)">
                </div>
            </div>
        </div>
    </div>

    <!-- WIFI MANAGER -->
    <div id="tab-wifi" class="panel">
        <div class="card">
            <h3>WiFi Connection</h3>
            <div class="flex-row" style="margin-bottom: 1rem">
                <div>Current: <strong id="wifi-current">...</strong></div>
                <div>IP: <strong id="wifi-ip">...</strong></div>
            </div>
            
            <button class="btn w-full" onclick="scanWifi()" id="btn-scan">🔁 Scan Networks</button>
            
            <div id="wifi-results" class="wifi-list" style="margin-top: 1.5rem">
                <!-- Results -->
            </div>
        </div>
    </div>

    <!-- SYSTEM -->
    <div id="tab-system" class="panel">
        <div class="grid">
            <div class="card">
                <h3>Firmware Update (OTA)</h3>
                <p class="text-mut text-sm">Select firmware.bin to update device.</p>
                <form id="ota-form" style="margin-top:1rem">
                    <input type="file" id="ota-file" class="form-control" accept=".bin">
                    <div style="height: 10px; background: var(--bg); border-radius: 5px; margin: 1rem 0; overflow: hidden">
                        <div id="ota-progress" style="width: 0%; height: 100%; background: var(--primary); transition: width 0.2s"></div>
                    </div>
                    <button type="button" class="btn w-full" onclick="startOTA()">🚀 Start Update</button>
                </form>
            </div>
            
            <div class="card">
                <h3>Device Actions</h3>
                <div style="display: flex; gap: 1rem; flex-wrap: wrap">
                    <button class="btn danger w-full" onclick="reboot()">🔄 Reboot Device</button>
                    <button class="btn danger w-full" onclick="reboot()">🔄 Reboot Device</button>
                     <button class="btn outline w-full" onclick="window.open('/api/sd/list', '_blank')">📂 Browse SD Card</button>
                     <button class="btn outline w-full" onclick="syncTime()">🕒 Sync Time</button>
                </div>
            </div>
        </div>
    </div>

</main>

<script>
    const el = i => document.getElementById(i);
    const api = async (ep, opts={}) => {
        try {
            const res = await fetch(ep, opts);
            if(!res.ok) throw new Error(res.statusText);
            return res.json();
        } catch(e) {
            console.error("API Error", e);
            return null;
        }
    };

    // --- Tab Logic ---
    function setTab(t){
        document.querySelectorAll('.panel').forEach(p => p.classList.remove('active'));
        document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
        el('tab-'+t).classList.add('active');
        // Find button that called this
        const btns = document.querySelectorAll('.tab-btn');
        // Simple hack based on index lol
        if(t==='dashboard') btns[0].classList.add('active');
        if(t==='camera') btns[1].classList.add('active');
        if(t==='wifi') { btns[2].classList.add('active'); updateWifiStatus(); }
        if(t==='system') btns[3].classList.add('active');
    }

    // --- Stream ---
    let streamActive = true;
    function toggleStream(){
        streamActive = !streamActive;
        el('stream').src = streamActive ? '/stream' : '';
    }
    function snap(){ window.open('/snapshot', '_blank'); }
    
    // --- Config ---
    function cfg(key, val){
        fetch('/api/config', {
            method: 'POST',
            body: JSON.stringify({[key]: val})
        });
    }
    function setAutoFlash(en){
         fetch('/api/autoflash', { method: 'POST', body: JSON.stringify({enabled: en}) });
         updateStatus();
    }
    function downloadSnapshot(){
        const link = document.createElement('a');
        link.href = '/snapshot';
        link.download = `snapshot_${new Date().getTime()}.jpg`;
        document.body.appendChild(link);
        link.click();
        document.body.removeChild(link);
    }
    
    // Optimistic Flash Toggle
    let flashState = false;
    function toggleFlash(){
        flashState = !flashState;
        // Visual Feedback immediate
        const btn = el('btn-flash');
        btn.style.background = flashState ? "var(--accent)" : "var(--primary)";
        btn.innerText = flashState ? "⚡ Flash ON" : "⚡ Flash OFF";
        
        // Send request
        fetch('/api/flash', { method: 'POST', body: JSON.stringify({state: flashState}) })
            .catch(() => {
                // Revert if failed
                flashState = !flashState;
                 btn.style.background = "var(--danger)";
                 btn.innerText = "Error";
            });
    }

    // --- WiFi ---
    async function updateWifiStatus(){
        const d = await api('/api/wifi/status');
        if(d){
            el('wifi-current').innerText = d.ssid || "Not Connected";
            el('wifi-ip').innerText = d.ip;
        }
    }
    
    async function scanWifi(){
        el('btn-scan').disabled = true;
        el('btn-scan').innerText = "Scanning...";
        el('wifi-results').innerHTML = '<div class="text-mut text-center">Searching...</div>';
        
        const d = await api('/api/wifi/scan');
        el('btn-scan').disabled = false;
        el('btn-scan').innerText = "🔁 Scan Networks";
        
        if(d && d.networks){
            let html = '';
            d.networks.forEach(n => {
                html += `
                <div class="wifi-item">
                    <div>
                        <div style="font-weight:600">${n.ssid}</div>
                        <div class="text-sm text-mut">Signal: ${n.rssi} dBm</div>
                    </div>
                    <button class="btn sm outline" onclick="connectWifi('${n.ssid}')">Connect</button>
                </div>`;
            });
            el('wifi-results').innerHTML = html;
        } else {
             el('wifi-results').innerHTML = '<div class="text-mut">No networks found</div>';
        }
    }
    
    async function connectWifi(ssid){
        const pass = prompt(`Enter password for ${ssid}:`);
        if(pass === null) return;
        
        alert(`Connecting to ${ssid}... This may take 10-20 seconds. The device will reboot if successful.`);
        await api('/api/wifi/connect', { method: 'POST', body: JSON.stringify({ssid, password: pass}) });
    }

    // --- System / OTA ---
    function reboot(){
        if(confirm("Reboot device?")) api('/api/reboot', {method: 'POST'});
    }
    
    function syncTime(){
        // Send current Browser Time (Epoch)
        const epoch = Math.floor(Date.now() / 1000);
        api('/api/time', {method: 'POST', body: JSON.stringify({epoch})})
            .then(() => alert("Time Synced with Browser!"));
    }
    
    function startOTA(){
        const fileInput = el('ota-file');
        if(!fileInput.files.length) return alert("Select a file first!");
        
        const file = fileInput.files[0];
        const fd = new FormData();
        fd.append("update", file);
        
        const xhr = new XMLHttpRequest();
        xhr.upload.addEventListener("progress", (e) => {
            if(e.lengthComputable){
                const pct = Math.round((e.loaded / e.total) * 100);
                el('ota-progress').style.width = pct + '%';
            }
        });
        
        xhr.open("POST", "/api/update");
        xhr.onload = () => {
             if(xhr.status === 200){
                 alert("Update Success! Device rebooting...");
                 setTimeout(() => location.reload(), 5000);
             } else {
                 alert("Update Failed!");
                 el('ota-progress').style.background = "var(--danger)";
             }
        };
        xhr.onerror = () => alert("Update Error");
        xhr.send(fd);
    }

    // --- Global Status Loop ---
    let interval;
    async function updateStatus(){
        const d = await api('/api/status');
        if(d){
            // Online
            const dot = document.querySelector('.status-dot');
            dot.classList.add('online');
            el('statusText').innerText = "Online";
            
            // Dashboard
            el('d-status').innerText = d.status;
            el('d-uptime').innerText = formatTime(d.uptime || 0);
            el('d-heap').innerText = Math.round((d.heap || 0) / 1024) + " KB";
            el('d-motion').innerText = d.motion ? "Detected" : "None";
            if(d.motion) el('d-motion').style.color = "var(--danger)";
            else el('d-motion').style.color = "var(--success)";
            el('d-autoflash').innerText = d.autoflash ? "On" : "Off";
            
            if(el('chk-autoflash')) el('chk-autoflash').checked = d.autoflash;
            
            el('rtsp-url').value = d.rtsp;
            el('onvif-url').value = d.onvif;
        } else {
            document.querySelector('.status-dot').classList.remove('online');
            el('statusText').innerText = "Offline";
        }
    }

    function formatTime(s){
        const h = Math.floor(s/3600);
        const m = Math.floor((s%3600)/60);
        return `${h}h ${m}m`;
    }

    // Init
    window.onload = () => {
        el('stream').src = '/stream';
        updateStatus();
        interval = setInterval(updateStatus, 3000);
    };

</script>
</body>
</html>
)rawliteral";
