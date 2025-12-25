#pragma once

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>ESP32-CAM PRO</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="theme-color" content="#0d0d12">
    <link rel="icon" href="data:,">
    <style>
        :root{--bg:#0d0d12;--panel:rgba(25,25,30,0.8);--prim:#00e5ff;--sec:#7000ff;--txt:#e0e0e0;--mut:#a0a0a0;--brd:rgba(255,255,255,0.1);--blur:blur(12px);--shd:0 8px 32px rgba(0,0,0,0.4);--font:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif;--rad:12px}
        body{font-family:var(--font);margin:0;background:var(--bg);color:var(--txt);min-height:100vh;display:flex;flex-direction:column;background-image:radial-gradient(at 0% 0%,rgba(112,0,255,0.15),transparent 50%),radial-gradient(at 100% 100%,rgba(0,229,255,0.15),transparent 50%);background-attachment:fixed}
        header{background:rgba(10,10,15,0.9);backdrop-filter:var(--blur);padding:1rem;border-bottom:1px solid var(--brd);display:flex;justify-content:space-between;align-items:center;position:sticky;top:0;z-index:100}
        .brand{font-size:1.4rem;font-weight:700;background:linear-gradient(135deg,var(--prim),var(--sec));-webkit-background-clip:text;-webkit-text-fill-color:transparent}
        .status{font-size:0.85rem;padding:0.3rem 0.8rem;border-radius:20px;background:rgba(255,255,255,0.05);border:1px solid var(--brd);color:var(--prim)}
        main{flex:1;width:100%;max-width:1000px;margin:0 auto;padding:1.5rem 1rem}
        .tabs{display:flex;justify-content:center;gap:0.5rem;margin-bottom:1.5rem;flex-wrap:wrap}
        .tab-btn{background:rgba(255,255,255,0.03);border:1px solid var(--brd);color:var(--mut);padding:0.6rem 1rem;border-radius:var(--rad);cursor:pointer;display:flex;align-items:center;gap:0.4rem;transition:0.3s}
        .tab-btn.active{background:linear-gradient(135deg,rgba(112,0,255,0.2),rgba(0,229,255,0.2));border-color:var(--prim);color:var(--prim)}
        .panel{background:var(--panel);backdrop-filter:var(--blur);border-radius:var(--rad);border:1px solid var(--brd);box-shadow:var(--shd);padding:1.5rem;display:none;animation:fadeIn 0.3s}
        @keyframes fadeIn{from{opacity:0;transform:translateY(10px)}to{opacity:1;transform:translateY(0)}}
        h2{color:var(--prim);margin:0 0 1rem 0;font-size:1.3rem;border-bottom:1px solid var(--brd);padding-bottom:0.5rem}
        .stream-container{width:100%;aspect-ratio:4/3;background:#000;border-radius:var(--rad);overflow:hidden;position:relative;border:1px solid var(--brd)}
        .video-preview{width:100%;height:100%;object-fit:contain}
        .btn{background:linear-gradient(90deg,var(--sec),var(--prim));color:#fff;border:none;padding:0.6rem 1.2rem;border-radius:8px;font-weight:600;cursor:pointer;margin:0.2rem;text-transform:uppercase;font-size:0.85rem}
        .btn:hover{transform:translateY(-2px);box-shadow:0 4px 12px rgba(0,229,255,0.3)}
        label{display:flex;justify-content:space-between;align-items:center;margin-bottom:0.8rem;color:var(--mut)}
        input,select{background:rgba(0,0,0,0.3);border:1px solid var(--brd);color:var(--txt);padding:0.5rem;border-radius:6px;outline:none}
        table{width:100%;border-collapse:collapse;margin-top:1rem}
        th,td{padding:0.8rem;text-align:left;border-bottom:1px solid var(--brd)}
        th{color:var(--prim)}
        .overlay{position:fixed;inset:0;background:rgba(0,0,0,0.85);backdrop-filter:blur(8px);z-index:1000;display:flex;justify-content:center;align-items:center}
        .login-box{background:var(--panel);padding:2.5rem;border-radius:20px;border:1px solid var(--brd);text-align:center;width:90%;max-width:320px}
        .footer{text-align:center;padding:1.5rem;color:var(--mut);font-size:0.8rem}
        /* Mobile */
        @media(max-width:600px){.panel{padding:1rem}.tab-btn{font-size:0.85rem;padding:0.5rem 0.8rem}}
    </style>
</head>
<body>
    <header>
        <div class="brand">ESP32-CAM PRO</div>
        <div class="status" id="status">Connecting...</div>
    </header>

    <main>
        <nav class="tabs">
            <button class="tab-btn active" onclick="showPanel('live')">📸 Live</button>
            <button class="tab-btn" onclick="showPanel('camera')">⚙️ Settings</button>
            <button class="tab-btn" onclick="showPanel('wifi')">📶 WiFi</button>
            <button class="tab-btn" onclick="showPanel('sd')">💾 SD</button>
            <button class="tab-btn" onclick="showPanel('system')">ℹ️ System</button>
        </nav>

        <section id="panel-live" class="panel" style="display:block">
            <h2>Live Stream</h2>
            <div class="stream-container">
                <img id="preview" class="video-preview" src="/stream" alt="Stream">
            </div>
            <div style="text-align:center;margin-top:1rem">
                <button class="btn" onclick="toggleFlash()">⚡ Flash</button>
                <button class="btn" onclick="window.open('/snapshot','_blank')">Snapshot</button>
                <button class="btn" onclick="refreshImage()">Refresh</button>
            </div>
        </section>

        <section id="panel-camera" class="panel">
            <h2>Image Settings</h2>
            <label>Resolution 
                <select id="resolution" onchange="cfg('resolution',this.value)">
                    <option value="UXGA">UXGA</option><option value="SXGA">SXGA</option><option value="SVGA">SVGA</option>
                    <option value="VGA" selected>VGA</option><option value="CIF">CIF</option><option value="QVGA">QVGA</option>
                </select>
            </label>
            <label>Quality <input type="range" min="10" max="63" value="12" onchange="cfg('quality',this.value)"></label>
            <label>Brightness <input type="range" min="-2" max="2" value="0" onchange="cfg('brightness',this.value)"></label>
            <label>Contrast <input type="range" min="-2" max="2" value="0" onchange="cfg('contrast',this.value)"></label>
            <div style="display:grid;grid-template-columns:1fr 1fr;gap:0.5rem">
                <label><input type="checkbox" id="autoflash" onchange="setAutoFlash(this.checked)"> Auto Flash</label>
                <label><input type="checkbox" onchange="cfg('awb',this.checked?1:0)"> AWB</label>
                <label><input type="checkbox" onchange="cfg('vflip',this.checked?1:0)"> V-Flip</label>
                <label><input type="checkbox" onchange="cfg('hmirror',this.checked?1:0)"> H-Mirror</label>
                <label><input type="checkbox" onchange="cfg('aec',this.checked?1:0)"> AEC</label>
            </div>
        </section>

        <section id="panel-wifi" class="panel">
            <h2>WiFi</h2>
            <div style="margin-bottom:1rem">
                <div>SSID: <b id="wifi-ssid">-</b></div>
                <div>IP: <b id="wifi-ip">-</b></div>
            </div>
            <button class="btn" onclick="scanWiFi()">Scan</button>
            <div id="networks" style="margin-top:1rem"></div>
        </section>

        <section id="panel-sd" class="panel">
            <h2>SD Card</h2>
            <button class="btn" onclick="loadSD()">Refresh</button>
            <table id="sd-table"><thead><tr><th>File</th><th>Size</th><th>Action</th></tr></thead><tbody></tbody></table>
        </section>

        <section id="panel-system" class="panel">
            <h2>System Info</h2>
            <div>RTSP: <span id="rtspUrl" style="color:var(--prim)">-</span></div>
            <div>ONVIF: <span id="onvifUrl" style="color:var(--prim)">-</span></div>
            <div style="margin-top:1rem">
                <button class="btn" onclick="api('/api/reboot',{method:'POST'});alert('Rebooting')">Reboot</button>
            </div>
        </section>
    </main>

    <div class="footer">ESP32-CAM PRO &copy; 2025 J0X</div>

    <script>
        let auth="";
        const el=i=>document.getElementById(i);
        const api=(u,o={})=>{return fetch(u,o)}; // Browser handles auth automatically now

        function showPanel(n){
            document.querySelectorAll('.panel').forEach(p=>p.style.display='none');
            document.querySelectorAll('.tab-btn').forEach(b=>b.classList.remove('active'));
            el('panel-'+n).style.display='block';
            document.querySelector(`button[onclick="showPanel('${n}')"]`).classList.add('active');
            if(n==='sd') loadSD();
            if(n==='wifi') updWiFi();
        }

        function toggleFlash(){
            let s = confirm("Toggle Flash?");
            if(s) api('/api/flash',{method:'POST',body:JSON.stringify({state:true})}); // Basic toggle, better to track state but this works for now
            else api('/api/flash',{method:'POST',body:JSON.stringify({state:false})});
        }
        
        function setAutoFlash(en){
             api('/api/autoflash',{method:'POST',body:JSON.stringify({enabled:en})});
        }

        function upd(){
            api('/api/status').then(r=>r.json()).then(d=>{
                el('status').innerText=d.status||'Online';
                el('rtspUrl').innerText=d.rtsp;
                el('onvifUrl').innerText=d.onvif;
                if(el('autoflash')) el('autoflash').checked = d.autoflash;
            }).catch(()=>el('status').innerText='Offline');
        }

        function cfg(k,v){ api('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({[k]:v})}); }
        
        function loadSD(){
            api('/api/sd/list').then(r=>r.json()).then(files=>{
                let h='';
                files.forEach(f=>h+=`<tr><td>${f.name}</td><td>${(f.size/1024).toFixed(1)}KB</td><td><a href="/api/sd/download?file=${f.name}" target="_blank">Get</a></td></tr>`);
                el('sd-table').querySelector('tbody').innerHTML=h;
            });
        }

        function scanWiFi(){
            el('networks').innerText='Scanning...';
            api('/api/wifi/scan').then(r=>r.json()).then(d=>{
                let h='<table>';
                d.networks.forEach(n=>h+=`<tr><td>${n.ssid}</td><td>${n.rssi}dBm</td><td><button onclick="con('${n.ssid}')">Connect</button></td></tr>`);
                el('networks').innerHTML=h+'</table>';
            });
        }
        
        function con(ssid){
            let p=prompt('Password for '+ssid);
            if(p) api('/api/wifi/connect',{method:'POST',body:JSON.stringify({ssid:ssid,password:p})});
        }

        function updWiFi(){
            api('/api/wifi/status').then(r=>r.json()).then(d=>{
                el('wifi-ssid').innerText=d.ssid;
                el('wifi-ip').innerText=d.ip;
            });
        }

        function refreshImage(){
            el('preview').src='/stream?t='+new Date().getTime();
        }

        setInterval(upd,5000);
    </script>
</body>
</html>
)rawliteral";
