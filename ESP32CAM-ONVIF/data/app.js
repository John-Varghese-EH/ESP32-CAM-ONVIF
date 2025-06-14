// --- Login Overlay Logic ---
let authHeader = "";
document.addEventListener("DOMContentLoaded", function() {
  if (localStorage.getItem("auth")) {
    authHeader = localStorage.getItem("auth");
    document.getElementById("login-overlay").style.display = "none";
    updateStatus();
  }
});
function login(e) {
  e.preventDefault();
  const user = document.getElementById("username").value;
  const pass = document.getElementById("password").value;
  authHeader = "Basic " + btoa(user + ":" + pass);
  fetch("/api/status", {headers: {"Authorization": authHeader}})
    .then(r => {
      if (r.status === 401) throw new Error("Unauthorized");
      return r.json();
    })
    .then(() => {
      document.getElementById("login-overlay").style.display = "none";
      localStorage.setItem("auth", authHeader);
      updateStatus();
    })
    .catch(() => {
      document.getElementById("login-error").textContent = "Invalid credentials!";
      authHeader = "";
      localStorage.removeItem("auth");
    });
  return false;
}

// --- API Helpers ---
function api(url, opts={}) {
  opts.headers = opts.headers || {};
  opts.headers["Authorization"] = authHeader;
  return fetch(url, opts);
}

// --- Tab Navigation ---
function showPanel(panel) {
  document.querySelectorAll('.tab-btn').forEach(btn => btn.classList.remove('active'));
  document.querySelectorAll('.panel').forEach(sec => sec.style.display = 'none');
  document.querySelector('.tab-btn[onclick="showPanel(\''+panel+'\')"]').classList.add('active');
  document.getElementById('panel-' + panel).style.display = '';
  if (panel === 'sd') refreshSD();
}

// --- SD Card Management ---
function refreshSD() {
  api('/api/sd/list').then(r => r.json()).then(files => {
    const tbody = document.querySelector('#sd-table tbody');
    tbody.innerHTML = '';
    files.forEach(file => {
      const tr = document.createElement('tr');
      tr.innerHTML = `<td>${file.name}</td>
        <td>${(file.size/1024).toFixed(1)} KB</td>
        <td>
          <a href="/api/sd/download?file=${encodeURIComponent(file.name)}" class="btn" target="_blank">Download</a>
          <button class="btn" onclick="deleteSD('${file.name}')">Delete</button>
        </td>`;
      tbody.appendChild(tr);
    });
  });
}
function deleteSD(filename) {
  if (!confirm('Delete ' + filename + '?')) return;
  api('/api/sd/delete', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({file: filename})
  }).then(refreshSD);
}

// --- UI Logic ---
// --- WiFi Management ---
function scanWiFi() {
  document.getElementById('scan-status').textContent = "Scanning...";
  document.getElementById('networks-list').style.display = 'none';
  document.getElementById('connect-form').style.display = 'none';
  
  api('/api/wifi/scan').then(r => r.json()).then(data => {
    const tbody = document.querySelector('#wifi-table tbody');
    tbody.innerHTML = '';
    
    if (data.networks && data.networks.length > 0) {
      data.networks.forEach(network => {
        const tr = document.createElement('tr');
        // Signal strength indicator
        const signalStrength = Math.min(100, Math.max(0, 2 * (network.rssi + 100)));
        const signalIcon = signalStrength > 70 ? '📶' : signalStrength > 40 ? '📶' : '📶';
        
        // Security type
        const security = network.encType > 0 ? '🔒' : '';
        
        tr.innerHTML = `
          <td>${network.ssid}</td>
          <td>${signalIcon} ${signalStrength}%</td>
          <td>${security}</td>
          <td><button class="btn" onclick="selectNetwork('${network.ssid}')">Connect</button></td>
        `;
        tbody.appendChild(tr);
      });
      
      document.getElementById('networks-list').style.display = '';
      document.getElementById('scan-status').textContent = `Found ${data.networks.length} networks`;
    } else {
      document.getElementById('scan-status').textContent = "No networks found";
    }
  }).catch(err => {
    document.getElementById('scan-status').textContent = "Scan failed: " + err.message;
  });
}

function selectNetwork(ssid) {
  document.getElementById('wifi-connect-ssid').value = ssid;
  document.getElementById('wifi-connect-password').value = '';
  document.getElementById('connect-form').style.display = '';
}

function connectWiFi(e) {
  e.preventDefault();
  
  const ssid = document.getElementById('wifi-connect-ssid').value;
  const password = document.getElementById('wifi-connect-password').value;
  
  api('/api/wifi/connect', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({ssid, password})
  }).then(r => r.json()).then(data => {
    if (data.success) {
      alert(`Connecting to ${ssid}. The device will restart if connection is successful.`);
    } else {
      alert(`Failed to connect: ${data.message}`);
    }
  }).catch(err => {
    alert("Connection error: " + err.message);
  });
  
  return false;
}

function updateWiFiStatus() {
  api('/api/wifi/status').then(r => r.json()).then(data => {
    document.getElementById('wifi-status').textContent = data.connected ? "Connected" : "Disconnected";
    document.getElementById('wifi-ssid').textContent = data.ssid || "-";
    document.getElementById('wifi-ip').textContent = data.ip || "-";
  }).catch(err => {
    console.error("Error getting WiFi status:", err);
  });
}

function updateStatus() {
  api('/api/status').then(r => r.json()).then(data => {
    document.getElementById('status').textContent = data.status || 'Online';
    document.getElementById('rtspUrl').textContent = data.rtsp || '';
    document.getElementById('onvifUrl').textContent = data.onvif || '';
    document.getElementById('motionStatus').textContent = "Motion: " + (data.motion ? "Detected" : "None");
    
    // Update WiFi status if on WiFi tab
    if (document.getElementById('panel-wifi').style.display !== 'none') {
      updateWiFiStatus();
    }
  }).catch(() => {
    document.getElementById('status').textContent = 'Offline';
    if (!document.getElementById("login-overlay").style.display || document.getElementById("login-overlay").style.display === "none") {
      document.getElementById("login-overlay").style.display = "flex";
    }
    localStorage.removeItem("auth");
  });
}
function setConfig(key, value) {
  api('/api/config', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({[key]: value})
  }).then(updateStatus);
}
function snapshot() {
  window.open('/snapshot', '_blank');
}
function toggleRecording() {
  api('/api/record', {method: 'POST'}).then(updateStatus);
}
function reboot() {
  api('/api/reboot', {method: 'POST'});
  alert("Rebooting...");
}
function factoryReset() {
  api('/api/factory_reset', {method: 'POST'});
  alert("Factory reset initiated.");
}
// Update status every 2 seconds, and WiFi status if on that tab
setInterval(() => {
  updateStatus();
}, 2000);

// Initial updates
updateStatus();

// Add panel-specific handlers
function showPanel(panel) {
  document.querySelectorAll('.tab-btn').forEach(btn => btn.classList.remove('active'));
  document.querySelectorAll('.panel').forEach(sec => sec.style.display = 'none');
  document.querySelector('.tab-btn[onclick="showPanel(\''+panel+'\')"]').classList.add('active');
  document.getElementById('panel-' + panel).style.display = '';
  
  // Panel-specific init
  if (panel === 'sd') refreshSD();
  if (panel === 'wifi') updateWiFiStatus();
}

// --- zoom-pan-live-view ---
const preview = document.getElementById('preview');
let scale = 1, panX = 0, panY = 0, lastScale = 1, lastPanX = 0, lastPanY = 0, startDist = 0, isDragging = false, dragStartX = 0, dragStartY = 0;
function update() {
  preview.style.transform = `scale(${scale}) translate(${panX/scale}px,${panY/scale}px)`;
}

// Pinch-to-zoom and pan (touch)
preview.addEventListener('touchstart', e => {
  if (e.touches.length === 2) {
    e.preventDefault();
    startDist = Math.hypot(
      e.touches[0].clientX - e.touches[1].clientX,
      e.touches[0].clientY - e.touches[1].clientY
    );
    lastScale = scale;
  } else if (e.touches.length === 1 && scale > 1) {
    isDragging = true;
    dragStartX = e.touches[0].clientX - lastPanX;
    dragStartY = e.touches[0].clientY - lastPanY;
  }
}, {passive:false});

preview.addEventListener('touchmove', e => {
  if (e.touches.length === 2) {
    e.preventDefault();
    let newDist = Math.hypot(
      e.touches[0].clientX - e.touches[1].clientX,
      e.touches[0].clientY - e.touches[1].clientY
    );
    scale = Math.max(1, Math.min(4, lastScale * newDist / startDist));
    update();
  } else if (e.touches.length === 1 && isDragging && scale > 1) {
    e.preventDefault();
    panX = e.touches[0].clientX - dragStartX;
    panY = e.touches[0].clientY - dragStartY;
    update();
  }
}, {passive:false});

preview.addEventListener('touchend', e => {
  if (e.touches.length < 2) {
    lastScale = scale; lastPanX = panX; lastPanY = panY; isDragging = false;
  }
}, {passive:false});

// Scroll-to-zoom (desktop)
preview.addEventListener('wheel', e => {
  e.preventDefault();
  let delta = e.deltaY < 0 ? 0.1 : -0.1;
  let newScale = Math.max(1, Math.min(4, scale + delta));
  // Optional: zoom towards mouse pointer
  if (newScale !== scale) {
    let rect = preview.getBoundingClientRect();
    let mx = e.clientX - rect.left - rect.width / 2, my = e.clientY - rect.top - rect.height / 2;
    panX = (panX - mx) * (newScale / scale) + mx;
    panY = (panY - my) * (newScale / scale) + my;
    scale = newScale; lastScale = scale; update();
  }
}, {passive:false});

// Pan with mouse drag (desktop)
preview.addEventListener('mousedown', e => {
  if (scale > 1) { isDragging = true; dragStartX = e.clientX - lastPanX; dragStartY = e.clientY - lastPanY; preview.style.cursor = "grabbing"; }
});
window.addEventListener('mousemove', e => {
  if (isDragging && scale > 1) { panX = e.clientX - dragStartX; panY = e.clientY - dragStartY; update(); }
});
window.addEventListener('mouseup', e => {
  if (isDragging) { isDragging = false; lastPanX = panX; lastPanY = panY; preview.style.cursor = "grab"; }
});
